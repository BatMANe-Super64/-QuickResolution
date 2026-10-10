#include <avm/config.h>
#include <avm/tv.h>
#include <wups.h>
#include <wups/storage.h>
#include <wups/button_combo/api.h>
#include <wups/config/WUPSConfigItemStub.h>
#include <notifications/notifications.h>
#include <vpad/input.h>
#include <wups/config/WUPSConfigCategory.h>
#include <wups/config/WUPSConfigItemBoolean.h>
#include <wups/config/WUPSConfigItemMultipleValues.h>
#include <wups/config_api.h>
#include <sysapp/launch.h>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include <optional>
#include <span>
#include <string>

WUPS_USE_STORAGE("quickresolution_settings");

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

constexpr uint32_t kDefaultBackgroundColor = 0;
constexpr uint32_t kBackgroundColorCount = 51;
constexpr uint32_t kUseGlobalBackgroundColor = kBackgroundColorCount;
constexpr size_t kMenuEntryCount = 3;
constexpr float kNotificationExitStaggerSeconds = 0.04f;
constexpr char kBackgroundColorStorageId[] = "background_color";
constexpr char kEntryColorStoragePrefix[] = "entry_color_";
uint32_t gBackgroundColorSelection = kDefaultBackgroundColor;
uint32_t gEntryBackgroundColors[kMenuEntryCount] = {
    kUseGlobalBackgroundColor, kUseGlobalBackgroundColor, kUseGlobalBackgroundColor
};
WUPSButtonCombo_ComboHandle gToggleCombo = {};
WUPSButtonCombo_ComboHandle gPreviousCombo = {};
WUPSButtonCombo_ComboHandle gNextCombo = {};
WUPSButtonCombo_ComboHandle gCyclePreviousCombo = {};
WUPSButtonCombo_ComboHandle gCycleNextCombo = {};
WUPSButtonCombo_ComboHandle gSelectCombo = {};
NotificationModuleHandle gNotificationHandles[kMenuEntryCount] = {};
bool gNotificationReady = false;
bool gOverlayVisible = false;
size_t gSelectedMenuEntry = 0;
char gOverlayStatus[96] = "NotificationModule: not initialized";

constexpr WUPSConfigItemMultipleValues::ValuePair kResolutionOptions[] = {
    {AVM_TV_RESOLUTION_480P, "480p"},
    {AVM_TV_RESOLUTION_720P, "720p"},
    {AVM_TV_RESOLUTION_1080P, "1080p"},
};

NMColor QuickResolutionColorForSelection(uint32_t selection) {
    switch (selection) {
        case 0: return NMColor{100, 100, 100, 255}; // Standard Notifications (native styling)
        case 1: return NMColor{24, 24, 28, 255};    // Black
        case 2: return NMColor{48, 52, 58, 255};    // Charcoal
        case 3: return NMColor{82, 86, 92, 255};    // Gray
        case 4: return NMColor{105, 125, 145, 255}; // Slate
        case 5: return NMColor{125, 135, 145, 255}; // Silver
        case 6: return NMColor{145, 155, 168, 255}; // Ghost Gray
        case 7: return NMColor{220, 35, 45, 255};   // Red
        case 8: return NMColor{235, 75, 65, 255};   // Red-Orange
        case 9: return NMColor{225, 35, 90, 255};   // Crimson
        case 10: return NMColor{245, 105, 95, 255}; // Coral
        case 11: return NMColor{250, 135, 35, 255}; // Orange
        case 12: return NMColor{245, 165, 45, 255}; // Orange-Yellow
        case 13: return NMColor{220, 105, 35, 255}; // Pumpkin
        case 14: return NMColor{195, 135, 30, 255}; // Amber
        case 15: return NMColor{190, 150, 30, 255}; // Gold
        case 16: return NMColor{185, 170, 25, 255}; // Yellow
        case 17: return NMColor{125, 160, 30, 255}; // Yellow-Green
        case 18: return NMColor{85, 150, 25, 255}; // Lime
        case 19: return NMColor{45, 175, 70, 255};  // Green
        case 20: return NMColor{35, 155, 105, 255}; // Green-Cyan
        case 21: return NMColor{50, 145, 100, 255}; // Mint
        case 22: return NMColor{35, 175, 155, 255}; // Teal
        case 23: return NMColor{45, 205, 205, 255}; // Cyan
        case 24: return NMColor{35, 145, 165, 255}; // Cyan-Blue
        case 25: return NMColor{45, 125, 190, 255}; // Sky Blue
        case 26: return NMColor{35, 125, 230, 255}; // Blue
        case 27: return NMColor{65, 90, 220, 255};  // Blue-Violet
        case 28: return NMColor{70, 105, 195, 255}; // Royal Blue
        case 29: return NMColor{70, 95, 175, 255}; // Periwinkle
        case 30: return NMColor{115, 85, 215, 255}; // Indigo
        case 31: return NMColor{150, 75, 215, 255}; // Purple
        case 32: return NMColor{175, 105, 225, 255}; // Purple-Magenta
        case 33: return NMColor{190, 95, 235, 255}; // Violet
        case 34: return NMColor{120, 90, 165, 255}; // Lavender
        case 35: return NMColor{220, 65, 190, 255}; // Magenta
        case 36: return NMColor{245, 75, 165, 255}; // Hot Pink
        case 37: return NMColor{190, 70, 130, 255}; // Pink
        case 38: return NMColor{220, 90, 125, 255}; // Rose
        case 39: return NMColor{160, 95, 55, 255};  // Brown
        case 40: return NMColor{190, 115, 65, 255}; // Chocolate
        case 41: return NMColor{205, 135, 85, 255}; // Copper
        case 42: return NMColor{155, 120, 65, 255}; // Sand
        case 43: return NMColor{165, 145, 95, 255}; // Cream
        case 44: return NMColor{75, 155, 85, 255};  // Forest Green
        case 45: return NMColor{55, 155, 205, 255}; // Ocean Blue
        case 46: return NMColor{20, 130, 190, 255}; // Neon Blue
        case 47: return NMColor{85, 155, 25, 255}; // Neon Green
        case 48: return NMColor{255, 90, 205, 255}; // Neon Pink
        case 49: return NMColor{200, 80, 50, 255}; // Sunset
        case 50: return NMColor{35, 145, 125, 255}; // Aurora
        default: return NMColor{100, 100, 100, 255};
    }
}

void ApplyNotificationBackgroundColors()
{
    if (!gNotificationReady) return;
    for (size_t i = 0; i < kMenuEntryCount; ++i)
    {
        const NotificationModuleHandle handle = gNotificationHandles[i];
        if (handle == 0) continue;
        const uint32_t entrySelection = gEntryBackgroundColors[i];
        const uint32_t selection = entrySelection == kUseGlobalBackgroundColor
            ? gBackgroundColorSelection : entrySelection;
        if (selection == kDefaultBackgroundColor) continue;
        (void)NotificationModule_UpdateDynamicNotificationBackgroundColor(
            handle, QuickResolutionColorForSelection(selection));
    }
}

const char* ResolutionLabel(AVMTvResolution resolution)
{
    for (const auto& option : kResolutionOptions)
        if (option.value == static_cast<uint32_t>(resolution)) return option.label;
    return "Unknown";
}

void BuildOverlayText(size_t index, char* buffer, size_t size)
{
    if (size == 0) return;
    if (index == 0)
        (void)std::snprintf(buffer, size, "Quick Resolution");
    else if (index == 1)
        (void)std::snprintf(buffer, size, "%c Resolution: %s",
            gSelectedMenuEntry == 2 ? '>' : ' ', ResolutionLabel(gPendingResolution));
    else
        (void)std::snprintf(buffer, size, "%c Apply resolution (restart Menu)",
            gSelectedMenuEntry == 1 ? '>' : ' ');
}

void HideResolutionOverlay()
{
    if (!gNotificationReady) return;
    float delay = 0.0f;
    for (size_t i = 0; i < kMenuEntryCount; ++i)
    {
        if (gNotificationHandles[i] != 0)
        {
            (void)NotificationModule_FinishDynamicNotification(gNotificationHandles[i], delay);
            gNotificationHandles[i] = 0;
            delay += kNotificationExitStaggerSeconds;
        }
    }
}

void RefreshResolutionOverlay()
{
    if (!gOverlayVisible || !gNotificationReady) return;
    for (size_t i = 0; i < kMenuEntryCount; ++i)
    {
        if (gNotificationHandles[i] == 0) continue;
        char text[96];
        BuildOverlayText(i, text, sizeof(text));
        const NotificationModuleStatus status =
            NotificationModule_UpdateDynamicNotificationText(gNotificationHandles[i], text);
        if (status != NOTIFICATION_MODULE_RESULT_SUCCESS)
        {
            HideResolutionOverlay();
            gOverlayVisible = false;
            (void)std::snprintf(gOverlayStatus, sizeof(gOverlayStatus),
                "Notification update failed: %.45s", NotificationModule_GetStatusStr(status));
            return;
        }
    }
    ApplyNotificationBackgroundColors();
}

void ShowResolutionOverlay()
{
    if (!gNotificationReady)
    {
        (void)std::snprintf(gOverlayStatus, sizeof(gOverlayStatus),
            "NotificationModule unavailable");
        return;
    }
    for (size_t i = 0; i < kMenuEntryCount; ++i)
    {
        char text[96];
        BuildOverlayText(i, text, sizeof(text));
        const NotificationModuleStatus status =
            NotificationModule_AddDynamicNotification(text, &gNotificationHandles[i]);
        if (status != NOTIFICATION_MODULE_RESULT_SUCCESS)
        {
            HideResolutionOverlay();
            (void)std::snprintf(gOverlayStatus, sizeof(gOverlayStatus),
                "Notification failed: %.45s", NotificationModule_GetStatusStr(status));
            return;
        }
    }
    ApplyNotificationBackgroundColors();
    (void)std::snprintf(gOverlayStatus, sizeof(gOverlayStatus), "Quick Resolution menu open");
}

void ToggleResolutionOverlay(WUPSButtonCombo_ControllerTypes, WUPSButtonCombo_ComboHandle, void*)
{
    gOverlayVisible = !gOverlayVisible;
    if (gOverlayVisible) ShowResolutionOverlay();
    else HideResolutionOverlay();
}

void MoveResolutionMenu(int direction)
{
    if (!gOverlayVisible) return;
    gSelectedMenuEntry = (gSelectedMenuEntry == 1)
        ? (direction > 0 ? 2 : 2)
        : 1;
    RefreshResolutionOverlay();
}

void PreviousMenuEntry(WUPSButtonCombo_ControllerTypes, WUPSButtonCombo_ComboHandle, void*)
{
    MoveResolutionMenu(-1);
}

void NextMenuEntry(WUPSButtonCombo_ControllerTypes, WUPSButtonCombo_ComboHandle, void*)
{
    MoveResolutionMenu(1);
}

void CycleResolution(int direction)
{
    if (!gOverlayVisible || gSelectedMenuEntry != 1) return;
    size_t current = 0;
    for (size_t i = 0; i < sizeof(kResolutionOptions) / sizeof(kResolutionOptions[0]); ++i)
        if (kResolutionOptions[i].value == static_cast<uint32_t>(gPendingResolution)) current = i;
    current = (current + sizeof(kResolutionOptions) / sizeof(kResolutionOptions[0]) +
        (direction > 0 ? 1 : static_cast<int>(sizeof(kResolutionOptions) / sizeof(kResolutionOptions[0])) - 1)) %
        (sizeof(kResolutionOptions) / sizeof(kResolutionOptions[0]));
    gPendingResolution = static_cast<AVMTvResolution>(kResolutionOptions[current].value);
    RefreshResolutionOverlay();
}

void PreviousResolution(WUPSButtonCombo_ControllerTypes, WUPSButtonCombo_ComboHandle, void*)
{
    CycleResolution(-1);
}

void NextResolution(WUPSButtonCombo_ControllerTypes, WUPSButtonCombo_ComboHandle, void*)
{
    CycleResolution(1);
}

bool ApplyPendingResolution()
{
    return SetResolution(static_cast<uint32_t>(gPendingResolution));
}

void SelectResolutionMenuEntry(WUPSButtonCombo_ControllerTypes, WUPSButtonCombo_ComboHandle, void*)
{
    if (!gOverlayVisible) return;
    if (gSelectedMenuEntry != 2) return;
    HideResolutionOverlay();
    gOverlayVisible = false;
    if (ApplyPendingResolution())
    {
        // Only restart the Wii U Menu after the system resolution write succeeds.
        SYSLaunchMenu();
    }
    else
    {
        (void)std::snprintf(gOverlayStatus, sizeof(gOverlayStatus),
            "Resolution write failed; Menu was not restarted");
    }
}

void OnBackgroundColorChanged(ConfigItemMultipleValues*, uint32_t value)
{
    gBackgroundColorSelection = value < kBackgroundColorCount ? value : kDefaultBackgroundColor;
    ApplyNotificationBackgroundColors();
    const WUPSStorageError error = WUPSStorageAPI::Store(kBackgroundColorStorageId, gBackgroundColorSelection);
    if (error != WUPS_STORAGE_ERROR_SUCCESS)
        (void)std::snprintf(gOverlayStatus, sizeof(gOverlayStatus), "Color save failed: %.40s",
            WUPSStorageAPI::GetStatusStr(error));
}

void OnEntryBackgroundColorChanged(size_t index, uint32_t value)
{
    if (index >= kMenuEntryCount) return;
    gEntryBackgroundColors[index] = value <= kUseGlobalBackgroundColor ? value : kUseGlobalBackgroundColor;
    char key[32];
    (void)std::snprintf(key, sizeof(key), "%s%u", kEntryColorStoragePrefix, static_cast<unsigned>(index));
    const WUPSStorageError error = WUPSStorageAPI::Store(key, gEntryBackgroundColors[index]);
    if (error != WUPS_STORAGE_ERROR_SUCCESS)
        (void)std::snprintf(gOverlayStatus, sizeof(gOverlayStatus), "Entry color save failed: %.38s",
            WUPSStorageAPI::GetStatusStr(error));
    ApplyNotificationBackgroundColors();
}

void OnResolutionEntryColorChanged(ConfigItemMultipleValues*, uint32_t value)
{
    OnEntryBackgroundColorChanged(1, value);
}
void OnApplyEntryColorChanged(ConfigItemMultipleValues*, uint32_t value)
{
    OnEntryBackgroundColorChanged(2, value);
}
void OnHeaderEntryColorChanged(ConfigItemMultipleValues*, uint32_t value)
{
    OnEntryBackgroundColorChanged(0, value);
}

void LoadBackgroundColors()
{
    (void)WUPSStorageAPI::GetOrStoreDefault<uint32_t>(
        kBackgroundColorStorageId, gBackgroundColorSelection, kDefaultBackgroundColor);
    if (gBackgroundColorSelection >= kBackgroundColorCount)
        gBackgroundColorSelection = kDefaultBackgroundColor;
    for (size_t i = 0; i < kMenuEntryCount; ++i)
    {
        char key[32];
        (void)std::snprintf(key, sizeof(key), "%s%u", kEntryColorStoragePrefix, static_cast<unsigned>(i));
        (void)WUPSStorageAPI::GetOrStoreDefault<uint32_t>(
            key, gEntryBackgroundColors[i], kUseGlobalBackgroundColor);
        if (gEntryBackgroundColors[i] > kUseGlobalBackgroundColor)
            gEntryBackgroundColors[i] = kUseGlobalBackgroundColor;
    }
}

constexpr WUPSConfigItemMultipleValues::ValuePair kBackgroundColorOptions[] = {
            {0, "Standard Notifications"},
            {1, "Black"},
            {2, "Charcoal"},
            {3, "Gray"},
            {4, "Slate Blue"},
            {5, "Silver"},
            {6, "Light Gray"},
            {7, "Red"},
            {8, "Red-Orange"},
            {9, "Crimson"},
            {10, "Coral"},
            {11, "Orange"},
            {12, "Orange-Yellow"},
            {13, "Pumpkin"},
            {14, "Amber"},
            {15, "Gold"},
            {16, "Yellow"},
            {17, "Yellow-Green"},
            {18, "Lime"},
            {19, "Green"},
            {20, "Green-Cyan"},
            {21, "Mint"},
            {22, "Teal"},
            {23, "Cyan"},
            {24, "Cyan-Blue"},
            {25, "Sky Blue"},
            {26, "Blue"},
            {27, "Blue-Violet"},
            {28, "Royal Blue"},
            {29, "Periwinkle"},
            {30, "Indigo"},
            {31, "Purple"},
            {32, "Purple-Magenta"},
            {33, "Violet"},
            {34, "Lavender"},
            {35, "Magenta"},
            {36, "Hot Pink"},
            {37, "Pink"},
            {38, "Rose"},
            {39, "Brown"},
            {40, "Chocolate"},
            {41, "Copper"},
            {42, "Sand"},
            {43, "Cream"},
            {44, "Forest Green"},
            {45, "Ocean Blue"},
            {46, "Neon Blue"},
            {47, "Neon Green"},
            {48, "Neon Pink"},
            {49, "Sunset"},
            {50, "Aurora"},
};

constexpr WUPSConfigItemMultipleValues::ValuePair kEntryBackgroundColorOptions[] = {
            {0, "Standard Notifications"},
            {1, "Black"},
            {2, "Charcoal"},
            {3, "Gray"},
            {4, "Slate Blue"},
            {5, "Silver"},
            {6, "Light Gray"},
            {7, "Red"},
            {8, "Red-Orange"},
            {9, "Crimson"},
            {10, "Coral"},
            {11, "Orange"},
            {12, "Orange-Yellow"},
            {13, "Pumpkin"},
            {14, "Amber"},
            {15, "Gold"},
            {16, "Yellow"},
            {17, "Yellow-Green"},
            {18, "Lime"},
            {19, "Green"},
            {20, "Green-Cyan"},
            {21, "Mint"},
            {22, "Teal"},
            {23, "Cyan"},
            {24, "Cyan-Blue"},
            {25, "Sky Blue"},
            {26, "Blue"},
            {27, "Blue-Violet"},
            {28, "Royal Blue"},
            {29, "Periwinkle"},
            {30, "Indigo"},
            {31, "Purple"},
            {32, "Purple-Magenta"},
            {33, "Violet"},
            {34, "Lavender"},
            {35, "Magenta"},
            {36, "Hot Pink"},
            {37, "Pink"},
            {38, "Rose"},
            {39, "Brown"},
            {40, "Chocolate"},
            {41, "Copper"},
            {42, "Sand"},
            {43, "Cream"},
            {44, "Forest Green"},
            {45, "Ocean Blue"},
            {46, "Neon Blue"},
            {47, "Neon Green"},
            {48, "Neon Pink"},
            {49, "Sunset"},
            {50, "Aurora"},
            {kUseGlobalBackgroundColor, "Use Global Color"},
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
    if (ApplyPendingResolution())
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

        root.add(WUPSConfigItemStub::Create("Overlay controls: TV + ZL toggle; TV + Up/Down select; TV + Left/Right change resolution; TV + A apply when Apply is selected"));
        root.add(WUPSConfigItemMultipleValues::CreateFromValue(
            std::optional<const std::string>(kBackgroundColorStorageId),
            "Background Color", kDefaultBackgroundColor, gBackgroundColorSelection,
            std::span<const WUPSConfigItemMultipleValues::ValuePair>(kBackgroundColorOptions),
            OnBackgroundColorChanged));
        root.add(WUPSConfigItemStub::Create("Per-Entry Background Colors"));
        root.add(WUPSConfigItemMultipleValues::CreateFromValue(
            std::optional<const std::string>("entry_color_0"), "Header Background",
            kUseGlobalBackgroundColor, gEntryBackgroundColors[0],
            std::span<const WUPSConfigItemMultipleValues::ValuePair>(kEntryBackgroundColorOptions),
            OnHeaderEntryColorChanged));
        root.add(WUPSConfigItemMultipleValues::CreateFromValue(
            std::optional<const std::string>("entry_color_1"), "Resolution Background",
            kUseGlobalBackgroundColor, gEntryBackgroundColors[1],
            std::span<const WUPSConfigItemMultipleValues::ValuePair>(kEntryBackgroundColorOptions),
            OnResolutionEntryColorChanged));
        root.add(WUPSConfigItemMultipleValues::CreateFromValue(
            std::optional<const std::string>("entry_color_2"), "Apply Background",
            kUseGlobalBackgroundColor, gEntryBackgroundColors[2],
            std::span<const WUPSConfigItemMultipleValues::ValuePair>(kEntryBackgroundColorOptions),
            OnApplyEntryColorChanged));
    }
    catch (...)
    {
        return WUPSCONFIG_API_CALLBACK_RESULT_ERROR;
    }
    return WUPSCONFIG_API_CALLBACK_RESULT_SUCCESS;
}

void ConfigMenuClosedCallback()
{
    const WUPSStorageError error = WUPSStorageAPI::SaveStorage();
    if (error != WUPS_STORAGE_ERROR_SUCCESS)
        (void)std::snprintf(gOverlayStatus, sizeof(gOverlayStatus), "Settings save failed: %.40s",
            WUPSStorageAPI::GetStatusStr(error));
}
} // namespace

DECL_FUNCTION(int32_t, VPADRead, VPADChan chan, VPADStatus* buffers, uint32_t count, VPADReadError* error)
{
    const int32_t result = real_VPADRead(chan, buffers, count, error);
    if (result > 0 && buffers != nullptr)
    {
        for (int32_t i = 0; i < result; ++i)
        {
            if ((buffers[i].hold & VPAD_BUTTON_TV) == 0) continue;
            if (gOverlayVisible)
            {
                constexpr uint32_t kOverlayButtons = VPAD_BUTTON_UP | VPAD_BUTTON_DOWN |
                    VPAD_BUTTON_LEFT | VPAD_BUTTON_RIGHT | VPAD_BUTTON_A;
                buffers[i].hold &= ~kOverlayButtons;
                buffers[i].trigger &= ~kOverlayButtons;
                buffers[i].release &= ~kOverlayButtons;
            }
        }
    }
    return result;
}
WUPS_MUST_REPLACE(VPADRead, WUPS_LOADER_LIBRARY_VPAD, VPADRead);

INITIALIZE_PLUGIN()
{
    AVMTvResolution configuredResolution = kDefaultResolution;
    if (AVMReadSystemVideoResConfig(&configuredResolution) == 0 &&
        IsSupportedResolution(static_cast<uint32_t>(configuredResolution)))
    {
        gCurrentResolution = configuredResolution;
    }
    gPendingResolution = gCurrentResolution;

    LoadBackgroundColors();
    const NotificationModuleStatus notificationStatus = NotificationModule_InitLibrary();
    gNotificationReady = notificationStatus == NOTIFICATION_MODULE_RESULT_SUCCESS;
    if (!gNotificationReady)
        (void)std::snprintf(gOverlayStatus, sizeof(gOverlayStatus), "NotificationModule unavailable: %.35s",
            NotificationModule_GetStatusStr(notificationStatus));

    WUPSButtonCombo_ComboStatus comboStatus = WUPS_BUTTON_COMBO_COMBO_STATUS_INVALID_STATUS;
    (void)WUPSButtonComboAPI_AddButtonComboPressDown(
        "QuickResolution menu (TV + ZL)",
        static_cast<WUPSButtonCombo_Buttons>(WUPS_BUTTON_COMBO_BUTTON_TV | WUPS_BUTTON_COMBO_BUTTON_ZL),
        ToggleResolutionOverlay, nullptr, &gToggleCombo, &comboStatus);
    (void)WUPSButtonComboAPI_AddButtonComboPressDown(
        "QuickResolution previous menu entry (TV + Up)",
        static_cast<WUPSButtonCombo_Buttons>(WUPS_BUTTON_COMBO_BUTTON_TV | WUPS_BUTTON_COMBO_BUTTON_UP),
        PreviousMenuEntry, nullptr, &gPreviousCombo, &comboStatus);
    (void)WUPSButtonComboAPI_AddButtonComboPressDown(
        "QuickResolution next menu entry (TV + Down)",
        static_cast<WUPSButtonCombo_Buttons>(WUPS_BUTTON_COMBO_BUTTON_TV | WUPS_BUTTON_COMBO_BUTTON_DOWN),
        NextMenuEntry, nullptr, &gNextCombo, &comboStatus);
    (void)WUPSButtonComboAPI_AddButtonComboPressDown(
        "QuickResolution previous resolution (TV + Left)",
        static_cast<WUPSButtonCombo_Buttons>(WUPS_BUTTON_COMBO_BUTTON_TV | WUPS_BUTTON_COMBO_BUTTON_LEFT),
        PreviousResolution, nullptr, &gCyclePreviousCombo, &comboStatus);
    (void)WUPSButtonComboAPI_AddButtonComboPressDown(
        "QuickResolution next resolution (TV + Right)",
        static_cast<WUPSButtonCombo_Buttons>(WUPS_BUTTON_COMBO_BUTTON_TV | WUPS_BUTTON_COMBO_BUTTON_RIGHT),
        NextResolution, nullptr, &gCycleNextCombo, &comboStatus);
    (void)WUPSButtonComboAPI_AddButtonComboPressDown(
        "QuickResolution select/apply (TV + A)",
        static_cast<WUPSButtonCombo_Buttons>(WUPS_BUTTON_COMBO_BUTTON_TV | WUPS_BUTTON_COMBO_BUTTON_A),
        SelectResolutionMenuEntry, nullptr, &gSelectCombo, &comboStatus);

    WUPSConfigAPIOptionsV1 configOptions = {.name = "QuickResolution"};
    (void)WUPSConfigAPI_Init(configOptions, ConfigMenuOpenedCallback, ConfigMenuClosedCallback);
}

DEINITIALIZE_PLUGIN()
{
    HideResolutionOverlay();
    if (gNotificationReady)
    {
        (void)NotificationModule_DeInitLibrary();
        gNotificationReady = false;
    }
    WUPSButtonCombo_ComboHandle* handles[] = {
        &gToggleCombo, &gPreviousCombo, &gNextCombo,
        &gCyclePreviousCombo, &gCycleNextCombo, &gSelectCombo
    };
    for (auto* handle : handles)
    {
        if (handle->handle != nullptr)
        {
            (void)WUPSButtonComboAPI_RemoveButtonCombo(*handle);
            handle->handle = nullptr;
        }
    }
}
