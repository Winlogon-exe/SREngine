//
// Created by Nikita on 27.11.2020.
//

#ifndef GAMEENGINE_COMPONENT_H
#define GAMEENGINE_COMPONENT_H

#include <Utils/ECS/EntityManager.h>
#include <Utils/Math/Vector3.h>
#include <Utils/Types/SafePointer.h>
#include <Utils/Common/Singleton.h>
#include <Utils/Types/Marshal.h>
#include <Utils/Types/SafeVariable.h>
#include <Utils/World/Scene.h>

/**
 *  Component adding if enabled:
 *      Reset() -> OnEnabled()
 *
 *  Component adding if disabled: nothing
 *
 *  Component removing if enabled:
 *      OnDisabled() -> OnDestroy()
 *
 *  Component removing if disabled and started:
 *      OnDestroy()
 */

namespace SR_HTYPES_NS {
    class DataStorage;
}

namespace SR_UTILS_NS {
    class ComponentManager;
    class Component;
    class Transform2D;
    class Transform3D;
    class Transform;
    class GameObject;

    class SR_DLL_EXPORT Component : public Entity {
        friend class GameObject;
        friend class ComponentManager;
    public:
        using GameObjectPtr = SR_HTYPES_NS::SafePtr<GameObject>;
    public:
        ~Component() override = default;

    public:
        virtual void OnMatrixDirty() { }

        /// Вызывается после добавления компонента к игровому объекту
        virtual void OnAttached() { }
        /// Вызывается кода компонент убирается с объекта, либо объект уничтожается
        virtual void OnDestroy() { }

        virtual void OnEnable() { m_isActive = true; }
        virtual void OnDisable() { m_isActive = false; }

        virtual void Awake() { m_isAwake = true; }
        virtual void Start() { m_isStarted = true; }
        virtual void Update(float_t dt) { }
        virtual void FixedUpdate() { }
        virtual void LateUpdate() { }

    public:
        void CheckActivity();

        void SetEnabled(bool value);

        /// Активен и компонент и его родительский объект
        SR_NODISCARD bool IsActive() const noexcept;
        /// Активен сам компонент, независимо от объекта
        SR_NODISCARD bool IsEnabled() const noexcept;

        SR_NODISCARD bool IsAwake() const noexcept { return m_isAwake; }
        SR_NODISCARD bool IsStarted() const noexcept { return m_isStarted; }

        SR_NODISCARD virtual bool ExecuteInEditMode() const { return false; }
        SR_NODISCARD virtual Math::FVector3 GetBarycenter() const { return Math::InfinityFV3; }
        SR_NODISCARD SR_INLINE std::string GetComponentName() const { return m_name; }
        SR_NODISCARD SR_INLINE size_t GetComponentId() const { return m_componentId; }
        SR_NODISCARD SR_INLINE Component* BaseComponent() { return this; }
        SR_NODISCARD SR_INLINE GameObject* GetParent() const;
        SR_NODISCARD SR_WORLD_NS::Scene::Ptr GetScene() const;
        SR_NODISCARD GameObjectPtr GetRoot() const;
        SR_NODISCARD Transform* GetTransform() const;

    protected:
        template<typename T> void InitComponent() {
            m_componentId = typeid(T).hash_code();
            m_name = StringUtils::BackRead(typeid(T).name(), ':');
        }

        SR_NODISCARD SR_HTYPES_NS::Marshal Save(SavableFlags flags) const override;

    private:
        void SetParent(GameObject* parent);

    protected:
        std::atomic<bool> m_isEnabled = true;

        bool m_isActive = false;
        bool m_isAwake = false;
        bool m_isStarted = false;

        /// TODO: need remove for optimization, use numeric id
        std::string m_name = "Unknown";

        uint64_t m_componentId = SIZE_MAX;
        SR_HTYPES_NS::SafeVar<GameObject*> m_parent = nullptr;

    };
}


#endif //GAMEENGINE_COMPONENT_H
