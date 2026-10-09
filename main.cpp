#include <wups.h>
#include <wups/config.h>
#include <wups/config/WUPSConfigItemMultipleValues.h>
#include <wups/config/WUPSConfigItemBoolean.h>

#include <coreinit/systeminfo.h> // Modern reliable fallback for system integers
#include <coreinit/launch.h>     // Modern reliable fallback for system launch
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
extern "C" {
    // Explicitly link against the core OS settings exports inside coreinit
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
// Plugin initialization & Configuration registration
// -----------------------------
WUPS_GET_CONFIG_ITEMS_V2(menuItems) {
    // 1. Fetch the actual hardware resolution first
    __SYSAppGetInteger("/config/system/display/resolution", &currentResolution);

    // 2. Find the correct matching index in our choices matrix
    uint32_t initialIndex = 1; // Default to 720p index
    for (size_t i = 0; i < resolutionOptions.size(); ++i) {
        if (resolutionOptions[i].value == currentResolution) {
            initialIndex = i;
            break;
        }
    }

    // 3. Populate and return elements dynamically via the matrix array
    // Fixed: Passed 6 arguments to match candidate 2 signature requirements
    menuItems.push_back(new WUPSConfigItemMultipleValues(WUPSConfigItemMultipleValues::CreateFromIndex(
        std::optional<std::string>("resolution"),
        "Display Resolution",
        initialIndex, // initial index
        initialIndex, // default index
        std::span<const WUPSConfigItemMultipleValues::ValuePair>(resolutionOptions.data(), resolutionOptions.size()),
        OnResolutionChanged
    )));

    // Fixed: Passed 5 arguments to match candidate 2 signature requirements
    menuItems.push_back(new WUPSConfigItemBoolean(WUPSConfigItemBoolean::Create(
        std::optional<std::string>("restart_menu"),
        "Restart System Menu (Apply)",
        false, // initial value
        false, // default value
        OnRestartToggled
    )));
}

INITIALIZE_PLUGIN()
{
    // Basic runtime registration point
}
