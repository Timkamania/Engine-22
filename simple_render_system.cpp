#include "simple_render_system.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>

namespace lve {

    SimpleRenderSystem::SimpleRenderSystem(LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : lveDevice{ device } {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
        

    }

    SimpleRenderSystem::~SimpleRenderSystem() {
        if (pipelineLayout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
        }
      
    }

    void SimpleRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {


     
        // 2. Обновляем создание PipelineLayout, передавая туда наш Layout дескриптора
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1; // ТЕПЕРЬ У НАС 1 LAYOUT
        pipelineLayoutInfo.pSetLayouts = &globalSetLayout; // Передаем его
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void SimpleRenderSystem::createPipeline(VkRenderPass renderPass) {
        assert(pipelineLayout != VK_NULL_HANDLE && "Нельзя создать конвейер до PipelineLayout!");

        PipelineConfigInfo pipelineConfig{};
        LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout; // Передаем лейаут

        // КАНОНИЧНОЕ ЗАПОЛНЕНИЕ: Система рендеринга связывает разметку вершин LveModel с конфигом пайплайна
        pipelineConfig.bindingDescriptions = LveModel::Vertex::getBindingDescriptions();
        pipelineConfig.attributeDescriptions = LveModel::Vertex::getAttributeDescriptions();
        lvePipeline = std::make_unique<LvePipeline>(lveDevice, pipelineConfig);
    }

    void SimpleRenderSystem::renderModel(FrameInfo& frameInfo, const TransformComponent& cubeTransform, LveModel* model) {
        vkCmdBindPipeline(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, lvePipeline->getGraphicsPipeline());

    

        // ДОБАВЛЕНО ИЗ УРОКА 20: Привязываем Uniform-буфер к конвейеру!
        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            0, 1,
            &frameInfo.globalDescriptorSet,
            0, nullptr
        );

       

        // Математика ручного разделения сдвига и вращения для вазы
        glm::mat4 modelMatrix = cubeTransform.mat4();


     

        SimplePushConstantData push{};
        push.modelMatrix = modelMatrix;
        push.normalMatrix = glm::mat4(cubeTransform.normalMatrix());

        vkCmdPushConstants(
            frameInfo.commandBuffer,
            pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(SimplePushConstantData),
            &push
        );

        if (model != nullptr) {
            model->bind(frameInfo.commandBuffer);
            model->draw(frameInfo.commandBuffer);
        }
    }
}   
