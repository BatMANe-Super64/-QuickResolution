#include <wups.h>
#include <string.h>

WUPS_PLUGIN_NAME("Quick Resolution");
WUPS_PLUGIN_DESCRIPTION("Quickly change Wii U display resolution");
WUPS_PLUGIN_VERSION("1.0");
WUPS_PLUGIN_AUTHOR("Super64");
WUPS_PLUGIN_LICENSE("GPL");

// -----------------------------
// CoreInit System Functions Linking
// -----------------------------
extern "C" {
    int32_t __SYSAppGetInteger(const char* path, int32_t* value);
    int32_t __SYSAppSetInteger(const char* path, int32_t value);
    int32_t __SYSAppSave(void);
    void __SYSLaunchMenu(void);
}

static int32_t currentResolution = 1;

// Resolution text layout choices matching choice index positions
static const char* resolutionOptions[] = {
    "480p",
    "720p",
    "1080p"
};

// Set resolution and commit to active system config
void SetResolution(int value)
{
    // Map list index choices back to hardware values (0=480p, 1=720p, 3=1080p)
    int hardwareValue = 1;
    if (value == 0) hardwareValue = 0;
    else if (value == 1) hardwareValue = 1;
    else if (value == 2) hardwareValue = 3;

    currentResolution = hardwareValue;
    __SYSAppSetInteger("/config/system/display/resolution", hardwareValue);
    __SYSAppSave();
}

// Restart system menu application
void RestartMenu()
{
    __SYSLaunchMenu();
}

// -----------------------------
// Classic WUPS Menu Callbacks
// -----------------------------
static void OnResolutionChanged(WUPSConfigHandle configHandle, const char* categoryId, const char* itemId, WUPSConfigValue newValue, void* user_data) {
    (void)configHandle; (void)categoryId; (void)itemId; (void)user_data;
    SetResolution(newValue.integer);
}

static void OnRestartToggled(WUPSConfigHandle configHandle, const char* categoryId, const char* itemId, WUPSConfigValue newValue, void* user_data) {
    (void)configHandle; (void)categoryId; (void)itemId; (void)user_data;
    if (newValue.boolean) {
        RestartMenu();
    }
}

// This runs automatically whenever a user opens your plugin submenu configuration screen
static void OnMenuOpened(WUPSConfigHandle configHandle, void* user_data) {
    (void)user_data;

    // 1. Add Category
    WUPSConfig_AddCategory(configHandle, "resolution_cat", "Quick Resolution Settings");

    // 2. Fetch active hardware setting positions
    __SYSAppGetInteger("/config/system/display/resolution", &currentResolution);
    
    int initialIndex = 1; // Fallback default to 720p
    if (currentResolution == 0) initialIndex = 0;
    else if (currentResolution == 1) initialIndex = 1;
    else if (currentResolution == 3) initialIndex = 2;

    // 3. Register Dropdown choices
    WUPSConfig_AddList(configHandle, "resolution_cat", "resolution_item", "Display Resolution", 
                       initialIndex, resolutionOptions, 3, OnResolutionChanged, NULL);

    // 4. Register Reboot Toggle button 
    WUPSConfig_AddBoolean(configHandle, "resolution_cat", "restart_item", "Restart System Menu (Apply)", 
                          false, OnRestartToggled, NULL);
}

// -----------------------------
// Plugin Lifecycle Hooks
// -----------------------------
INITIALIZE_PLUGIN()
{
    // Register our screen generator natively using the classic C-API registration function hook
    WUPS_RegisterConfigCallback(OnMenuOpened, NULL);
}

DEINITIALIZE_PLUGIN()
{
    // Runtime cleanup
}
