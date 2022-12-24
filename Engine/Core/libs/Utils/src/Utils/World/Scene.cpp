//
// Created by Nikita on 30.11.2020.
//

#include <Utils/World/Scene.h>

#include <Utils/World/Region.h>
#include <Utils/World/Chunk.h>
#include <Utils/World/SceneAllocator.h>
#include <Utils/World/SceneLogic.h>
#include <Utils/World/SceneCubeChunkLogic.h>

#include <Utils/ECS/Component.h>
#include <Utils/ECS/GameObject.h>

namespace SR_WORLD_NS {
    Scene::Scene(const std::string &name)
        : Super(this)
        , m_name(name)
        , m_logic(new SceneCubeChunkLogic(GetThis()))
    { }

    Scene::~Scene() {
        SRAssert(m_isDestroy);

        if (Debug::Instance().GetLevel() >= Debug::Level::Low) {
            SR_LOG("Scene::~Scene() : free \"" + std::string(m_name) + "\" scene pointer...");
        }
    }

    GameObject::Ptr Scene::Instance(const std::string& name) {
        if (Debug::Instance().GetLevel() >= Debug::Level::High) {
            SR_LOG("Scene::Instance() : instance \"" + name + "\" game object at \"" + std::string(m_name) + "\" scene.");
        }

        const uint64_t id = m_freeObjIndices.empty() ? m_gameObjects.size() : m_freeObjIndices.front();

        GameObject::Ptr gm = *(new GameObject(GetThis(), name));

        gm->SetIdInScene(id);

        if (m_freeObjIndices.empty()) {
            m_gameObjects.emplace_back(gm);
        }
        else {
            m_gameObjects[m_freeObjIndices.front()] = gm;
            m_freeObjIndices.erase(m_freeObjIndices.begin());
        }

        m_isHierarchyChanged = true;

        return gm;
    }

    Scene::GameObjectPtr Scene::FindOrInstance(const std::string &name) {
        if (auto&& pFound = Find(name)) {
            return pFound;
        }

        return Instance(name);
    }

    Scene::GameObjectPtr Scene::Instance(SR_HTYPES_NS::Marshal &marshal) {
        return GameObject::Load(marshal, *this, [this](const GameObject::Ptr& ptr) -> uint64_t {
            const uint64_t id = m_freeObjIndices.empty() ? m_gameObjects.size() : m_freeObjIndices.front();

            if (m_freeObjIndices.empty()) {
                m_gameObjects.emplace_back(ptr);
            }
            else {
                m_gameObjects[m_freeObjIndices.front()] = ptr;
                m_freeObjIndices.erase(m_freeObjIndices.begin());
            }

            m_isHierarchyChanged = true;

            return id;
        });
    }

    Scene::Ptr Scene::New(const Path& path) {
        if (Debug::Instance().GetLevel() > Debug::Level::None) {
            SR_LOG("Scene::New() : creating new scene...");
        }

        if (SR_PLATFORM_NS::IsExists(path)) {
            SR_ERROR("Scene::New() : scene already exists!");
            return Scene::Ptr();
        }

        auto&& scene = SceneAllocator::Instance().Allocate();

        if (!scene) {
            SR_ERROR("Scene::New() : failed to allocate scene!");
            return Scene::Ptr();
        }

        scene->SetPath(path);
        scene->SetName(path.GetBaseName());

        return scene;
    }

    Scene::Ptr World::Scene::Load(const Path& path) {
        if (Debug::Instance().GetLevel() > Debug::Level::None) {
            SR_LOG("Scene::Load() : loading scene...\n\tPath: " + path.ToString());
        }

        auto&& scene = SceneAllocator::Instance().Allocate();

        if (!scene) {
            SR_ERROR("Scene::Load() : failed to allocate scene!");
            return Scene::Ptr();
        }

        scene->SetPath(path);
        scene->SetName(path.GetBaseName());

        auto&& componentsPath = scene->GetPath().Concat("data/components.bin");
        if (auto&& rootComponentsMarshal = SR_HTYPES_NS::Marshal::LoadPtr(componentsPath)) {
            auto&& components = SR_UTILS_NS::ComponentManager::Instance().LoadComponents(*rootComponentsMarshal);
            delete rootComponentsMarshal;
            for (auto&& pComponent : components) {
                scene->LoadComponent(pComponent);
            }
        }
        else {
            SR_WARN("Scene::Load() : file not found!\n\tPath: " + componentsPath.ToString());
        }

        return scene;
    }

    bool Scene::Destroy() {
        if (m_isDestroy) {
            SR_ERROR("Scene::Destroy() : scene \"" + std::string(m_name) + "\" already destroyed!");
            return false;
        }

        IComponentable::DestroyComponents();

        m_logic->Destroy();

        SR_SAFE_DELETE_PTR(m_logic);

        if (Debug::Instance().GetLevel() > Debug::Level::None) {
            SR_LOG("Scene::Destroy() : complete unloading!");
            SR_LOG("Scene::Destroy() : destroying \"" + std::string(m_name) + "\" scene contains "+ std::to_string(m_gameObjects.size()) +" game objects...");
        }

        for (auto gameObject : GetRootGameObjects()) {
            gameObject.AutoFree([](GameObject* gm) {
                gm->Destroy(GAMEOBJECT_DESTROY_BY_SCENE);
            });
        }

        if (m_gameObjects.size() != m_freeObjIndices.size()) {
            SR_WARN(Format("Scene::Destroy() : after destroying the root objects, "
                                       "there are %i objects left!", m_gameObjects.size() - m_freeObjIndices.size()));
            m_gameObjects.clear();
        }
        m_freeObjIndices.clear();

        m_isDestroy = true;
        m_isHierarchyChanged = true;

        if (Debug::Instance().GetLevel() > Debug::Level::None) {
            SR_LOG("Scene::Destroy() : scene successfully destroyed!");
        }

        return true;
    }

    Scene::GameObjects & Scene::GetRootGameObjects() {
        if (!m_isHierarchyChanged) {
            return m_rootObjects;
        }

        m_rootObjects.clear();
        m_rootObjects.reserve(m_gameObjects.size() / 2);

        for (auto&& gameObject : m_gameObjects) {
            if (gameObject && !gameObject->GetParent().Valid()) {
                m_rootObjects.emplace_back(gameObject);
            }
        }

        m_isHierarchyChanged = false;

        return m_rootObjects;
    }

    GameObject::Ptr Scene::FindByComponent(const std::string &name) {
        for (auto&& gameObject : m_gameObjects) {
            if (gameObject->ContainsComponent(name)) {
                return gameObject;
            }
        }

        return GameObject::Ptr();
    }

    void Scene::OnChanged() {
        m_isHierarchyChanged = true;
    }

    bool Scene::Save() {
        return SaveAt(m_path);
    }

    bool Scene::SaveAt(const Path& path) {
        SR_INFO(SR_FORMAT("Scene::SaveAt() : save scene...\n\tPath: %s", path.CStr()));

        SetPath(path);
        SetName(path.GetBaseName());

        auto&& pSceneRootMarshal = SaveComponents(nullptr, SAVABLE_FLAG_NONE);
        if (!pSceneRootMarshal->Save(m_path.Concat("data/components.bin"))) {
            SR_ERROR("Scene::SaveAt() : failed to save scene components!");
        }
        SR_SAFE_DELETE_PTR(pSceneRootMarshal);

        if (!m_logic->Save(path)) {
            SR_ERROR("Scene::SaveAt() : failed to save scene logic!");
            return false;
        }

        return SR_XML_NS::Document::New().Save(path.Concat("main.scene"));
    }

    void Scene::Update(float_t dt) {
        m_logic->Update(dt);
    }

    bool Scene::Remove(const GameObject::Ptr &gameObject) {
        const uint64_t idInScene = gameObject->GetIdInScene();

        if (idInScene >= m_gameObjects.size()) {
            SRHalt("Scene::Remove() : invalid game object id!");
            return false;
        }

        if (m_gameObjects.at(idInScene) != gameObject) {
            SRHalt("Scene::Remove() : game objects do not match!");
            return false;
        }

        m_gameObjects.at(idInScene) = GameObject::Ptr();
        m_freeObjIndices.emplace_back(idInScene);
        OnChanged();

        return true;
    }

    GameObject::Ptr Scene::Instance(const SR_HTYPES_NS::RawMesh *rawMesh) {
        SRAssert2(false, "Method isn't implemented!");
        return GameObject::Ptr();
    }

    GameObject::Ptr Scene::InstanceFromFile(const std::string &path) {
        if (auto&& raw = SR_HTYPES_NS::RawMesh::Load(path)) {
            GameObject::Ptr root = Instance(raw); //TODO:Сделать обратимость

            if (raw->GetCountUses() == 0) {
                raw->Destroy();
            }

            return root;
        }

        return GameObject::Ptr();
    }

    bool Scene::Reload() {
        SR_INFO("Scene::Reload() : reload scene...");
        return m_logic->Reload();
    }

    GameObject::Ptr Scene::Find(const std::string &name) {
        auto&& hashName = SR_UTILS_NS::HashCombine(name);

        for (auto&& object : m_gameObjects) {
            /// блокировать объекты не нужно, так как уничтожиться они могут только из сцены
            /// Но стоит предусмотреть защиту от одновременного изменения имени
            if (object && object->GetHashName() == hashName) {
                return object;
            }
        }

        return GameObject::Ptr();
    }
}