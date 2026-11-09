#include <avm/config.h>
#include <avm/tv.h>
#include <wups.h>
#include <wups/config/WUPSConfigCategory.h>
#include <wups/config/WUPSConfigItemBoolean.h>
#include <wups/config/WUPSConfigItemMultipleValues.h>
#include <wups/config/WUPSConfigItemStub.h>
#include <wups/config_api.h>
#include <sysapp/launch.h>
#include <coreinit/title.h>

#include <optional>
#include <span>
#include <string>

WUPS_PLUGIN_NAME("Quick Resolution");
WUPS_PLUGIN_DESCRIPTION("Quickly change Wii U display resolution");
WUPS_PLUGIN_VERSION("v1.2.0");
WUPS_PLUGIN_AUTHOR("Super64");
WUPS_PLUGIN_LICENSE("GPL-3.0");

namespace
{
constexpr AVMTvResolution kDefaultResolution = AVM_TV_RESOLUTION_720P;
AVMTvResolution gCurrentResolution = kDefaultResolution;
AVMTvResolution gPendingResolution = kDefaultResolution;
bool gApplyOnMenuClose = false;
uint64_t gRelaunchTitleId = 0;

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

// Apply the live TV mode, then persist the setting and use the original relaunch.
bool SetResolution(uint32_t value)
{
    if (!IsSupportedResolution(value))
    {
        return false;
    }

    const auto resolution = static_cast<AVMTvResolution>(value);
    if (AVMSetTVScanResolution(resolution) != 0)
    {
        return false;
    }

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

void OnApplyAndRelaunchToggled(ConfigItemBoolean* item, bool value)
{
    (void)item;
    if (!value)
    {
        return;
    }

    // Stage the request only. Let Aroma close and save its configuration
    // before changing the live output or requesting a title relaunch.
    gRelaunchTitleId = OSGetTitleID();
    gApplyOnMenuClose = true;
}

WUPSConfigAPICallbackStatus ConfigMenuOpenedCallback(WUPSConfigCategoryHandle rootHandle)
{
    WUPSConfigCategory root(rootHandle);
    try
    {
        root.add(WUPSConfigItemStub::Create("WARNING: Save game first"));
        root.add(WUPSConfigItemStub::Create("After Apply, close Aroma menu to restart"));

        root.add(WUPSConfigItemMultipleValues::CreateFromValue(
            std::optional<const std::string>("resolution"),
            "Display Resolution (staged until Apply)",
            static_cast<uint32_t>(kDefaultResolution),
            static_cast<uint32_t>(gPendingResolution),
            std::span<const WUPSConfigItemMultipleValues::ValuePair>(kResolutionOptions),
            OnResolutionChanged));

        root.add(WUPSConfigItemBoolean::Create(
            std::optional<const std::string>("apply_relaunch"),
            "Apply Resolution & Restart Game",
            false,
            false,
            OnApplyAndRelaunchToggled));
    }
    catch (...)
    {
        return WUPSCONFIG_API_CALLBACK_RESULT_ERROR;
    }
    return WUPSCONFIG_API_CALLBACK_RESULT_SUCCESS;
}

void ConfigMenuClosedCallback()
{
    if (!gApplyOnMenuClose)
    {
        return;
    }

    // One-shot: never repeat the launch on a subsequent menu close.
    gApplyOnMenuClose = false;
    const uint64_t titleId = gRelaunchTitleId;
    gRelaunchTitleId = 0;

    if (!SetResolution(static_cast<uint32_t>(gPendingResolution)))
    {
        return;
    }

    if (titleId != 0)
    {
        SYSLaunchTitle(titleId);
    }
    else
    {
        SYSLaunchMenu();
    }
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
