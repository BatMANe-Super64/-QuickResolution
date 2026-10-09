#include <wups.h>
#include <wups/config.h>
#include <wups/config/WUPSConfigItemMultipleValues.h>
#include <wups/config/WUPSConfigItemBoolean.h>

#include <string>
#include <vector>
#include <optional>
#include <span>

WUPS_PLUGIN_NAME("Quick Resolution");
WUPS_PLUGIN_DESCRIPTION("Quickly change Wii U display resolution");
WUPS_PLUGIN_VERSION("1.0");
WUPS_PLUGIN_AUTHOR("Super64");
WUPS_PLUGIN_LICENSE("GPL");

// -----------------------------
// Global plugin state & CoreInit Linking
// -----------------------------
extern "C" {
    int32_t __SYSAppGetInteger(const char* path, int32_t* value);
    int32_t __SYSAppSetInteger(const char* path, int32_t value);
    int32_t __SYSAppSave(void);
    void __SYSLaunchMenu(void);
}

static int32_t currentResolution = 1;

// Resolution list mapping
static const std::vector<WUPSConfigItemMultipleValues::ValuePair> resolutionOptions = {
    {0, "480p"},
    {1, "720p"},
    {3, "1080p"}
};

// Set resolution and commit to active system config
void SetResolution(int value)
{
    currentResolution = value;
    __SYSAppSetInteger("/config/system/display/resolution", value);
    __SYSAppSave();
}

// Restart system menu application
void RestartMenu()
{
    __SYSLaunchMenu();
}

// -----------------------------
// Callbacks
// -----------------------------
void OnResolutionChanged(WUPSConfigItemMultipleValues* item, int32_t newValue)
{
    (void)item; 
    SetResolution(newValue);
}

void OnRestartToggled(WUPSConfigItemBoolean* item, bool value)
{
    (void)item; 
    if (value) {
        RestartMenu();
    }
}

// -----------------------------
// Standard WUPS Configuration System Callback
// -----------------------------
static WUPSConfigCategory openMenuCallback(void) {
    // 1. Fetch current display settings
    __SYSAppGetInteger("/config/system/display/resolution", &currentResolution);

    // 2. Locate starting resolution index position
    uint32_t initialIndex = 1; 
    for (size_t i = 0; i < resolutionOptions.size(); ++i) {
        if (resolutionOptions[i].value == currentResolution) {
            initialIndex = i;
            break;
        }
    }

    // 3. Construct category tree manually (Universal fallback compatibility)
    WUPSConfigCategory category = WUPSConfigCategory::Create("Quick Resolution Settings");

    category.Add(WUPSConfigItemMultipleValues::CreateFromIndex(
        std::optional<std::string>("resolution"),
        "Display Resolution",
        initialIndex, 
        initialIndex, 
        std::span<const WUPSConfigItemMultipleValues::ValuePair>(resolutionOptions.data(), resolutionOptions.size()),
        OnResolutionChanged
    ));

    category.Add(WUPSConfigItemBoolean::Create(
        std::optional<std::string>("restart_menu"),
        "Restart System Menu (Apply)",
        false, 
        false, 
        OnRestartToggled
    ));

    return category;
}

// -----------------------------
// Plugin Lifecycle Hooks
// -----------------------------
INITIALIZE_PLUGIN()
{
    // Register the layout generation callback directly into the runtime core
    WUPS_RegisterConfigCallback(openMenuCallback);
}

DEINITIALIZE_PLUGIN()
{
    // Runtime cleanup hook
}
