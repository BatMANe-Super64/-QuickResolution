#include <wups.h>
#include <wups/config.h>
#include <wups/config/WUPSConfigItemMultipleValues.h>

#include <cstdint>
#include <map>
#include <string>

WUPS_PLUGIN_NAME("Quick Resolution");
WUPS_PLUGIN_DESCRIPTION("Quickly change Wii U display resolution");
WUPS_PLUGIN_VERSION("1.0");
WUPS_PLUGIN_AUTHOR("Super64");
WUPS_PLUGIN_LICENSE("GPL");

namespace {
    // These are proposed values from the original code.
    // They are NOT yet verified as valid system resolution values.
    constexpr int32_t RES_480P  = 0;
    constexpr int32_t RES_720P  = 1;
    constexpr int32_t RES_1080P = 3;

    int32_t selectedResolution = RES_720P;

    void OnResolutionChanged(
        WUPSConfigItemMultipleValues* item,
        int32_t newValue
    ) {
        (void)item;

        switch (newValue) {
            case RES_480P:
            case RES_720P:
            case RES_1080P:
                selectedResolution = newValue;
                // System-setting changes will be added after
                // verifying the correct Wii U API.
                break;

            default:
                break;
        }
    }
}

WUPS_GET_CONFIG() {
    auto* config = new WUPSConfig("Quick Resolution");
    auto* category = config->addCategory("Display");

    std::map<int32_t, std::string> resolutions;
    resolutions[RES_480P]  = "480p";
    resolutions[RES_720P]  = "720p";
    resolutions[RES_1080P] = "1080p";

    category->addItem(
        new WUPSConfigItemMultipleValues(
            "resolution",
            "Display Resolution",
            selectedResolution,
            resolutions,
            &OnResolutionChanged
        )
    );

    return config;
}

INITIALIZE_PLUGIN() {
    // Initialize plugin state here.
}

DEINITIALIZE_PLUGIN() {
    // Release plugin resources here.
}
