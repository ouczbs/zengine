#pragma once
#include "module/module.h"
namespace api {
	class UI_API UIModule : public IStaticModule
	{
	public:
		void OnLoad(int argc, char** argv) override;
		void OnUnload() override;
		void InitMetaData(void) override {};
		void Initialize()override;

		void SetThemeProviders();
	};
}