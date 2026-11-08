
#include "plugin_api.h"

#include <cmath>
#include <stdexcept>

static double sinDegrees(double x) {
    if (!std::isfinite(x)) {
        throw std::domain_error(
            "sin: argument must be finite"
        );
    }

    const double pi = std::acos(-1.0);
    return std::sin(x * pi / 180.0);
}

extern "C" CALC_PLUGIN_EXPORT
const PluginInfo* CALC_PLUGIN_CALL get_plugin_info() {
    static const PluginInfo info = {
        "sin",
        sinDegrees
    };

    return &info;
}
