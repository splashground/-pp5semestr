
#pragma once

#ifdef _WIN32
#define CALC_PLUGIN_EXPORT __declspec(dllexport)
#define CALC_PLUGIN_CALL __cdecl
#else
#define CALC_PLUGIN_EXPORT
#define CALC_PLUGIN_CALL
#endif

#ifdef __cplusplus
extern "C" {
#endif

    typedef double (CALC_PLUGIN_CALL* MathFunction)(double);

    typedef struct PluginInfo {
        const char* name;
        MathFunction function;
    } PluginInfo;

    typedef const PluginInfo* (CALC_PLUGIN_CALL* GetPluginInfo)();

#ifdef __cplusplus
}
#endif
