#include "os/shared_library.h"
#include <Windows.h>
namespace api {
	bool SharedLibrary::Load(PackagePath path)
	{
        if (path)
        {
            auto rpath = path.RealPath();
            mHandle = GetModuleHandle(rpath.c_str());
            if (mHandle == NULL)
            {
                mHandle = LoadLibrary(rpath.c_str());
            }
        }
        else
        {
            mHandle = GetModuleHandle(nullptr);
        }
        return mHandle;
	}
    void* SharedLibrary::GetSymbol(const char* name)
    {
        void* addr = (void*)GetProcAddress((HMODULE)mHandle, name);
        return addr;
    }
    string_view SharedLibrary::GetExtensionName()
    {
#ifdef _WIN32
        return string_view(".dll");
#elif __linux__
        return string_view(".so");
#endif
    }
}

