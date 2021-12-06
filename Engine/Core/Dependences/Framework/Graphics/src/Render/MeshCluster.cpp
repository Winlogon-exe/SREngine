//
// Created by Monika on 31.10.2021.
//

#include <Render/MeshCluster.h>
#include <Types/Geometry/IndexedMesh.h>
#include <Debug.h>

bool Framework::Graphics::ShadedMeshSubCluster::Remove(Framework::Graphics::Types::Mesh *mesh)  {
    auto* vertex = dynamic_cast<Types::VertexMesh *>(mesh);

    int32_t groupID = vertex ? vertex->GetVBO<false>() : SR_ID_INVALID;
    if (groupID == SR_ID_INVALID) {
        Helper::Debug::Error("ShadedMeshSubCluster::Remove() : failed get mesh group id to remove mesh!");
        return false;
    }

    if (auto find = m_groups.find(groupID); find != m_groups.end()) {
        auto &meshes = find->second;
        for (uint32_t i = 0; i < m_counters[groupID]; ++i) {
            if (meshes[i] == mesh) {
                --m_counters[groupID];
                --m_total;

                meshes.erase(meshes.begin() + i);
                mesh->RemoveUsePoint();

                if (m_counters[groupID] == 0) {
                    m_groups.erase(groupID);
                    m_counters.erase(groupID);
                }
                return true;
            }
        }
    }
    else {
        Helper::Debug::Error("ShadedMeshSubCluster::Remove() : mesh group to remove mesh not found!");
        return false;
    }

    Helper::Debug::Error("ShadedMeshSubCluster::Remove() : mesh not found!");

    return false;
}

bool Framework::Graphics::ShadedMeshSubCluster::Add(Framework::Graphics::Types::Mesh *mesh)  {
    auto* indexed = dynamic_cast<Types::IndexedMesh *>(mesh);

    int32_t groupID = indexed ? indexed->GetVBO<false>() : SR_ID_INVALID;
    if (groupID == SR_ID_INVALID) {
        Helper::Debug::Error("ShadedMeshSubCluster::Add() : failed get mesh group id to remove mesh!");
        return false;
    }

    if (auto find = this->m_groups.find(groupID); find == m_groups.end())
        m_groups[groupID] = { indexed };
    else
        find->second.push_back(indexed);

    ++m_counters[groupID];
    ++m_total;

    return true;
}

bool Framework::Graphics::ShadedMeshSubCluster::Empty() const {
    return m_total == 0;
}

bool Framework::Graphics::MeshCluster::Add(Framework::Graphics::Types::Mesh *mesh) {
    const auto&& shader = mesh->GetShader();
    if (auto&& subCluster = m_subClusters.find(shader); subCluster == m_subClusters.end()) {
        return (m_subClusters[shader] = ShadedMeshSubCluster()).Add(mesh);
    } else {
        return subCluster->second.Add(mesh);
    }
}

bool Framework::Graphics::MeshCluster::Remove(Framework::Graphics::Types::Mesh *mesh) {
    const auto&& shader = mesh->GetShader();
    if (auto&& subCluster = m_subClusters.find(shader); subCluster == m_subClusters.end()) {
        return false;
    } else {
        auto const result = subCluster->second.Remove(mesh);

        if (subCluster->second.Empty())
            m_subClusters.erase(subCluster);

        return result;
    }
}
