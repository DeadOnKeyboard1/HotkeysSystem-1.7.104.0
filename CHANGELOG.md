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
    - Entire HUD Diamond cluster (synchronously moves attached bottom potion widget).
    - Individual weapon and shout diamond widgets.
    - All Equipset widgets (Potions, Gear sets, Cycle sets).
    - Text labels (Lefthand, Righthand, Shout, all 32 Armor slots, and Equipset names/amounts/hotkeys) separately from their parent icons.
    - Individual armor slot widgets (freely positionable across the screen).
    - Shift + Drag shortcut to move the entire armor stack simultaneously.
  - Interactive visual gold/cyan ring highlights and text bounding box indicators with informative tooltips.
  - Automatic persistence to configuration upon mouse release.
- **Smart Diamond HUD Potion Placement**:
  - Automatically calculates and positions new potion widgets into the 4th (bottom) slot of the HUD Diamond cluster.
  - Dedicated "Snap to Diamond HUD (Bottom Slot)" button in both creation and editor dialogs.
- **Symbol-to-Box Scale Containment**:
  - Automatic dynamic geometric sizing ensuring symbols fit cleanly within square (~65%) and diamond (~46%) background frames without clipping or overflowing borders.
- **HUD Scaling Sliders**:
  - Proportional widget size sliders (0% to 200%) with automatic diamond background adjustment (bgSize = round(widgetSize * 2.625)).
  - Independent font size sliders (0% to 200%) for all equipment sections.
- **Comprehensive 13-Language Multilingual Localization**:
  - Native translations added for English, German, French, Spanish, Italian, Russian, Polish, Czech, Chinese (Simplified), Japanese, Korean, Thai, and Vietnamese.
  - Human-readable descriptive slot labels for all equipment slots (_SLOT_30 through _SLOT_61).
- **Resolution Auto-Scaling**:
  - Universal 720p virtual stage transformation ensuring consistent HUD positioning across 1080p, 1440p, 4K, 16:10, and 21:9 Ultrawide displays.

### Fixed
- Resolved text label overlap between Left Hand and Right Hand when wielding items with long names.
- Resolved armor slot stacking collision: Single-column upward stacking anchored at Boots (Slot 37) prevents modded armor slots from jumping into the center of the screen.
- Resolved icon overflow in sloped diamond boxes and undersized icons in classic square boxes.
- Fixed Create Equipset modal window clipping so confirmation (OK/Cancel) buttons are always visible without scrolling.
- Fixed Potion name resolution so custom selected potions display their actual item name rather than set ID.
