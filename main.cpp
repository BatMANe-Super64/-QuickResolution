#include <wups.h>
#include <wups/config.h>
#include <wups/config/WUPSConfigItemMultipleValues.h>
#include <wups/config/WUPSConfigItemBoolean.h>

#include <config.h>   // SYSAppGetInteger / SYSAppSetInteger / SYSAppSave
#include <launch.h>   // SYSLaunchMenu
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
// Global plugin state
// -----------------------------
static int32_t currentResolution = 1;

// Resolution list mapping
static const std::vector<WUPSConfigItemMultipleValues::ValuePair> resolutionOptions = {
    {0, "480p"},
    {1, "720p"},
    {3, "1080p"}
};

// Config menu items declared as pointers to avoid initialization racing
static WUPSConfigItemMultipleValues* resolutionDropdown = nullptr;
static WUPSConfigItemBoolean* restartButton = nullptr;

// Set resolution and commit to active system config
void SetResolution(int value)
{
    currentResolution = value;
    SYSAppSetInteger("/config/system/display/resolution", value);
    SYSAppSave();
}

// Restart system menu application
void RestartMenu()
{
    SYSLaunchMenu();
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
// Plugin initialization
// -----------------------------
INITIALIZE_PLUGIN()
{
    // 1. Fetch the actual hardware resolution first
    SYSAppGetInteger("/config/system/display/resolution", &currentResolution);

    // 2. Find the correct matching index in our choices matrix
    uint32_t initialIndex = 1; // Default to 720p index
    for (size_t i = 0; i < resolutionOptions.size(); ++i) {
        if (resolutionOptions[i].value == currentResolution) {
            initialIndex = i;
            break;
        }
    }

    // 3. Instantiate the UI components inside runtime initialization scope
    resolutionDropdown = new WUPSConfigItemMultipleValues(WUPSConfigItemMultipleValues::CreateFromIndex(
        std::optional<std::string>("resolution"),
        "Display Resolution",
        initialIndex, 
        std::span<const WUPSConfigItemMultipleValues::ValuePair>(resolutionOptions.data(), resolutionOptions.size()),
        OnResolutionChanged
    ));

    restartButton = new WUPSConfigItemBoolean(WUPSConfigItemBoolean::Create(
        std::optional<std::string>("restart_menu"),
        "Restart System Menu (Apply)",
        false,
        OnRestartToggled
    ));
}

// Register configuration pointers to the active tracking matrix
WUPS_CONFIG_ITEMS_V2(
    resolutionDropdown,
    restartButton
);
