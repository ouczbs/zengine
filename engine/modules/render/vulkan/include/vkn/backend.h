#pragma once
#include "vkn/type.h"
#include "wrapper/descriptorpool.h"
namespace vkn {
	class Device;
	class Instance;
	class Backend {
	protected:
		Instance* mInstance;
		Device* mDevice;
		DescriptorPool* mPool;
	public:
		Backend(string_view appName);
		~Backend();
		template<typename Worker>
		Worker* InitWorker(Name name, VkCommandPoolCreateFlags flag);
		Instance& GetInstance() {
			return *mInstance;
		}
		Device& GetDevice() {
			return *mDevice;
		}
		DescriptorPool& GetPool() {
			return *mPool;
		}
	public:
		static struct BufferWorker*  TransferWorker;
		static struct CommandWorker* RenderWorker;
	};
};