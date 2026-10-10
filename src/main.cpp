#include <wups.h>
#include <wups/config/WUPSConfigCategory.h>
#include <wups/config/WUPSConfigItemBoolean.h>
#include <wups/config/WUPSConfigItemMultipleValues.h>
#include <wups/config_api.h>

#include <optional>
#include <span>
#include <string>

#include <sysapp/launch.h>

WUPS_PLUGIN_NAME("Quick Resolution");
WUPS_PLUGIN_DESCRIPTION("Quickly change Wii U display resolution");
WUPS_PLUGIN_VERSION("1.0");
WUPS_PLUGIN_AUTHOR("Super64");
WUPS_PLUGIN_LICENSE("GPL-3.0");

namespace
{
constexpr char kResolutionPath[] = "/config/system/display/resolution";
constexpr int32_t kDefaultResolution = 1;

int32_t gCurrentResolution = kDefaultResolution;

constexpr WUPSConfigItemMultipleValues::ValuePair kResolutionOptions[] = {
    {0, "480p"},
    {1, "720p"},
    {3, "1080p"},
};

extern "C"
{
int32_t SYSAppGetInteger(const char* path, int32_t* value);
int32_t SYSAppSetInteger(const char* path, int32_t value);
int32_t SYSAppSave(void);
}

bool IsSupportedResolution(int32_t value)
{
    for (const auto& option : kResolutionOptions)
    {
        if (static_cast<int32_t>(option.value) == value)
        {
            return true;
        }
    }
    return false;
}

void SetResolution(int32_t value)
{
    if (!IsSupportedResolution(value))
    {
        return;
    }

    if (SYSAppSetInteger(kResolutionPath, value) == 0 &&
        SYSAppSave() == 0)
    {
        gCurrentResolution = value;
    }
}

void OnResolutionChanged(ConfigItemMultipleValues* item, uint32_t newValue)
{
    (void)item;
    SetResolution(static_cast<int32_t>(newValue));
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
            kDefaultResolution,
            gCurrentResolution,
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
    int32_t configuredResolution = kDefaultResolution;
    if (SYSAppGetInteger(kResolutionPath, &configuredResolution) == 0 &&
        IsSupportedResolution(configuredResolution))
    {
        gCurrentResolution = configuredResolution;
    }

    WUPSConfigAPIOptionsV1 configOptions = {.name = "QuickResolution"};
    (void)WUPSConfigAPI_Init(configOptions, ConfigMenuOpenedCallback, ConfigMenuClosedCallback);
}

DEINITIALIZE_PLUGIN()
{
}
