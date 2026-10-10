#include <avm/config.h>
#include <avm/tv.h>
#include <wups.h>
#include <wups/config/WUPSConfigCategory.h>
#include <wups/config/WUPSConfigItemBoolean.h>
#include <wups/config/WUPSConfigItemMultipleValues.h>
#include <wups/config_api.h>
#include <sysapp/launch.h>

#include <optional>
#include <span>
#include <string>

WUPS_PLUGIN_NAME("Quick Resolution");
WUPS_PLUGIN_DESCRIPTION("Quickly change Wii U display resolution");
WUPS_PLUGIN_VERSION("1.0");
WUPS_PLUGIN_AUTHOR("Super64");
WUPS_PLUGIN_LICENSE("GPL-3.0");

namespace
{
constexpr AVMTvResolution kDefaultResolution = AVM_TV_RESOLUTION_720P;
AVMTvResolution gCurrentResolution = kDefaultResolution;

constexpr WUPSConfigItemMultipleValues::ValuePair kResolutionOptions[] = {
    {AVM_TV_RESOLUTION_480P, "480p"},
    {AVM_TV_RESOLUTION_720P, "720p"},
    {AVM_TV_RESOLUTION_1080P, "1080p"},
};

bool IsSupportedResolution(uint32_t value)
{
    for (const auto& option : kResolutionOptions)
    {
        if (option.value == value)
        {
            return true;
        }
    }
    return false;
}

// WUT's AVM configuration API writes the system TV resolution setting.
// A zero return value indicates success.
void SetResolution(uint32_t value)
{
    if (!IsSupportedResolution(value))
    {
        return;
    }

    const auto resolution = static_cast<AVMTvResolution>(value);
    if (AVMWriteSystemVideoResConfig(resolution) == 0)
    {
        gCurrentResolution = resolution;
    }
}

void OnResolutionChanged(ConfigItemMultipleValues* item, uint32_t newValue)
{
    (void)item;
    SetResolution(newValue);
}

void OnRestartToggled(ConfigItemBoolean* item, bool value)
{
    (void)item;
    if (value)
    {
        SYSLaunchMenu();
    }
}

WUPSConfigAPICallbackStatus ConfigMenuOpenedCallback(WUPSConfigCategoryHandle rootHandle)
{
    WUPSConfigCategory root(rootHandle);
    try
    {
        root.add(WUPSConfigItemMultipleValues::CreateFromValue(
            std::optional<const std::string>("resolution"),
            "Display Resolution",
            static_cast<uint32_t>(kDefaultResolution),
            static_cast<uint32_t>(gCurrentResolution),
            std::span<const WUPSConfigItemMultipleValues::ValuePair>(kResolutionOptions),
            OnResolutionChanged));

        root.add(WUPSConfigItemBoolean::Create(
            std::optional<const std::string>("restart_menu"),
            "Restart System Menu (Apply)",
            false,
            false,
            OnRestartToggled));
    }
    catch (...)
    {
        return WUPSCONFIG_API_CALLBACK_RESULT_ERROR;
    }
    return WUPSCONFIG_API_CALLBACK_RESULT_SUCCESS;
}

void ConfigMenuClosedCallback()
{
}
} // namespace

INITIALIZE_PLUGIN()
{
    AVMTvResolution configuredResolution = kDefaultResolution;
    if (AVMReadSystemVideoResConfig(&configuredResolution) == 0 &&
        IsSupportedResolution(static_cast<uint32_t>(configuredResolution)))
    {
        gCurrentResolution = configuredResolution;
    }

    WUPSConfigAPIOptionsV1 configOptions = {.name = "QuickResolution"};
    (void)WUPSConfigAPI_Init(configOptions, ConfigMenuOpenedCallback, ConfigMenuClosedCallback);
}

DEINITIALIZE_PLUGIN()
{
}
