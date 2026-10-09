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
// Callbacks (Using the explicit required WUPS base types)
// -----------------------------
void OnResolutionChanged(ConfigItemMultipleValues* item, uint32_t newValue)
{
    (void)item; 
    SetResolution(newValue);
}

void OnRestartToggled(ConfigItemBoolean* item, bool value)
{
    (void)item; 
    if (value) {
        RestartMenu();
    }
}

// -----------------------------
// Native WUPS Configuration Item Callback Registration
// -----------------------------
WUPS_GET_CONFIG_ITEMS(menuItems) {
    // 1. Fetch current display settings
    __SYSAppGetInteger("/config/system/display/resolution", &currentResolution);

    // 2. Locate starting resolution index position
    int initialIndex = 1; 
    for (size_t i = 0; i < resolutionOptions.size(); ++i) {
        if (resolutionOptions[i].value == currentResolution) {
            initialIndex = i;
            break;
        }
    }

    // 3. Directly feed the items into the tracking array vector using version-compliant parameters
    menuItems.push_back(new WUPSConfigItemMultipleValues(WUPSConfigItemMultipleValues::CreateFromIndex(
        std::optional<const std::string>("resolution"),
        "Display Resolution",
        initialIndex, 
        initialIndex, 
        std::span<const WUPSConfigItemMultipleValues::ValuePair>(resolutionOptions.data(), resolutionOptions.size()),
        OnResolutionChanged
    )));

    menuItems.push_back(new WUPSConfigItemBoolean(WUPSConfigItemBoolean::Create(
        std::optional<const std::string>("restart_menu"),
        "Restart System Menu (Apply)",
        false, 
        false, 
        OnRestartToggled
    )));
}

// -----------------------------
// Plugin Lifecycle Hooks
// -----------------------------
INITIALIZE_PLUGIN()
{
    // Native framework initialization
}

DEINITIALIZE_PLUGIN()
{
    // Native framework cleanup
}
