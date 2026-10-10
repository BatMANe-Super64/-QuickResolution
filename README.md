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

Open the Aroma plugin configuration menu and select **Quick Resolution**. Choose a resolution; this only stages the selection and does not change the system setting yet. To apply it, select **WARNING: Save game first; Apply restarts Menu**. This writes the selected resolution and launches the Wii U Menu.\n\n**Warning:** Applying while a game is running can interrupt gameplay and cause loss of unsaved progress. Save your game first. If the selected mode is not supported by your display setup, the picture may become unusable until you restore a compatible resolution.

## License

Licensed under GNU GPL v3.0. See [LICENSE](LICENSE).
