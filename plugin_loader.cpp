
#include "plugin_loader.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <system_error>

namespace fs = std::filesystem;

namespace {

    std::string windowsError(DWORD code) {
        return "Windows error code: " + std::to_string(code);
    }

    void closeLibrary(void* handle) {
        if (handle) {
            FreeLibrary(reinterpret_cast<HMODULE>(handle));
        }
    }

} // namespace

PluginLoader::~PluginLoader() {
    functions_.clear();

    for (auto it = plugins_.rbegin();
        it != plugins_.rend(); ++it) {
        closeLibrary(it->handle);
    }
}

void PluginLoader::loadFromDirectory(
    const fs::path& directory
) {
    std::error_code ec;

    if (!fs::is_directory(directory, ec) || ec) {
        std::cerr << "Warning: plugin directory not found: "
            << directory.string() << '\n';
        return;
    }

    fs::directory_iterator it(directory, ec);
    fs::directory_iterator end;

    if (ec) {
        std::cerr << "Cannot scan plugin directory: "
            << ec.message() << '\n';
        return;
    }

    for (; it != end; it.increment(ec)) {
        if (ec) {
            std::cerr << "Directory scan error: "
                << ec.message() << '\n';
            ec.clear();
            continue;
        }

        const fs::directory_entry& entry = *it;

        std::error_code fileError;

        if (!entry.is_regular_file(fileError) || fileError) {
            continue;
        }

        std::wstring extension = entry.path().extension().wstring();

        if (extension != L".dll" && extension != L".DLL") {
            continue;
        }

        SetLastError(ERROR_SUCCESS);

        HMODULE module = LoadLibraryW(entry.path().c_str());

        if (!module) {
            DWORD errorCode = GetLastError();

            std::cerr << "Cannot load "
                << entry.path().filename().string()
                << ": " << windowsError(errorCode) << '\n';
            continue;
        }

        auto getInfo = reinterpret_cast<GetPluginInfo>(
            GetProcAddress(module, "get_plugin_info")
            );

        if (!getInfo) {
            std::cerr << "Invalid plugin: "
                << entry.path().filename().string()
                << " (missing get_plugin_info)\n";

            FreeLibrary(module);
            continue;
        }

        bool registered = false;

        try {
            const PluginInfo* info = getInfo();

            if (!info || !info->name ||
                !info->function || info->name[0] == '\0') {
                std::cerr << "Invalid plugin metadata: "
                    << entry.path().filename().string()
                    << '\n';

                FreeLibrary(module);
                continue;
            }

            std::string name(info->name);

            if (functions_.count(name)) {
                std::cerr << "Duplicate function name: "
                    << name << ". Plugin skipped.\n";

                FreeLibrary(module);
                continue;
            }

            // Сначала сохраняем дескриптор библиотеки.
            // Если выделение памяти завершится исключением,
            // библиотека будет закрыта в обработчике ниже.
            plugins_.push_back({
                reinterpret_cast<void*>(module),
                name,
                info->function
                });

            try {
                auto result = functions_.emplace(
                    name, info->function
                );

                if (!result.second) {
                    plugins_.pop_back();
                    FreeLibrary(module);
                    continue;
                }

                registered = true;
            }
            catch (...) {
                plugins_.pop_back();
                throw;
            }

            std::cout << "Loaded function: "
                << name << '\n';

        }
        catch (const std::exception& e) {
            if (!registered) {
                // Если дескриптор уже записан в plugins_,
                // его удалили из вектора в блоке выше.
                FreeLibrary(module);
            }

            std::cerr << "Plugin initialization failed: "
                << e.what() << '\n';

        }
        catch (...) {
            if (!registered) {
                FreeLibrary(module);
            }

            std::cerr << "Plugin initialization failed "
                "with an unknown exception.\n";
        }
    }
}

double PluginLoader::call(
    const std::string& name,
    double argument
) const {
    auto it = functions_.find(name);

    if (it == functions_.end()) {
        throw std::runtime_error(
            "Unknown function: " + name
        );
    }

    // Исключения математических функций обрабатываются
    // в main.cpp.
    return it->second(argument);
}

bool PluginLoader::contains(
    const std::string& name
) const {
    return functions_.count(name) != 0;
}

std::vector<std::string> PluginLoader::names() const {
    std::vector<std::string> result;
    result.reserve(functions_.size());

    for (const auto& item : functions_) {
        result.push_back(item.first);
    }

    std::sort(result.begin(), result.end());
    return result;
}
