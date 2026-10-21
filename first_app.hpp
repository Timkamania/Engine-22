
#if 0
#pragma once

#include "lve_device.hpp"
#include "lve_pipeline.hpp"
#include "lve_swap_chain.hpp"
#include "Window.h"



namespace lve {



	class FirstApp {
	public:

		FirstApp();
		~FirstApp();

		FirstApp(const FirstApp&) = delete;
		FirstApp& operator=(const FirstApp&) = delete;


		void recreateSwapChain();

	private:
		std::unique_ptr<LveSwapChain> lveSwapChain;
		std::unique_ptr<LvePipeline> lvePipeline;
		
		void drawFrame();
		

		void  recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

	};
}
#endif