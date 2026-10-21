// Engine.cpp : Определяет точку входа для приложения.
//

#include "framework.h"
#include "Engine.h"
#include "lve_device.hpp"
#include "lve_swap_chain.hpp"
#include "lve_pipeline.hpp"
#include "lve_model.hpp"
#include "Window.h"
#include "lve_renderer.hpp"
#include "simple_render_system.hpp"
#include "lve_descriptors.hpp"
#include <chrono>
#include "keyboard_movement_controller.hpp"
#include "transform_component.hpp"

#include <glm/gtc/matrix_transform.hpp>
HWND hwnd;
HINSTANCE hInst; //= NULL;



int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow) {
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
  
    /*
    struct PointLight {
        glm::vec4 position; // Превратили в vec4, w-компонент пока просто 1.0f
        glm::vec4 color;    // xyz - цвет, w - интенсивность
    };
  
  */
    struct GlobalUbo {
        glm::mat4 projectionView{ 1.f };           // offset 0,  size 64
        glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, 0.02f }; // offset 64, size 16
        glm::vec4 lightPosition{ -0.5f, -1.0f, 0.0f, 1.0f }; // offset 80, size 16
        glm::vec4 lightColor{ 1.f, 1.f, 1.f, 1.0f };         // offset 96, size 16
    };

    // Контроль выравнивания std140
    static_assert(offsetof(GlobalUbo, projectionView) == 0, "UBO layout mismatch");
    static_assert(offsetof(GlobalUbo, ambientLightColor) == 64, "UBO layout mismatch");
    static_assert(offsetof(GlobalUbo, lightPosition) == 80, "UBO layout mismatch");
    static_assert(offsetof(GlobalUbo, lightColor) == 96, "UBO layout mismatch");
    static_assert(sizeof(GlobalUbo) == 112, "UBO size mismatch");

    Window w(hInstance);


    AllocConsole();
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    FILE* fp;
    if (freopen_s(&fp, "CONOUT$", "w", stdout) != 0)
    {
        MessageBoxA(NULL, "Не удалось перенаправить stdout в консоль", "Ошибка", MB_OK | MB_ICONERROR);
    }

    int x = std::max(1, 2);

    std::cout << "MAAAAAAX::" << x;
   

    lve::LveDevice lveDevice{ w.hwnd, w.hInst };

    lve::LveRenderer lveRenderer{ w, lveDevice };


    // КАНОНИЧНОЕ СОЗДАНИЕ LAYOUT ЧЕРЕЗ БИЛДЕР
    std::unique_ptr<lve::LveDescriptorSetLayout> globalSetLayout =
        lve::LveDescriptorSetLayout::Builder(lveDevice)
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
        .build();


    // СТАЛО: Загрузка модели из внешнего файла .obj!
    std::string modelPath = "models/smooth_vase.obj"; // Укажите путь к файлу модели
    auto lveModel = lve::LveModel::createModelFromFile(lveDevice, modelPath);
   
   // ДОБАВЛЕНО: Загрузка модели плоского пола
        std::string floorModelPath = "models/quad.obj";
    auto lveFloorModel = lve::LveModel::createModelFromFile(lveDevice, floorModelPath);

    lve::SimpleRenderSystem simpleRenderSystem{
       lveDevice,
       lveRenderer.getSwapChainRenderPass(),
       globalSetLayout->getDescriptorSetLayout()
    };
   

    lve::LveCamera camera{};

    camera.setPerspectiveProjection(glm::radians(50.f), 16.0f / 9.0f, 0.1f, 10.f);
    // Камера (Viewer) — управляет полетом на WASD
    lve::TransformComponent cameraTransform{};
    cameraTransform.translation = { 0.f, 0.f, -2.5f }; // Отодвинута назад, чтобы видеть центр
    // Куб — будет управляться стрелочками клавиатуры
    lve::TransformComponent cubeTransform{};
    cubeTransform.translation = { 0.f, 0.f, 0.0f }; // Наш куб в центре мира


    // ДОБАВЛЕНО: Настройка положения пола под вазой
    lve::TransformComponent floorTransform{};
    // Опускаем пол чуть ниже центра вазы (по оси Y вниз)
    floorTransform.translation = { 0.f, 0.5f, 0.0f };
    // Растягиваем квадрат (например, 3 на 3 метра), чтобы получилась большая комната
    floorTransform.scale = { 3.f, 1.f, 3.f };

    lve::KeyboardMovementController inputController{};
    auto currentTime = std::chrono::high_resolution_clock::now();


    // КАНОНИЧНОЕ СОЗДАНИЕ ПУЛА ДЕСКРИПТОРОВ ЧЕРЕЗ БИЛДЕР
    std::unique_ptr<lve::LveDescriptorPool> globalPool =
        lve::LveDescriptorPool::Builder(lveDevice)
        .setMaxSets(lve::LveSwapChain::MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, lve::LveSwapChain::MAX_FRAMES_IN_FLIGHT)
        .build();




    // Создание и маппинг буферов UBO
    std::vector<std::unique_ptr<lve::LveBuffer>> uboBuffers(lve::LveSwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < uboBuffers.size(); i++) {
        uboBuffers[i] = std::make_unique<lve::LveBuffer>(
            lveDevice,
            sizeof(GlobalUbo),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            64
        );
        uboBuffers[i]->map();
    }



    // КАНОНИЧНАЯ СБОРКА И ЗАПИСЬ ДЕСКРИПТОРОВ ЧЕРЕЗ WRITER
    std::vector<VkDescriptorSet> globalDescriptorSets(lve::LveSwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < globalDescriptorSets.size(); i++) {
        auto bufferInfo = uboBuffers[i]->descriptorInfo();
        lve::LveDescriptorWriter(*globalSetLayout, *globalPool)
            .writeBuffer(0, &bufferInfo)
            .build(globalDescriptorSets[i]);
    }

        return w.Run([&]() {




            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            if (frameTime > 0.1f) frameTime = 0.1f;

            inputController.moveCamera(w.hwnd, frameTime, cameraTransform);
            inputController.rotateCube(w.hwnd, frameTime, cubeTransform);
            camera.setViewYXZ(cameraTransform.translation, cameraTransform.rotation);




            if (auto commandBuffer = lveRenderer.beginFrame()) {
                int frameIndex = lveRenderer.getFrameIndex(); // Получаем текущий индекс кадра (0 или 1)

                // Сборка FrameInfo с дескриптором внутри кадра
                lve::FrameInfo frameInfo{
                    frameIndex,
                    frameTime,
                    commandBuffer,
                    camera,
                    globalDescriptorSets[frameIndex] // ПЕРЕДАЕМ СЮДА
                };



                GlobalUbo ubo{};
                ubo.projectionView = camera.getProjection() * camera.getView();
                ubo.ambientLightColor = glm::vec4{ 1.f, 1.f, 1.f, 0.02f };

 
                ubo.lightPosition = glm::vec4{ 1.0f, -1.0f, -1.0f, 1.0f };
                ubo.lightColor = glm::vec4{ 1.f, 1.f, 1.f, 3.0f };

                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush(); // Сбрасываем изменения на видеокарту

                // Шаг В. Отрендеринг через компактный вызов систем
                lveRenderer.beginSwapChainRenderPass(commandBuffer);


                // Компактный вызов отрисовки без лишних аргументов
                simpleRenderSystem.renderModel(frameInfo, cubeTransform, lveModel.get());

                // ДОБАВЛЕНО: Отрисовка объекта пола под вазой
                simpleRenderSystem.renderModel(frameInfo, floorTransform, lveFloorModel.get());

                lveRenderer.endSwapChainRenderPass(commandBuffer);
                lveRenderer.endFrame();
            }



            });
      

        
    

}


   




