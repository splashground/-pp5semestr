
#include "plugin_api.h"

#include <cmath>
#include <stdexcept>

static double naturalLogarithm(double x) {
    if (x <= 0.0 || !std::isfinite(x)) {
        throw std::domain_error(
            "ln: argument must be positive and finite"
        );
    }

    return std::log(x);
}

extern "C" CALC_PLUGIN_EXPORT
const PluginInfo* CALC_PLUGIN_CALL get_plugin_info() {
    static const PluginInfo info = {
        "ln",
        naturalLogarithm
    };

    return &info;
}
