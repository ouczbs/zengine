#include "os/file_handle.h"
#include <algorithm>
#define READ_DATA_SIZE 100
#define TOKEN_SIZE 100
namespace api {
	FileHandle& FileHandle::Open(FILE_OP _op, bool is_binarry)
	{
		if (!pool) {
			pool = FramePool();
		}
		op = _op;
		pmr::string file_path = RealPath();
		switch (op)
		{
		case api::FILE_OP::READ:
		{
			vfile = std::ifstream(file_path.c_str(), is_binarry ? std::ios::binary | std::ios::ate : std::ios::ate);
			auto& fi = Reader();
			if (!fi.is_open()) {
				return *this;
			}
			size = fi.tellg();
			fi.seekg(0);
			break;
		}
		case api::FILE_OP::WRITE:
			vfile = std::ofstream(file_path.c_str(), is_binarry ? std::ios::binary : 0);
			break;
		case api::FILE_OP::APPEND:
			vfile = std::ofstream(file_path.c_str(), is_binarry ? std::ios::app | std::ios::binary : std::ios::app);
			break;
		default:
			break;
		}
		flag = FileFlag::File_Success | (is_binarry ? FileFlag::File_Binary : 0);
		return *this;
	}
	int FileHandle::Read(void* data, int size)
	{
		auto& fi = Reader();
		if (!fi) {
			return -1;
		}
		fi.read((char*)data, size);
		return fi.gcount();
	}
	bool FileHandle::FindNext(const char* begin, const char* end, size_t& stop)
	{
		auto& fi = Reader();
		size_t start = fi.tellg();
		size_t cur = start;
		char chunk[READ_DATA_SIZE + TOKEN_SIZE];
		const char* last = chunk + READ_DATA_SIZE + TOKEN_SIZE;
		while (cur < stop) {
			fi.read(chunk + TOKEN_SIZE, READ_DATA_SIZE);
			size_t count = fi.gcount();
			if (count < READ_DATA_SIZE || cur + count > stop) {
				fi.clear();
			}
			const char* first = cur == start ? chunk + TOKEN_SIZE : chunk;
			const char* it = std::search(first, last, begin, end);
			if (it != last) {
				fi.seekg(start);
				stop = cur + (it - first) - TOKEN_SIZE;
				return true;
			}
			if (count < READ_DATA_SIZE) {
				fi.seekg(start);
				stop = cur + count;
				return false;
			}
			cur += count;
			memcpy(chunk, chunk + READ_DATA_SIZE, TOKEN_SIZE);
		}
		fi.seekg(start);
		return false;
	}
}
