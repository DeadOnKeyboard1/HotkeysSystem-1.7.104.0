#include "GuiMenu.h"

#include <RE/C/ControlMap.h>
#include <RE/U/UserEventEnabled.h>
#include <imgui.h>
#include <imgui_internal.h>

#include <filesystem>

#include "Config.h"
#include "Data.h"
#include "Draw.h"
#include "Equipment.h"
#include "EquipsetManager.h"
#include "Translate.h"
#include "WidgetHandler.h"
#include "Utility.h"
#include "extern/IconsFontAwesome5.h"
#include "extern/imgui_impl_dx11.h"
#include "extern/imgui_stdlib.h"

GuiMenu::GuiMenu() {
    ConfigHandler::GetSingleton()->LoadConfig();
    EquipmentManager::GetSingleton()->Load();
    logger::info("GuiMenu initialized!");
}

void ToggleControls(RE::ControlMap* control, RE::ControlMap::UEFlag a_flags, bool a_enable) {
    if (control) {
        control->ToggleControls(a_flags, a_enable, false);
    }
}

void GuiMenu::DisableInput(bool _status) {
    auto control = RE::ControlMap::GetSingleton();
    if (!control) return;

    control->ToggleControls(RE::ControlMap::UEFlag::kAll, !_status, false);
}

void GuiMenu::Toggle(std::optional<bool> enabled = std::nullopt) {
    if (!ImGui::GetCurrentContext()) return;

    auto dataHandler = DataHandler::GetSingleton();
    if (!dataHandler) return;

    auto drawHelper = DrawHelper::GetSingleton();
    if (!drawHelper) return;

    auto widgetHandler = WidgetHandler::GetSingleton();
    if (!widgetHandler) return;

    auto& io = ImGui::GetIO();
    io.ClearInputCharacters();
    io.ClearInputKeys();
    show = enabled.value_or(!show);
    DisableInput(show);
    if (show) {
        dataHandler->Init();
        widgetHandler->CloseExpireTimer();
        widgetHandler->SetMenuAlpha(100);
    } else {
        drawHelper->NotifyReload(true);
        dataHandler->Clear();
        widgetHandler->ProcessFadeOut();

        auto config = ConfigHandler::GetSingleton();
        if (!config) return;
        config->SaveConfig();

        auto equipment = EquipmentManager::GetSingleton();
        if (!equipment) return;
        equipment->Save();
    }
}

void GuiMenu::DrawMain() {
    if (!imgui_inited) return;

    auto ui = RE::UI::GetSingleton();
    if (!ui) return;

    auto config = ConfigHandler::GetSingleton();
    if (!config) return;

    auto ts = Translator::GetSingleton();
    if (!ts) return;

    auto manager = EquipsetManager::GetSingleton();
    if (!manager) return;

    auto dataHandler = DataHandler::GetSingleton();
    if (!dataHandler) return;

    auto drawHelper = DrawHelper::GetSingleton();
    if (!drawHelper) return;

    auto equipment = EquipmentManager::GetSingleton();
    if (!equipment) return;

    auto& io = ImGui::GetIO();
    if (ImGui::IsKeyPressed(config->Gui.hotkey) && !ui->IsMenuOpen(RE::MainMenu::MENU_NAME) &&
        !ui->IsMenuOpen(RE::LoadingMenu::MENU_NAME)) {
        Toggle();
    }
    // if (ImGui::IsKeyPressed(config->Gui.hotkey)) {
    //     Toggle();
    // }
    io.MouseDrawCursor = show;
    io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
    io.ConfigDebugHighlightIdConflicts = false;

    if (!show) return;

    if (font) ImGui::PushFont(font);
    auto& style = ImGui::GetStyle();
    switch (static_cast<Config::GuiStyle>(config->Gui.style)) {
        case Config::GuiStyle::DARK:
            ImGui::StyleColorsDark();
            break;
        case Config::GuiStyle::LIGHT:
            ImGui::StyleColorsLight();
            break;
        case Config::GuiStyle::CLASSIC:
            ImGui::StyleColorsClassic();
            break;
    }
    style.WindowRounding = config->Gui.rounding;
    style.ChildRounding = config->Gui.rounding;
    style.FrameRounding = config->Gui.rounding;
    style.PopupRounding = config->Gui.rounding;
    style.ScrollbarRounding = config->Gui.rounding;
    style.GrabRounding = config->Gui.rounding;
    style.WindowBorderSize = config->Gui.windowBorder ? 1.0f : 0.0f;
    style.FrameBorderSize = config->Gui.frameBorder ? 1.0f : 0.0f;
    style.ScaleAllSizes(config->Gui.fontScaling);

    // Reload data whenever user changes 'Favorited only' option.
    static bool ShouldReloadData = false;
    if (ShouldReloadData != config->Settings.favorOnly) {
        ShouldReloadData = config->Settings.favorOnly;
        dataHandler->Init();
    }

    auto viewport = ImGui::GetMainViewport();
    if (!viewport) return;

    ImGui::SetNextWindowPos({0, 0}, ImGuiCond_Once);
    ImGui::SetNextWindowSize({viewport->Size.x / 3, viewport->Size.y}, ImGuiCond_Once);
    if (ImGui::Begin(fmt::format("UI-Integrated Hotkeys System {}",
                                 SKSE::PluginDeclaration::GetSingleton()->GetVersion().string())
                         .c_str(),
                     nullptr,
                     ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse |
                         ImGuiWindowFlags_MenuBar)) {
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu(C_TRANSLATE("_MENUBAR_FILE"))) {
                ImGui::PushID("EquipsetsFileMenu");
                ImGui::MenuItem(C_TRANSLATE("_TAB_EQUIPSETS"), NULL, false, false);
                ImGui::Separator();
                if (ImGui::MenuItem(C_TRANSLATE("_MENUBAR_SAVE"))) {
                    manager->ExportEquipsets();
                }
                if (ImGui::MenuItem(C_TRANSLATE("_MENUBAR_LOAD"))) {
                    manager->RemoveAllWidget();
                    manager->RemoveAll();
                    manager->ImportEquipsets();
                    manager->SyncSortOrder();
                    manager->CreateAllWidget();
                }
                ImGui::PopID();

                ImGui::MenuItem("##BLANK_EQUIP", NULL, false, false);

                ImGui::PushID("EquipmentFileMenu");
                ImGui::MenuItem(C_TRANSLATE("_TAB_EQUIPMENT"), NULL, false, false);
                ImGui::Separator();
                if (ImGui::MenuItem(C_TRANSLATE("_MENUBAR_SAVE"))) {
                    equipment->Save();
                }
                if (ImGui::MenuItem(C_TRANSLATE("_MENUBAR_LOAD"))) {
                    equipment->RemoveAllArmorWidget();
                    equipment->RemoveAllWeaponWidget();
                    equipment->RemoveAllShoutWidget();
                    equipment->Load();
                    equipment->CreateAllArmorWidget();
                    equipment->CreateAllWeaponWidget();
                    equipment->CreateAllShoutWidget();
                }
                ImGui::PopID();

                ImGui::MenuItem("##BLANK_CONFIG", NULL, false, false);

                ImGui::PushID("ConfigFileMenu");
                ImGui::MenuItem(C_TRANSLATE("_TAB_CONFIG"), NULL, false, false);
                ImGui::Separator();
                if (ImGui::MenuItem(C_TRANSLATE("_MENUBAR_SAVE"))) {
                    config->SaveConfig();
                }
                if (ImGui::MenuItem(C_TRANSLATE("_MENUBAR_LOAD"))) {
                    config->LoadConfig();
                    ts->Load();
                    dataHandler->Init();
                    GuiMenu::NotifyFontReload();
                }
                ImGui::PopID();

                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        ImGui::BeginChild("main", {0.f, -ImGui::GetFontSize() - 2.f});

        if (ImGui::BeginTabBar("##")) {
            if (ImGui::BeginTabItem(C_TRANSLATE("_TAB_EQUIPSETS"))) {
                auto newSize = ImGui::CalcTextSize(C_TRANSLATE("_NEW"));
                if (ImGui::Button(C_TRANSLATE("_NEW"), ImVec2(newSize.x + 30.0f, 0.0f))) {
                    ImGui::SetNextWindowSize({200, 230}, ImGuiCond_Once);
                    ImGui::OpenPopup(C_TRANSLATE("_SELECT_NEW_POPUP"));
                }
                ImGui::Separator();

                ImVec2 center = ImGui::GetMainViewport()->GetCenter();
                ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

                if (ImGui::BeginPopupModal(C_TRANSLATE("_SELECT_NEW_POPUP"), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                    bool shouldClose = false;

                    auto normalSize = ImGui::CalcTextSize(C_TRANSLATE("_SELECT_NEW_NORMAL"));
                    auto potionSize = ImGui::CalcTextSize(C_TRANSLATE("_SELECT_NEW_POTION"));
                    auto cycleSize = ImGui::CalcTextSize(C_TRANSLATE("_SELECT_NEW_CYCLE"));
                    auto cancelSize = ImGui::CalcTextSize(C_TRANSLATE("_CANCEL"));

                    std::vector<float> vecX = {normalSize.x, potionSize.x, cycleSize.x, cancelSize.x};
                    std::vector<float> vecY = {normalSize.y, potionSize.y, cycleSize.y, cancelSize.y};

                    auto compare = [](float a, float b) { return a > b; };

                    std::sort(vecX.begin(), vecX.end(), compare);
                    std::sort(vecY.begin(), vecY.end(), compare);
                    float sizeX = *vecX.begin() * 2.5f;
                    float sizeY = *vecY.begin() * 2.0f;

                    if (ImGui::Button(C_TRANSLATE("_SELECT_NEW_NORMAL"), ImVec2(sizeX, sizeY))) {
                        drawHelper->NotifyReload(true);
                        ImGui::OpenPopup(C_TRANSLATE("_SELECT_NEW_OPEN_NORMAL"));
                    }
                    ImGui::SetNextWindowSize({viewport->Size.x / 3, viewport->Size.y / 5 * 3}, ImGuiCond_Once);
                    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
                    if (ImGui::BeginPopupModal(C_TRANSLATE("_SELECT_NEW_OPEN_NORMAL"), NULL)) {
                        shouldClose = Draw::CreateNormal();
                        ImGui::EndPopup();
                    }

                    if (ImGui::Button(C_TRANSLATE("_SELECT_NEW_POTION"), ImVec2(sizeX, sizeY))) {
                        drawHelper->NotifyReload(true);
                        ImGui::OpenPopup(C_TRANSLATE("_SELECT_NEW_OPEN_POTION"));
                    }
                    ImGui::SetNextWindowSize({viewport->Size.x / 3, viewport->Size.y / 5 * 3}, ImGuiCond_Once);
                    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
                    if (ImGui::BeginPopupModal(C_TRANSLATE("_SELECT_NEW_OPEN_POTION"), NULL)) {
                        shouldClose = Draw::CreatePotion();
                        ImGui::EndPopup();
                    }

                    if (ImGui::Button(C_TRANSLATE("_SELECT_NEW_CYCLE"), ImVec2(sizeX, sizeY))) {
                        drawHelper->NotifyReload(true);
                        ImGui::OpenPopup(C_TRANSLATE("_SELECT_NEW_OPEN_CYCLE"));
                    }
                    ImGui::SetNextWindowSize({viewport->Size.x / 3, viewport->Size.y / 5 * 3}, ImGuiCond_Once);
                    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
                    if (ImGui::BeginPopupModal(C_TRANSLATE("_SELECT_NEW_OPEN_CYCLE"), NULL)) {
                        shouldClose = Draw::CreateCycle();
                        ImGui::EndPopup();
                    }

                    ImGui::Dummy(ImVec2(sizeX, sizeY));
                    ImGui::Separator();
                    if (ImGui::Button(C_TRANSLATE("_CANCEL"), ImVec2(sizeX, sizeY))) {
                        ImGui::CloseCurrentPopup();
                    }
                    if (shouldClose) {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }

                auto sort = static_cast<Config::SortType>(config->Settings.sort);
                std::vector<Equipset*> equipsetVec;
                if (sort == Config::SortType::CREATEASC) {
                    equipsetVec = manager->equipsetVec;
                } else if (sort == Config::SortType::CREATEDESC) {
                    auto compare = [](Equipset* _first, Equipset* _second) {
                        if (!_first || !_second) return false;

                        return _first->order > _second->order;
                    };
                    equipsetVec = manager->equipsetVec;
                    std::sort(equipsetVec.begin(), equipsetVec.end(), compare);
                } else if (sort == Config::SortType::NAMEASC) {
                    auto compare = [](Equipset* _first, Equipset* _second) {
                        if (!_first || !_second) return false;

                        return _first->name < _second->name;
                    };
                    equipsetVec = manager->equipsetVec;
                    std::sort(equipsetVec.begin(), equipsetVec.end(), compare);
                } else if (sort == Config::SortType::NAMEDESC) {
                    auto compare = [](Equipset* _first, Equipset* _second) {
                        if (!_first || !_second) return false;

                        return _first->name > _second->name;
                    };
                    equipsetVec = manager->equipsetVec;
                    std::sort(equipsetVec.begin(), equipsetVec.end(), compare);
                }
                for (int i = 0; i < equipsetVec.size(); i++) {
                    auto equipset = equipsetVec[i];
                    if (!equipset) continue;

                    if (current_opened.load() != i) {
                        ImGui::SetNextItemOpen(false);
                    }

                    if (ImGui::CollapsingHeader(equipset->name.c_str())) {
                        ImGui::Indent();
                        ImGui::PushID(i);

                        if (equipset->type == Equipset::TYPE::NORMAL) {
                            Draw::ShowNormal(static_cast<NormalSet*>(equipset), i);
                        } else if (equipset->type == Equipset::TYPE::POTION) {
                            Draw::ShowPotion(static_cast<PotionSet*>(equipset), i);
                        } else if (equipset->type == Equipset::TYPE::CYCLE) {
                            Draw::ShowCycle(static_cast<CycleSet*>(equipset), i);
                        }

                        ImGui::PopID();
                        ImGui::Unindent();
                    }
                }

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem(C_TRANSLATE("_TAB_EQUIPMENT"))) {
                DrawEquipment();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem(C_TRANSLATE("_TAB_CONFIG"))) {
                DrawConfig();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        ImGui::EndChild();
    } else {
        Toggle(false);
    }
    ImGui::End();

    ProcessWidgetDragging();

    if (font) ImGui::PopFont();
}

void GuiMenu::LoadFont() {
    if (!reload_font.load()) return;
    reload_font.store(false);

    if (!ImGui::GetCurrentContext()) return;

    auto& io = ImGui::GetIO();
    auto config = ConfigHandler::GetSingleton();
    std::filesystem::path path = "Data/SKSE/Plugins/UIHS/Fonts/" + config->Gui.fontPath;
    if (std::filesystem::is_regular_file(path) && ((path.extension() == ".ttf") || (path.extension() == ".otf"))) {
        ImVector<ImWchar> ranges;
        ImFontGlyphRangesBuilder builder;
        builder.AddRanges(io.Fonts->GetGlyphRangesDefault());
        if (config->Gui.language == (int)Config::LangType::CHINESE) {
            builder.AddRanges(io.Fonts->GetGlyphRangesChineseFull());
        } else if (config->Gui.language == (int)Config::LangType::JAPANESE) {
            builder.AddRanges(io.Fonts->GetGlyphRangesJapanese());
        } else if (config->Gui.language == (int)Config::LangType::KOREAN) {
            builder.AddRanges(io.Fonts->GetGlyphRangesKorean());
        } else if (config->Gui.language == (int)Config::LangType::RUSSIAN) {
            builder.AddRanges(io.Fonts->GetGlyphRangesCyrillic());
        } else if (config->Gui.language == (int)Config::LangType::THAI) {
            builder.AddRanges(io.Fonts->GetGlyphRangesThai());
        } else if (config->Gui.language == (int)Config::LangType::VIETNAMESE) {
            builder.AddRanges(io.Fonts->GetGlyphRangesVietnamese());
        }
        builder.BuildRanges(&ranges);

        io.Fonts->Clear();
        io.Fonts->AddFontDefault();
        font = io.Fonts->AddFontFromFileTTF(path.string().c_str(), config->Gui.fontSize, NULL, ranges.Data);

        std::filesystem::path iconPath = "Data/SKSE/Plugins/UIHS/Fonts/fa-solid-900.ttf";
        if (std::filesystem::is_regular_file(iconPath)) {
            static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_16_FA, 0};
            ImFontConfig icons_config;
            icons_config.MergeMode = true;
            icons_config.PixelSnapH = true;
            io.Fonts->AddFontFromFileTTF(iconPath.string().c_str(), config->Gui.fontSize, &icons_config, icons_ranges);
        }

        if (io.Fonts->Build()) {
            ImGui_ImplDX11_ReCreateFontsTexture();
            logger::info("Font loaded.");
            return;
        } else
            logger::error("Failed to build font {}", config->Gui.fontPath);
    } else {
        font = nullptr;
        logger::info("{} is not a font file", config->Gui.fontPath);
    }

    io.Fonts->Clear();
    io.Fonts->AddFontDefault();
    io.Fonts->Build();

    ImGui_ImplDX11_ReCreateFontsTexture();

    logger::info("Font loaded.");
}

void GuiMenu::DrawEquipment() {
    auto config = ConfigHandler::GetSingleton();
    if (!config) return;

    auto ts = Translator::GetSingleton();
    if (!ts) return;

    auto equipment = EquipmentManager::GetSingleton();
    if (!equipment) return;

    bool shouldReload = false;
    auto Reload = [&shouldReload]() { shouldReload = true; };

    auto DrawWidgetIconSection = [ts, Reload](WidgetIcon* _widget, int* _size = nullptr, int* _bgSize = nullptr) {
        if (ImGui::Checkbox(C_TRANSLATE("_WIDGET_ENABLE"), &_widget->enable)) {
            Reload();
        }
        if (_size) {
            if (Draw::SliderInt(C_TRANSLATE("_WIDGET_SIZE"), _size, 0, 200, "%d%%",
                                ImGuiSliderFlags_AlwaysClamp)) {
                if (_bgSize && *_bgSize > 0) {
                    *_bgSize = static_cast<int>(std::round((float)(*_size) * 2.625f));
                }
                Reload();
            }
        }
        if (Draw::SliderInt(C_TRANSLATE("_WIDGET_OFFSETX"), &_widget->offsetX, Config::icon_smin, Config::icon_smax,
                            "%d", ImGuiSliderFlags_AlwaysClamp)) {
            Reload();
        }
        if (Draw::SliderInt(C_TRANSLATE("_WIDGET_OFFSETY"), &_widget->offsetY, Config::icon_smin, Config::icon_smax,
                            "%d", ImGuiSliderFlags_AlwaysClamp)) {
            Reload();
        }
    };
    auto DrawWidgetTextSection = [ts, Reload](WidgetText* _widget, int* _fontSize = nullptr) {
        std::vector<std::string> align_items = {TRANSLATE("_ALIGN_LEFT"), TRANSLATE("_ALIGN_RIGHT"),
                                                TRANSLATE("_ALIGN_CENTER")};

        if (ImGui::Checkbox(C_TRANSLATE("_WIDGET_ENABLE"), &_widget->enable)) {
            Reload();
        }
        if (_fontSize) {
            if (Draw::SliderInt(C_TRANSLATE("_FONT_SIZE"), _fontSize, 0, 200, "%d%%",
                                ImGuiSliderFlags_AlwaysClamp)) {
                Reload();
            }
        }
        uint32_t* align = reinterpret_cast<uint32_t*>(&_widget->align);
        if (Draw::Combo(align_items, align, C_TRANSLATE("_WIDGET_ALIGN"))) {
            Reload();
        }
        if (Draw::SliderInt(C_TRANSLATE("_WIDGET_OFFSETX"), &_widget->offsetX, Config::text_smin, Config::text_smax,
                            "%d", ImGuiSliderFlags_AlwaysClamp)) {
            Reload();
        }
        if (Draw::SliderInt(C_TRANSLATE("_WIDGET_OFFSETY"), &_widget->offsetY, Config::text_smin, Config::text_smax,
                            "%d", ImGuiSliderFlags_AlwaysClamp)) {
            Reload();
        }
    };

    auto hudLabel = TRANSLATE("_WIDGET_HUD_DIAMOND");
    if (hudLabel == "_WIDGET_HUD_DIAMOND") hudLabel = "HUD Diamond (Bottom-Right)";
    if (ImGui::Checkbox(hudLabel.c_str(), &config->Widget.General.hudDiamondEnable)) {
        Reload();
    }

    auto hudLeftLabel = TRANSLATE("_WIDGET_HUD_DIAMOND_LEFT");
    if (hudLeftLabel == "_WIDGET_HUD_DIAMOND_LEFT") hudLeftLabel = "HUD Diamond Left (Left Hand)";
    if (ImGui::Checkbox(hudLeftLabel.c_str(), &config->Widget.General.hudDiamondLeftEnable)) {
        Reload();
    }

    auto hudArmorLabel = TRANSLATE("_WIDGET_HUD_ARMOR");
    if (hudArmorLabel == "_WIDGET_HUD_ARMOR") hudArmorLabel = "HUD Armor Stack (Left)";
    if (ImGui::Checkbox(hudArmorLabel.c_str(), &config->Widget.General.hudArmorEnable)) {
        Reload();
    }

    auto emptyLabel = TRANSLATE("_WIDGET_SHOW_EMPTY");
    if (emptyLabel == "_WIDGET_SHOW_EMPTY") emptyLabel = "Show Empty Slots (Unequipped)";
    if (ImGui::Checkbox(emptyLabel.c_str(), &config->Widget.General.showEmptySlots)) {
        Reload();
    }

    auto autoScaleLabel = TRANSLATE("_WIDGET_AUTO_SCALE");
    if (autoScaleLabel == "_WIDGET_AUTO_SCALE") autoScaleLabel = "Auto-Scale with Screen Resolution";
    if (ImGui::Checkbox(autoScaleLabel.c_str(), &config->Widget.General.autoResolutionScale)) {
        Reload();
    }

    auto resetLabel = TRANSLATE("_RESET_DEFAULTS");
    if (resetLabel == "_RESET_DEFAULTS") resetLabel = "Reset to Bottom-Right Defaults";
    if (ImGui::Button(resetLabel.c_str())) {
        equipment->ResetToDefaults();
        equipment->Save();
        Reload();
    }

    auto dragDropLabel = TRANSLATE("_WIDGET_DRAG_DROP");
    if (dragDropLabel == "_WIDGET_DRAG_DROP") dragDropLabel = "Mouse Drag & Drop (Move widgets with mouse)";
    ImGui::Checkbox(dragDropLabel.c_str(), &this->mouseDragEnabled);

    ImGui::Separator();

    if (ImGui::CollapsingHeader(C_TRANSLATE("_TAB_EQUIPMENT_ARMOR"))) {
        ImGui::Indent();

        auto autoStackLabel = TRANSLATE("_WIDGET_HUD_ARMOR_AUTO_STACK");
        if (autoStackLabel == "_WIDGET_HUD_ARMOR_AUTO_STACK") autoStackLabel = "Auto-Arrange Armor Stack (Multi-Column)";
        if (ImGui::Checkbox(autoStackLabel.c_str(), &config->Widget.General.hudArmorAutoStack)) {
            if (config->Widget.General.hudArmorAutoStack) {
                equipment->AutoArrangeArmorSlots();
                equipment->Save();
            }
            Reload();
        }
        ImGui::SameLine();
        auto arrangeBtnLabel = TRANSLATE("_AUTO_ARRANGE_ARMOR_BTN");
        if (arrangeBtnLabel == "_AUTO_ARRANGE_ARMOR_BTN") arrangeBtnLabel = "Auto-Arrange Now";
        if (ImGui::Button(arrangeBtnLabel.c_str())) {
            equipment->AutoArrangeArmorSlots();
            equipment->Save();
            Reload();
        }

        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
        if (Draw::SliderInt(C_TRANSLATE("_WIDGET_SIZE"), &config->Widget.Equipment.Armor.widgetSize, 0, 200, "%d%%",
                            ImGuiSliderFlags_AlwaysClamp)) {
            Reload();
        }
        if (Draw::SliderInt(C_TRANSLATE("_FONT_SIZE"), &config->Widget.Equipment.Armor.fontSize, 0, 200, "%d%%",
                            ImGuiSliderFlags_AlwaysClamp)) {
            Reload();
        }
        ImGui::PopItemWidth();

        static std::vector<ImVec2> groupSize(32, ImVec2(0.0f, 0.0f));
        static std::vector<ImVec2> groupLeftSize(32, ImVec2(0.0f, 0.0f));
        static std::vector<ImVec2> groupRightSize(32, ImVec2(0.0f, 0.0f));
        int maxArmorSlots = std::min(32, static_cast<int>(equipment->armor.size()));
        for (int i = 0; i < maxArmorSlots; i++) {
            auto desc = GetSlotDescription(i + 30);
            auto treeName = desc.empty() ? fmt::format("Slot{}", i + 30) : fmt::format("Slot{} ({})", i + 30, desc);
            if (ImGui::TreeNode(treeName.c_str())) {
                ImGui::PushID(i);

                auto groupLabel = fmt::format("{}  {}", ICON_FA_IMAGE, TRANSLATE("_TAB_CONFIG_WIDGET"));
                Draw::BeginGroupPanel(groupLabel.c_str(), ImVec2(-FLT_MIN, 0.0f), ImVec2(15.0f, 10.0f));
                {
                    ImGui::BeginChild("LeftRegion", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, groupSize[i].y),
                                      false);
                    {
                        ImGui::BeginGroup();
                        {
                            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                            if (ImGui::TreeNode(C_TRANSLATE("_WIDGET_ICON"))) {
                                bool oldIconEnable = equipment->armor[i].widgetIcon.enable;
                                DrawWidgetIconSection(&equipment->armor[i].widgetIcon, &config->Widget.Equipment.Armor.widgetSize, &config->Widget.Equipment.Armor.bgSize);
                                if (equipment->armor[i].widgetIcon.enable != oldIconEnable) {
                                    equipment->armor[i].widgetName.enable = equipment->armor[i].widgetIcon.enable;
                                    if (config->Widget.General.hudArmorAutoStack) {
                                        equipment->AutoArrangeArmorSlots();
                                        equipment->Save();
                                    }
                                    Reload();
                                }
                                ImGui::TreePop();
                            }
                        }
                        ImGui::EndGroup();
                        auto size = ImGui::GetItemRectSize();
                        if (size.y != 0.0f) groupLeftSize[i] = size;
                    }
                    ImGui::EndChild();
                    ImGui::SameLine();
                    ImGui::BeginChild("RightRegion", ImVec2(0.0f, groupSize[i].y), false);
                    {
                        ImGui::BeginGroup();
                        {
                            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                            if (ImGui::TreeNode(C_TRANSLATE("_WIDGET_NAME"))) {
                                DrawWidgetTextSection(&equipment->armor[i].widgetName, &config->Widget.Equipment.Armor.fontSize);
                                ImGui::TreePop();
                            }
                        }
                        ImGui::EndGroup();
                        auto size = ImGui::GetItemRectSize();
                        if (size.y != 0.0f) groupRightSize[i] = size;
                    }
                    ImGui::EndChild();

                    groupSize[i] = groupLeftSize[i].y > groupRightSize[i].y ? groupLeftSize[i] : groupRightSize[i];
                }
                Draw::EndGroupPanel();

                ImGui::PopID();
                ImGui::TreePop();
            }
        }

        ImGui::Unindent();
    }

    if (ImGui::CollapsingHeader(C_TRANSLATE("_TAB_EQUIPMENT_WEAPON"))) {
        ImGui::Indent();

        if (ImGui::TreeNode(C_TRANSLATE("_WEAPON_LEFTHAND"))) {
            static auto groupSize = ImVec2(0.0f, 0.0f);
            static auto groupLeftSize = ImVec2(0.0f, 0.0f);
            static auto groupRightSize = ImVec2(0.0f, 0.0f);
            auto groupLabel = fmt::format("{}  {}", ICON_FA_IMAGE, TRANSLATE("_TAB_CONFIG_WIDGET"));
            Draw::BeginGroupPanel(groupLabel.c_str(), ImVec2(-FLT_MIN, 0.0f), ImVec2(15.0f, 10.0f));
            {
                ImGui::BeginChild("LeftRegion", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, groupSize.y), false);
                {
                    ImGui::BeginGroup();
                    {
                        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                        if (ImGui::TreeNode(C_TRANSLATE("_WIDGET_ICON"))) {
                            DrawWidgetIconSection(&equipment->lefthand.widgetIcon, &config->Widget.Equipment.Weapon.widgetSize, &config->Widget.Equipment.Weapon.bgSize);
                            ImGui::TreePop();
                        }
                    }
                    ImGui::EndGroup();
                    auto size = ImGui::GetItemRectSize();
                    if (size.y != 0.0f) groupLeftSize = size;
                }
                ImGui::EndChild();
                ImGui::SameLine();
                ImGui::BeginChild("RightRegion", ImVec2(0.0f, groupSize.y), false);
                {
                    ImGui::BeginGroup();
                    {
                        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                        if (ImGui::TreeNode(C_TRANSLATE("_WIDGET_NAME"))) {
                            DrawWidgetTextSection(&equipment->lefthand.widgetName, &config->Widget.Equipment.Weapon.fontSize);
                            ImGui::TreePop();
                        }
                    }
                    ImGui::EndGroup();
                    auto size = ImGui::GetItemRectSize();
                    if (size.y != 0.0f) groupRightSize = size;
                }
                ImGui::EndChild();

                groupSize = groupLeftSize.y > groupRightSize.y ? groupLeftSize : groupRightSize;
            }
            Draw::EndGroupPanel();
            ImGui::TreePop();
        }
        if (ImGui::TreeNode(C_TRANSLATE("_WEAPON_RIGHTHAND"))) {
            static auto groupSize = ImVec2(0.0f, 0.0f);
            static auto groupLeftSize = ImVec2(0.0f, 0.0f);
            static auto groupRightSize = ImVec2(0.0f, 0.0f);
            auto groupLabel = fmt::format("{}  {}", ICON_FA_IMAGE, TRANSLATE("_TAB_CONFIG_WIDGET"));
            Draw::BeginGroupPanel(groupLabel.c_str(), ImVec2(-FLT_MIN, 0.0f), ImVec2(15.0f, 10.0f));
            {
                ImGui::BeginChild("LeftRegion", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, groupSize.y), false);
                {
                    ImGui::BeginGroup();
                    {
                        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                        if (ImGui::TreeNode(C_TRANSLATE("_WIDGET_ICON"))) {
                            DrawWidgetIconSection(&equipment->righthand.widgetIcon, &config->Widget.Equipment.Weapon.widgetSize, &config->Widget.Equipment.Weapon.bgSize);
                            ImGui::TreePop();
                        }
                    }
                    ImGui::EndGroup();
                    auto size = ImGui::GetItemRectSize();
                    if (size.y != 0.0f) groupLeftSize = size;
                }
                ImGui::EndChild();
                ImGui::SameLine();
                ImGui::BeginChild("RightRegion", ImVec2(0.0f, groupSize.y), false);
                {
                    ImGui::BeginGroup();
                    {
                        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                        if (ImGui::TreeNode(C_TRANSLATE("_WIDGET_NAME"))) {
                            DrawWidgetTextSection(&equipment->righthand.widgetName, &config->Widget.Equipment.Weapon.fontSize);
                            ImGui::TreePop();
                        }
                    }
                    ImGui::EndGroup();
                    auto size = ImGui::GetItemRectSize();
                    if (size.y != 0.0f) groupRightSize = size;
                }
                ImGui::EndChild();

                groupSize = groupLeftSize.y > groupRightSize.y ? groupLeftSize : groupRightSize;
            }
            Draw::EndGroupPanel();
            ImGui::TreePop();
        }

        ImGui::Unindent();
    }

    if (ImGui::CollapsingHeader(C_TRANSLATE("_TAB_EQUIPMENT_SHOUT"))) {
        ImGui::Indent();

        static auto groupSize = ImVec2(0.0f, 0.0f);
        static auto groupLeftSize = ImVec2(0.0f, 0.0f);
        static auto groupRightSize = ImVec2(0.0f, 0.0f);
        auto groupLabel = fmt::format("{}  {}", ICON_FA_IMAGE, TRANSLATE("_TAB_CONFIG_WIDGET"));
        Draw::BeginGroupPanel(groupLabel.c_str(), ImVec2(-FLT_MIN, 0.0f), ImVec2(15.0f, 10.0f));
        {
            ImGui::BeginChild("LeftRegion", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, groupSize.y), false);
            {
                ImGui::BeginGroup();
                {
                    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                    if (ImGui::TreeNode(C_TRANSLATE("_WIDGET_ICON"))) {
                        DrawWidgetIconSection(&equipment->shout.widgetIcon, &config->Widget.Equipment.Shout.widgetSize, &config->Widget.Equipment.Shout.bgSize);
                        ImGui::TreePop();
                    }
                }
                ImGui::EndGroup();
                auto size = ImGui::GetItemRectSize();
                if (size.y != 0.0f) groupLeftSize = size;
            }
            ImGui::EndChild();
            ImGui::SameLine();
            ImGui::BeginChild("RightRegion", ImVec2(0.0f, groupSize.y), false);
            {
                ImGui::BeginGroup();
                {
                    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                    if (ImGui::TreeNode(C_TRANSLATE("_WIDGET_NAME"))) {
                        DrawWidgetTextSection(&equipment->shout.widgetName, &config->Widget.Equipment.Shout.fontSize);
                        ImGui::TreePop();
                    }
                }
                ImGui::EndGroup();
                auto size = ImGui::GetItemRectSize();
                if (size.y != 0.0f) groupRightSize = size;
            }
            ImGui::EndChild();

            groupSize = groupLeftSize.y > groupRightSize.y ? groupLeftSize : groupRightSize;
        }
        Draw::EndGroupPanel();
        ImGui::Unindent();
    }

    if (shouldReload) {
        auto equipment = EquipmentManager::GetSingleton();
        if (!equipment) return;

        auto equipset = EquipsetManager::GetSingleton();
        if (!equipset) return;

        auto widgetHandler = WidgetHandler::GetSingleton();
        if (!widgetHandler) return;

        widgetHandler->CloseWidgetMenu();
        widgetHandler->OpenWidgetMenu();
        equipset->RemoveAllWidget();
        equipment->RemoveAllArmorWidget();
        equipment->RemoveAllWeaponWidget();
        equipment->RemoveAllShoutWidget();
        equipment->CreateAllArmorWidget();
        equipment->CreateAllWeaponWidget();
        equipment->CreateAllShoutWidget();
        equipset->CreateAllWidget();
    }
}

void GuiMenu::DrawConfig() {
    auto config = ConfigHandler::GetSingleton();
    if (!config) return;

    auto ts = Translator::GetSingleton();
    if (!ts) return;

    bool shouldReload = false;
    auto Reload = [&shouldReload]() { shouldReload = true; };

    auto DrawWidgetSection = [ts, Reload](ConfigHandler::WidgetBase* _widget) {
        if (Draw::ComboBackground(&_widget->bgType, C_TRANSLATE("_BACKGROUND_TYPE"))) {
            Reload();
        }
        if (Draw::SliderInt(C_TRANSLATE("_BACKGROUND_SIZE"), &_widget->bgSize, 0, 200, "%d%%",
                            ImGuiSliderFlags_AlwaysClamp)) {
            Reload();
        }
        if (Draw::SliderInt(C_TRANSLATE("_BACKGROUND_ALPHA"), &_widget->bgAlpha, 0, 100, "%d%%",
                            ImGuiSliderFlags_AlwaysClamp)) {
            Reload();
        }
        if (Draw::SliderInt(C_TRANSLATE("_WIDGET_SIZE"), &_widget->widgetSize, 0, 200, "%d%%",
                            ImGuiSliderFlags_AlwaysClamp)) {
            Reload();
        }
        if (Draw::SliderInt(C_TRANSLATE("_FONT_SIZE"), &_widget->fontSize, 0, 200, "%d%%",
                            ImGuiSliderFlags_AlwaysClamp)) {
            Reload();
        }
        if (ImGui::Checkbox(C_TRANSLATE("_FONT_SHADOW"), &_widget->fontShadow)) {
            Reload();
        }
    };

    static auto groupWidgetSize = ImVec2(0.0f, 0.0f);
    static auto groupWidgetLeftSize = ImVec2(0.0f, 0.0f);
    static auto groupWidgetRightSize = ImVec2(0.0f, 0.0f);

    ImGui::Text(" ");

    auto groupWidgetLabel = fmt::format("{}  {}", ICON_FA_IMAGE, TRANSLATE("_TAB_CONFIG_WIDGET"));
    Draw::BeginGroupPanel(groupWidgetLabel.c_str(), ImVec2(-FLT_MIN, 0.0f), ImVec2(15.0f, 10.0f));
    {
        ImGui::BeginChild("WidgetRegionLeft", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, groupWidgetSize.y),
                          false);
        {
            ImGui::BeginGroup();
            {
                ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
                if (ImGui::TreeNode(C_TRANSLATE("_TAB_CONFIG_WIDGET_GENERAL"))) {
                    if (Draw::Combo(config->fontVec, &config->Widget.General.font,
                                    C_TRANSLATE("_TAB_CONFIG_WIDGET_GENERAL_FONT"))) {
                        Reload();
                    }

                    std::vector<std::string> displayVec = {TRANSLATE("_DISPLAYMODE_ALWAYS"),
                                                           TRANSLATE("_DISPLAYMODE_INCOMBAT")};
                    Draw::Combo(displayVec, &config->Widget.General.displayMode,
                                C_TRANSLATE("_TAB_CONFIG_WIDGET_GENERAL_DISPLAY"));

                    std::vector<std::string> animVec = {TRANSLATE("_ANIMATIONTYPE_FADE"),
                                                        TRANSLATE("_ANIMATIONTYPE_INSTANT")};
                    Draw::Combo(animVec, &config->Widget.General.animType,
                                C_TRANSLATE("_TAB_CONFIG_WIDGET_GENERAL_ANIM"));

                    auto msg = "%.1f" + TRANSLATE("_TIMESECOND");
                    Draw::SliderFloat(C_TRANSLATE("_TAB_CONFIG_WIDGET_GENERAL_ANIMDELAY"),
                                      &config->Widget.General.animDelay, 1.0f, 5.0f, msg.c_str(),
                                      ImGuiSliderFlags_AlwaysClamp);

                    // Theme Preset Selector
                    std::vector<std::string> themePresets = {
                        TRANSLATE("_THEME_PRESET_SELECT"),
                        TRANSLATE("_THEME_PRESET_NORDIC"),
                        TRANSLATE("_THEME_PRESET_CELTIC"),
                        TRANSLATE("_THEME_PRESET_MINIMAL"),
                        TRANSLATE("_THEME_PRESET_COMPASS"),
                        TRANSLATE("_THEME_PRESET_ARCANE"),
                        TRANSLATE("_THEME_PRESET_GOTHIC"),
                        TRANSLATE("_THEME_PRESET_DRAGON"),
                        TRANSLATE("_THEME_PRESET_OBLIVION"),
                        TRANSLATE("_THEME_PRESET_FALMER"),
                        TRANSLATE("_THEME_PRESET_SNOWELF"),
                        TRANSLATE("_THEME_PRESET_HIGHELF"),
                        TRANSLATE("_THEME_PRESET_DWEMER"),
                        TRANSLATE("_THEME_PRESET_DUNMER"),
                        TRANSLATE("_THEME_PRESET_NIGHTINGALE"),
                        TRANSLATE("_THEME_PRESET_CLASSIC_DIAMOND"),
                        TRANSLATE("_THEME_PRESET_CLASSIC_SQUARE"),
                        TRANSLATE("_THEME_PRESET_NONE")
                    };
                    static uint32_t currentThemePreset = 0;
                    if (Draw::Combo(themePresets, &currentThemePreset, C_TRANSLATE("_THEME_PRESET_LABEL"))) {
                        auto ApplyTheme = [&](const std::string& diamondBg, const std::string& squareBg, uint32_t armorAlpha = 100) {
                            config->Widget.Equipment.Weapon.bgType = diamondBg;
                            config->Widget.Equipment.Shout.bgType = diamondBg;
                            config->Widget.Equipment.Armor.bgType = squareBg;
                            config->Widget.Equipment.Armor.bgAlpha = armorAlpha;
                            config->Widget.Equipset.Normal.bgType = diamondBg;
                            config->Widget.Equipset.Potion.bgType = diamondBg;
                            config->Widget.Equipset.Cycle.bgType = diamondBg;
                            Reload();
                        };

                        switch (currentThemePreset) {
                            case 1:
                                ApplyTheme("_BG_NORDIC_DIAMOND", "_BG_NORDIC_SQUARE");
                                break;
                            case 2:
                                ApplyTheme("_BG_CELTIC_DIAMOND", "_BG_CELTIC_SQUARE");
                                break;
                            case 3:
                                ApplyTheme("_BG_MINIMAL_DIAMOND", "_BG_MINIMAL_SQUARE");
                                break;
                            case 4:
                                ApplyTheme("_BG_COMPASS", "_BG_COMPASS");
                                break;
                            case 5:
                                ApplyTheme("_BG_ARCANE_DIAMOND", "_BG_ARCANE_CIRCLE");
                                break;
                            case 6:
                                ApplyTheme("_BG_GOTHIC_DIAMOND", "_BG_GOTHIC_SQUARE");
                                break;
                            case 7:
                                ApplyTheme("_BG_DRAGON", "_BG_NORDIC_SQUARE");
                                break;
                            case 8:
                                ApplyTheme("_BG_OBLIVION_DIAMOND", "_BG_OBLIVION_CIRCLE");
                                break;
                            case 9:
                                ApplyTheme("_BG_FALMER_DIAMOND", "_BG_FALMER_SQUARE");
                                break;
                            case 10:
                                ApplyTheme("_BG_SNOWELF_DIAMOND", "_BG_SNOWELF_SQUARE");
                                break;
                            case 11:
                                ApplyTheme("_BG_HIGHELF_DIAMOND", "_BG_HIGHELF_SQUARE");
                                break;
                            case 12:
                                ApplyTheme("_BG_DWEMER_DIAMOND", "_BG_DWEMER_SQUARE");
                                break;
                            case 13:
                                ApplyTheme("_BG_DUNMER_DIAMOND", "_BG_DUNMER_SQUARE");
                                break;
                            case 14:
                                ApplyTheme("_BG_NIGHTINGALE_DIAMOND", "_BG_NIGHTINGALE_SQUARE");
                                break;
                            case 15:
                                ApplyTheme("_BACKGROUND4", "_BACKGROUND4");
                                break;
                            case 16:
                                ApplyTheme("_BACKGROUND2", "_BACKGROUND2");
                                break;
                            case 17:
                                ApplyTheme("_NONE", "_NONE", 0);
                                break;
                            default:
                                break;
                        }
                    }

                    auto hudLabel = TRANSLATE("_WIDGET_HUD_DIAMOND");
                    if (hudLabel == "_WIDGET_HUD_DIAMOND") hudLabel = "HUD Diamond (Bottom-Right)";
                    if (ImGui::Checkbox(hudLabel.c_str(), &config->Widget.General.hudDiamondEnable)) {
                        Reload();
                    }

                    auto hudLeftLabel = TRANSLATE("_WIDGET_HUD_DIAMOND_LEFT");
                    if (hudLeftLabel == "_WIDGET_HUD_DIAMOND_LEFT") hudLeftLabel = "HUD Diamond Left (Left Hand)";
                    if (ImGui::Checkbox(hudLeftLabel.c_str(), &config->Widget.General.hudDiamondLeftEnable)) {
                        Reload();
                    }

                    auto hudArmorLabel = TRANSLATE("_WIDGET_HUD_ARMOR");
                    if (hudArmorLabel == "_WIDGET_HUD_ARMOR") hudArmorLabel = "HUD Armor Stack (Left)";
                    if (ImGui::Checkbox(hudArmorLabel.c_str(), &config->Widget.General.hudArmorEnable)) {
                        Reload();
                    }

                    auto autoStackLabel = TRANSLATE("_WIDGET_HUD_ARMOR_AUTO_STACK");
                    if (autoStackLabel == "_WIDGET_HUD_ARMOR_AUTO_STACK") autoStackLabel = "Auto-Arrange Armor Stack (Multi-Column)";
                    if (ImGui::Checkbox(autoStackLabel.c_str(), &config->Widget.General.hudArmorAutoStack)) {
                        Reload();
                    }

                    auto emptyLabel = TRANSLATE("_WIDGET_SHOW_EMPTY");
                    if (emptyLabel == "_WIDGET_SHOW_EMPTY") emptyLabel = "Show Empty Slots (Unequipped)";
                    if (ImGui::Checkbox(emptyLabel.c_str(), &config->Widget.General.showEmptySlots)) {
                        Reload();
                    }

                    auto autoScaleLabel = TRANSLATE("_WIDGET_AUTO_SCALE");
                    if (autoScaleLabel == "_WIDGET_AUTO_SCALE") autoScaleLabel = "Auto-Scale with Screen Resolution";
                    if (ImGui::Checkbox(autoScaleLabel.c_str(), &config->Widget.General.autoResolutionScale)) {
                        Reload();
                    }

                    ImGui::TreePop();
                }
                ImGui::PopItemWidth();
            }
            ImGui::EndGroup();
            auto size = ImGui::GetItemRectSize();
            if (size.y != 0.0f) groupWidgetLeftSize = size;
        }
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("WidgetRegionRight", ImVec2(0.0f, groupWidgetSize.y), false);
        {
            ImGui::BeginGroup();
            {
                ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
                ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                if (ImGui::TreeNode(C_TRANSLATE("_TAB_EQUIPSETS"))) {
                    if (ImGui::TreeNode(C_TRANSLATE("_TAB_EQUIPSETS_NORMAL"))) {
                        DrawWidgetSection(&config->Widget.Equipset.Normal);
                        ImGui::TreePop();
                    }
                    if (ImGui::TreeNode(C_TRANSLATE("_TAB_EQUIPSETS_POTION"))) {
                        DrawWidgetSection(&config->Widget.Equipset.Potion);
                        ImGui::TreePop();
                    }
                    if (ImGui::TreeNode(C_TRANSLATE("_TAB_EQUIPSETS_CYCLE"))) {
                        DrawWidgetSection(&config->Widget.Equipset.Cycle);
                        ImGui::TreePop();
                    }
                    ImGui::TreePop();
                }
                ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                if (ImGui::TreeNode(C_TRANSLATE("_TAB_EQUIPMENT"))) {
                    if (ImGui::TreeNode(C_TRANSLATE("_TAB_EQUIPMENT_ARMOR"))) {
                        DrawWidgetSection(&config->Widget.Equipment.Armor);
                        ImGui::TreePop();
                    }
                    if (ImGui::TreeNode(C_TRANSLATE("_TAB_EQUIPMENT_WEAPON"))) {
                        DrawWidgetSection(&config->Widget.Equipment.Weapon);
                        ImGui::TreePop();
                    }
                    if (ImGui::TreeNode(C_TRANSLATE("_TAB_EQUIPMENT_SHOUT"))) {
                        DrawWidgetSection(&config->Widget.Equipment.Shout);
                        ImGui::TreePop();
                    }
                    ImGui::TreePop();
                }
                ImGui::PopItemWidth();
            }
            ImGui::EndGroup();
            auto size = ImGui::GetItemRectSize();
            if (size.y != 0.0f) groupWidgetRightSize = size;
        }
        ImGui::EndChild();
    }
    Draw::EndGroupPanel();
    groupWidgetSize = groupWidgetLeftSize.y > groupWidgetRightSize.y ? groupWidgetLeftSize : groupWidgetRightSize;

    ImGui::Text(" ");

    static auto settingsSize = ImVec2(0.0f, 0.0f);
    auto groupSettingLabel = fmt::format("{}  {}", ICON_FA_COG, TRANSLATE("_TAB_CONFIG_SETTINGS"));
    Draw::BeginGroupPanel(groupSettingLabel.c_str(), ImVec2(-FLT_MIN, 0.0f), ImVec2(15.0f, 10.0f));
    {
        ImGui::BeginChild("SettingsRegionLeft", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, settingsSize.y), false);
        {
            ImGui::BeginGroup();
            {
                Draw::InputButton(&config->Settings.modifier1, "Modifier1", TRANSLATE("_EDIT"),
                                  TRANSLATE("_MODIFIER1"));
                Draw::InputButton(&config->Settings.modifier2, "Modifier2", TRANSLATE("_EDIT"),
                                  TRANSLATE("_MODIFIER2"));
                Draw::InputButton(&config->Settings.modifier3, "Modifier3", TRANSLATE("_EDIT"),
                                  TRANSLATE("_MODIFIER3"));
            }
            ImGui::EndGroup();
            auto size = ImGui::GetItemRectSize();
            if (size.y != 0.0f) settingsSize = size;
        }
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("SettingsRegionRight", ImVec2(0.0f, settingsSize.y), false);
        {
            ImGui::BeginGroup();
            {
                std::vector<std::string> items = {
                    TRANSLATE("_TAB_CONFIG_SETTINGS_SORT_CREATEASC"), TRANSLATE("_TAB_CONFIG_SETTINGS_SORT_CREATEDESC"),
                    TRANSLATE("_TAB_CONFIG_SETTINGS_SORT_NAMEASC"), TRANSLATE("_TAB_CONFIG_SETTINGS_SORT_NAMEDESC")};
                Draw::Combo(items, &config->Settings.sort, C_TRANSLATE("_TAB_CONFIG_SETTINGS_SORTORDER"));
                ImGui::Checkbox(C_TRANSLATE("_TAB_CONFIG_SETTINGS_FAVOR"), &config->Settings.favorOnly);
            }
            ImGui::EndGroup();
        }
        ImGui::EndChild();
    }
    Draw::EndGroupPanel();

    ImGui::Text(" ");

    static auto guiSize = ImVec2(0.0f, 0.0f);
    auto groupGuiLabel = fmt::format("{}  {}", ICON_FA_DESKTOP, TRANSLATE("_TAB_CONFIG_GUI"));
    Draw::BeginGroupPanel(groupGuiLabel.c_str(), ImVec2(-FLT_MIN, 0.0f), ImVec2(15.0f, 10.0f));
    {
        ImGui::BeginChild("GuiRegionLeft", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, guiSize.y), false);
        {
            ImGui::BeginGroup();
            {
                auto hotkey = reinterpret_cast<uint32_t*>(&config->Gui.hotkey);
                if (hotkey)
                    Draw::InputButton(hotkey, "GuiHotkey", TRANSLATE("_EDIT"), TRANSLATE("_TAB_CONFIG_GUI_HOTKEY"));
                ImGui::Checkbox(C_TRANSLATE("_TAB_CONFIG_GUI_WINDOWBORDER"), &config->Gui.windowBorder);
                ImGui::Checkbox(C_TRANSLATE("_TAB_CONFIG_GUI_FRAMEBORDER"), &config->Gui.frameBorder);
            }
            ImGui::EndGroup();
        }
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("GuiRegionRight", ImVec2(0.0f, guiSize.y), false);
        {
            ImGui::BeginGroup();
            {
                {
                    std::vector<std::string> items = {"Chinese", "Czech",    "English",   "French", "German",
                                                      "Italian", "Japanese", "Korean",    "Polish", "Russian",
                                                      "Spanish", "Thai",     "Vietnamese"};
                    if (Draw::Combo(items, &config->Gui.language, TRANSLATE("_TAB_CONFIG_GUI_LANGUAGE"))) {
                        reload_font.store(true);
                        Translator::GetSingleton()->Load();
                        DataHandler::GetSingleton()->Init();
                    }
                }
                {
                    std::vector<std::string> items = {TRANSLATE("_STYLE_DARK"), TRANSLATE("_STYLE_LIGHT"),
                                                      TRANSLATE("_STYLE_CLASSIC")};
                    Draw::Combo(items, &config->Gui.style, TRANSLATE("_TAB_CONFIG_GUI_STYLE"));
                }
                Draw::SliderFloat(TRANSLATE("_TAB_CONFIG_GUI_ROUNDING"), &config->Gui.rounding, 0.0f, 12.0f, "%.1f",
                                  ImGuiSliderFlags_AlwaysClamp);
                ImGui::InputFloat(C_TRANSLATE("_FONT_SIZE"), &config->Gui.fontSize, 1.0f, 0.0f, "%.0f");
                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    if (config->Gui.fontSize < 13.0f) {
                        config->Gui.fontSize = 13.0f;
                    } else if (config->Gui.fontSize > 64.0f) {
                        config->Gui.fontSize = 64.0f;
                    }
                    reload_font.store(true);
                }
            }
            ImGui::EndGroup();
            auto size = ImGui::GetItemRectSize();
            if (size.y != 0.0f) guiSize = size;
        }
        ImGui::EndChild();
    }
    Draw::EndGroupPanel();

    if (shouldReload) {
        auto equipment = EquipmentManager::GetSingleton();
        if (!equipment) return;

        auto equipset = EquipsetManager::GetSingleton();
        if (!equipset) return;

        auto widgetHandler = WidgetHandler::GetSingleton();
        if (!widgetHandler) return;

        widgetHandler->CloseWidgetMenu();
        widgetHandler->OpenWidgetMenu();
        equipset->RemoveAllWidget();
        equipment->RemoveAllArmorWidget();
        equipment->RemoveAllWeaponWidget();
        equipment->RemoveAllShoutWidget();
        equipment->CreateAllArmorWidget();
        equipment->CreateAllWeaponWidget();
        equipment->CreateAllShoutWidget();
        equipset->CreateAllWidget();
    }
}

void GuiMenu::ProcessWidgetDragging() {
    if (!this->show || !this->mouseDragEnabled) return;

    auto config = ConfigHandler::GetSingleton();
    if (!config) return;

    auto equipment = EquipmentManager::GetSingleton();
    if (!equipment) return;

    auto resInfo = Utility::GetResolutionInfo();
    float S = resInfo.screenHeight / 720.0f;
    if (S <= 0.0f) S = 1.0f;

    auto& io = ImGui::GetIO();
    float mouseStageX = io.MousePos.x / S;
    float mouseStageY = io.MousePos.y / S;

    enum DragTarget {
        NONE = 0,
        DIAMOND_CLUSTER,
        LEFTHAND,
        LEFTHAND_TEXT,
        RIGHTHAND,
        RIGHTHAND_TEXT,
        SHOUT,
        SHOUT_TEXT,
        ARMOR_STACK,
        ARMOR_SLOT,
        ARMOR_SLOT_TEXT
    };

    static DragTarget currentDragTarget = NONE;
    static int draggedArmorSlotIndex = -1;
    static float dragStartMouseStageX = 0.0f;
    static float dragStartMouseStageY = 0.0f;
    static int origLefthandX = 0, origLefthandY = 0;
    static int origLefthandNameX = 0, origLefthandNameY = 0;
    static int origRighthandX = 0, origRighthandY = 0;
    static int origRighthandNameX = 0, origRighthandNameY = 0;
    static int origShoutX = 0, origShoutY = 0;
    static int origShoutNameX = 0, origShoutNameY = 0;
    static int origArmorBaseX = 0, origArmorBaseY = 0;
    static int origSlotX = 0, origSlotY = 0;
    static int origSlotNameX = 0, origSlotNameY = 0;

    auto drawList = ImGui::GetForegroundDrawList();

    auto GetTextBounds = [&](int iconX, int iconY, int nameX, int nameY, WidgetText::ALIGN_TYPE align, const std::string& text, int fontSizePercent) -> std::pair<ImVec2, ImVec2> {
        float tX = static_cast<float>(iconX + nameX) * S;
        float tY = static_cast<float>(iconY + nameY) * S;
        float fontScale = (fontSizePercent > 0 ? static_cast<float>(fontSizePercent) : 100.0f) / 100.0f;
        float textH = 22.0f * S * fontScale;
        float charW = 8.5f * S * fontScale;
        float textW = std::max(60.0f * S, static_cast<float>(text.length()) * charW);

        float minX = 0.0f, maxX = 0.0f;
        if (align == WidgetText::ALIGN_TYPE::CENTER) {
            minX = tX - textW * 0.5f - 6.0f * S;
            maxX = tX + textW * 0.5f + 6.0f * S;
        } else if (align == WidgetText::ALIGN_TYPE::RIGHT) {
            minX = tX - textW - 6.0f * S;
            maxX = tX + 6.0f * S;
        } else { // LEFT
            minX = tX - 6.0f * S;
            maxX = tX + textW + 6.0f * S;
        }
        float minY = tY - 3.0f * S;
        float maxY = tY + textH + 3.0f * S;

        return { ImVec2(minX, minY), ImVec2(maxX, maxY) };
    };

    auto IsInside = [](const ImVec2& pt, const ImVec2& minPt, const ImVec2& maxPt) -> bool {
        return pt.x >= minPt.x && pt.x <= maxPt.x && pt.y >= minPt.y && pt.y <= maxPt.y;
    };

    auto BoxDistance = [](const ImVec2& pt, const ImVec2& minPt, const ImVec2& maxPt) -> float {
        float cx = (minPt.x + maxPt.x) * 0.5f;
        float cy = (minPt.y + maxPt.y) * 0.5f;
        return std::hypot(pt.x - cx, pt.y - cy);
    };

    // 1. If currently dragging:
    if (currentDragTarget != NONE) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);

        int deltaX = static_cast<int>(std::round(mouseStageX - dragStartMouseStageX));
        int deltaY = static_cast<int>(std::round(mouseStageY - dragStartMouseStageY));

        std::string dragLabel = "";

        if (currentDragTarget == DIAMOND_CLUSTER) {
            equipment->lefthand.widgetIcon.offsetX = origLefthandX + deltaX;
            equipment->lefthand.widgetIcon.offsetY = origLefthandY + deltaY;
            equipment->righthand.widgetIcon.offsetX = origRighthandX + deltaX;
            equipment->righthand.widgetIcon.offsetY = origRighthandY + deltaY;
            equipment->shout.widgetIcon.offsetX = origShoutX + deltaX;
            equipment->shout.widgetIcon.offsetY = origShoutY + deltaY;

            equipment->CreateAllWeaponWidget();
            equipment->CreateAllShoutWidget();

            dragLabel = fmt::format("HUD Diamond (X: {}, Y: {})", equipment->shout.widgetIcon.offsetX, equipment->lefthand.widgetIcon.offsetY);

            ImVec2 clusterCenter(equipment->shout.widgetIcon.offsetX * S, equipment->lefthand.widgetIcon.offsetY * S);
            drawList->AddCircle(clusterCenter, 45.0f * S, IM_COL32(255, 215, 0, 220), 32, 2.5f);
        } else if (currentDragTarget == LEFTHAND) {
            equipment->lefthand.widgetIcon.offsetX = origLefthandX + deltaX;
            equipment->lefthand.widgetIcon.offsetY = origLefthandY + deltaY;
            equipment->CreateAllWeaponWidget();
            dragLabel = fmt::format("Lefthand (X: {}, Y: {})", equipment->lefthand.widgetIcon.offsetX, equipment->lefthand.widgetIcon.offsetY);
            drawList->AddCircle(ImVec2(equipment->lefthand.widgetIcon.offsetX * S, equipment->lefthand.widgetIcon.offsetY * S), 30.0f * S, IM_COL32(0, 255, 255, 220), 32, 2.0f);
        } else if (currentDragTarget == LEFTHAND_TEXT) {
            equipment->lefthand.widgetName.offsetX = origLefthandNameX + deltaX;
            equipment->lefthand.widgetName.offsetY = origLefthandNameY + deltaY;
            equipment->lefthand.CreateWidgetText1();
            dragLabel = fmt::format("Lefthand Text (Rel X: {}, Rel Y: {})", equipment->lefthand.widgetName.offsetX, equipment->lefthand.widgetName.offsetY);
            auto [minPt, maxPt] = GetTextBounds(equipment->lefthand.widgetIcon.offsetX, equipment->lefthand.widgetIcon.offsetY, equipment->lefthand.widgetName.offsetX, equipment->lefthand.widgetName.offsetY, equipment->lefthand.widgetName.align, GetWeaponWidgetName(true), config->Widget.Equipment.Weapon.fontSize);
            drawList->AddRect(minPt, maxPt, IM_COL32(255, 215, 0, 220), 4.0f, 0, 2.0f);
            drawList->AddRectFilled(minPt, maxPt, IM_COL32(255, 215, 0, 50), 4.0f);
        } else if (currentDragTarget == RIGHTHAND) {
            equipment->righthand.widgetIcon.offsetX = origRighthandX + deltaX;
            equipment->righthand.widgetIcon.offsetY = origRighthandY + deltaY;
            equipment->CreateAllWeaponWidget();
            dragLabel = fmt::format("Righthand (X: {}, Y: {})", equipment->righthand.widgetIcon.offsetX, equipment->righthand.widgetIcon.offsetY);
            drawList->AddCircle(ImVec2(equipment->righthand.widgetIcon.offsetX * S, equipment->righthand.widgetIcon.offsetY * S), 30.0f * S, IM_COL32(0, 255, 255, 220), 32, 2.0f);
        } else if (currentDragTarget == RIGHTHAND_TEXT) {
            equipment->righthand.widgetName.offsetX = origRighthandNameX + deltaX;
            equipment->righthand.widgetName.offsetY = origRighthandNameY + deltaY;
            equipment->righthand.CreateWidgetText1();
            dragLabel = fmt::format("Righthand Text (Rel X: {}, Rel Y: {})", equipment->righthand.widgetName.offsetX, equipment->righthand.widgetName.offsetY);
            auto [minPt, maxPt] = GetTextBounds(equipment->righthand.widgetIcon.offsetX, equipment->righthand.widgetIcon.offsetY, equipment->righthand.widgetName.offsetX, equipment->righthand.widgetName.offsetY, equipment->righthand.widgetName.align, GetWeaponWidgetName(false), config->Widget.Equipment.Weapon.fontSize);
            drawList->AddRect(minPt, maxPt, IM_COL32(255, 215, 0, 220), 4.0f, 0, 2.0f);
            drawList->AddRectFilled(minPt, maxPt, IM_COL32(255, 215, 0, 50), 4.0f);
        } else if (currentDragTarget == SHOUT) {
            equipment->shout.widgetIcon.offsetX = origShoutX + deltaX;
            equipment->shout.widgetIcon.offsetY = origShoutY + deltaY;
            equipment->CreateAllShoutWidget();
            dragLabel = fmt::format("Shout (X: {}, Y: {})", equipment->shout.widgetIcon.offsetX, equipment->shout.widgetIcon.offsetY);
            drawList->AddCircle(ImVec2(equipment->shout.widgetIcon.offsetX * S, equipment->shout.widgetIcon.offsetY * S), 30.0f * S, IM_COL32(0, 255, 255, 220), 32, 2.0f);
        } else if (currentDragTarget == SHOUT_TEXT) {
            equipment->shout.widgetName.offsetX = origShoutNameX + deltaX;
            equipment->shout.widgetName.offsetY = origShoutNameY + deltaY;
            equipment->shout.CreateWidgetText1();
            dragLabel = fmt::format("Shout Text (Rel X: {}, Rel Y: {})", equipment->shout.widgetName.offsetX, equipment->shout.widgetName.offsetY);
            auto [minPt, maxPt] = GetTextBounds(equipment->shout.widgetIcon.offsetX, equipment->shout.widgetIcon.offsetY, equipment->shout.widgetName.offsetX, equipment->shout.widgetName.offsetY, equipment->shout.widgetName.align, GetShoutWidgetName(), config->Widget.Equipment.Shout.fontSize);
            drawList->AddRect(minPt, maxPt, IM_COL32(255, 215, 0, 220), 4.0f, 0, 2.0f);
            drawList->AddRectFilled(minPt, maxPt, IM_COL32(255, 215, 0, 50), 4.0f);
        } else if (currentDragTarget == ARMOR_STACK) {
            equipment->armorStackBaseX = origArmorBaseX + deltaX;
            equipment->armorStackBaseY = origArmorBaseY + deltaY;
            equipment->AutoArrangeArmorSlots();
            equipment->CreateAllArmorWidget();
            dragLabel = fmt::format("Armor Stack (Base X: {}, Base Y: {})", equipment->armorStackBaseX, equipment->armorStackBaseY);
            drawList->AddCircle(ImVec2(equipment->armorStackBaseX * S, equipment->armorStackBaseY * S), 25.0f * S, IM_COL32(255, 215, 0, 220), 32, 2.5f);
        } else if (currentDragTarget == ARMOR_SLOT && draggedArmorSlotIndex >= 0 && draggedArmorSlotIndex < static_cast<int>(equipment->armor.size())) {
            config->Widget.General.hudArmorAutoStack = false;
            equipment->armor[draggedArmorSlotIndex].widgetIcon.offsetX = origSlotX + deltaX;
            equipment->armor[draggedArmorSlotIndex].widgetIcon.offsetY = origSlotY + deltaY;
            equipment->armor[draggedArmorSlotIndex].CreateWidgetBackground();
            equipment->armor[draggedArmorSlotIndex].CreateWidgetIcon();
            equipment->armor[draggedArmorSlotIndex].CreateWidgetText1();
            dragLabel = fmt::format("Armor Slot {} (X: {}, Y: {})", draggedArmorSlotIndex + 30, equipment->armor[draggedArmorSlotIndex].widgetIcon.offsetX, equipment->armor[draggedArmorSlotIndex].widgetIcon.offsetY);
            drawList->AddCircle(ImVec2(equipment->armor[draggedArmorSlotIndex].widgetIcon.offsetX * S, equipment->armor[draggedArmorSlotIndex].widgetIcon.offsetY * S), 20.0f * S, IM_COL32(0, 255, 255, 220), 32, 2.0f);
        } else if (currentDragTarget == ARMOR_SLOT_TEXT && draggedArmorSlotIndex >= 0 && draggedArmorSlotIndex < static_cast<int>(equipment->armor.size())) {
            config->Widget.General.hudArmorAutoStack = false;
            equipment->armor[draggedArmorSlotIndex].widgetName.offsetX = origSlotNameX + deltaX;
            equipment->armor[draggedArmorSlotIndex].widgetName.offsetY = origSlotNameY + deltaY;
            equipment->armor[draggedArmorSlotIndex].CreateWidgetText1();
            dragLabel = fmt::format("Armor Slot {} Text (Rel X: {}, Rel Y: {})", draggedArmorSlotIndex + 30, equipment->armor[draggedArmorSlotIndex].widgetName.offsetX, equipment->armor[draggedArmorSlotIndex].widgetName.offsetY);
            auto [minPt, maxPt] = GetTextBounds(equipment->armor[draggedArmorSlotIndex].widgetIcon.offsetX, equipment->armor[draggedArmorSlotIndex].widgetIcon.offsetY, equipment->armor[draggedArmorSlotIndex].widgetName.offsetX, equipment->armor[draggedArmorSlotIndex].widgetName.offsetY, equipment->armor[draggedArmorSlotIndex].widgetName.align, GetArmorWidgetName(equipment->armor[draggedArmorSlotIndex].slotid), config->Widget.Equipment.Armor.fontSize);
            drawList->AddRect(minPt, maxPt, IM_COL32(255, 215, 0, 220), 4.0f, 0, 2.0f);
            drawList->AddRectFilled(minPt, maxPt, IM_COL32(255, 215, 0, 50), 4.0f);
        }

        ImGui::SetTooltip("%s", dragLabel.c_str());

        if (ImGui::IsMouseReleased(0)) {
            currentDragTarget = NONE;
            draggedArmorSlotIndex = -1;
            equipment->Save();
            config->SaveConfig();
        }
        return;
    }

    // 2. Hover detection across active widgets if mouse is not captured by ImGui window:
    if (io.WantCaptureMouse) return;

    DragTarget hoveredTarget = NONE;
    int hoveredSlotIndex = -1;
    float bestDist = 1e9f;

    auto CheckTextHover = [&](DragTarget target, int slotIdx, int iconX, int iconY, int nameX, int nameY, WidgetText::ALIGN_TYPE align, const std::string& text, int fontSz) {
        auto [minPt, maxPt] = GetTextBounds(iconX, iconY, nameX, nameY, align, text, fontSz);
        if (IsInside(io.MousePos, minPt, maxPt)) {
            float d = BoxDistance(io.MousePos, minPt, maxPt);
            if (d < bestDist) {
                bestDist = d;
                hoveredTarget = target;
                hoveredSlotIndex = slotIdx;
            }
        }
    };

    // 1. Check Diamond Texts:
    if (config->Widget.General.hudDiamondEnable) {
        if (equipment->shout.widgetIcon.enable && equipment->shout.widgetName.enable) {
            CheckTextHover(SHOUT_TEXT, -1, equipment->shout.widgetIcon.offsetX, equipment->shout.widgetIcon.offsetY, equipment->shout.widgetName.offsetX, equipment->shout.widgetName.offsetY, equipment->shout.widgetName.align, GetShoutWidgetName(), config->Widget.Equipment.Shout.fontSize);
        }
        if (equipment->lefthand.widgetIcon.enable && equipment->lefthand.widgetName.enable && config->Widget.General.hudDiamondLeftEnable) {
            CheckTextHover(LEFTHAND_TEXT, -1, equipment->lefthand.widgetIcon.offsetX, equipment->lefthand.widgetIcon.offsetY, equipment->lefthand.widgetName.offsetX, equipment->lefthand.widgetName.offsetY, equipment->lefthand.widgetName.align, GetWeaponWidgetName(true), config->Widget.Equipment.Weapon.fontSize);
        }
        if (equipment->righthand.widgetIcon.enable && equipment->righthand.widgetName.enable) {
            CheckTextHover(RIGHTHAND_TEXT, -1, equipment->righthand.widgetIcon.offsetX, equipment->righthand.widgetIcon.offsetY, equipment->righthand.widgetName.offsetX, equipment->righthand.widgetName.offsetY, equipment->righthand.widgetName.align, GetWeaponWidgetName(false), config->Widget.Equipment.Weapon.fontSize);
        }
    }

    // 2. Check Armor Texts:
    if (config->Widget.General.hudArmorEnable) {
        auto player = RE::PlayerCharacter::GetSingleton();
        for (int i = 0; i < static_cast<int>(equipment->armor.size()); i++) {
            if (!equipment->armor[i].widgetIcon.enable || !equipment->armor[i].widgetName.enable) continue;
            if (!config->Widget.General.showEmptySlots && player && !IsArmorSlotEquipped(equipment->armor[i].slotid)) continue;

            CheckTextHover(ARMOR_SLOT_TEXT, i, equipment->armor[i].widgetIcon.offsetX, equipment->armor[i].widgetIcon.offsetY, equipment->armor[i].widgetName.offsetX, equipment->armor[i].widgetName.offsetY, equipment->armor[i].widgetName.align, GetArmorWidgetName(equipment->armor[i].slotid), config->Widget.Equipment.Armor.fontSize);
        }
    }

    // 3. Check Diamond Icons & Cluster:
    if (config->Widget.General.hudDiamondEnable) {
        int clusterCenterX = equipment->shout.widgetIcon.offsetX;
        int clusterCenterY = equipment->lefthand.widgetIcon.offsetY;
        float distCluster = std::hypot(io.MousePos.x - clusterCenterX * S, io.MousePos.y - clusterCenterY * S);

        if (distCluster < 35.0f * S && distCluster < bestDist) {
            bestDist = distCluster;
            hoveredTarget = DIAMOND_CLUSTER;
        }

        if (equipment->shout.widgetIcon.enable) {
            float dist = std::hypot(io.MousePos.x - equipment->shout.widgetIcon.offsetX * S, io.MousePos.y - equipment->shout.widgetIcon.offsetY * S);
            if (dist < 28.0f * S && dist < bestDist) {
                bestDist = dist;
                hoveredTarget = SHOUT;
            }
        }
        if (equipment->lefthand.widgetIcon.enable && config->Widget.General.hudDiamondLeftEnable) {
            float dist = std::hypot(io.MousePos.x - equipment->lefthand.widgetIcon.offsetX * S, io.MousePos.y - equipment->lefthand.widgetIcon.offsetY * S);
            if (dist < 28.0f * S && dist < bestDist) {
                bestDist = dist;
                hoveredTarget = LEFTHAND;
            }
        }
        if (equipment->righthand.widgetIcon.enable) {
            float dist = std::hypot(io.MousePos.x - equipment->righthand.widgetIcon.offsetX * S, io.MousePos.y - equipment->righthand.widgetIcon.offsetY * S);
            if (dist < 28.0f * S && dist < bestDist) {
                bestDist = dist;
                hoveredTarget = RIGHTHAND;
            }
        }
    }

    // 4. Check Armor Icons:
    if (config->Widget.General.hudArmorEnable) {
        auto player = RE::PlayerCharacter::GetSingleton();
        for (int i = 0; i < static_cast<int>(equipment->armor.size()); i++) {
            if (!equipment->armor[i].widgetIcon.enable) continue;
            if (!config->Widget.General.showEmptySlots && player && !IsArmorSlotEquipped(equipment->armor[i].slotid)) {
                continue;
            }
            float dist = std::hypot(io.MousePos.x - equipment->armor[i].widgetIcon.offsetX * S, io.MousePos.y - equipment->armor[i].widgetIcon.offsetY * S);
            if (dist < 24.0f * S && dist < bestDist) {
                bestDist = dist;
                if (io.KeyShift) {
                    hoveredTarget = ARMOR_STACK;
                } else {
                    hoveredTarget = ARMOR_SLOT;
                    hoveredSlotIndex = i;
                }
            }
        }
    }

    if (hoveredTarget != NONE) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);

        std::string tip = "";

        if (hoveredTarget == DIAMOND_CLUSTER) {
            int cx = equipment->shout.widgetIcon.offsetX;
            int cy = equipment->lefthand.widgetIcon.offsetY;
            drawList->AddCircle(ImVec2(cx * S, cy * S), 45.0f * S, IM_COL32(255, 215, 0, 180), 32, 2.0f);
            tip = "HUD Diamond (Drag to move cluster)";
        } else if (hoveredTarget == LEFTHAND) {
            drawList->AddCircle(ImVec2(equipment->lefthand.widgetIcon.offsetX * S, equipment->lefthand.widgetIcon.offsetY * S), 28.0f * S, IM_COL32(255, 215, 0, 180), 32, 2.0f);
            tip = "Lefthand Diamond (Drag to move)";
        } else if (hoveredTarget == LEFTHAND_TEXT) {
            auto [minPt, maxPt] = GetTextBounds(equipment->lefthand.widgetIcon.offsetX, equipment->lefthand.widgetIcon.offsetY, equipment->lefthand.widgetName.offsetX, equipment->lefthand.widgetName.offsetY, equipment->lefthand.widgetName.align, GetWeaponWidgetName(true), config->Widget.Equipment.Weapon.fontSize);
            drawList->AddRect(minPt, maxPt, IM_COL32(255, 215, 0, 220), 4.0f, 0, 2.0f);
            drawList->AddRectFilled(minPt, maxPt, IM_COL32(255, 215, 0, 40), 4.0f);
            tip = fmt::format("Lefthand Text: \"{}\" (Drag to move label)", GetWeaponWidgetName(true));
        } else if (hoveredTarget == RIGHTHAND) {
            drawList->AddCircle(ImVec2(equipment->righthand.widgetIcon.offsetX * S, equipment->righthand.widgetIcon.offsetY * S), 28.0f * S, IM_COL32(255, 215, 0, 180), 32, 2.0f);
            tip = "Righthand Diamond (Drag to move)";
        } else if (hoveredTarget == RIGHTHAND_TEXT) {
            auto [minPt, maxPt] = GetTextBounds(equipment->righthand.widgetIcon.offsetX, equipment->righthand.widgetIcon.offsetY, equipment->righthand.widgetName.offsetX, equipment->righthand.widgetName.offsetY, equipment->righthand.widgetName.align, GetWeaponWidgetName(false), config->Widget.Equipment.Weapon.fontSize);
            drawList->AddRect(minPt, maxPt, IM_COL32(255, 215, 0, 220), 4.0f, 0, 2.0f);
            drawList->AddRectFilled(minPt, maxPt, IM_COL32(255, 215, 0, 40), 4.0f);
            tip = fmt::format("Righthand Text: \"{}\" (Drag to move label)", GetWeaponWidgetName(false));
        } else if (hoveredTarget == SHOUT) {
            drawList->AddCircle(ImVec2(equipment->shout.widgetIcon.offsetX * S, equipment->shout.widgetIcon.offsetY * S), 28.0f * S, IM_COL32(255, 215, 0, 180), 32, 2.0f);
            tip = "Shout Diamond (Drag to move)";
        } else if (hoveredTarget == SHOUT_TEXT) {
            auto [minPt, maxPt] = GetTextBounds(equipment->shout.widgetIcon.offsetX, equipment->shout.widgetIcon.offsetY, equipment->shout.widgetName.offsetX, equipment->shout.widgetName.offsetY, equipment->shout.widgetName.align, GetShoutWidgetName(), config->Widget.Equipment.Shout.fontSize);
            drawList->AddRect(minPt, maxPt, IM_COL32(255, 215, 0, 220), 4.0f, 0, 2.0f);
            drawList->AddRectFilled(minPt, maxPt, IM_COL32(255, 215, 0, 40), 4.0f);
            tip = fmt::format("Shout Text: \"{}\" (Drag to move label)", GetShoutWidgetName());
        } else if (hoveredTarget == ARMOR_STACK) {
            drawList->AddCircle(ImVec2(equipment->armorStackBaseX * S, equipment->armorStackBaseY * S), 25.0f * S, IM_COL32(255, 215, 0, 180), 32, 2.5f);
            tip = "Armor Stack (Drag to move whole stack)";
        } else if (hoveredTarget == ARMOR_SLOT && hoveredSlotIndex >= 0 && hoveredSlotIndex < static_cast<int>(equipment->armor.size())) {
            drawList->AddCircle(ImVec2(equipment->armor[hoveredSlotIndex].widgetIcon.offsetX * S, equipment->armor[hoveredSlotIndex].widgetIcon.offsetY * S), 20.0f * S, IM_COL32(255, 215, 0, 180), 32, 2.0f);
            tip = fmt::format("Armor Slot {} ({}) (Drag to move | Hold Shift to move entire stack)", hoveredSlotIndex + 30, GetSlotDescription(hoveredSlotIndex + 30));
        } else if (hoveredTarget == ARMOR_SLOT_TEXT && hoveredSlotIndex >= 0 && hoveredSlotIndex < static_cast<int>(equipment->armor.size())) {
            auto [minPt, maxPt] = GetTextBounds(equipment->armor[hoveredSlotIndex].widgetIcon.offsetX, equipment->armor[hoveredSlotIndex].widgetIcon.offsetY, equipment->armor[hoveredSlotIndex].widgetName.offsetX, equipment->armor[hoveredSlotIndex].widgetName.offsetY, equipment->armor[hoveredSlotIndex].widgetName.align, GetArmorWidgetName(equipment->armor[hoveredSlotIndex].slotid), config->Widget.Equipment.Armor.fontSize);
            drawList->AddRect(minPt, maxPt, IM_COL32(255, 215, 0, 220), 4.0f, 0, 2.0f);
            drawList->AddRectFilled(minPt, maxPt, IM_COL32(255, 215, 0, 40), 4.0f);
            tip = fmt::format("Armor Slot {} Text: \"{}\" (Drag to move label)", hoveredSlotIndex + 30, GetArmorWidgetName(equipment->armor[hoveredSlotIndex].slotid));
        }

        ImGui::SetTooltip("%s", tip.c_str());

        if (ImGui::IsMouseClicked(0)) {
            currentDragTarget = hoveredTarget;
            draggedArmorSlotIndex = hoveredSlotIndex;
            dragStartMouseStageX = mouseStageX;
            dragStartMouseStageY = mouseStageY;

            origLefthandX = equipment->lefthand.widgetIcon.offsetX;
            origLefthandY = equipment->lefthand.widgetIcon.offsetY;
            origLefthandNameX = equipment->lefthand.widgetName.offsetX;
            origLefthandNameY = equipment->lefthand.widgetName.offsetY;

            origRighthandX = equipment->righthand.widgetIcon.offsetX;
            origRighthandY = equipment->righthand.widgetIcon.offsetY;
            origRighthandNameX = equipment->righthand.widgetName.offsetX;
            origRighthandNameY = equipment->righthand.widgetName.offsetY;

            origShoutX = equipment->shout.widgetIcon.offsetX;
            origShoutY = equipment->shout.widgetIcon.offsetY;
            origShoutNameX = equipment->shout.widgetName.offsetX;
            origShoutNameY = equipment->shout.widgetName.offsetY;

            origArmorBaseX = equipment->armorStackBaseX;
            origArmorBaseY = equipment->armorStackBaseY;

            if (hoveredSlotIndex >= 0 && hoveredSlotIndex < static_cast<int>(equipment->armor.size())) {
                origSlotX = equipment->armor[hoveredSlotIndex].widgetIcon.offsetX;
                origSlotY = equipment->armor[hoveredSlotIndex].widgetIcon.offsetY;
                origSlotNameX = equipment->armor[hoveredSlotIndex].widgetName.offsetX;
                origSlotNameY = equipment->armor[hoveredSlotIndex].widgetName.offsetY;
            }

            if (currentDragTarget == ARMOR_SLOT || currentDragTarget == ARMOR_SLOT_TEXT) {
                config->Widget.General.hudArmorAutoStack = false;
            }
        }
    }
}