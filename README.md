# QuickResolution

**Change your Wii U display resolution in seconds — and jump straight back into the game you were playing.**

QuickResolution is a free [Aroma](https://github.com/wiiu-env/Aroma) / WUPS plugin for switching between **480p, 720p, and 1080p** without navigating through Wii U System Settings. Choose a resolution, apply it, and QuickResolution **relaunches the currently running game** in the selected mode. When used from the Wii U Menu, it relaunches the Menu instead.

## ⚡ Real Wii U hardware benchmark

| Method | Measured time |
| :-- | --: |
| Wii U System Settings | **59 seconds** |
| **QuickResolution** | **18 seconds** |

```text
Time to change display resolution (lower is better)

System Settings   ██████████████████████████████  59 s
QuickResolution   █████████                       18 s
```

**41 seconds saved · 69.5% less time · 3.28× faster**

These are user-measured times on real Wii U hardware. The 59-second System Settings measurement **does not include the time needed to launch System Settings**, so the full conventional workflow can take longer. Results may vary by setup and scenario.

## Features

- **480p / 720p / 1080p** resolution choices.
- **Restart the game you were already playing** after applying the new resolution — no need to manually find and launch it again.
- **Wii U Menu-aware:** applying from the Menu restarts the Menu in the selected resolution.
- **Staged selection:** changing the dropdown alone does nothing until you explicitly apply it.
- **Save-first warning** shown at the top of the plugin's configuration screen.

## Requirements

- A Wii U running the Aroma homebrew environment and WUPS.

## Install

Download the `QuickResolution-Plugin` artifact from [GitHub Actions](https://github.com/BatMANe-Super64/-QuickResolution/actions/workflows/build.yml), extract the `.wps` file, and copy it to:

```text
sd:/wiiu/environments/[ENVIRONMENT]/plugins/
```

Replace `[ENVIRONMENT]` with your Aroma environment name. Restart the environment or reload the plugin as appropriate.

> **Experimental game-relaunch functionality** is currently on the `experiment/relaunch-current-title` branch. Use an artifact built from that branch to test it.

## Usage

1. **Save your game first.** Applying a resolution restarts the active title and unsaved progress may be lost.
2. Open the Aroma plugin configuration menu and select **Quick Resolution**.
3. Choose **Display Resolution (staged until Apply)**: 480p, 720p, or 1080p.
4. Activate **Apply Resolution & Restart Game**.
5. QuickResolution writes the resolution setting and requests a relaunch of the current game, or the Wii U Menu if that's what is running.

**WARNING: Save game first** appears at the top of the configuration menu as informational text, not a separate switch.

**Display compatibility:** If your display cannot handle the selected mode, the picture may become unusable until you restore a supported resolution.

## Build

```sh
make clean
make
```

The WUPS build produces a `.wps` plugin. GitHub Actions also publishes a downloadable workflow artifact.

## License

Licensed under GNU GPL v3.0. See [LICENSE](LICENSE).

---

Made by **Super64** for the Wii U homebrew community.
