#pragma once
#include "module/module.h"
#include "data/engine_config.h"
namespace api{
	class APP_API AppModule : public IStaticModule
	{
	public:
		void OnLoad(int argc, char** argv) override;
		void OnUnload() override;
		void InitMetaData(void) override {};
	};
	template<typename T>
	class AppImpl;
	class App {
	public:
		template<typename T>
		void Run(EngineConfig config, AppImpl<T>& impl);
	};
}
#include "app.inl"