# UI-Integrated Hotkeys System (Skyrim 1.7.104.0 Port)

A powerful SKSE64 plugin that equips multiple items via hotkeys or modifier combinations, featuring an interactive HUD display, full equipment overview, and customizable widgets.

This repository provides an updated, feature-enhanced port compatible with:
- **The Elder Scrolls V: Skyrim Special Edition / Anniversary Edition 1.7.104.0**
- **SKSE 2.3.1**

---

## Original Credits & Attribution

- **Original Author**: [neogulcity](https://github.com/neogulcity)
- **Original Repository**: [neogulcity/HotkeysSystem](https://github.com/neogulcity/HotkeysSystem)
- **Plugin Template**: [CommonLibSSE-NG Sample Plugin](https://gitlab.com/colorglass/commonlibsse-sample-plugin) by [CharmedBaryon](https://gitlab.com/colorglass)

---

## Compatibility & Enhancements in this Port

- **Skyrim 1.7.104.0 & SKSE 2.3.1 Support**:
  - Rebuilt with CommonLibSSE-NG v8.0.0 for seamless compatibility with the 1.7.104.0 runtime.
  - Native Address Library integration for AE 1.7.104.0.
- **Interactive Mouse Drag & Drop**:
  - Open the in-game UI menu (**F6**) to position any widget directly on screen with the mouse cursor.
  - **HUD Diamond Cluster**: Click the center to move all three weapon/shout diamonds together.
  - **Individual Diamonds**: Click any individual weapon or shout diamond to move it independently.
  - **Independent Text Label Positioning**: Click and drag text labels (Lefthand, Righthand, Shout, and Armor slots) freely and separately from the icon widgets.
  - **Armor Slot Widgets**: Move individual armor slots freely on screen, or hold **Shift** while dragging to move the entire armor stack.
  - **Real-Time Visual Indicators**: Gold highlighting rings and text bounding boxes provide clear feedback while hovering and dragging.
  - Automatic persistent coordinate saving to configuration files upon mouse release.
- **Widget & Font Size Sliders**:
  - Proportional widget scaling (0% to 200%) with automatic diamond background adjustment (`bgSize = round(widgetSize * 2.625)`).
  - Independent font size sliders for all equipment widgets.
- **Full Multilingual Localization (13 Languages)**:
  - Complete translations for all new options, HUD settings, and slot descriptions (Slots 30–61):
    English, German, French, Spanish, Italian, Russian, Polish, Czech, Chinese (Simplified), Japanese, Korean, Thai, and Vietnamese.
- **Resolution Scaling**:
  - Universal virtual stage scaling ($S = \text{ScreenHeight} / 720.0f$) providing pixel-perfect alignment across 1080p, 1440p, 4K, 16:10, and 21:9 Ultrawide displays.

---

## Build Requirements

- [Visual Studio 2022 Community](https://visualstudio.microsoft.com/) (MSVC v143+, Desktop development with C++)
- [CMake](https://cmake.org/) (Version 3.21 or higher)
- [PowerShell 7+](https://github.com/PowerShell/PowerShell)
- [vcpkg](https://github.com/microsoft/vcpkg)
- **CommonLib**: [CommonLibSSE-NG](https://github.com/alandtse/CommonLibSSE-NG) (tested with revision `1504349dddfc622d4d25704bba19e2ade669dc5a` / v8.0.0+)

---

## How to Build

### Using CMake and Visual Studio:

1. Clone this repository:
   ```powershell
   git clone https://github.com/<your-repo>/HotkeysSystem.git
   cd HotkeysSystem
   ```

2. Set your `VCPKG_ROOT` environment variable to your vcpkg installation path:
   ```powershell
   $env:VCPKG_ROOT = "C:/path/to/vcpkg"
   ```

3. Configure the project:
   ```powershell
   cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
   ```

4. Build the plugin DLL:
   ```powershell
   cmake --build build --config Release
   ```

The compiled `HotkeysSystem.dll` and `HotkeysSystem.pdb` will be generated in `build/Release/`.

---

## License

- **HotkeysSystem**: [MIT License](LICENSE) &copy; 2023 neogulcity
- **CommonLibSSE-NG**: [Apache License 2.0](https://github.com/alandtse/CommonLibSSE-NG/blob/main/LICENSE) / [MIT License](https://github.com/alandtse/CommonLibSSE-NG/blob/main/LICENSE-MIT)
- **Dear ImGui**: [MIT License](https://github.com/ocornut/imgui/blob/master/LICENSE.txt) &copy; 2014-2026 Omar Cornut
- **toml++**: [MIT License](https://github.com/marzer/tomlplusplus/blob/master/LICENSE) &copy; 2019-2026 Mark Gillard
