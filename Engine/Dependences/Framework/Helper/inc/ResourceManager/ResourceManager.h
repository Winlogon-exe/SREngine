//
// Created by Nikita on 16.11.2020.
//

#ifndef HELPER_RESOURCEMANAGER_H
#define HELPER_RESOURCEMANAGER_H

#include <string>
#include <functional>
#include <mutex>
#include <thread>

#include "IResource.h"
#include <Utils/Singleton.h>

#include <Types/Map.h>

#include <vector>

using namespace Framework::Helper::Types;

namespace Framework::Helper {
    enum class PathType {
        Resources, Other, Mods
    };

    class ResourceManager : public Helper::Singleton<ResourceManager> {
        friend class Singleton<ResourceManager>;
    private:
        ~ResourceManager() override = default;
    private:
        std::vector<IResource *> m_resourcesToDestroy = std::vector<IResource *>();
        std::atomic<uint64_t> m_countResourcesToDestroy = 0;

        Map<std::string, std::vector<IResource *>> m_resources = Map<std::string, std::vector<IResource *>>();
    private:
        std::string m_resourcesFolder = "UnSelected";
        bool m_isInit = false;
        std::recursive_mutex m_mutex = std::recursive_mutex();
        std::thread m_thread = std::thread();
    private:
        /** \brief remove resource from resource manager */
        void Remove(IResource *resource);

        /** \brief Grabble collector */
        void GC();
    public:
        void Synchronize();
        IResource* Find(const std::string& Name, const std::string& ID);

        bool RegisterType(const std::string& type_name);

        /** \warning Call only from IResource parents \brief Register resource in resource manager */
        void RegisterResource(IResource *resource);

        /** \warning Call only from IResource parents \brief Register resource to destroy in resource manager */
        bool Destroy(IResource *resource);
    public:
        /** \brief Init resource manager */
        bool Init(const std::string& resourcesFolder);

        /** \brief Stop resource manager */
        bool Stop();

        /** \brief Get current application memory usage of bytes */
        static uint64_t GetUsedMemoryLoad() noexcept;

        std::string GetResourcesFolder() noexcept { return m_resourcesFolder; }
    public:
        void PrintMemoryDump();
    };
}

#endif //HELPER_RESOURCEMANAGER_H
