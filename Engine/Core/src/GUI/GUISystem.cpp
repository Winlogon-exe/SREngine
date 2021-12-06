//
// Created by Nikita on 19.07.2021.
//

#define IMGUI_DEFINE_MATH_OPERATORS

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>

#include <GUI/GUISystem.h>
#include <Input/InputSystem.h>

#include <Events/EventManager.h>
#include <ResourceManager/ResourceManager.h>
#include <Engine.h>

using namespace Framework::Core::GUI;

void GUISystem::BeginDockSpace() {
    const float toolbarSize = 0;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos + ImVec2(0, toolbarSize));
    ImGui::SetNextWindowSize(viewport->Size - ImVec2(0, toolbarSize));
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGuiWindowFlags window_flags = 0
                                    | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking
                                    | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse
                                    | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
                                    | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("Master DockSpace", nullptr, window_flags);
    ImGuiID dockMain = ImGui::GetID("MyDockspace");

    // Save off menu bar height for later.
    //float menuBarHeight = ImGui::GetCurrentWindow()->MenuBarHeight();

    ImGui::DockSpace(dockMain);
    ImGui::End();
    ImGui::PopStyleVar(3);
}

void GUISystem::EndDockSpace() {

}

bool GUISystem::BeginWindow(const char* name) {
    return ImGui::Begin(name);
}

void GUISystem::EndWindow() {
    ImGui::End();
}

bool GUISystem::BeginChildWindow(const char* name) {
    return ImGui::BeginChild(name);
}

void GUISystem::EndChildWindow() {
    ImGui::EndChild();
}

void GUISystem::DrawTexture(
    Framework::Helper::Math::IVector2 winSize,
    Framework::Helper::Math::IVector2 texSize, uint32_t id,
    bool centralize)
{
    const float_t dx = static_cast<float_t>(winSize.x) / texSize.x;
    const float_t dy = static_cast<float_t>(winSize.y) / texSize.y;

    if (dy > dx)
        texSize *= dx;
    else
        texSize *= dy;

    // Because I use the texture from OpenGL, I need to invert the V from the UV.

    if (centralize) {
        ImVec2 initialCursorPos = ImGui::GetCursorPos();
        auto res = (winSize - texSize) * 0.5f;
        ImVec2 centralizedCursorPos = { (float)res.x, (float)res.y };
        centralizedCursorPos = ImClamp(centralizedCursorPos, initialCursorPos, centralizedCursorPos);
        ImGui::SetCursorPos(centralizedCursorPos);
    }

    if (m_pipeLine == Graphics::PipeLine::OpenGL)
        DrawImage(reinterpret_cast<ImTextureID>(id), ImVec2(texSize.x, texSize.y), ImVec2(0, 1), ImVec2(1, 0), {1, 1, 1, 1 }, {0, 0, 0, 0 }, true);
    else {
        DrawImage(m_env->GetDescriptorSetFromTexture(id, true), ImVec2(texSize.x, texSize.y), ImVec2(-1, 0), ImVec2(0, 1), {1, 1, 1, 1}, {0, 0, 0, 0}, true);
    }
}

void GUISystem::DrawImage(
    ImTextureID user_texture_id,
    const ImVec2& size,
    const ImVec2& uv0,
    const ImVec2& uv1,
    const ImVec4& tint_col,
    const ImVec4& border_col,
    bool imposition)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
    if (border_col.w > 0.0f)
        bb.Max = bb.Max + ImVec2(2, 2);

    if (!imposition) {
        ImGui::ItemSize(bb);
        if (!ImGui::ItemAdd(bb, 0))
            return;
    }

    if (border_col.w > 0.0f) {
        window->DrawList->AddRect(bb.Min, bb.Max, ImGui::GetColorU32(border_col), 0.0f);
        window->DrawList->AddImage(user_texture_id, bb.Min + ImVec2(1, 1), bb.Max - ImVec2(1, 1), uv0, uv1, ImGui::GetColorU32(tint_col));
    }
    else
        window->DrawList->AddImage(user_texture_id, bb.Min, bb.Max, uv0, uv1, ImGui::GetColorU32(tint_col));
}

void GUISystem::DrawHierarchy(Framework::Helper::Types::SafePtr<Framework::Helper::World::Scene> scene) {
    m_shiftPressed = Helper::Input::GetKey(Helper::KeyCode::LShift);

    if (scene.LockIfValid()) {
        unsigned long i = 0;

        if (ImGui::TreeNodeEx(scene->GetName().c_str(), ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetFontSize() * 3);

            scene->ForEachRootObjects([&i, this](Helper::Types::SafePtr<Helper::GameObject> gm) {
                if (gm->HasChildren()) {
                    bool open = ImGui::TreeNodeEx((void *) (intptr_t) i,
                                                  g_node_flags_with_child | (gm->IsSelect() ? ImGuiTreeNodeFlags_Selected : 0),
                                                  "%s", gm->GetName().c_str());
                    CheckSelected(gm);

                    if (open)
                        this->DrawChild(gm);
                } else {
                    ImGui::TreeNodeEx((void *) (intptr_t) i,
                                      g_node_flags_without_child | (gm->IsSelect() ? ImGuiTreeNodeFlags_Selected : 0),
                                      "%s", gm->GetName().c_str());

                    CheckSelected(gm);
                }
                i++;
            });
            ImGui::TreePop();
            ImGui::PopStyleVar();
        }

        scene.Unlock();
    }
}

void GUISystem::DrawChild(Framework::Helper::Types::SafePtr<Framework::Helper::GameObject> root) {
    unsigned long i = 0;

    root->ForEachChild([&i, this](Helper::Types::SafePtr<Helper::GameObject> child){
        if (child->HasChildren()) {
            bool open = ImGui::TreeNodeEx((void *) (intptr_t) i,
                                          g_node_flags_with_child |
                                          (child->IsSelect() ? ImGuiTreeNodeFlags_Selected : 0),
                                          "%s", child->GetName().c_str()
            );

            CheckSelected(child);

            if (open)
                DrawChild(child);
        } else {
            ImGui::TreeNodeEx((void *) (intptr_t) i,
                              g_node_flags_without_child | (child->IsSelect() ? ImGuiTreeNodeFlags_Selected : 0),
                              "%s", child->GetName().c_str()
            );

            CheckSelected(child);
        }

        i++;
    });

    ImGui::TreePop();
}

bool GUISystem::CollapsingHeader(const char *label, ImGuiTreeNodeFlags flags) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiID id = window->GetID(label);
    flags |= ImGuiTreeNodeFlags_CollapsingHeader;
    flags |= ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_ClipLabelForTrailingButton;
    bool is_open = ImGui::TreeNodeBehavior(id, flags, label);

    return is_open;
}

void GUISystem::DrawComponents(const Helper::Types::SafePtr<GameObject>& gameObject) {
    if (ImGui::BeginPopupContextWindow("InspectorMenu")) {
        if (ImGui::BeginMenu("Add component")) {
            for (const auto &a : Component::GetComponentsNames()) {
                if (ImGui::MenuItem(a.c_str())) {
                    gameObject->AddComponent(Component::CreateComponentOfName(a));
                    break;
                }
            }
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }

    gameObject->ForEachComponent([gameObject, this](Component* component) -> bool {
        if (ImGui::BeginPopupContextWindow("InspectorMenu")) {
            if (ImGui::BeginMenu("Remove component")) {
                if (ImGui::MenuItem(component->GetComponentName().c_str())) {
                    gameObject->RemoveComponent(component);
                    goto exit;
                }
                ImGui::EndMenu();
            }
            ImGui::EndPopup();
        }

        if (ImGui::CollapsingHeader(component->GetComponentName().c_str()))
            component->DrawOnInspector();

        return true;

        exit:
            ImGui::EndMenu();
            ImGui::EndPopup();
            return false;
    });
}

void GUISystem::DrawInspector(Framework::Helper::Types::SafePtr<Framework::Helper::World::Scene> scene) {
    Helper::Types::SafePtr<Helper::GameObject> gameObject;
    if (scene.LockIfValid()) {
        gameObject = scene->GetSelected();
        scene.Unlock();
    }

    if (gameObject.LockIfValid()) {
        if (bool v = gameObject->IsActive(); ImGui::Checkbox("Enabled", &v))
            gameObject->SetActive(v);

        std::string gm_name = gameObject->GetName();
        if (ImGui::InputText("Name", &gm_name))
            gameObject->SetNameFromInspector(gm_name);

        ImGui::Separator();
        DrawTextOnCenter("Transform");

        auto position = gameObject->GetTransform()->GetPosition().ToGLM();
        auto rotation = gameObject->GetTransform()->GetRotation().ToGLM();
        auto scale    = gameObject->GetTransform()->GetScale().ToGLM();

        ImGui::Text("[Global]");

        if (ImGui::InputFloat3("G Tr", &position[0], "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
            gameObject->GetTransform()->SetGlobalPosition(position);

        if (ImGui::InputFloat3("G Rt", &rotation[0], "%.3f", ImGuiInputTextFlags_ReadOnly))
            gameObject->GetTransform()->SetRotation(rotation);

        if (ImGui::InputFloat3("G Sc", &scale[0], "%.3f", ImGuiInputTextFlags_ReadOnly))
            gameObject->GetTransform()->SetScale(scale);

        ImGui::Text("[Local]");

        position = gameObject->GetTransform()->GetPosition(true).ToGLM();
        rotation = gameObject->GetTransform()->GetRotation(true).ToGLM();
        scale    = gameObject->GetTransform()->GetScale(true).ToGLM();

        if (ImGui::InputFloat3("L Tr", &position[0], "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
            gameObject->GetTransform()->SetLocalPosition(position);

        if (ImGui::InputFloat3("L Rt", &rotation[0], "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
            gameObject->GetTransform()->SetLocalRotation(rotation);

        if (ImGui::InputFloat3("L Sc", &scale[0], "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
            gameObject->GetTransform()->SetLocalScale(position);

        this->DrawComponents(gameObject);

        /*
        ImGui::Text("[Parent direction]");

        std::vector<Framework::Helper::Component *> comps = gameObject->GetComponents();
        for (Framework::Helper::Component *comp : comps) {
            std::string name = comp->GetComponentName();

            if (ImGui::CollapsingHeader(name.c_str()))
                comp->DrawOnInspector();
        }

        ImGui::Separator();

        ImGui::InvisibleButton("void", ImVec2(ImGui::GetCurrentWindow()->Size.x, ImGui::GetCurrentWindow()->Size.y - ImGui::GetItemRectMin().y));

        if (ImGui::BeginPopupContextItem("InspectorMenu", 1)) {
            if (ImGui::BeginMenu("Add component")) {
                for (const auto& a : Component::GetComponentsNames()) {
                    if (ImGui::MenuItem(a.c_str()))
                        gameObject->AddComponent(Component::CreateComponentOfName(a));
                }
                ImGui::EndMenu();
            }
            ImGui::EndPopup();
        }*/

        gameObject.Unlock();
    }
}

void GUISystem::CheckSelected(const Helper::Types::SafePtr<Helper::GameObject>& gm) {
    if (ImGui::IsItemClicked()) {
        if (!m_shiftPressed && gm->GetScene().Valid())
            gm->GetScene()->UnselectAll();

        gm->SetSelect(true);
    }
}

void GUISystem::DrawGuizmoTools() {
    static bool snapAct = true;

    if (ButtonWithId("engine_tool_move", "M", m_sizeB, 0, true,
                     ImVec2(m_space, m_space), m_currentGuizmoOperation == ImGuizmo::TRANSLATE ? m_act : m_def)) {
        m_currentGuizmoOperation = ImGuizmo::TRANSLATE;
        m_boundsActive = false;
    }

    if (ButtonWithId("engine_tool_rotate", "R", m_sizeB, 0, true,
                     ImVec2(m_space * 2 + m_sizeB.x, m_space),
                     m_currentGuizmoOperation == ImGuizmo::ROTATE ? m_act : m_def)) {
        m_currentGuizmoOperation = ImGuizmo::ROTATE;
        m_boundsActive = false;
    }

    if (ButtonWithId("engine_tool_scale", m_boundsActive ? "S+" : "S", m_sizeB, 0, true,
                     ImVec2(m_space * 3 + m_sizeB.x * 2, m_space),
                     m_currentGuizmoOperation == ImGuizmo::SCALE ? m_act : m_def)) {
        if (m_currentGuizmoOperation == ImGuizmo::SCALE)
            m_boundsActive = !m_boundsActive;

        m_currentGuizmoOperation = ImGuizmo::SCALE;
    }

    if (ButtonWithId("engine_tool_mode", "L", m_sizeB, 0, true,
                     ImVec2(m_space * 5 + m_sizeB.x * 4, m_space), m_currentGuizmoMode == ImGuizmo::LOCAL ? m_act : m_def)) {
        if (m_currentGuizmoMode == ImGuizmo::LOCAL)
            m_currentGuizmoMode = ImGuizmo::WORLD;
        else
            m_currentGuizmoMode = ImGuizmo::LOCAL;
    }

    if (ButtonWithId("engine_tool_center", "C", m_sizeB, 0, true,
                     ImVec2(m_space * 6 + m_sizeB.x * 5, m_space), m_centerActive ? m_act : m_def)) {
        m_centerActive = !m_centerActive;
    }

    std::string snap_str = std::to_string(m_snapValue / 100.0);
    snap_str.resize(4);
    if (ButtonWithId("engine_tool_snap", (snap_str + "x").c_str(),
                     m_sizeB + ImVec2(5, 0), 0, true,
                     ImVec2(m_space * 7 + m_sizeB.x * 6, m_space), snapAct ? m_act : m_def)) {
        if (m_snapValue >= 400) {
            m_snapValue = 25;
            snapAct = false;
        } else {
            if (snapAct)
                m_snapValue *= 2;
            else
                snapAct = true;
        }
    }
}

void GUISystem::DrawWorldEdit(Types::SafePtr<Helper::World::Scene> scene) {
    if (scene.LockIfValid()) {
        const auto&& observer = scene->GetObserver();
        const auto offset = observer.m_offset;

        ImGui::Separator();
        DrawTextOnCenter("Current");

        ImGui::InputFloat3("Chunk", &observer.m_chunk.ToGLM()[0], "%.3f", ImGuiInputTextFlags_ReadOnly);
        ImGui::InputFloat2("Region", &observer.m_region.ToGLM()[0], "%.2f", ImGuiInputTextFlags_ReadOnly);

        ImGui::Separator();
        DrawTextOnCenter("Offset");

        auto chunkOffset = offset.m_chunk.ToGLM();
        if (ImGui::InputFloat3("Chunk offset", &chunkOffset[0], "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
            scene->SetWorldOffset(World::Offset(offset.m_region, chunkOffset));

        auto regionOffset = offset.m_region.ToGLM();
        if (ImGui::InputFloat2("Region offset", &regionOffset[0], "%.2f", ImGuiInputTextFlags_EnterReturnsTrue))
            scene->SetWorldOffset(World::Offset(regionOffset, offset.m_chunk));

        scene.Unlock();
    }
}

void GUISystem::DrawGuizmo(Framework::Graphics::Camera *camera, Helper::Types::SafePtr<Helper::GameObject> gameObject) {
    if (!camera)
        return;

    if (gameObject.LockIfValid()) {
        ImGuiWindow *window = ImGui::GetCurrentWindow();
        if (!window || window->SkipItems)
            return;

        auto img_size = camera->GetSize();

        auto win_size = Math::FVector2(window->Size.x, window->Size.y);

        const Helper::Math::Unit dx = win_size.x / img_size.x;
        const Helper::Math::Unit dy = win_size.y / img_size.y;

        if (dy > dx)
            img_size *= dx;
        else
            img_size *= dy;

        static float bounds[] = {-0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f};
        static float boundsSnap[] = {0.1f, 0.1f, 0.1f};

        glm::vec3 snap = glm::vec3(1, 1, 1) * ((float) m_snapValue / 100.f);

        glm::mat4 delta;

        auto barycenter = gameObject->GetBarycenter();

        glm::mat4 mat = [=]() -> auto {
            if (m_centerActive && !barycenter.IsInfinity())
                return gameObject->GetTransform()->GetMatrix(Helper::Graph::PipeLine::OpenGL, barycenter);
            return gameObject->GetTransform()->GetMatrix(Helper::Graph::PipeLine::OpenGL);
        }();

        static float axis[3] = {0, 0, 0};
        static float value = 0.0;
        static float old_rotate = 0.0;
        static float old_scale = 0.0;

        ImGuizmo::SetRect(
                (float)(ImGui::GetWindowPos().x + (win_size.x - img_size.x) / 2.f),
                (float)(ImGui::GetWindowPos().y + (win_size.y - img_size.y) / 2.f),
                img_size.x,
                img_size.y);

        if (ImGuizmo::Manipulate(
                &camera->GetImGuizmoView()[0][0],
                &camera->GetProjection()[0][0],
                m_boundsActive ? ImGuizmo::BOUNDS : m_currentGuizmoOperation, m_currentGuizmoMode,
                &mat[0][0],
                &delta[0][0], nullptr, nullptr, boundsSnap,
                &value, &axis[0])) {
            if (m_currentGuizmoOperation == ImGuizmo::OPERATION::ROTATE) {
                if (abs((value - old_rotate)) < 1) {
                    if (m_centerActive && !barycenter.IsInfinity()) {
                        gameObject->GetTransform()->RotateAround(
                                barycenter,
                                FVector3(axis).InverseAxis(1),
                                (value - old_rotate) * 20.0,
                                m_currentGuizmoMode == ImGuizmo::LOCAL);
                    } else {
                        if (m_currentGuizmoMode == ImGuizmo::LOCAL)
                            gameObject->GetTransform()->RotateAxis(FVector3(axis).InverseAxis(1), (value - old_rotate) * 20.0);
                        else
                            gameObject->GetTransform()->GlobalRotateAxis(FVector3(axis).InverseAxis(1), (value - old_rotate) * 20.0);
                    }
                }
            } else if (m_currentGuizmoOperation == ImGuizmo::OPERATION::TRANSLATE) {
                if (value < 1) {
                    if (m_currentGuizmoMode == ImGuizmo::LOCAL)
                        gameObject->GetTransform()->Translate(
                                gameObject->GetTransform()->Direction(FVector3(axis).InverseAxis(0), true) * value);
                    else
                        gameObject->GetTransform()->GlobalTranslate(FVector3(axis).InverseAxis(0) * value);
                }
            } else if (m_currentGuizmoOperation == ImGuizmo::OPERATION::SCALE) {
                if (value == 0)
                    old_scale = 0;

                if (m_currentGuizmoMode == ImGuizmo::MODE::LOCAL)
                    gameObject->GetTransform()->Scaling(FVector3(axis) * (value - old_scale));

                old_scale = value;
            }
        }

        old_rotate = value;

        gameObject.Unlock();
    }
}

bool GUISystem::ButtonWithId(
    const char *_id,
    const char *label,
    ImVec2 button_size,
    ImGuiButtonFlags flags,
    bool imposition,
    ImVec2 offset,
    ImVec4 color)
{
    const bool has_color = !Vec4Null(color);

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(_id);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos + offset;
    if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
        pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
    ImVec2 size = ImGui::CalcItemSize(button_size, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

    const ImRect bb(pos, pos + size);

    if (has_color)
        ImGui::PushStyleColor(ImGuiCol_Button, color);

    if (!imposition)
    {
        ImGui::ItemSize(size, style.FramePadding.y);

        if (!ImGui::ItemAdd(bb, id))
            return false;
    }

    if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat)
        flags |= ImGuiButtonFlags_Repeat;
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);

    // Render
    const ImU32 col = ImGui::GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    ImGui::RenderNavHighlight(bb, id);
    ImGui::RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
    ImGui::RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags);

    if (has_color)
        ImGui::PopStyleColor();

    return pressed;
}

void GUISystem::SetGuizmoTool(uint8_t toolId) {
    switch (toolId) {
        case 0: m_currentGuizmoOperation = ImGuizmo::OPERATION::TRANSLATE; break;
        case 1: m_currentGuizmoOperation = ImGuizmo::OPERATION::ROTATE;    break;
        case 2: m_currentGuizmoOperation = ImGuizmo::OPERATION::SCALE;     break;
        default:
            Helper::Debug::Error("GUISystem::SetGuizmoTool() : unknown tool id!");
            return;
    }
}

bool GUISystem::BeginMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New scene")) {

            }

            if (ImGui::MenuItem("Load scene")) {

            }

            if (ImGui::MenuItem("Save scene")) {
                if (auto scene = Engine::Instance().GetScene(); scene.LockIfValid()) {
                    auto scenesPath = Helper::StringUtils::MakePath(Helper::ResourceManager::Instance().GetResourcesFolder() + "/Scenes/", false);
                    if (auto path = FileSystem::SaveFileDialog(scenesPath, "Scene Files(*.scene)"); !path.empty()) {
                        auto sceneName = StringUtils::GetFileNameFromFullPath(path);
                        auto folder = StringUtils::GetDirToFileFromFullPath(path);

                        scene->SetName(sceneName);

                        if (scene->Save(folder)) {
                            Helper::Debug::System("GUISystem::BeginMenuBar() : scene saved as \"" + path + "\"");
                        } else {
                            Helper::Debug::Error("GUISystem::BeginMenuBar() : failed to save scene! \n\tPath: \"" + path + "\"");
                        }
                    }
                    scene.Unlock();
                } else {
                    Helper::Debug::Warn("GUISystem::BeginMenuBar() : scene isn't valid!");
                }
            }

            if (ImGui::MenuItem("Close scene")) {
                Engine::Instance().CloseScene();
            }

            if (ImGui::MenuItem("Exit")) {
                EventManager::Push(Helper::EventManager::Event::Exit);
            }

            ImGui::EndMenu();
        }
        return true;
    } else
        return false;
}

void GUISystem::EndMenuBar() {
    ImGui::EndMainMenuBar();
}

