#pragma once
#include <string>
#include "pmr/name.h"
namespace api
{
	using std::string_view;
	using pmr::Name;
	enum FileFlag : uint32_t {
		File_Default = 0,
		File_Success = 1 << 0,
		File_Binary = 1 << 1,
		File_Compress = 1 << 2,
		File_Http = 1 << 3,
		File_Not_Exist = 1 << 4,
		File_Error = 1 << 5,
	};
	//占位符浪费了四个字节
	struct FileBlock {
		uint32_t flag{ 0 };
		Name path;
		Name addr;
		operator bool() {
			return !(flag & FileFlag::File_Not_Exist);
		}
	};
	struct PackagePath {
		string_view path;
		PackagePath() {};
		PackagePath(const char* path) : path(path) {}
		PackagePath(string_view path) : path(path) {}
		PackagePath(const std::string& path) : path(path) {}
		PackagePath(const pmr::string& path) : path(path) {}
		PackagePath(pmr::Name name) : path(name.ToStringView()) {}
		operator bool() {
			return path != "";
		}
		string_view operator()() const {
			return path;
		}
		size_t size() const {
			return path.size();
		}
		pmr::string operator+(const char* suffix) const {
			return pmr::string(path) + suffix;
		}
		string_view GetPackage()const {
			string_view name;
			if (path[0] == '/') {
				size_t pos = path.find('/', 1);
				if (pos != std::string::npos) {
					return path.substr(1, pos - 1);
				}
			}
			return name;
		}
		string_view GetFileName()const {
			size_t pos = path.rfind('/');
			if (pos != std::string::npos)
				return path.substr(pos + 1);
			return "";
		}
		Name GetExtension() const {
			size_t pos = path.rfind('.');
			if (pos != std::string::npos)
				return path.substr(pos);
			return "";
		}
		pmr::string RealPath()const;
		static pmr::string RealPath(const char* str);
	};
}