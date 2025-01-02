#pragma once
#include "package_path.h"
#include <vector>
#include <fstream>
#include <variant>
namespace api {
	using std::vector;
	enum class FILE_OP
	{
		READ,
		WRITE,
		APPEND,
		NONE
	};
	class FileHandle : PackagePath{
	protected:
		uint32_t flag{0};
		FILE_OP op{ FILE_OP::NONE};
		size_t size{0};
		pmr::memory_resource* pool{nullptr};
		std::variant<std::ifstream, std::ofstream, nullptr_t> vfile{nullptr};
	public:
		using PackagePath::PackagePath;
		FileHandle(PackagePath path, pmr::memory_resource* pool = FramePool()) : PackagePath(path) , pool(pool) {}
		uint32_t Flag() {
			return flag;
		}
		operator bool() {
			return flag & File_Success;
		}
		std::ifstream& Reader() {
			return std::get<std::ifstream>(vfile);
		}
		std::ofstream& Writer() {
			return std::get<std::ofstream>(vfile);
		}
		FileHandle& Open(FILE_OP op, bool is_binarry = false);
		int Read(void* data, int size);
		template<typename T = pmr::vector<char>>
		T ReadAll() {
			Reader().seekg(0);
			return ReadLeft<T>();
		}
		template<typename T = pmr::vector<char>>
		T ReadLeft();
		bool FindNext(const char* begin, const char* end, size_t& stop);
		//这里出问题了,二进制的\r\n会被读取成\n,大小对不上
		template<typename T>
		T ReadUntil(const T& token);
		void Write(const char* data, size_t size){ Writer().write(data, size); }
		template<typename T = pmr::string>
		void Write(const T& data) {
			Write(data.data(), data.size());
		}
		template<typename T = pmr::string>
		void WriteLine(const T& data) {
			Write<T>(data);
			Write<T>(T{ "\n"});
		}
	};
}
#include "file_handle.inl"