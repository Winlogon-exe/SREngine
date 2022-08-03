//
// Created by Nikita on 29.12.2020.
//

#ifndef GAMEENGINE_ENGINE_H
#define GAMEENGINE_ENGINE_H

#include <Core/GUI/EditorGUI.h>
#include <Core/EvoScriptAPI.h>
#include <Core/EngineCommands.h>

#include <Utils/Events/EventManager.h>
#include <Utils/Types/Time.h>
#include <Utils/Types/Timer.h>
#include <Utils/FileSystem/FileSystem.h>
#include <Utils/Types/Thread.h>
#include <Utils/Types/SafePointer.h>
#include <Utils/Input/InputDispatcher.h>
#include <Utils/Input/InputHandler.h>
#include <Utils/Types/Function.h>

namespace SR_WORLD_NS {
    class Scene;
}

namespace SR_GRAPH_NS {
    class Window;
    class Render;
    class Environment;
    class RenderScene;
    class RenderContext;
}

namespace SR_GTYPES_NS {
    class Camera;
}

namespace Framework {
    class Engine : public Helper::Singleton<Engine> {
        friend class Singleton<Engine>;
        friend class API;
        using PipelinePtr = SR_GRAPH_NS::Environment*;
        using RenderScenePtr = SR_HTYPES_NS::SafePtr<SR_GRAPH_NS::RenderScene>;
        using ScenePtr = Helper::Types::SafePtr<SR_WORLD_NS::Scene>;
        using CameraPtr = SR_GTYPES_NS::Camera*;
        using Clock = std::chrono::high_resolution_clock;
        using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;
    private:
        Engine() = default;
        ~Engine() override = default;

    public:
        void Reload();
        bool SetScene(const ScenePtr& scene);

        SR_NODISCARD SR_INLINE ScenePtr GetScene() const { return m_scene; }
        SR_NODISCARD SR_INLINE Graphics::Window* GetWindow() const { return m_window; }
        SR_NODISCARD SR_INLINE bool IsRun() const { return m_isRun; }
        SR_NODISCARD SR_INLINE Core::GUI::EditorGUI* GetEditor() const { return m_editor; }
        SR_NODISCARD SR_INLINE SR_UTILS_NS::CmdManager* GetCmdManager() const { return m_cmdManager; }

    public:
        bool Create(Graphics::Window* window);
        bool Init();
        bool Run();
        void Await();
        bool Close();

    private:
        void Prepare();
        void FixedUpdate();
        void Update(float_t dt);
        void ResizeCallback(const SR_MATH_NS::IVector2& size);
        void DrawCallback();
        bool RegisterLibraries();
        void WorldThread();

    private:
        std::atomic<bool> m_isCreate    = false;
        std::atomic<bool> m_isInit      = false;
        std::atomic<bool> m_isRun       = false;

        std::atomic<bool> m_exitEvent   = false;
        std::atomic<bool> m_isActive    = false;

        float_t m_speed = 1.f;
        float_t m_updateFrequency = 1.f;
        float_t m_accumulator = 1.f;
        TimePoint m_timeStart;

        SR_UTILS_NS::CmdManager* m_cmdManager  = nullptr;
        SR_GRAPH_NS::Window*     m_window      = nullptr;
        SR_GRAPH_NS::Render*     m_render      = nullptr;

        SR_HTYPES_NS::Thread::Ptr     m_worldThread = nullptr;

        Core::GUI::EditorGUI*         m_editor      = nullptr;
        ScenePtr                      m_scene       = ScenePtr();
        RenderScenePtr                m_renderScene = RenderScenePtr();
        SR_UTILS_NS::InputDispatcher* m_input       = nullptr;
        PipelinePtr                   m_pipeline    = nullptr;

    };
}

#endif //GAMEENGINE_ENGINE_H
