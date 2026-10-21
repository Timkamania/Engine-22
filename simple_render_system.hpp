#pragma once

#include "lve_device.hpp"
#include "lve_pipeline.hpp"
#include "lve_model.hpp"
#include <memory>
#include "lve_camera.hpp"
#include "transform_component.hpp"
#include "lve_frame_info.hpp"
namespace lve {

 

    class SimpleRenderSystem {
    public:
        SimpleRenderSystem(LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~SimpleRenderSystem();

        SimpleRenderSystem(const SimpleRenderSystem&) = delete;
        SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

        
        void renderModel(FrameInfo& frameInfo, const TransformComponent& cubeTransform, LveModel* model);

       // VkDescriptorSetLayout getGlobalDescriptorSetLayout() const { return globalDescriptorSetLayout; }

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);
       // VkDescriptorSetLayout globalDescriptorSetLayout;
        LveDevice& lveDevice;
        std::unique_ptr<LvePipeline> lvePipeline;
        VkPipelineLayout pipelineLayout;
    };
}