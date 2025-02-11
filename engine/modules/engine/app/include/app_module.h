#pragma once
#include "module/module.h"
namespace api{
	class APP_API AppModule : public IStaticModule
	{
	public:
		void OnLoad(int argc, char** argv) override;
		void OnUnload() override;
		void InitMetaData(void) override {};
	};
}