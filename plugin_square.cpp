
#include "plugin_api.h"

#include <cmath>
#include <stdexcept>

static double square(double x) {
    if (!std::isfinite(x)) {
        throw std::domain_error(
            "square: argument must be finite"
        );
    }

    double result = x * x;

    if (!std::isfinite(result)) {
        throw std::overflow_error(
            "square: result is too large"
        );
    }

    return result;
}

extern "C" CALC_PLUGIN_EXPORT
const PluginInfo* CALC_PLUGIN_CALL get_plugin_info() {
    static const PluginInfo info = {
        "square",
        square
    };

    return &info;
}
