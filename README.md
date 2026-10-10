# Quick Resolution

A WUPS plugin for Aroma that provides a configuration-menu option for selecting the Wii U display resolution (480p, 720p, or 1080p).

## Requirements

- Aroma
- [WiiUPluginLoaderBackend](https://github.com/wiiu-env/WiiUPluginLoaderBackend)
- The WUPS and WUT development libraries supplied by the Wii U plugin toolchain

## Build

```sh
make clean
make
```

The standard WUPS rules produce `QuickResolution.wps` in the repository root. GitHub Actions builds it and publishes it as a workflow artifact.

## Install

Copy `QuickResolution.wps` to:

```text
sd:/wiiu/environments/[ENVIRONMENT]/plugins/QuickResolution.wps
```

Replace `[ENVIRONMENT]` with the name of your Aroma environment, then launch or restart that environment.

## Configuration

Open the Aroma plugin configuration menu and select **Quick Resolution**.

1. Choose **Display Resolution**. This only stages the selection.
2. Set **Apply resolution** to **APPLY NOW**. WUPS invokes the changed Boolean item's callback when the configuration menu closes, so the write and menu launch happen as you exit the config menu.
3. The Apply item intentionally has no saved identifier, so the one-shot choice is not persisted as a plugin setting.

The plugin writes the staged resolution first and launches the Wii U Menu only if the write succeeds.

**Warning:** Applying while a game is running can interrupt gameplay and cause loss of unsaved progress. Save your game first. If the selected mode is not supported by your display setup, the picture may become unusable until you restore a compatible resolution.

## License

Licensed under GNU GPL v3.0. See [LICENSE](LICENSE).
