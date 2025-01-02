#include "vkn/wrapper/queue.h"
namespace vkn {
	const Name Queue::TransferQueue = "transfer";
	const Name Queue::RenderQueue   = "render"  ;
	Queue::Queue(Name name, uint32_t queueFamilyIndex, VkQueue queue)
		: mName(name), mPtr(queue), mQueueFamilyIndex(queueFamilyIndex)
	{

	}
}