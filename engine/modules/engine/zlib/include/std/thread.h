#pragma once
#include "channel.h"
#include <thread>
namespace zstd{
    template<typename value_type, typename Worker>
	class ThreadWorker {
	public:
		using Element = value_type;
		std::thread mThread;
		channel<Element> mChannel;
		void WorkLoop() {
			mThread.detach();
			Loop();
		}
		void Loop() {
			Worker* worker = (Worker*)(this);
			worker->Loop();
		}
	public:
		ThreadWorker() : ThreadWorker(64) {}
		ThreadWorker(int buffer) : mChannel(buffer)
		{
			mThread = std::thread(&ThreadWorker::WorkLoop, this);
		}

		void Invoke(const Element& elem) {
			mChannel.release(elem);
		}
	};
}