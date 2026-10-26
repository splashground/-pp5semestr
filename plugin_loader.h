
#pragma once

#include "plugin_api.h"

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

class PluginLoader {
public:
    PluginLoader() = default;
    ~PluginLoader();

    PluginLoader(const PluginLoader&) = delete;
    PluginLoader& operator=(const PluginLoader&) = delete;

    void loadFromDirectory(
        const std::filesystem::path& directory
    );

    double call(
        const std::string& name,
        double argument
    ) const;

    bool contains(const std::string& name) const;
    std::vector<std::string> names() const;

private:
    struct LoadedPlugin {
        void* handle;
        std::string name;
        MathFunction function;
    };

    std::vector<LoadedPlugin> plugins_;
    std::unordered_map<std::string, MathFunction> functions_;
};
