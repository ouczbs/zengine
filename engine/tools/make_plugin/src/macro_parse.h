#include <memory_resource>
namespace pmr {
    using std::pmr::monotonic_buffer_resource;
    using std::pmr::vector;
    using std::pmr::string;
}
inline pmr::monotonic_buffer_resource pool;
inline const char* PUBLIC_MODULE_DEPENDENCY = "PUBLIC_MODULE_DEPENDENCY";
inline const char* PRIVATE_MODULE_DEPENDENCY = "PRIVATE_MODULE_DEPENDENCY";
inline const char* IMPLEMENT_DYNAMIC_MODULE = "IMPLEMENT_DYNAMIC_MODULE";
inline const char* EXPORT_MODULE = "EXPORT_MODULE";
struct MacroData {
    const char* macro{ nullptr };
    pmr::vector<pmr::string> args;
    MacroData() :args(&pool) {}
    MacroData(const pmr::vector<pmr::string>& args) :args(args) {}
    operator bool() const { return macro; }
};
struct ParseData {
    pmr::string  moduleFile;
    pmr::string  exportText;
    pmr::vector<MacroData> mdList;
};
void readMacroFile(const char* file_path, ParseData& pd);