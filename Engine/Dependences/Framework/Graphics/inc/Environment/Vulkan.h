//
// Created by Nikita on 25.03.2021.
//

#ifndef GAMEENGINE_VULKAN_H
#define GAMEENGINE_VULKAN_H

#include <easy/profiler.h>
#include <Environment/Environment.h>

#define GLFW_EXPOSE_NATIVE_WIN32

#include <glm/glm.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <Debug.h>

#define VK_PROTOTYPES

#ifdef WIN32
    #define VK_USE_PLATFORM_WIN32_KHR
    #include <Environment/Win32Window.h>
#endif

#include <vulkan/vulkan.h>

#include <Environment/Vulkan/VulkanTools.h>

#include <Environment/Vulkan/VulkanShader.h>

constexpr double CIRCLE_RAD		= M_PI * 2;
constexpr double CIRCLE_THIRD	= CIRCLE_RAD / 3.0;
constexpr double CIRCLE_THIRD_1	= 0;
constexpr double CIRCLE_THIRD_2	= CIRCLE_THIRD;
constexpr double CIRCLE_THIRD_3	= CIRCLE_THIRD * 2;



namespace Framework::Graphics {
    VKAPI_ATTR VkBool32 VKAPI_CALL DebugVulkanCallback(
            VkDebugReportFlagsEXT flags,
            VkDebugReportObjectTypeEXT objType,
            uint64_t obj,
            size_t location,
            int32_t code,
            const char* layerPrefix,
            const char* msg,
            void* userData);

    class Vulkan : public Environment {
    public:
        Vulkan(Vulkan &) = delete;
    public:
        Vulkan() = default;
        ~Vulkan() = default;
    private:
        inline static const unsigned __int32 g_maxCountVBOs        = 5000;
        inline static VkBuffer*              g_VertexBuffers       = nullptr;
        inline static VkDeviceMemory*        g_VertexBuffersMemory = nullptr;

        std::vector<const char*> m_extRequired = {
                VK_KHR_SURFACE_EXTENSION_NAME,
                VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
                VK_EXT_DEBUG_REPORT_EXTENSION_NAME
        };
        std::vector<const char*> m_validLayersRequired = {
            "VK_LAYER_LUNARG_standard_validation"
        };

        VkSemaphore                       m_renderCompleteSemaphore     = VK_NULL_HANDLE;
        VkCommandBuffer                   m_currentCommandBuffer        = VK_NULL_HANDLE;

        VkInstance                        m_vkInstance                  = VK_NULL_HANDLE;
        VkDebugReportCallbackEXT          m_validationReportCallBack    = VK_NULL_HANDLE;
        VkSurfaceKHR                      m_vkSurface                   = VK_NULL_HANDLE;
        //!VkRenderPass                      m_vkRenderPass                = VK_NULL_HANDLE;

        //!std::vector<VkFramebuffer>        m_vkFrameBuffers              = {};

        VulkanTools::DepthStencil         m_depthStencil                = {};
        VulkanTools::Swapchain            m_swapchain                   = {};
        VulkanTools::Device               m_device                      = {};

        std::vector<VkSemaphore>          m_waitSemaphores              = {};
        VkResult                          m_presentResult               = VkResult::VK_RESULT_MAX_ENUM;
    private:
        bool ReCreateAfterResize();
        bool CreateSwapchain();

        /*![[nodiscard]] SR_FORCE_INLINE VkExtent2D GetVulkanSurfaceSize() const noexcept {
            return { m_basicWindow->GetWidth(), m_basicWindow->GetHeight() };
        }
        [[nodiscard]] SR_FORCE_INLINE VkFramebuffer GetVulkanActiveFramebuffer() const noexcept {
            return m_vkFrameBuffers[m_swapchain.m_activeSwapchainImageID];
        }*/
    public:
        [[nodiscard]] SR_FORCE_INLINE std::string GetPipeLineName() const noexcept override { return "Vulkan"; }
        [[nodiscard]] SR_FORCE_INLINE PipeLine GetPipeLine() const noexcept override { return PipeLine::Vulkan; }
    public:
        bool PreInit(unsigned int smooth_samples, const std::string& appName, const std::string& engineName) override;
        bool Init(int swapInterval) override;

        bool MakeWindow(const char* winName, bool fullScreen, bool resizable) override;
        bool CloseWindow() override;
        bool SetContextCurrent() override {
            return true;
        }

        [[nodiscard]] glm::vec2 GetWindowSize() const noexcept override {
            return { this->m_basicWindow->GetWidth(), this->m_basicWindow->GetHeight() };
        }

        [[nodiscard]] SR_FORCE_INLINE bool IsWindowOpen() const noexcept override {
             return m_basicWindow->IsWindowOpen();
        }

        SR_FORCE_INLINE void SwapBuffers() const noexcept override  {
            this->m_basicWindow->SwapBuffers();
        }

        SR_FORCE_INLINE void BeginRender() override {
            SR_CHECK_ERROR(vkAcquireNextImageKHR(
                    m_device.m_logicalDevice,
                    m_swapchain.m_vkSwapchainKhr,
                    UINT64_MAX,
                    VK_NULL_HANDLE,
                    m_swapchain.m_swapchainImageAvailable,
                    &m_swapchain.m_activeSwapchainImageID), VK_SUCCESS, "Vulkan::BeginRender() : failed to acquire next image!");

            //SR_CHECK_ERROR(
            //        vkWaitForFences(m_device.m_logicalDevice, 1, & m_swapchain.m_swapchainImageAvailable, VK_TRUE, UINT64_MAX),
            //        VK_SUCCESS,
            //        "Vulkan::BeginRender() : failed to wait for fences!");

            //SR_CHECK_ERROR(
            //        vkResetFences(m_device.m_logicalDevice, 1, & m_swapchain.m_swapchainImageAvailable),
            //        VK_SUCCESS,
            //        "Vulkan::BeginRender() : failed to reset fences!");

            #ifdef SR_RELEASE
                    vkQueueWaitIdle(m_device.m_queue.m_hQueue);
            #else
                    switch (vkQueueWaitIdle(m_device.m_queue.m_hQueue)) {
                            case VK_SUCCESS:
                                break;
                            case VK_ERROR_OUT_OF_HOST_MEMORY:
                                Helper::Debug::Error("Vulkan::BeginRender() : failed to queue wait idle! Out of host memory.");
                                break;
                            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                                Helper::Debug::Error("Vulkan::BeginRender() : failed to queue wait idle! Out of device memory.");
                                break;
                            case VK_ERROR_DEVICE_LOST:
                                Helper::Debug::Error("Vulkan::BeginRender() : failed to queue wait idle! Device lost.");
                                break;
                            default:
                                break;
                        }
            #endif

            {
                // Record command buffer
                VkCommandBufferBeginInfo commandBufferBeginInfo = {};
                commandBufferBeginInfo.sType				    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                commandBufferBeginInfo.flags				    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

                m_currentCommandBuffer = m_swapchain.m_commandBuffers[0];

                vkBeginCommandBuffer(m_currentCommandBuffer, &commandBufferBeginInfo);
            }
        }
        SR_FORCE_INLINE void EndRender() override
        {
            {
                vkEndCommandBuffer(m_currentCommandBuffer);

                // Submit command buffer
                VkSubmitInfo submitInfo             = {};
                submitInfo.sType					= VK_STRUCTURE_TYPE_SUBMIT_INFO;
                submitInfo.waitSemaphoreCount		= 0;
                submitInfo.pWaitSemaphores			= nullptr;
                submitInfo.pWaitDstStageMask		= nullptr;
                submitInfo.commandBufferCount		= 1;
                submitInfo.pCommandBuffers			= &m_currentCommandBuffer;
                submitInfo.signalSemaphoreCount	    = 1;
                submitInfo.pSignalSemaphores		= &m_renderCompleteSemaphore;

                vkQueueSubmit(m_device.m_queue.m_hQueue, 1, &submitInfo, VK_NULL_HANDLE);
            }

            VkPresentInfoKHR presentInfoKhr   = {};
            presentInfoKhr.sType			  = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            presentInfoKhr.waitSemaphoreCount = m_waitSemaphores.size();
            presentInfoKhr.pWaitSemaphores	  = m_waitSemaphores.data();
            presentInfoKhr.swapchainCount	  = 1;
            presentInfoKhr.pSwapchains		  = &m_swapchain.m_vkSwapchainKhr;
            presentInfoKhr.pImageIndices	  = &m_swapchain.m_activeSwapchainImageID;

            switch (vkQueuePresentKHR(m_device.m_queue.m_hQueue, &presentInfoKhr)) {
                case VK_SUCCESS: break;
                case VK_ERROR_OUT_OF_HOST_MEMORY:
                    Helper::Debug::Error("Vulkan::EndRender() : out of host memory!");
                    this->m_hasErrors = true;
                    break;
                case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                    Helper::Debug::Error("Vulkan::EndRender() : out of device memory!");
                    this->m_hasErrors = true;
                    break;
                case VK_ERROR_DEVICE_LOST:
                    Helper::Debug::Error("Vulkan::EndRender() : device lost!");
                    this->m_hasErrors = true;
                    break;
                case VK_ERROR_OUT_OF_DATE_KHR:
                    Helper::Debug::System("Vulkan::EndRender() : out of date KHR! Re-create vulkan...");
                    if (m_basicWindow->IsWindowOpen()) {
                        if (!this->ReCreateAfterResize()){
                            Helper::Debug::Error("Vulkan::EndRender() : failed to re-create vulkan after window resizing!");
                            this->m_hasErrors = true;
                        }
                    }
                    break;
                case VK_ERROR_SURFACE_LOST_KHR:
                    Helper::Debug::Error("Vulkan::EndRender() : surface lost KHR!");
                    this->m_hasErrors = true;
                    break;
                case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
                    Helper::Debug::Error("Vulkan::EndRender() : ful screen exclusive mode lost KHR!");
                    this->m_hasErrors = true;
                    break;
                default:
                    Helper::Debug::Error("Vulkan::EndRender() : an exception has been occurred!");
                    this->m_hasErrors = true;
                    break;
            };
        }

        SR_FORCE_INLINE void ClearColorBuffers(float r, float g, float b, float a) const noexcept override {
            static VkClearColorValue color = { r, g, b };

            VkImageSubresourceRange imageSubresourceRange = {};
            imageSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageSubresourceRange.baseMipLevel = 0;
            imageSubresourceRange.levelCount = 1;
            imageSubresourceRange.baseArrayLayer = 0;
            imageSubresourceRange.layerCount = 1;

            vkCmdClearColorImage(
                    m_currentCommandBuffer,
                    m_swapchain.m_swapchainImages[m_swapchain.m_activeSwapchainImageID],
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    &color, 1, &imageSubresourceRange);
        }

        SR_FORCE_INLINE void PollEvents() const noexcept override {
            this->m_basicWindow->PollEvents();
        }

        void SetWindowPosition(int x, int y) override;
        void SetWindowSize(unsigned int w, unsigned int h) override;

        //!===============================================[SHADERS]=====================================================

        [[nodiscard]] IShaderProgram* AllocShaderProgram() const noexcept override {
            return (VulkanShader*)malloc(sizeof(VulkanShader));
        }
        SR_FORCE_INLINE void DeleteShader(IShaderProgram* shaderProgram) const noexcept override {
            VulkanTools::DestroyShaderModule(m_device, &((VulkanShader*)shaderProgram)->m_vertShaderModule);
            VulkanTools::DestroyShaderModule(m_device, &((VulkanShader*)shaderProgram)->m_fragShaderModule);

            if (((VulkanShader*)shaderProgram)->m_shaderStages)
                free(((VulkanShader*)shaderProgram)->m_shaderStages);

            ((VulkanShader*)shaderProgram)->m_countStages = 0;
        }
        void FreeShaderProgram(IShaderProgram* shaderProgram) const noexcept override {
            if (shaderProgram != nullptr)
                free((VulkanShader*)shaderProgram);
        }
        bool CompileShader(const std::string& name, IShaderProgram* shaderProgram) const noexcept override;
        bool LinkShader(IShaderProgram* shaderProgram) const noexcept override { return true; };

        //!===============================================[SHADERS]=====================================================

        //!================================================[MESH]=======================================================

        SR_FORCE_INLINE bool CalculateMesh(unsigned int& VBO, unsigned int& VAO, std::vector<Vertex>& vertices, size_t countVerts) const noexcept {
            // find free cell in VBO array
            for (unsigned __int32 i = 0; i < g_maxCountVBOs; i++)
                if (g_VertexBuffers[i] == VK_NULL_HANDLE) {
                    VkBufferCreateInfo bufferInfo = {};
                    bufferInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                    bufferInfo.size               = sizeof(vertices[0]) * vertices.size();
                    bufferInfo.usage              = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
                    bufferInfo.sharingMode        = VK_SHARING_MODE_EXCLUSIVE;

                    if (vkCreateBuffer(m_device.m_logicalDevice, &bufferInfo, nullptr, &g_VertexBuffers[i]) != VK_SUCCESS) {
                        Helper::Debug::Error("Vulkan::CalculateMesh() : failed to create vertex buffer!");
                        return false;
                    } // vkDestroyBuffer(device, vertexBuffer, nullptr);

                    VkMemoryRequirements memRequirements;
                    vkGetBufferMemoryRequirements(m_device.m_logicalDevice, g_VertexBuffers[i], &memRequirements);

                    VkMemoryAllocateInfo allocInfo{};
                    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                    allocInfo.allocationSize = memRequirements.size;
                    allocInfo.memoryTypeIndex = VulkanTools::FindMemoryType(
                            m_device.m_physicalDevice,
                            memRequirements.memoryTypeBits,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

                    if (vkAllocateMemory(m_device.m_logicalDevice, &allocInfo, nullptr, &g_VertexBuffersMemory[i]) != VK_SUCCESS) {
                        Helper::Debug::Error("Vulkan::CalculateMesh() : failed to allocate vertex buffer memory!");
                        return false;
                    } //  vkFreeMemory(device, vertexBufferMemory, nullptr);

                    vkBindBufferMemory(m_device.m_logicalDevice, g_VertexBuffers[i], g_VertexBuffersMemory[i], 0);

                    void* data;
                    vkMapMemory(m_device.m_logicalDevice, g_VertexBuffersMemory[i], 0, bufferInfo.size, 0, &data);
                    memcpy(data, vertices.data(), (size_t) bufferInfo.size);
                    vkUnmapMemory(m_device.m_logicalDevice, g_VertexBuffersMemory[i]);

                    { // Set VAO and VBO index
                        VAO = i;
                        VBO = i;
                    }

                    return true;
                }

            Helper::Debug::Error("Vulkan::CalculateMesh() : failed to find free cell in VBO array!");
            return false;
        }

        SR_FORCE_INLINE void DrawTriangles(const unsigned int& VAO, const unsigned int& count_vertices) const noexcept override {
            //std::cout << VAO << std::endl;
            //std::cout << g_VertexBuffers[VAO] << std::endl;

            VkBuffer vertexBuffers[] = { g_VertexBuffers[VAO] };
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(m_currentCommandBuffer, 0, 1, vertexBuffers, offsets);

            vkCmdDraw(m_currentCommandBuffer, count_vertices, 1, 0, 0);
        }

        //!================================================[MESH]=======================================================
    };
}

#endif //GAMEENGINE_VULKAN_H