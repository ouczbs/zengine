#include "vkn/thread/command_worker.h"
#include "vkn/wrapper/device.h"
namespace vkn {
	void CommandWorker::InvokeBuffer(const commandFn& fn, const Element& callback)
	{
		Invoke([=, this]() {
			CommandBuffer cmd = mCommandPool.Pop();
			Buffer(cmd, fn, callback);
			mCommandPool.Push(cmd);
		});
	}
	void CommandWorker::Buffer(CommandBuffer& cmd, const commandFn& fn, const Element& callback)
	{
		cmd.BeginRecord(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		fn(cmd);
		cmd.EndRecord();
		VkFence fence = mDevice.PopFence();
		cmd.Submit(mQueue.Ptr(), fence);
		mDevice.PushWaitFence(fence);
		callback();
	}

	void CommandWorker::Flush()
	{
		SyncInvoke([]{});
	}
	bool CommandWorker::Present(VkPresentInfoKHR& presentInfo)
	{
		VkResult result = vkQueuePresentKHR(mQueue.Ptr(), &presentInfo);
		return result == VK_SUCCESS;
	}
	void CommandWorker::SyncInvoke(const Element& fn)
	{
		Invoke([=, this]() {
			fn();
			mSemaphore.release();
			});
		mSemaphore.acquire();
	}
	void CommandWorker::Loop()
	{
		while (true) {
			Element elem = mChannel.acquire();
			elem();
		}
	}
}
