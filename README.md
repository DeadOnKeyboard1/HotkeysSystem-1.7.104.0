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
- **Automatic Symbol-to-Box Scale Containment**:
  - All widgets (Armor stack on the left; Weapons, Shout, and Equipsets on the right) automatically scale icons to fit the active background box.
  - **Square & Round Themes**: Icons expand to fill ~65% of the visual box size, perfectly centered with clean border padding (fixing undersized icons in square frames).
  - **Diamond Themes**: Inscribed square geometry (~46%) ensures icon corners never poke through sloped diagonal borders.
  - **Standard Theme (No Box)**: Icons use pure symbol scaling without box padding.
- **Pure Theme Swapping**:
  - Selecting a theme preset from the dropdown swaps only the background textures and transparency.
  - Custom positions, alignments, and user layouts are completely preserved—no unwanted shifting or re-aligning.
- **Single-Column Upward Armor Stacking**:
  - Auto-arranges armor slots into a clean, single vertical column anchored at the bottom-most slot (Boots / Slot 37) upwards.
  - Newly activated slots (e.g. Cloak, Shield, Rings) stack directly above the helmet without jumping into the center of the screen.
- **Full Equipment & Layout Reset**:
  - Dedicated "Reset to Defaults" option restores authentic baseline layout, spacing, and dimensions.
- **Widget & Font Size Sliders**:
  - Proportional widget scaling (0% to 200%) with synchronous background adjustment.
  - Independent font size sliders for all equipment widgets.
- **Full Multilingual Localization (13 Languages)**:
  - Complete translations for all new options, HUD settings, and slot descriptions (Slots 30–61):
    English, German, French, Spanish, Italian, Russian, Polish, Czech, Chinese (Simplified), Japanese, Korean, Thai, and Vietnamese.
- **Rich Collection of Lore & Background Themes (26 High-Res DXT5 Textures)**:
  - Both Diamond (Weapons/Shout HUD) and Square/Round (Armor & Equipset stacks) styles:
    - **Oblivion / Daedric Gate**: Fiery molten Daedric "Oht" crest, demonic horns, and obsidian spikes.
    - **Falmer / Chaurus Chitin**: Serrated organic chitin shell plates with toxic bioluminescent glowing pores.
    - **Snow Elf / Ancient Falmer**: Radiant solar rays of Auri-El, gleaming ivory and white marble filigree.
    - **High Elf / Altmer**: Regal eagle wing motifs of Alinor, sweeping golden curves, and moonstone inlays.
    - **Dwemer / Clockwork Engineering**: Heavy brass/bronze gear wheels with interlocking teeth and tonal resonator discs.
    - **Morrowind / Dunmer**: Carved ashlander bonemold, House Redoran beetle-shell ridges, and Daedric Dunmeri runes.
    - **Nightingale / Shadow**: Midnight obsidian and dark steel featuring the iconic Nightingale crest and crescent moons.
    - **Nordic Runes**: Weathered stone and iron frame with authentic Elder Futhark rune engravings.
    - **Celtic Knotwork**: Interlaced Celtic knots and braided silver filigree.
    - **Minimalist Modern**: Crisp, razor-thin double outline with precision corner brackets (SkyUI / TrueHUD style).
    - **Compass / Astrolabe**: Astronomical celestial dial with tick marks and dual-tone cardinal star pointers.
    - **Arcane Magic**: Glowing runic summoning circles and mystical glyphs.
    - **Gothic Dark Iron**: Heavy riveted forged dark steel with sharp spiked corners and crimson accents.
    - **Skyrim Imperial Dragon**: Imperial gold frame with stylized dragon wings and imperial crest accents.
  - **1-Click Theme Preset Selector**: Choose a preset style from the in-game menu (`F6`) to instantly theme all widgets, or customize each individually.
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
   git clone https://github.com/DeadOnKeyboard1/HotkeysSystem-1.7.104.0.git
   cd HotkeysSystem-1.7.104.0
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

## License & Legal Notices

This repository and its compiled binary distributions are licensed under the **GNU General Public License v3.0 or later** ([GPL-3.0-or-later](LICENSE)).

### Why GPL-3.0?
While the original upstream mod was created under the permissive MIT License, this port builds with and statically links **[CommonLibSSE-NG](https://github.com/alandtse/CommonLibSSE-NG)**, which transitioned to **GPL-3.0-or-later** (with Modding and Linking Exceptions). Because CommonLibSSE-NG is statically compiled into the resulting SKSE plugin binary (`HotkeysSystem.dll`), the compiled plugin and any distribution form a combined work that must be licensed under the GPL-3.0-or-later.

### Component Breakdown & Attribution
- **Original HotkeysSystem**: &copy; 2023 [neogulcity](https://github.com/neogulcity) &mdash; Released under the [MIT License](https://github.com/neogulcity/HotkeysSystem/blob/main/LICENSE). Original copyright notices and permission terms are preserved.
- **Port & Enhancements (Skyrim 1.7.104.0)**: &copy; 2026 DeadOnKeyboard / Contributors &mdash; Licensed under the [GNU General Public License v3.0 or later](LICENSE).
- **CommonLibSSE-NG**: &copy; CommonLibSSE-NG Contributors &mdash; Licensed under [GPL-3.0-or-later](licenses/CommonLibSSE-NG/COPYING.txt) with [Modding and Linking Exceptions](licenses/CommonLibSSE-NG/EXCEPTIONS.md).
- **Dear ImGui**: &copy; 2014-2026 Omar Cornut &mdash; Licensed under the [MIT License](licenses/DearImGui/LICENSE.txt).
- **toml++**: &copy; 2019-2026 Mark Gillard &mdash; Licensed under the [MIT License](licenses/tomlplusplus/LICENSE).

