//
// Created by Nikita on 12.06.2021.
//

#ifndef GAMEENGINE_VULKANPOSTPROCESSING_H
#define GAMEENGINE_VULKANPOSTPROCESSING_H

#include <Render/PostProcessing.h>

namespace Framework::Graphics {
    class VulkanPostProcessing : public PostProcessing {
    private:
        ~VulkanPostProcessing() override = default;
    public:
        VulkanPostProcessing(Camera *camera) : PostProcessing(camera) {}
    public:
        bool BeginGeometry() override {
            if (m_frameBuffer >= 0) {
                m_env->BindFrameBuffer(m_frameBuffer);
                m_env->ClearBuffers();
                return true;
            } else
                return false;
        }
        void EndGeometry() override {
            m_env->BindFrameBuffer(0);
        }

        void BeginSkybox() override {}
        void EndSkybox() override {}

        void Complete() override { }
    public:
        bool Free() override {
            Helper::Debug::Graph("VulkanPostProcessing::Free() : free post processing pointer...");
            delete this;
            return true;
        }

        bool Destroy() override {
            return PostProcessing::Destroy();
        }
    };
}

#endif //GAMEENGINE_VULKANPOSTPROCESSING_H
