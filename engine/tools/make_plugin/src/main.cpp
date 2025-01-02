#include "macro_parse.h"
#include "archive/pch.h"
#include "module/module.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
std::string_view readFile(const char* file_path) {
    std::ifstream file(file_path, std::ios::ate);
    if (!file.is_open()) {
        return "";
    }
    size_t size = file.tellg();
    file.seekg(0);
    char* ptr = new(FramePool())char[size];
    file.read(ptr, size);
    return std::string_view(ptr, size);
}
void writeFile(const char* file_path, std::string_view data) {
    std::ofstream file(file_path, 0);
    file.write(data.data(), data.size());
    file.close();
}
void genLua(const char* file_path, const pmr::vector<MacroData>& mdList) {
    std::ostringstream oss;
    oss << "{\n";
    for (auto& md : mdList) {
        if (md.macro == PUBLIC_MODULE_DEPENDENCY || md.macro == PRIVATE_MODULE_DEPENDENCY) {
            oss << "\t{";
            for (auto& args : md.args) {
                oss << '"' << args << "\", ";
            }
            oss << (md.macro == PUBLIC_MODULE_DEPENDENCY ? "{public = true}},\n" : "{public = false}},\n");
        }
    }
    oss << '}';
    writeFile(file_path, oss.str());
}
void genPlugin(const char* file_path, ParseData& pd) {
    readMacroFile(file_path, pd);
    api::ModuleInfo info;
    std::string_view text = pd.exportText;
    api::TextDeserialize<api::ModuleInfo>(text, &info);
    info.dependencies.clear();
    for (auto& md : pd.mdList) {
        if (md.macro == IMPLEMENT_DYNAMIC_MODULE) {
            info.name = md.args[1];
        }
        else if (md.macro == PUBLIC_MODULE_DEPENDENCY) {
            for (auto& args : md.args) {
                std::pmr::string upname{ args};
                std::transform(upname.begin(), upname.end(), upname.begin(), std::toupper);
                info.dependencies.push_back({args, upname + "_VERSION", upname + "_KIND"});
            }
        }
    }
    std::ostringstream oss;
    std::string name = info.name.ToString();
    std::transform(name.begin(), name.end(), name.begin(), std::toupper);
    oss << "#include \"" << pd.moduleFile << "\"\n";
    oss << "#define EXPORT_MODULE(json)" << name << "_API const char* __module_meta_" << info.name.data() << "__ = json\n";
    oss << "EXPORT_MODULE(R\"(" << api::TextSerialize(info) << ")\");\n";
    writeFile(file_path, oss.str());
}
//已废弃
int main(int argc, char* argv[]) {
    if (argc < 100) {
        return -1;
    }
    std::string project_dir = argv[1];
    std::string lua_file = project_dir + "\\" + argv[2];
    std::string script_dir = argv[3];
    std::string module_file = script_dir + "\\" + argv[4];
    std::string plugin_file = script_dir + "\\" + argv[5];
    ParseData pd;
    pd.moduleFile = argv[4];
	readMacroFile(module_file.c_str(), pd);
    genLua(lua_file.c_str(), pd.mdList);
    genPlugin(plugin_file.c_str(), pd);
	return 0;
}