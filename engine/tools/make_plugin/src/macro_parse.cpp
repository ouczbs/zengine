#include "macro_parse.h"
#include <string>
#include <stack>
#include <optional>
#include <iostream>
#include <fstream>
std::string_view module_macro[] = { PUBLIC_MODULE_DEPENDENCY, PRIVATE_MODULE_DEPENDENCY , IMPLEMENT_DYNAMIC_MODULE, EXPORT_MODULE };
pmr::vector<pmr::string> parseArgs(std::string_view& str) {
    pmr::vector<pmr::string> args(&pool);
    std::stack<char> stack;
    enum EParseState {
        EEmpty,
        EBody,
    };
    EParseState state = EEmpty;
    int n = 0;
    char segment[1024];
    for (char c : str) {
        switch (state)
        {
        case EEmpty:
        {
            if (c == ' ' || c == '\t')
                break;
            state = EBody;
            if (c == '(' && stack.empty()) {
                stack.push(c);
                break;
            }
        }
        case EBody:
        {
            if (c == '(' || c == '{' || c == '[' || (c == '"' && stack.top() != '"')) {
                stack.push(c);
            }
            else {
                char t = stack.top();
                if ((t == '(' && c == ')') || (t == '[' && c == ']') || (t == '{' && c == '}') || (t == '"' && c == '"')) {
                    stack.pop();
                }
            }
            if (stack.empty()) {
                std::string_view view(segment, n);
                args.push_back(pmr::string(view, &pool));
                return args;
            }
            if (c == ',' && stack.size() == 1) {
                std::string_view view(segment, n);
                args.push_back(pmr::string(view, &pool));
                n = 0;
                state = EEmpty;
            }
            else {
                segment[n++] = c;
            }
            break;
        }
        default:
            break;
        }
    }
    return args;
}
std::optional<MacroData> parseLine(std::string_view line) {
    for (auto macro : module_macro) {
        size_t pos = line.find(macro);
        if (pos != std::string_view::npos) {
            line = line.substr(pos + macro.size());
            MacroData md{ parseArgs(line) };
            md.macro = macro.data();
            //std::cout << line << std::endl;
            return md;
        }
    }
    return std::optional<MacroData>{};
}
// 读取文件并返回每一行内容
void readMacroFile(const char* file_path, ParseData& pd) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        //std::cerr << "Failed to open file: " << file_path << std::endl;
        return;
    }
    std::string line;
    while (std::getline(file, line)) {
        std::string_view line_view(line);
        if (auto md = parseLine(line_view)) {
            const char* macro = md.value().macro;
            if (macro == EXPORT_MODULE) {
                std::string_view text = md.value().args[0];
                text = text.substr(3, text.size() - 5);//R(" +  )" =  3 + 2 = 5;
                pd.exportText = text;
            }
            else
                pd.mdList.push_back(md.value());
        }
    }
    file.close();
}
