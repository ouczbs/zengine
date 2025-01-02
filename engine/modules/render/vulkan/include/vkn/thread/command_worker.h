#pragma once
#include "worker.h"
#include <semaphore>
namespace vkn {
	class Device;
	class Queue;
	class CommandWorker : public ThreadWorker<voidFn, CommandWorker> {
	protected:
		std::binary_semaphore mSemaphore{0};
	public:
		using ThreadWorker<voidFn, CommandWorker>::ThreadWorker;
		void InvokeBuffer(const commandFn& fn, const Element& callback);
		void Buffer(CommandBuffer& cmd, const commandFn& fn, const Element& callback);
		void Flush();
		bool Present(VkPresentInfoKHR& presentInfo);
		void Loop();
		void SyncInvoke(const Element& fn);
	};
};