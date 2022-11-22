//
// Created by Monika on 28.07.2022.
//

#ifndef SRENGINE_RIGIDBODY_H
#define SRENGINE_RIGIDBODY_H

#include <Physics/PhysicsLib.h>
#include <Physics/CollisionShape.h>

#include <Utils/Common/Measurement.h>
#include <Utils/ECS/Component.h>
#include <Utils/Types/SafePointer.h>
#include <Utils/Math/Matrix4x4.h>

namespace SR_PHYSICS_NS {
    class PhysicsScene;
    class LibraryImpl;
}

namespace SR_PTYPES_NS {
    class Rigidbody : public SR_UTILS_NS::Component {
        friend class SR_PHYSICS_NS::PhysicsScene;
        SR_ENTITY_SET_VERSION(1001);
        using Super = SR_UTILS_NS::Component;
        using LibraryPtr = SR_PHYSICS_NS::LibraryImpl*;
        using PhysicsScenePtr = SR_HTYPES_NS::SafePtr<PhysicsScene>;
    public:
        explicit Rigidbody(LibraryPtr pLibrary);
        ~Rigidbody() override;

    public:
        static ComponentPtr LoadComponent(SR_HTYPES_NS::Marshal& marshal, const SR_HTYPES_NS::DataStorage* dataStorage);

        SR_NODISCARD SR_HTYPES_NS::Marshal::Ptr Save(SR_HTYPES_NS::Marshal::Ptr pMarshal, SR_UTILS_NS::SavableFlags flags) const override;

        virtual bool UpdateMatrix();
        virtual bool UpdateShape() { return false; }

        SR_NODISCARD virtual SR_UTILS_NS::Measurement GetMeasurement() const;

        SR_NODISCARD ShapeType GetType() const noexcept;
        SR_NODISCARD SR_MATH_NS::FVector3 GetCenter() const noexcept;
        SR_NODISCARD SR_MATH_NS::FVector3 GetCenterDirection() const noexcept;
        SR_NODISCARD float_t GetMass() const noexcept;
        SR_NODISCARD bool IsDirty() const noexcept { return m_dirty; }
        SR_NODISCARD virtual void* GetHandle() const noexcept = 0;

        void SetDirty(bool value) { m_dirty = value; }

        virtual void AddLocalVelocity(const SR_MATH_NS::FVector3& velocity) { }
        virtual void AddGlobalVelocity(const SR_MATH_NS::FVector3& velocity) { }
        virtual void SetVelocity(const SR_MATH_NS::FVector3& velocity) { }

        virtual void SetCenter(const SR_MATH_NS::FVector3& center);
        virtual void SetMass(float_t mass);
        virtual void SetType(ShapeType type);

    protected:
        void OnEnable() override;
        void OnDisable() override;
        void OnAttached() override;
        void OnDestroy() override;

        void UpdateDebugShape();

        void OnMatrixDirty() override;

        virtual bool InitBody();
        virtual void DeInitBody();

        PhysicsScenePtr GetPhysicsScene();

    protected:
        CollisionShape::Ptr m_shape = nullptr;
        LibraryPtr m_library = nullptr;

        PhysicsScenePtr m_physicsScene;

        ShapeType m_type = ShapeType::Unknown;

        SR_MATH_NS::FVector3 m_translation;
        SR_MATH_NS::Quaternion m_rotation;
        SR_MATH_NS::FVector3 m_scale;

        SR_MATH_NS::FVector3 m_center;

        bool m_dirty = false;

        float_t m_mass = 1.f;
        uint64_t m_debugId = SR_ID_INVALID;

    };
}

#endif //SRENGINE_RIGIDBODY_H
