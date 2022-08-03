//
// Created by Nikita on 27.11.2020.
//

#ifndef GAMEENGINE_GAMEOBJECT_H
#define GAMEENGINE_GAMEOBJECT_H

#include <Utils/ECS/EntityManager.h>

#include <Utils/Math/Vector3.h>
#include <Utils/Types/SafePointer.h>
#include <Utils/Types/SafeVariable.h>

namespace SR_UTILS_NS::World {
    class Scene;
}

namespace SR_UTILS_NS {
    class Transform3D;
    class Component;

    SR_ENUM(GameObjectDestroyBy,
        GAMEOBJECT_DESTROY_BY_UNKNOWN    = 1 << 0,
        GAMEOBJECT_DESTROY_BY_SCENE      = 1 << 1,
        GAMEOBJECT_DESTROY_BY_GAMEOBJECT = 1 << 2,
        GAMEOBJECT_DESTROY_BY_OTHER      = 1 << 3,
        GAMEOBJECT_DESTROY_BY_COMMAND    = 1 << 4,
    );
    typedef uint64_t GODestroyByBits;

    SR_ENUM(GameObjectFlags,
        GAMEOBJECT_FLAG_NONE    = 1 << 0,
        GAMEOBJECT_FLAG_NO_SAVE = 1 << 1,
    );
    typedef uint64_t GameObjectFlagBits;

    class SR_DLL_EXPORT GameObject : public Types::SafePtr<GameObject>, public Entity {
        SR_ENTITY_SET_VERSION(1001);
    private:
        friend class World::Scene;
        friend class Transform3D;
        friend class Transform2D;
        friend class Transform;
        friend class Component;
    public:
        typedef Types::SafePtr<GameObject> Ptr;
        using GameObjects = std::list<GameObject::Ptr>;
        using Components = SR_HTYPES_NS::SafeVar<std::list<Component*>>;

    private:
        GameObject(const Types::SafePtr<World::Scene>& scene, std::string name, std::string tag = "Untagged");
        ~GameObject() override;

    public:
        SR_NODISCARD Types::SafePtr<World::Scene> GetScene() const { return m_scene; }
        SR_NODISCARD Transform* GetParentTransform() const { return m_parent ? m_parent->m_transform : nullptr; }
        SR_NODISCARD Transform* GetTransform() const { return m_transform; }
        SR_NODISCARD GameObject::Ptr GetParent() const { return m_parent; }
        SR_NODISCARD std::string GetName() const;
        SR_NODISCARD bool HasTag() const;
        SR_NODISCARD bool IsActive() const;
        SR_NODISCARD bool IsEnabled() const { return m_isEnabled; }
        SR_NODISCARD SR_INLINE bool HasChildren() const { return !m_children.empty(); }
        SR_NODISCARD SR_INLINE GameObjects& GetChildrenRef() { return m_children; }
        SR_NODISCARD SR_INLINE GameObjects GetChildren() const { return m_children; }
        SR_NODISCARD SR_INLINE GameObjectFlagBits GetFlags() const { return m_flags; }

        SR_NODISCARD SR_HTYPES_NS::Marshal Save(SavableFlags flags) const override;
        SR_NODISCARD std::list<EntityBranch> GetEntityBranches() const override;

        Math::FVector3 GetBarycenter();
        Math::FVector3 GetHierarchyBarycenter();

        void ForEachChild(const std::function<void(Types::SafePtr<GameObject>&)>& fun);
        void ForEachChild(const std::function<void(const Types::SafePtr<GameObject>&)>& fun) const;
        bool SetParent(const GameObject::Ptr& parent);
        void SetName(const std::string& name);
        void SetTag(const std::string& tag);

        Component* GetComponent(const std::string& name);
        Component* GetComponent(size_t id);
        std::list<Component*> GetComponents() { return m_components; }
        //std::list<Component*>& GetComponentsRef() { return m_components; }

        /** Отличие от AddComponent в том, что используется только при загрузке объекта,
         * не вызывая при этом Awake -> OnEnable -> Start */
        bool LoadComponent(Component* component);
        /** Добавляет новый компонент на объект, вызывает у компонента
         * последовательность Awake -> OnEnable -> Start */
        bool AddComponent(Component* component);

        bool RemoveComponent(Component* component);
        bool ReplaceComponent(Component* source, Component* destination);
        bool ContainsComponent(const std::string& name);
        void ForEachComponent(const std::function<bool(Component*)>& fun);

        bool Contains(const GameObject::Ptr& child);
        void SetEnabled(bool value);
        void Destroy(GODestroyByBits by = GAMEOBJECT_DESTROY_BY_OTHER);
        void SetTransform(Transform* transform);
        void SetFlags(GameObjectFlagBits flags) { m_flags = flags; }

        bool MoveToTree(const GameObject::Ptr& destination);
        bool AddChild(const GameObject::Ptr& child);
        void RemoveChild(const GameObject::Ptr& child);

        void Awake();
        void Start();
        void FixedUpdate();
        void Update(float_t dt);

        void CheckActivity();

    public:
        template<typename T> T* GetComponent() {
            return dynamic_cast<T*>(GetComponent(typeid(T).hash_code()));
        }

    private:
        void OnAttached();
        void OnMatrixDirty();

        /// TODO: remove this method
        void Free();

        bool UpdateEntityPath();

    private:
        std::atomic<bool>                   m_isEnabled      = true;
        std::atomic<bool>                   m_isActive       = false;
        std::atomic<bool>                   m_isDestroy      = false;

        GameObject::Ptr                     m_parent         = GameObject::Ptr();
        std::list<GameObject::Ptr>          m_children       = { };

        Types::SafePtr<World::Scene>        m_scene          = Types::SafePtr<World::Scene>();
        Transform*                          m_transform      = nullptr;

        Components                          m_components     = std::list<Component*>();

        std::string                         m_name           = "Unnamed";
        std::string                         m_tag            = "Untagged";

        GameObjectFlagBits                  m_flags          = GAMEOBJECT_FLAG_NONE;

    };
}

#endif //GAMEENGINE_GAMEOBJECT_H
