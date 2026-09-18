#include "Equipment.h"
#include "EquipsetManager.h"
#include "WidgetHandler.h"
#include "Config.h"
#include "Actor.h"
#include "Translate.h"
#include "Utility.h"

#include <filesystem>
#include <toml++/toml.h>

const std::filesystem::path equipment_path = "Data/SKSE/Plugins/UIHS/Equipment.toml";

EquipmentManager::EquipmentManager() {
    auto manager = EquipsetManager::GetSingleton();
    if (!manager) logger::error("Failed to get Equipset Manager.");

    for (int i = 0; i < 32; i++) {
        EquipmentArmor initArmor;
        initArmor.type = EquipmentBase::Type::ARMOR;
        initArmor.slotid = fmt::format("_SLOT{}", i + 30);
        initArmor.widgetID.background = manager->AssignWidgetID();
        initArmor.widgetID.icon = manager->AssignWidgetID();
        initArmor.widgetID.text1 = manager->AssignWidgetID();
        this->armor.push_back(initArmor);
    }
    for (int i = 0; i < 2; i++) {
        EquipmentWeapon initWeapon;
        initWeapon.type = EquipmentBase::Type::WEAPON;
        initWeapon.widgetID.background = manager->AssignWidgetID();
        initWeapon.widgetID.icon = manager->AssignWidgetID();
        initWeapon.widgetID.text1 = manager->AssignWidgetID();
        if (i == 0) {
            initWeapon.isLeft = true;
            this->lefthand = initWeapon;
        } else if (i == 1) {
            initWeapon.isLeft = false;
            this->righthand = initWeapon;
        }
    }

    EquipmentBase initShout;
    initShout.type = EquipmentBase::Type::SHOUT;
    initShout.widgetID.background = manager->AssignWidgetID();
    initShout.widgetID.icon = manager->AssignWidgetID();
    initShout.widgetID.text1 = manager->AssignWidgetID();
    this->shout = initShout;
}

bool IsArmorSlotEquipped(const std::string& _slot) {
    if (_slot.length() != 7) return false;
    auto player = RE::PlayerCharacter::GetSingleton();
    if (!player) return false;

    int slot = 0;
    try {
        slot = std::stoi(_slot.substr(5, std::string::npos)) - 30;
    } catch (...) {
        return false;
    }

    auto inv = player->GetInventory();
    for (const auto& [item, data] : inv) {
        const auto& [numItem, entry] = data;
        if (item && numItem > 0 && item->Is(RE::FormType::Armor)) {
            auto armor = item->As<RE::TESObjectARMO>();
            if (!armor) continue;

            auto extraLists = entry ? entry->extraLists : nullptr;
            if (!extraLists) continue;

            bool isWorn = false;
            for (auto& _xList : *extraLists) {
                if (_xList && _xList->HasType(RE::ExtraDataType::kWorn)) {
                    isWorn = true;
                    break;
                }
            }
            if (!isWorn) continue;

            auto armor_slot = static_cast<uint32_t>(armor->GetSlotMask());
            if (armor_slot & (1U << slot)) {
                return true;
            }
        }
    }
    return false;
}

std::string GetArmorWidgetName(const std::string& _slot) {
    auto ts = Translator::GetSingleton();
    if (!ts) logger::error("Failed to get Translator");

    std::string result = TRANSLATE("_NOT_EQUIPPED");
    if (result == "_NOT_EQUIPPED") {
        auto config = ConfigHandler::GetSingleton();
        if (config && config->Gui.language == (uint32_t)Config::LangType::GERMAN) {
            result = "Nicht ausgerüstet";
        } else {
            result = "Not Equipped";
        }
    }

    if (_slot.length() != 7) return result;

    auto player = RE::PlayerCharacter::GetSingleton();
    if (!player) return result;

    int slot = 0;
    try {
        slot = std::stoi(_slot.substr(5, std::string::npos)) - 30;
    } catch (...) {
        return result;
    }

    auto inv = player->GetInventory();
    for (const auto& [item, data] : inv) {
        const auto& [numItem, entry] = data;
        if (item && numItem > 0 && item->Is(RE::FormType::Armor)) {
            auto armor = item->As<RE::TESObjectARMO>();
            if (!armor) continue;

            auto extraLists = entry ? entry->extraLists : nullptr;
            if (!extraLists) continue;

            bool doNext = false;
            for (auto& _xList : *extraLists) {
                if (_xList && _xList->HasType(RE::ExtraDataType::kWorn)) {
                    doNext = true;
                    break;
                }
            }

            if (!doNext) continue;

            auto armor_slot = static_cast<uint32_t>(armor->GetSlotMask());
            int flag = 1;
            if (armor_slot & (flag << slot)) {
                const char* itemName = item->GetName();
                if (itemName) {
                    result = itemName;
                }
            }
        }
    }

    return result;
}

std::string GetSlotDescription(int slotNum) {
    auto ts = Translator::GetSingleton();
    std::string key = fmt::format("_SLOT_{}", slotNum);
    std::string desc = ts ? ts->Translate(key) : key;
    if (desc != key && !desc.empty()) {
        return desc;
    }

    auto config = ConfigHandler::GetSingleton();
    bool isGerman = config && config->Gui.language == (uint32_t)Config::LangType::GERMAN;
    switch (slotNum) {
        case 30: return isGerman ? "Kopf / Haare" : "Head / Hair";
        case 31: return isGerman ? "Haare" : "Hair";
        case 32: return isGerman ? "Rüstung" : "Armor";
        case 33: return isGerman ? "Handschuhe" : "Gauntlets";
        case 34: return isGerman ? "Unterarme" : "Forearms";
        case 35: return isGerman ? "Halskette / Amulett" : "Necklace / Amulet";
        case 36: return isGerman ? "Ring" : "Ring";
        case 37: return isGerman ? "Stiefel" : "Boots";
        case 38: return isGerman ? "Waden" : "Calves";
        case 39: return isGerman ? "Schild" : "Shield";
        case 40: return isGerman ? "Schwanz" : "Tail";
        case 41: return isGerman ? "Lange Haare" : "Long Hair";
        case 42: return isGerman ? "Helm / Reif" : "Helmet / Circlet";
        case 43: return isGerman ? "Ohren" : "Ears";
        case 44: return isGerman ? "Gesicht / Maske" : "Face / Mask";
        case 45: return isGerman ? "Hals" : "Neck";
        case 46: return isGerman ? "Umhang / Cape" : "Cloak / Cape";
        case 47: return isGerman ? "Rucksack" : "Backpack";
        case 48: return isGerman ? "Bandolier" : "Bandolier";
        case 49: return isGerman ? "Taschen / Beutel" : "Pouches / Bags";
        case 50: return isGerman ? "Zusatz 50" : "Extra 50";
        case 51: return isGerman ? "Zusatz 51" : "Extra 51";
        case 52: return isGerman ? "Unterwäsche" : "Underwear";
        case 53: return isGerman ? "Rechtes Bein" : "Right Leg";
        case 54: return isGerman ? "Linkes Bein" : "Left Leg";
        case 55: return isGerman ? "Augen" : "Eyes";
        case 56: return isGerman ? "Schwertscheide" : "Scabbard";
        case 57: return isGerman ? "Zusatz 57" : "Extra 57";
        case 58: return isGerman ? "Zusatz 58" : "Extra 58";
        case 59: return isGerman ? "Schulter" : "Shoulder";
        case 60: return isGerman ? "Sonstiges" : "Misc";
        case 61: return isGerman ? "Effekte / FX" : "FX / Magic";
        default: return "";
    }
}

void EquipmentManager::NotifyArmor(bool _isEquip, const std::vector<uint32_t>& _slot, const std::string& _name) {
    auto config = ConfigHandler::GetSingleton();
    if (!config || !config->Widget.General.hudArmorEnable) return;

    auto widgetHandler = WidgetHandler::GetSingleton();
    if (!widgetHandler) return;

    auto ts = Translator::GetSingleton();
    if (!ts) return;

    if (!config->Widget.General.showEmptySlots) {
        RemoveAllArmorWidget();
        CreateAllArmorWidget();
        return;
    }

    std::string notEquippedText = TRANSLATE("_NOT_EQUIPPED");
    if (notEquippedText == "_NOT_EQUIPPED") {
        notEquippedText = (config->Gui.language == (uint32_t)Config::LangType::GERMAN) ? "Nicht ausgerüstet" : "Not Equipped";
    }

    if (_isEquip) {
        for (auto elem : _slot) {
            if (elem < this->armor.size() && this->armor[elem].widgetName.enable) {
                widgetHandler->SetText(this->armor[elem].widgetID.text1, _name);
            }
        }
    } else {
        for (auto elem : _slot) {
            if (elem < this->armor.size() && this->armor[elem].widgetName.enable) {
                widgetHandler->SetText(this->armor[elem].widgetID.text1, notEquippedText);
            }
        }
    }
}

std::string GetWeaponWidgetName(bool _isLeft) {
    auto ts = Translator::GetSingleton();
    if (!ts) logger::error("Failed to get Translator");

    std::string result = TRANSLATE("_UNARMED");
    if (result == "_UNARMED") {
        auto config = ConfigHandler::GetSingleton();
        if (config && config->Gui.language == (uint32_t)Config::LangType::GERMAN) {
            result = "Waffenlos";
        } else {
            result = "Unarmed";
        }
    }

    auto player = RE::PlayerCharacter::GetSingleton();
    if (!player) return result;

    auto hand = player->GetEquippedObject(_isLeft);
    if (!hand) return result;

    const char* handName = hand->GetName();
    result = handName ? handName : "";

    return result;
}

std::string GetWeaponWidgetPath(bool _isLeft) {
    std::string result = "";
    std::string id = ID::Null;
    std::vector<std::string> keyword;
    RE::FormID formid = 0U;

    auto player = RE::PlayerCharacter::GetSingleton();
    if (!player) return result;

    auto config = ConfigHandler::GetSingleton();
    if (!config) return result;

    auto TESDataHandler = RE::TESDataHandler::GetSingleton();
    if (!TESDataHandler) return result;

    auto hand = player->GetEquippedObject(_isLeft);

    if (!hand) {
        id = ID::Unarmed;
    } else {
        auto weapon = hand->As<RE::TESObjectWEAP>();
        if (weapon) {
            auto type = weapon->GetWeaponType();
            switch (type) {
                case RE::WEAPON_TYPE::kHandToHandMelee:
                    id = ID::Unarmed;
                    break;
                case RE::WEAPON_TYPE::kOneHandSword:
                    id = ID::Sword;
                    break;
                case RE::WEAPON_TYPE::kOneHandDagger:
                    id = ID::Dagger;
                    break;
                case RE::WEAPON_TYPE::kOneHandAxe:
                    id = ID::Axe;
                    break;
                case RE::WEAPON_TYPE::kOneHandMace:
                    id = ID::Mace;
                    break;
                case RE::WEAPON_TYPE::kTwoHandSword:
                    id = ID::Greatsword;
                    break;
                case RE::WEAPON_TYPE::kTwoHandAxe:
                    id = ID::Battleaxe;
                    break;
                case RE::WEAPON_TYPE::kBow:
                    id = ID::Bow;
                    break;
                case RE::WEAPON_TYPE::kStaff:
                    id = ID::Staff;
                    break;
                case RE::WEAPON_TYPE::kCrossbow:
                    id = ID::Crossbow;
                    break;
            }
        }
        auto spell = hand->As<RE::SpellItem>();
        if (spell) {
            auto type = spell->GetSpellType();
            if (type == RE::MagicSystem::SpellType::kSpell) {
                auto skillType = spell->GetAssociatedSkill();
                switch (skillType) {
                    case RE::ActorValue::kAlteration:
                        id = ID::Alteration;
                        break;
                    case RE::ActorValue::kConjuration:
                        id = ID::Conjuration;
                        break;
                    case RE::ActorValue::kDestruction:
                        id = ID::Destruction;
                        break;
                    case RE::ActorValue::kIllusion:
                        id = ID::Illusion;
                        break;
                    case RE::ActorValue::kRestoration:
                        id = ID::Restoration;
                        break;
                }
            }
        }

        auto armor = hand->As<RE::TESObjectARMO>();
        if (armor) {
            if (armor->IsShield()) {
                id = ID::Shield;
            }
        }

        auto light = hand->As<RE::TESObjectLIGH>();
        if (light) {
            id = ID::Torch;
        }

        auto keywordForm = hand->As<RE::BGSKeywordForm>();
        if (keywordForm) {
            auto keywords = keywordForm->GetKeywords();
            for (auto elem : keywords) {
                if (elem) {
                    auto str = static_cast<std::string>(elem->formEditorID);
                    keyword.push_back(str);
                }
            }
        }

        formid = hand->GetFormID();
    }

    // First, assign default equipment widget.
    for (auto& elem : config->eq_widgetVec) {
        if (elem.id == id) {
            result = elem.path;
        }
    }

    // Second, assign cusotm equipment widget by keyword.
    for (auto& elem : config->eq_widgetVec) {
        for (auto& elemkeyword : keyword) {
            if (elem.keyword == elemkeyword) {
                result = elem.path;
            }
        }
    }

    // Third, assign cusotm equipment widget by formid.
    if (formid != 0) {
        for (auto& elem : config->eq_widgetVec) {
            auto compare = TESDataHandler->LookupFormID(elem.formid, elem.modname);
            if (formid == compare) {
                result = elem.path;
                break;
            }
        }
    }

    return result;
}

void EquipmentManager::NotifyWeapon(bool _isEquip, RE::TESForm* _weapon) {
    auto player = RE::PlayerCharacter::GetSingleton();
    if (!player) return;

    auto config = ConfigHandler::GetSingleton();
    if (!config) return;

    auto widgetHandler = WidgetHandler::GetSingleton();
    if (!widgetHandler) return;

    auto ts = Translator::GetSingleton();
    if (!ts) return;

    if (!_weapon) return;

    auto lefthand = player->GetEquippedObject(true);
    auto righthand = player->GetEquippedObject(false);
    std::string unarmedText = TRANSLATE("_UNARMED");
    if (unarmedText == "_UNARMED") {
        unarmedText = (config->Gui.language == (uint32_t)Config::LangType::GERMAN) ? "Waffenlos" : "Unarmed";
    }

    if (_isEquip) {
        if (lefthand && lefthand->GetFormID() == _weapon->GetFormID()) {
            widgetHandler->UnloadWidget(this->lefthand.widgetID.background);
            widgetHandler->UnloadWidget(this->lefthand.widgetID.icon);
            if (config->Widget.General.hudDiamondEnable && config->Widget.General.hudDiamondLeftEnable) {
                this->lefthand.CreateWidgetBackground();
                this->lefthand.CreateWidgetIcon();
                const char* wName = _weapon->GetName();
                widgetHandler->SetText(this->lefthand.widgetID.text1, wName ? wName : "");
            } else {
                widgetHandler->UnloadText(this->lefthand.widgetID.text1);
            }
        }
        if (righthand && righthand->GetFormID() == _weapon->GetFormID()) {
            widgetHandler->UnloadWidget(this->righthand.widgetID.background);
            widgetHandler->UnloadWidget(this->righthand.widgetID.icon);
            if (config->Widget.General.hudDiamondEnable) {
                this->righthand.CreateWidgetBackground();
                this->righthand.CreateWidgetIcon();
                const char* wName = _weapon->GetName();
                widgetHandler->SetText(this->righthand.widgetID.text1, wName ? wName : "");
            } else {
                widgetHandler->UnloadText(this->righthand.widgetID.text1);
            }
        }
    } else {
        if (!lefthand) {
            widgetHandler->UnloadWidget(this->lefthand.widgetID.background);
            widgetHandler->UnloadWidget(this->lefthand.widgetID.icon);
            if (config->Widget.General.showEmptySlots && config->Widget.General.hudDiamondEnable && config->Widget.General.hudDiamondLeftEnable) {
                this->lefthand.CreateWidgetBackground();
                this->lefthand.CreateWidgetIcon();
                widgetHandler->SetText(this->lefthand.widgetID.text1, unarmedText);
            } else {
                widgetHandler->UnloadText(this->lefthand.widgetID.text1);
            }
        }
        if (!righthand) {
            widgetHandler->UnloadWidget(this->righthand.widgetID.background);
            widgetHandler->UnloadWidget(this->righthand.widgetID.icon);
            if (config->Widget.General.showEmptySlots && config->Widget.General.hudDiamondEnable) {
                this->righthand.CreateWidgetBackground();
                this->righthand.CreateWidgetIcon();
                widgetHandler->SetText(this->righthand.widgetID.text1, unarmedText);
            } else {
                widgetHandler->UnloadText(this->righthand.widgetID.text1);
            }
        }
    }
}

std::string GetShoutWidgetName() {
    auto ts = Translator::GetSingleton();
    if (!ts) logger::error("Failed to get Translator");

    std::string result = TRANSLATE("_NOT_EQUIPPED");
    if (result == "_NOT_EQUIPPED") {
        auto config = ConfigHandler::GetSingleton();
        if (config && config->Gui.language == (uint32_t)Config::LangType::GERMAN) {
            result = "Nicht ausgerüstet";
        } else {
            result = "Not Equipped";
        }
    }

    auto player = RE::PlayerCharacter::GetSingleton();
    if (!player) return result;

    auto shout = Actor::GetEquippedShout(player);
    if (!shout) return result;

    const char* shoutName = shout->GetName();
    result = shoutName ? shoutName : "";

    return result;
}

std::string GetShoutWidgetPath(bool _unequip = false) {
    std::string result = "";
    std::string id = ID::Null;
    std::vector<std::string> keyword;
    RE::FormID formid = 0U;

    auto player = RE::PlayerCharacter::GetSingleton();
    if (!player) return result;

    auto config = ConfigHandler::GetSingleton();
    if (!config) return result;

    auto TESDataHandler = RE::TESDataHandler::GetSingleton();
    if (!TESDataHandler) return result;

    if (_unequip) {
        for (auto& elem : config->eq_widgetVec) {
            if (elem.id == id) {
                result = elem.path;
            }
        }
        return result;
    }

    auto shout = Actor::GetEquippedShout(player);

    if (!shout) {
        id = ID::Null;
    } else {
        auto spell = shout->As<RE::SpellItem>();
        if (spell) {
            auto type = spell->GetSpellType();
            if (type == RE::MagicSystem::SpellType::kPower ||
                type == RE::MagicSystem::SpellType::kLesserPower) {
                id = ID::Power;
            }
        }

        if (shout->Is(RE::FormType::Shout)) {
            id = ID::Shout;
        }

        auto keywordForm = shout->As<RE::BGSKeywordForm>();
        if (keywordForm) {
            auto keywords = keywordForm->GetKeywords();
            for (auto elem : keywords) {
                if (elem) {
                    auto str = static_cast<std::string>(elem->formEditorID);
                    keyword.push_back(str);
                }
            }
        }

        formid = shout->GetFormID();
    }

    // First, assign default equipment widget.
    for (auto& elem : config->eq_widgetVec) {
        if (elem.id == id) {
            result = elem.path;
        }
    }

    // Second, assign cusotm equipment widget by keyword.
    for (auto& elem : config->eq_widgetVec) {
        for (auto& elemkeyword : keyword) {
            if (elem.keyword == elemkeyword) {
                result = elem.path;
            }
        }
    }

    // Third, assign cusotm equipment widget by formid.
    if (formid != 0) {
        for (auto& elem : config->eq_widgetVec) {
            auto compare = TESDataHandler->LookupFormID(elem.formid, elem.modname);
            if (formid == compare) {
                result = elem.path;
                break;
            }
        }
    }

    return result;
}

void EquipmentManager::NotifyShout(bool _isEquip, RE::TESForm* _shout) {
    auto player = RE::PlayerCharacter::GetSingleton();
    if (!player) return;

    auto config = ConfigHandler::GetSingleton();
    if (!config) return;

    auto widgetHandler = WidgetHandler::GetSingleton();
    if (!widgetHandler) return;

    auto ts = Translator::GetSingleton();
    if (!ts) return;

    if (!_shout) return;

    auto shout = Actor::GetEquippedShout(player);
    if (_isEquip) {
        if (shout && shout->GetFormID() == _shout->GetFormID()) {
            widgetHandler->UnloadWidget(this->shout.widgetID.background);
            widgetHandler->UnloadWidget(this->shout.widgetID.icon);
            this->shout.CreateWidgetBackground();
            this->shout.CreateWidgetIcon();
            const char* sName = _shout->GetName();
            widgetHandler->SetText(this->shout.widgetID.text1, sName ? sName : "");
        }
    } else {
        widgetHandler->UnloadWidget(this->shout.widgetID.background);
        widgetHandler->UnloadWidget(this->shout.widgetID.icon);
        if (config->Widget.General.showEmptySlots && config->Widget.General.hudDiamondEnable) {
            this->shout.CreateWidgetBackground();
            this->shout.CreateWidgetIcon(true);
            widgetHandler->SetText(this->shout.widgetID.text1, TRANSLATE("_NOT_EQUIPPED"));
        } else {
            widgetHandler->UnloadText(this->shout.widgetID.text1);
        }
    }
}

void EquipmentBase::CreateWidgetBackground() {
    auto config = ConfigHandler::GetSingleton();
    if (!config) return;

    auto widgetHandler = WidgetHandler::GetSingleton();
    if (!widgetHandler) return;

    float resScale = config->Widget.General.autoResolutionScale ? Utility::GetResolutionScale() : 1.0f;
    auto player = RE::PlayerCharacter::GetSingleton();

    if (this->type == EquipmentBase::Type::ARMOR) {
        auto armor = static_cast<EquipmentArmor*>(this);
        if (!armor) return;

        if (!config->Widget.General.hudArmorEnable) return;
        if (!config->Widget.General.showEmptySlots && !IsArmorSlotEquipped(armor->slotid)) {
            return;
        }
        if (config->Widget.Equipment.Armor.bgType == "_NONE" || config->Widget.Equipment.Armor.bgAlpha == 0) {
            return;
        }

        if (armor->widgetIcon.enable) {
            auto id = armor->widgetID.background;
            auto path = config->GetWidgetPath(config->Widget.Equipment.Armor.bgType);
            auto offsetX = armor->widgetIcon.offsetX;
            auto offsetY = armor->widgetIcon.offsetY;
            auto width = static_cast<int32_t>(1.3f * (float)config->Widget.Equipment.Armor.bgSize * resScale);
            auto height = static_cast<int32_t>(1.3f * (float)config->Widget.Equipment.Armor.bgSize * resScale);
            auto alpha = config->Widget.Equipment.Armor.bgAlpha;

            widgetHandler->LoadWidget(id, path, offsetX, offsetY, width, height, alpha);
        }

    } else if (this->type == EquipmentBase::Type::WEAPON) {
        auto weapon = static_cast<EquipmentWeapon*>(this);
        if (!weapon) return;

        if (!config->Widget.General.hudDiamondEnable) return;
        if (weapon->isLeft && !config->Widget.General.hudDiamondLeftEnable) return;
        if (!config->Widget.General.showEmptySlots && player && !player->GetEquippedObject(weapon->isLeft)) {
            return;
        }
        if (config->Widget.Equipment.Weapon.bgType == "_NONE" || config->Widget.Equipment.Weapon.bgAlpha == 0) {
            return;
        }

        if (weapon->widgetIcon.enable) {
            auto id = weapon->widgetID.background;
            auto path = config->GetWidgetPath(config->Widget.Equipment.Weapon.bgType);
            auto offsetX = weapon->widgetIcon.offsetX;
            auto offsetY = weapon->widgetIcon.offsetY;
            auto width = static_cast<int32_t>(1.3f * (float)config->Widget.Equipment.Weapon.bgSize * resScale);
            auto height = static_cast<int32_t>(1.3f * (float)config->Widget.Equipment.Weapon.bgSize * resScale);
            auto alpha = config->Widget.Equipment.Weapon.bgAlpha;

            widgetHandler->LoadWidget(id, path, offsetX, offsetY, width, height, alpha);
        }
    } else if (this->type == EquipmentBase::Type::SHOUT) {
        auto shout = this;

        if (!config->Widget.General.hudDiamondEnable) return;
        if (!config->Widget.General.showEmptySlots && player && !Actor::GetEquippedShout(player)) {
            return;
        }
        if (config->Widget.Equipment.Shout.bgType == "_NONE" || config->Widget.Equipment.Shout.bgAlpha == 0) {
            return;
        }

        if (shout->widgetIcon.enable) {
            auto id = shout->widgetID.background;
            auto path = config->GetWidgetPath(config->Widget.Equipment.Shout.bgType);
            auto offsetX = shout->widgetIcon.offsetX;
            auto offsetY = shout->widgetIcon.offsetY;
            auto width = static_cast<int32_t>(1.3f * (float)config->Widget.Equipment.Shout.bgSize * resScale);
            auto height = static_cast<int32_t>(1.3f * (float)config->Widget.Equipment.Shout.bgSize * resScale);
            auto alpha = config->Widget.Equipment.Shout.bgAlpha;

            widgetHandler->LoadWidget(id, path, offsetX, offsetY, width, height, alpha);
        }
    }
}

void EquipmentBase::CreateWidgetIcon(bool _unequip) {
    auto config = ConfigHandler::GetSingleton();
    if (!config) return;

    auto widgetHandler = WidgetHandler::GetSingleton();
    if (!widgetHandler) return;

    float resScale = config->Widget.General.autoResolutionScale ? Utility::GetResolutionScale() : 1.0f;
    auto player = RE::PlayerCharacter::GetSingleton();

    if (this->type == EquipmentBase::Type::ARMOR) {
        auto armor = static_cast<EquipmentArmor*>(this);
        if (!armor) return;

        if (!config->Widget.General.hudArmorEnable) return;
        if (!config->Widget.General.showEmptySlots && !IsArmorSlotEquipped(armor->slotid)) {
            return;
        }

        if (armor->widgetIcon.enable) {
            auto id = armor->widgetID.icon;
            std::string path = "";
            auto offsetX = armor->widgetIcon.offsetX;
            auto offsetY = armor->widgetIcon.offsetY;
            auto width = static_cast<int32_t>((float)config->Widget.Equipment.Armor.widgetSize * resScale);
            auto height = static_cast<int32_t>((float)config->Widget.Equipment.Armor.widgetSize * resScale);

            for (auto& elem : config->eq_widgetVec) {
                if (elem.id == armor->slotid) {
                    path = elem.path;
                }
            }

            widgetHandler->LoadWidget(id, path, offsetX, offsetY, width, height, 100);
        }
    } else if (this->type == EquipmentBase::Type::WEAPON) {
        auto weapon = static_cast<EquipmentWeapon*>(this);
        if (!weapon) return;

        if (!config->Widget.General.hudDiamondEnable) return;
        if (weapon->isLeft && !config->Widget.General.hudDiamondLeftEnable) return;
        if (!config->Widget.General.showEmptySlots && player && !player->GetEquippedObject(weapon->isLeft)) {
            return;
        }

        if (weapon->widgetIcon.enable) {
            auto id = weapon->widgetID.icon;
            auto path = GetWeaponWidgetPath(weapon->isLeft);
            auto offsetX = weapon->widgetIcon.offsetX;
            auto offsetY = weapon->widgetIcon.offsetY;
            auto width = static_cast<int32_t>((float)config->Widget.Equipment.Weapon.widgetSize * resScale);
            auto height = static_cast<int32_t>((float)config->Widget.Equipment.Weapon.widgetSize * resScale);

            widgetHandler->LoadWidget(id, path, offsetX, offsetY, width, height, 100);
        }
    } else if (this->type == EquipmentBase::Type::SHOUT) {
        auto shout = this;

        if (!config->Widget.General.hudDiamondEnable) return;
        if (!config->Widget.General.showEmptySlots && player && !Actor::GetEquippedShout(player)) {
            return;
        }

        if (shout->widgetIcon.enable) {
            auto id = shout->widgetID.icon;
            auto path = GetShoutWidgetPath(_unequip);
            auto offsetX = shout->widgetIcon.offsetX;
            auto offsetY = shout->widgetIcon.offsetY;
            auto width = static_cast<int32_t>((float)config->Widget.Equipment.Shout.widgetSize * resScale);
            auto height = static_cast<int32_t>((float)config->Widget.Equipment.Shout.widgetSize * resScale);

            widgetHandler->LoadWidget(id, path, offsetX, offsetY, width, height, 100);
        }
    }
}

void EquipmentBase::CreateWidgetText1() {
    auto config = ConfigHandler::GetSingleton();
    if (!config) return;

    auto widgetHandler = WidgetHandler::GetSingleton();
    if (!widgetHandler) return;

    auto getFont = [](ConfigHandler* cfg) -> std::string {
        if (cfg && !cfg->fontVec.empty()) {
            if (cfg->Widget.General.font < cfg->fontVec.size()) {
                return cfg->fontVec[cfg->Widget.General.font];
            }
            return cfg->fontVec[0];
        }
        return "$EverywhereFont";
    };

    float resScale = config->Widget.General.autoResolutionScale ? Utility::GetResolutionScale() : 1.0f;
    auto player = RE::PlayerCharacter::GetSingleton();

    if (this->type == EquipmentBase::Type::ARMOR) {
        auto armor = static_cast<EquipmentArmor*>(this);
        if (!armor) return;

        if (!config->Widget.General.hudArmorEnable) return;
        if (!config->Widget.General.showEmptySlots && !IsArmorSlotEquipped(armor->slotid)) {
            return;
        }

        if (armor->widgetIcon.enable && armor->widgetName.enable) {
            auto id = armor->widgetID.text1;
            auto text = GetArmorWidgetName(armor->slotid);
            auto font = getFont(config);
            auto offsetX = armor->widgetName.offsetX + armor->widgetIcon.offsetX;
            auto offsetY = armor->widgetName.offsetY + armor->widgetIcon.offsetY;
            auto align = static_cast<uint32_t>(armor->widgetName.align);
            auto size = static_cast<int32_t>(0.25f * (float)config->Widget.Equipment.Armor.fontSize * resScale);
            auto shadow = config->Widget.Equipment.Armor.fontShadow;

            widgetHandler->LoadText(id, text, font, offsetX, offsetY, align, size, 100, shadow);
        }
    } else if (this->type == EquipmentBase::Type::WEAPON) {
        auto weapon = static_cast<EquipmentWeapon*>(this);
        if (!weapon) return;

        if (!config->Widget.General.hudDiamondEnable) return;
        if (weapon->isLeft && !config->Widget.General.hudDiamondLeftEnable) return;
        if (!config->Widget.General.showEmptySlots && player && !player->GetEquippedObject(weapon->isLeft)) {
            return;
        }

        if (weapon->widgetIcon.enable && weapon->widgetName.enable) {
            auto id = weapon->widgetID.text1;
            auto text = GetWeaponWidgetName(weapon->isLeft);
            auto font = getFont(config);
            auto offsetX = weapon->widgetName.offsetX + weapon->widgetIcon.offsetX;
            auto offsetY = weapon->widgetName.offsetY + weapon->widgetIcon.offsetY;
            auto align = static_cast<uint32_t>(weapon->widgetName.align);
            auto size = static_cast<int32_t>(0.25f * (float)config->Widget.Equipment.Weapon.fontSize * resScale);
            auto shadow = config->Widget.Equipment.Weapon.fontShadow;

            widgetHandler->LoadText(id, text, font, offsetX, offsetY, align, size, 100, shadow);
        }
    } else if (this->type == EquipmentBase::Type::SHOUT) {
        auto shout = this;

        if (!config->Widget.General.hudDiamondEnable) return;
        if (!config->Widget.General.showEmptySlots && player && !Actor::GetEquippedShout(player)) {
            return;
        }

        if (shout->widgetIcon.enable && shout->widgetName.enable) {
            auto id = shout->widgetID.text1;
            auto text = GetShoutWidgetName();
            auto font = getFont(config);
            auto offsetX = shout->widgetName.offsetX + shout->widgetIcon.offsetX;
            auto offsetY = shout->widgetName.offsetY + shout->widgetIcon.offsetY;
            auto align = static_cast<uint32_t>(shout->widgetName.align);
            auto size = static_cast<int32_t>(0.25f * (float)config->Widget.Equipment.Shout.fontSize * resScale);
            auto shadow = config->Widget.Equipment.Shout.fontShadow;

            widgetHandler->LoadText(id, text, font, offsetX, offsetY, align, size, 100, shadow);
        }
    }
}

void EquipmentManager::CreateAllArmorWidget() {
    auto config = ConfigHandler::GetSingleton();
    if (config && config->Widget.General.hudArmorAutoStack) {
        AutoArrangeArmorSlots();
    }
    for (auto& elem : this->armor) {
        elem.CreateWidgetBackground();
        elem.CreateWidgetIcon();
        elem.CreateWidgetText1();
    }
}

void EquipmentManager::RemoveAllArmorWidget() {
    auto widgetHandler = WidgetHandler::GetSingleton();
    if (!widgetHandler) return;

    for (auto& elem : this->armor) {
        widgetHandler->UnloadWidget(elem.widgetID.background);
        widgetHandler->UnloadWidget(elem.widgetID.icon);
        widgetHandler->UnloadText(elem.widgetID.text1);
    }
}

void EquipmentManager::CreateAllWeaponWidget() {
    this->lefthand.CreateWidgetBackground();
    this->lefthand.CreateWidgetIcon();
    this->lefthand.CreateWidgetText1();
    this->righthand.CreateWidgetBackground();
    this->righthand.CreateWidgetIcon();
    this->righthand.CreateWidgetText1();
}

void EquipmentManager::RemoveAllWeaponWidget() {
    auto widgetHandler = WidgetHandler::GetSingleton();
    if (!widgetHandler) return;

    widgetHandler->UnloadWidget(this->lefthand.widgetID.background);
    widgetHandler->UnloadWidget(this->lefthand.widgetID.icon);
    widgetHandler->UnloadText(this->lefthand.widgetID.text1);
    widgetHandler->UnloadWidget(this->righthand.widgetID.background);
    widgetHandler->UnloadWidget(this->righthand.widgetID.icon);
    widgetHandler->UnloadText(this->righthand.widgetID.text1);
}

void EquipmentManager::CreateAllShoutWidget() {
    this->shout.CreateWidgetBackground();
    this->shout.CreateWidgetIcon();
    this->shout.CreateWidgetText1();
}

void EquipmentManager::RemoveAllShoutWidget() {
    auto widgetHandler = WidgetHandler::GetSingleton();
    if (!widgetHandler) return;

    widgetHandler->UnloadWidget(this->shout.widgetID.background);
    widgetHandler->UnloadWidget(this->shout.widgetID.icon);
    widgetHandler->UnloadText(this->shout.widgetID.text1);
}

void EquipmentManager::AutoArrangeArmorSlots() {
    auto config = ConfigHandler::GetSingleton();
    if (!config) return;

    // The 4 core armor symbols ALWAYS stay together at the bottom:
    // Slot 37: Stiefel (Boots) - bottom-most anchor (Priority 1)
    // Slot 33: Handschuhe (Gauntlets) (Priority 2)
    // Slot 32: Rüstung (Cuirass) (Priority 3)
    // Slot 42: Helm (Helmet) - top of the core 4 (Priority 4)
    // Any new / additional slots start directly ABOVE the helmet (Priority >= 5)!
    auto GetSlotPriority = [](int slotIndex) -> int {
        int slotNum = slotIndex + 30;
        switch (slotNum) {
            // Core 4 slots (1..4) - always stay together at the bottom:
            case 37: return 1;   // Feet / Boots (Stiefel - base anchor)
            case 33: return 2;   // Hands / Gauntlets (Handschuhe)
            case 32: return 3;   // Body / Cuirass (Rüstung)
            case 42: return 4;   // Head / Helmet (Helm - top of core 4)

            // Additional slots start directly ABOVE the helm (5+):
            case 39: return 5;   // Shield (Schild)
            case 35: return 6;   // Necklace / Amulet (Halskette / Amulett)
            case 36: return 7;   // Ring
            case 46: return 8;   // Cloak / Cape (Umhang / Cape)
            case 47: return 9;   // Backpack (Rucksack)
            case 48: return 10;  // Bandolier
            case 49: return 11;  // Pouches / Bags (Taschen / Beutel)
            case 44: return 12;  // Face / Mask (Gesicht / Maske)
            case 43: return 13;  // Ears (Ohren)
            case 45: return 14;  // Neck (Hals)
            case 34: return 15;  // Forearms (Unterarme)
            case 38: return 16;  // Calves (Waden)
            case 40: return 17;  // Tail (Schwanz)
            case 30: return 18;  // Hair / Alternate Head (Kopf / Haare)
            case 31: return 19;  // Hair (Haare)
            case 41: return 20;  // Long Hair (Lange Haare)
            default: return 21 + slotIndex;  // Other modded slots 50-61
        }
    };

    auto player = RE::PlayerCharacter::GetSingleton();
    std::vector<int> activeIndices;
    for (int i = 0; i < static_cast<int>(this->armor.size()); i++) {
        if (!this->armor[i].widgetIcon.enable) continue;
        if (!config->Widget.General.showEmptySlots && player && !IsArmorSlotEquipped(this->armor[i].slotid)) {
            continue;
        }
        activeIndices.push_back(i);
    }

    std::sort(activeIndices.begin(), activeIndices.end(), [&](int a, int b) {
        return GetSlotPriority(a) < GetSlotPriority(b);
    });

    float resScale = config->Widget.General.autoResolutionScale ? Utility::GetResolutionScale() : 1.0f;
    float iconH = (float)config->Widget.Equipment.Armor.widgetSize * resScale;
    float bgH = (config->Widget.Equipment.Armor.bgType != "_NONE" && config->Widget.Equipment.Armor.bgAlpha > 0) ?
                (1.3f * (float)config->Widget.Equipment.Armor.bgSize * resScale) : 0.0f;
    float visualIconH = std::max(iconH, bgH);
    float textH = 0.25f * (float)config->Widget.Equipment.Armor.fontSize * resScale;
    float slotH = std::max(visualIconH, textH);

    // Dynamic vertical step: icon height plus breathing gap (never overlap at ANY scale)
    float gap = std::max(8.0f * resScale, slotH * 0.25f);
    int32_t stepY = static_cast<int32_t>(std::ceil(slotH + gap));

    // Dynamic horizontal text offset: from icon center to beyond icon right edge plus padding
    float textGap = std::max(10.0f * resScale, visualIconH * 0.20f);
    int32_t nameOffsetX = static_cast<int32_t>(std::ceil((visualIconH * 0.5f) + textGap));

    const int32_t baseX = this->armorStackBaseX;
    const int32_t baseY = this->armorStackBaseY;
    const int32_t minY = 60;          // Never go above screen top edge
    const int32_t columnWidth = static_cast<int32_t>(std::max(240.0f * resScale, visualIconH + 220.0f * resScale));

    int32_t currentX = baseX;
    int32_t currentY = baseY;

    for (int idx : activeIndices) {
        if (currentY < minY) {
            // Screen edge reached going upwards: start 2nd column next to it!
            currentX += columnWidth;
            currentY = baseY;
        }

        this->armor[idx].widgetIcon.offsetX = currentX;
        this->armor[idx].widgetIcon.offsetY = currentY;

        this->armor[idx].widgetName.align = WidgetText::ALIGN_TYPE::LEFT;
        this->armor[idx].widgetName.offsetX = nameOffsetX;
        this->armor[idx].widgetName.offsetY = 0;

        currentY -= stepY;
    }
}

void EquipmentManager::AutoArrangeDiamondCluster() {
    auto config = ConfigHandler::GetSingleton();
    if (!config) return;

    int32_t centerX = (this->lefthand.widgetIcon.offsetX + this->righthand.widgetIcon.offsetX) / 2;
    int32_t centerY = this->lefthand.widgetIcon.offsetY;

    if (centerX <= 0 || centerY <= 0) {
        float stageW = Utility::GetStageWidth();
        float stageH = Utility::GetStageHeight();
        centerX = static_cast<int32_t>(stageW - 180.0f);
        centerY = static_cast<int32_t>(stageH - 140.0f);
    }

    float resScale = config->Widget.General.autoResolutionScale ? Utility::GetResolutionScale() : 1.0f;
    float weaponIcon = (float)config->Widget.Equipment.Weapon.widgetSize * resScale;
    float shoutIcon = (float)config->Widget.Equipment.Shout.widgetSize * resScale;
    float weaponBg = (config->Widget.Equipment.Weapon.bgType != "_NONE" && config->Widget.Equipment.Weapon.bgAlpha > 0) ?
                     (1.3f * (float)config->Widget.Equipment.Weapon.bgSize * resScale) : 0.0f;
    float shoutBg = (config->Widget.Equipment.Shout.bgType != "_NONE" && config->Widget.Equipment.Shout.bgAlpha > 0) ?
                    (1.3f * (float)config->Widget.Equipment.Shout.bgSize * resScale) : 0.0f;

    float maxVisual = std::max({weaponIcon, shoutIcon, weaponBg, shoutBg});
    int32_t radius = std::max(40, static_cast<int32_t>(std::ceil(maxVisual * 0.85f)));

    this->shout.widgetIcon.offsetX = centerX;
    this->shout.widgetIcon.offsetY = centerY - radius;
    this->shout.widgetName.align = WidgetText::ALIGN_TYPE::CENTER;
    this->shout.widgetName.offsetX = 0;
    this->shout.widgetName.offsetY = -static_cast<int32_t>(std::ceil((radius * 0.35f) + (shoutIcon * 0.5f) + 10.0f * resScale));

    this->lefthand.widgetIcon.offsetX = centerX - radius;
    this->lefthand.widgetIcon.offsetY = centerY;
    this->lefthand.widgetName.align = WidgetText::ALIGN_TYPE::CENTER;
    this->lefthand.widgetName.offsetX = -static_cast<int32_t>(std::ceil(radius * 0.30f));
    this->lefthand.widgetName.offsetY = static_cast<int32_t>(std::ceil((radius * 0.35f) + (weaponIcon * 0.5f) + 8.0f * resScale));

    this->righthand.widgetIcon.offsetX = centerX + radius;
    this->righthand.widgetIcon.offsetY = centerY;
    this->righthand.widgetName.align = WidgetText::ALIGN_TYPE::CENTER;
    this->righthand.widgetName.offsetX = static_cast<int32_t>(std::ceil(radius * 0.30f));
    this->righthand.widgetName.offsetY = static_cast<int32_t>(std::ceil((radius * 0.35f) + (weaponIcon * 0.5f) + 26.0f * resScale));
}

void EquipmentManager::ResetToDefaults() {
    float stageW = Utility::GetStageWidth();
    float stageH = Utility::GetStageHeight();

    int32_t base_X = static_cast<int32_t>(stageW - 180.0f);
    int32_t base_Y = static_cast<int32_t>(stageH - 140.0f);

    this->shout.widgetIcon.enable = true;
    this->shout.widgetName.enable = true;
    this->lefthand.widgetIcon.enable = true;
    this->lefthand.widgetName.enable = true;
    this->righthand.widgetIcon.enable = true;
    this->righthand.widgetName.enable = true;

    this->shout.widgetIcon.offsetX = base_X;
    this->lefthand.widgetIcon.offsetY = base_Y;
    this->righthand.widgetIcon.offsetX = base_X + 40;
    this->lefthand.widgetIcon.offsetX = base_X - 40;
    AutoArrangeDiamondCluster();

    // Setup Armor default layout on the left side:
    this->armorStackBaseX = 35;
    this->armorStackBaseY = 466;
    for (int i = 0; i < 32; i++) {
        this->armor[i].widgetIcon.enable = false;
        this->armor[i].widgetIcon.offsetX = 35;
        this->armor[i].widgetIcon.offsetY = 0;
        this->armor[i].widgetName.enable = false;
        this->armor[i].widgetName.align = WidgetText::ALIGN_TYPE::LEFT;
        this->armor[i].widgetName.offsetX = 22;
        this->armor[i].widgetName.offsetY = 0;
    }

    // Default slots: Head (42 -> index 12), Body (32 -> index 2), Hands (33 -> index 3), Feet (37 -> index 7)
    this->armor[12].widgetIcon.enable = true;
    this->armor[12].widgetName.enable = true;

    this->armor[2].widgetIcon.enable = true;
    this->armor[2].widgetName.enable = true;

    this->armor[3].widgetIcon.enable = true;
    this->armor[3].widgetName.enable = true;

    this->armor[7].widgetIcon.enable = true;
    this->armor[7].widgetName.enable = true;

    AutoArrangeArmorSlots();
}

void EquipmentManager::Load() {
    if (!std::filesystem::exists(equipment_path)) {
        ResetToDefaults();
        Save();
        return;
    }

    try {
        auto tbl = toml::parse_file(equipment_path.c_str());

        this->armorStackBaseX = tbl["ArmorStack"]["baseX"].value_or<int32_t>(35);
        this->armorStackBaseY = tbl["ArmorStack"]["baseY"].value_or<int32_t>(466);
        
        for (int i = 0; i < 32; i++) {
            auto& armor = this->armor[i];
            auto section = fmt::format("Armor{}", i);

            armor.widgetIcon.enable = tbl[section]["icon_enable"].value_or<bool>(false);
            armor.widgetIcon.offsetX = tbl[section]["icon_offsetX"].value_or<int>(0);
            armor.widgetIcon.offsetY = tbl[section]["icon_offsetY"].value_or<int>(0);
            armor.widgetName.enable = tbl[section]["name_enable"].value_or<bool>(false);
            armor.widgetName.align =
                static_cast<WidgetText::ALIGN_TYPE>(tbl[section]["name_align"].value_or<uint32_t>(2));
            armor.widgetName.offsetX = tbl[section]["name_offsetX"].value_or<int>(0);
            armor.widgetName.offsetY = tbl[section]["name_offsetY"].value_or<int>(0);
        }

        for (int i = 0; i < 2; i++) {
            auto& weapon = i == 0 ? this->lefthand : this->righthand;
            auto section = i == 0 ? "Lefthand" : "Righthand";

            weapon.widgetIcon.enable = tbl[section]["icon_enable"].value_or<bool>(false);
            weapon.widgetIcon.offsetX = tbl[section]["icon_offsetX"].value_or<int>(0);
            weapon.widgetIcon.offsetY = tbl[section]["icon_offsetY"].value_or<int>(0);
            weapon.widgetName.enable = tbl[section]["name_enable"].value_or<bool>(false);
            weapon.widgetName.align =
                static_cast<WidgetText::ALIGN_TYPE>(tbl[section]["name_align"].value_or<uint32_t>(2));
            weapon.widgetName.offsetX = tbl[section]["name_offsetX"].value_or<int>(0);
            weapon.widgetName.offsetY = tbl[section]["name_offsetY"].value_or<int>(0);
        }

        {
            this->shout.widgetIcon.enable = tbl["Shout"]["icon_enable"].value_or<bool>(false);
            this->shout.widgetIcon.offsetX = tbl["Shout"]["icon_offsetX"].value_or<int>(0);
            this->shout.widgetIcon.offsetY = tbl["Shout"]["icon_offsetY"].value_or<int>(0);
            this->shout.widgetName.enable = tbl["Shout"]["name_enable"].value_or<bool>(false);
            this->shout.widgetName.align =
                static_cast<WidgetText::ALIGN_TYPE>(tbl["Shout"]["name_align"].value_or<uint32_t>(2));
            this->shout.widgetName.offsetX = tbl["Shout"]["name_offsetX"].value_or<int>(0);
            this->shout.widgetName.offsetY = tbl["Shout"]["name_offsetY"].value_or<int>(0);
        }

        if ((this->lefthand.widgetIcon.offsetX == 0 && this->lefthand.widgetIcon.offsetY == 0 &&
             this->righthand.widgetIcon.offsetX == 0 && this->righthand.widgetIcon.offsetY == 0 &&
             this->shout.widgetIcon.offsetX == 0 && this->shout.widgetIcon.offsetY == 0) ||
            (this->armor[2].widgetIcon.offsetY == 405 && this->armor[3].widgetIcon.offsetY == 450)) {
            ResetToDefaults();
            Save();
        }

        logger::info("Equipment data loaded.");
    } catch (const toml::parse_error& err) {
        logger::warn("Failed to parse equipment file. Using default settings.\nError: {}", err.description());
        ResetToDefaults();
    }
}

void EquipmentManager::Save() {
    std::ofstream f(equipment_path);
    if (!f.is_open()) {
        logger::error("Failed to save equipment!");
        return;
    }

    toml::table mainTbl;
    
    for (int i = 0; i < 32; i++) {
        auto& armor = this->armor[i];
        auto tbl = toml::table{
            {"icon_enable", armor.widgetIcon.enable},
            {"icon_offsetX", armor.widgetIcon.offsetX},
            {"icon_offsetY", armor.widgetIcon.offsetY},
            {"name_enable", armor.widgetName.enable},
            {"name_align", static_cast<uint32_t>(armor.widgetName.align)},
            {"name_offsetX", armor.widgetName.offsetX},
            {"name_offsetY", armor.widgetName.offsetY},
        };

        mainTbl.insert(fmt::format("Armor{}", i), tbl);
    }

    for (int i = 0; i < 2; i++) {
        auto& weapon = i == 0 ? this->lefthand : this->righthand;
        auto tbl = toml::table{
            {"icon_enable", weapon.widgetIcon.enable},
            {"icon_offsetX", weapon.widgetIcon.offsetX},
            {"icon_offsetY", weapon.widgetIcon.offsetY},
            {"name_enable", weapon.widgetName.enable},
            {"name_align", static_cast<uint32_t>(weapon.widgetName.align)},
            {"name_offsetX", weapon.widgetName.offsetX},
            {"name_offsetY", weapon.widgetName.offsetY},
        };

        std::string weaponstr = i == 0 ? "Lefthand" : "Righthand";
        mainTbl.insert(weaponstr, tbl);
    }

    {
        auto tbl = toml::table{
            {"icon_enable", this->shout.widgetIcon.enable},
            {"icon_offsetX", this->shout.widgetIcon.offsetX},
            {"icon_offsetY", this->shout.widgetIcon.offsetY},
            {"name_enable", this->shout.widgetName.enable},
            {"name_align", static_cast<uint32_t>(this->shout.widgetName.align)},
            {"name_offsetX", this->shout.widgetName.offsetX},
            {"name_offsetY", this->shout.widgetName.offsetY},
        };

        mainTbl.insert("Shout", tbl);
    }

    {
        auto tbl = toml::table{
            {"baseX", this->armorStackBaseX},
            {"baseY", this->armorStackBaseY}
        };
        mainTbl.insert("ArmorStack", tbl);
    }

    f << mainTbl;
    logger::info("Equipment saved.");
}