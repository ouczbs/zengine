#include "vkn/backend.h"
#include "vkn/wrapper/device.h"
#include "vkn/wrapper/device_create.h"
#include "vkn/wrapper/instance.h"
#include "vkn/wrapper/instance_create.h"
#include "vkn/wrapper/queue.h"
#include "vkn/thread/buffer_worker.h"
#include "vkn/thread/command_worker.h"
#include "vkn/wrapper/descriptorpool.h"
namespace vkn {
	BufferWorker*  Backend::TransferWorker;
	CommandWorker* Backend::RenderWorker;
	template<typename Worker>
	inline Worker* Backend::InitWorker(Name name, VkCommandPoolCreateFlags flag)
	{
		auto queue = mDevice->GetQueue(name);
		return new Worker(name, *mDevice, *queue, flag);
	}
	Backend::Backend(string_view appName)
	{
		InstanceCreator instanceCreator{};
		mInstance = new (GlobalPool()) Instance(instanceCreator);

		DeviceCreator deviceCreator = DeviceCreator{ *mInstance };
		deviceCreator.AddWindowExtension();
		deviceCreator.AddQueue(Queue::RenderQueue,  VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT, 1.0);
		deviceCreator.AddQueue(Queue::TransferQueue, VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT, 1.0);
		mDevice = new (GlobalPool()) Device(deviceCreator);

		Backend::RenderWorker = InitWorker<CommandWorker>(Queue::RenderQueue, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		Backend::TransferWorker = InitWorker<BufferWorker>(Queue::TransferQueue, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		Backend::TransferWorker->InitVmaAllocator(mInstance->Ptr());

		auto poolSizes = DescriptorPool::DefaultDescriptorPoolSize();
		mPool = new DescriptorPool(*mDevice, poolSizes, 8192);
	}
	Backend::~Backend()
	{
		mInstance->~Instance();
		mDevice->~Device();
	}
}
