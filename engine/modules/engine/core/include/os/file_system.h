#include <string>
namespace fs {
	std::string GetExecutablePath();
	std::string GetWorkPath();
	void        EnsurePathExists(std::string_view path);
}