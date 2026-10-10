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
AVMTvResolution gPendingResolution = kDefaultResolution;

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

// A zero return value from WUT's AVM API indicates success.
bool SetResolution(uint32_t value)
{
    if (!IsSupportedResolution(value))
    {
        return false;
    }

    const auto resolution = static_cast<AVMTvResolution>(value);
    if (AVMWriteSystemVideoResConfig(resolution) != 0)
    {
        return false;
    }

    gCurrentResolution = resolution;
    return true;
}

// Selecting a resolution only stages it. It does not write the system setting.
void OnResolutionChanged(ConfigItemMultipleValues* item, uint32_t newValue)
{
    (void)item;
    if (IsSupportedResolution(newValue))
    {
        gPendingResolution = static_cast<AVMTvResolution>(newValue);
    }
}

void OnApplyToggled(ConfigItemBoolean* item, bool value)
{
    (void)item;
    if (!value)
    {
        return;
    }

    // This is a one-shot action, not a saved preference. The item intentionally
    // has no identifier so the Apply choice is not persisted between sessions.
    if (SetResolution(static_cast<uint32_t>(gPendingResolution)))
    {
        // WUPS calls config item callbacks when the configuration menu closes.
        // Restarting the System Menu during gameplay can lose unsaved progress.
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
            "Display Resolution (staged until Apply)",
            static_cast<uint32_t>(kDefaultResolution),
            static_cast<uint32_t>(gPendingResolution),
            std::span<const WUPSConfigItemMultipleValues::ValuePair>(kResolutionOptions),
            OnResolutionChanged));

        root.add(WUPSConfigItemBoolean::CreateEx(
            std::nullopt,
            "Apply resolution (save game first; restarts Menu)",
            false,
            false,
            OnApplyToggled,
            "APPLY NOW",
            "Not applying"));
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
    gPendingResolution = gCurrentResolution;

    WUPSConfigAPIOptionsV1 configOptions = {.name = "QuickResolution"};
    (void)WUPSConfigAPI_Init(configOptions, ConfigMenuOpenedCallback, ConfigMenuClosedCallback);
}

DEINITIALIZE_PLUGIN()
{
}
