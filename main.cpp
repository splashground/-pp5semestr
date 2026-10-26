
#include "parser.h"
#include "plugin_loader.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static fs::path getExecutableDirectory() {
    std::vector<wchar_t> buffer(1024);

    while (true) {
        DWORD size = GetModuleFileNameW(
            nullptr,
            buffer.data(),
            static_cast<DWORD>(buffer.size())
        );

        if (size == 0) {
            throw std::runtime_error(
                "Cannot determine executable path"
            );
        }

        if (size < buffer.size()) {
            return fs::path(
                std::wstring(buffer.data(), size)
            ).parent_path();
        }

        buffer.resize(buffer.size() * 2);
    }
}

int main() {
    try {
        fs::path executableDir = getExecutableDirectory();
        fs::path pluginDirectory = executableDir / "plugins";

        PluginLoader plugins;

        std::cout << "Plugin directory: "
            << pluginDirectory.string() << "\n\n";

        plugins.loadFromDirectory(pluginDirectory);

        std::cout << "\nAvailable plugin functions:\n";

        for (const auto& name : plugins.names()) {
            std::cout << "  " << name << '\n';
        }

        Parser parser(plugins);

        std::cout << "\nCalculator started.\n";
        std::cout << "Enter an expression or 'exit' to quit.\n\n";

        std::string line;

        while (true) {
            std::cout << "> ";

            if (!std::getline(std::cin, line)) {
                break;
            }

            if (line == "exit" || line == "quit") {
                break;
            }

            if (line.empty()) {
                continue;
            }

            try {
                double result = parser.evaluate(line);
                std::cout.precision(15);
                std::cout << "= " << result << "\n";
            }
            catch (const std::exception& e) {
                std::cout << "Error: " << e.what() << "\n";
            }
            catch (...) {
                std::cout << "Error: unknown exception\n";
            }
        }

        std::cout << "Calculator finished.\n";

    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << '\n';
        return 1;
    }
    catch (...) {
        std::cerr << "Fatal error: unknown exception\n";
        return 1;
    }

    return 0;
}
