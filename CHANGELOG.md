# Changelog

All notable changes to the **UI-Integrated Hotkeys System (Skyrim 1.7.104.0 Port)** will be documented in this file.

## [2.0.1-1.7.104.0] - 2026-09-18

### Added
- **Skyrim Runtime 1.7.104.0 & SKSE 2.3.1 Compatibility**:
  - Recompiled against CommonLibSSE-NG v8.0.0 (1504349dddfc622d4d25704bba19e2ade669dc5a).
  - Native Address Library integration for Skyrim SE/AE 1.7.104.0.
- **Interactive In-Game Mouse Drag & Drop**:
  - Direct manipulation of widgets on screen while the UI menu (F6) is open.
  - Independent positioning for:
    - Entire HUD Diamond cluster.
    - Individual weapon and shout diamond widgets.
    - Text labels (Lefthand, Righthand, Shout, and all 32 Armor slots) separately from their parent icons.
    - Individual armor slot widgets (freely positionable across the screen).
    - Shift + Drag shortcut to move the entire armor stack simultaneously.
  - Interactive visual gold ring highlights and text bounding box indicators with informative tooltips.
  - Automatic persistence to configuration upon mouse release.
- **HUD Scaling Sliders**:
  - Proportional widget size sliders (0% to 200%) with automatic diamond background adjustment (gSize = round(widgetSize * 2.625)).
  - Independent font size sliders (0% to 200%) for all equipment sections.
- **Comprehensive 13-Language Multilingual Localization**:
  - Native translations added for English, German, French, Spanish, Italian, Russian, Polish, Czech, Chinese (Simplified), Japanese, Korean, Thai, and Vietnamese.
  - Human-readable descriptive slot labels for all equipment slots (_SLOT_30 through _SLOT_61).
- **Resolution Auto-Scaling**:
  - Universal 720p virtual stage transformation ensuring consistent HUD positioning across 1080p, 1440p, 4K, 16:10, and 21:9 Ultrawide displays.

### Fixed
- Resolved text label overlap between Left Hand and Right Hand when wielding items with long names.
- Resolved armor slot stacking collision by establishing strict two-tiered priority: base armor slots (Feet, Hands, Chest, Helmet) remain clustered, and accessory/modded slots stack upwards with auto multi-column wrap.
