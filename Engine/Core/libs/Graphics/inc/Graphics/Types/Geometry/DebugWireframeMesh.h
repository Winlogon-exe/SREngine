//
// Created by Monika on 05.10.2021.
//

#ifndef GAMEENGINE_DEBUGWIREFRAMEMESH_H
#define GAMEENGINE_DEBUGWIREFRAMEMESH_H

#include <Utils/Types/IRawMeshHolder.h>

#include <Graphics/Types/Geometry/IndexedMesh.h>
#include <Graphics/Types/Vertices.h>
#include <Graphics/Types/Uniforms.h>

namespace SR_GTYPES_NS {
    class DebugWireframeMesh final : public IndexedMesh, public SR_HTYPES_NS::IRawMeshHolder {
        using Super = IndexedMesh;
    public:
        typedef Vertices::SimpleVertex VertexType;

    public:
        DebugWireframeMesh();

    private:
        ~DebugWireframeMesh() override = default;

    public:
        void Draw() override;

        void SetMatrix(const SR_MATH_NS::Matrix4x4& matrix4X4);
        void SetColor(const SR_MATH_NS::FVector4& color);

        bool OnResourceReloaded(SR_UTILS_NS::IResource* pResource) override;

        void OnRawMeshChanged() override;

        void SetConvex(bool isConvex) { m_isConvex = isConvex; }

        SR_NODISCARD std::vector<uint32_t> GetIndices() const override;
        SR_NODISCARD std::string GetMeshIdentifier() const override;
        SR_NODISCARD const SR_MATH_NS::Matrix4x4& GetModelMatrix() const override;

        SR_NODISCARD SR_UTILS_NS::StringAtom GetMeshLayer() const override {
            const static SR_UTILS_NS::StringAtom debugLayer = "Debug";
            return debugLayer;
        }

        bool Calculate() override;

        void UseMaterial() override;

    private:
        SR_MATH_NS::Matrix4x4 m_modelMatrix = SR_MATH_NS::Matrix4x4::Identity();
        SR_MATH_NS::FVector4 m_color;

        bool m_isConvex = false;

    };
}

#endif //GAMEENGINE_DEBUGWIREFRAMEMESH_H
