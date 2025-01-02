#pragma once
#include "os/package_path.h"
namespace api {
	using NativeLibHandle = void*;
	class SharedLibrary {
		NativeLibHandle mHandle;
	public:
		bool Load(PackagePath path = "");
		void* GetSymbol(const char* name);

		static string_view GetExtensionName();
	};
}