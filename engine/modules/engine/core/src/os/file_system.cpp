#include "os/file_system.h"
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <unistd.h>
#include <limits.h>
#elif __APPLE__
#include <mach-o/dyld.h>
#include <limits.h>
#endif
namespace fs {

    std::string _GetExecutablePath() {
#ifdef _WIN32
        char path[MAX_PATH];
        GetModuleFileName(NULL, path, MAX_PATH);
        return std::string(path);
#elif __linux__
        char result[PATH_MAX];
        ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
        return std::string(result, (count > 0) ? count : 0);
#elif __APPLE__
        char path[PATH_MAX];
        uint32_t size = sizeof(path);
        if (_NSGetExecutablePath(path, &size) == 0) {
            char realPath[PATH_MAX];
            realpath(path, realPath);
            return std::string(realPath);
        }
        else {
            return std::string("");
        }
#endif
    }
    std::string GetExecutablePath() {
        std::string path = _GetExecutablePath();
        size_t pos = path.find_last_of("\\/");
        return (std::string::npos == pos) ? "" : path.substr(0, pos);
    }
    std::string GetWorkPath() {
        auto path = std::filesystem::current_path();
        return path.string();
    }

    void EnsurePathExists(std::string_view path)
    {
        size_t pos = path.find_last_of("\\/");
        if (std::string::npos != pos){
            std::string_view dir = path.substr(0, pos);
            if (!std::filesystem::exists(dir)) {
                std::filesystem::create_directories(dir);  // 如果目录不存在，则创建它
            }
        }
    }

}