#include "Equipset.h"

#include "EquipsetManager.h"
#include "Actor.h"
#include "Offset.h"
#include "Data.h"
#include "WidgetHandler.h"
#include "Config.h"
#include "ExtraData.h"
#include "Utility.h"

#include <future>

static void EquipItem(RE::TESForm* _form, RE::BGSEquipSlot* _slot, bool _sound, RE::ExtraDataList* _xList,
                      bool _queue = true, bool _force = false) {
    if (!_form) return;

    auto equipManager = RE::ActorEquipManager::GetSingleton();
    if (!equipManager) return;

    auto player = RE::PlayerCharacter::GetSingleton();
    if (!player) return;

    // Spell
    if (_form->Is(RE::FormType::Spell)) {
        RE::SpellItem* spell = _form->As<RE::SpellItem>();
        if (spell) {
            if (Actor::HasMagic(player, spell)) {
                equipManager->EquipSpell(player, spell, _slot);
            }
        }

    // Shout
    } else if (_form->Is(RE::FormType::Shout)) {
        RE::TESShout* shout = _form->As<RE::TESShout>();
        if (shout && Actor::HasShout(player, shout)) {
            equipManager->EquipShout(player, shout);
        }

    // Items
    } else {
        if (_form->Is(RE::FormType::Light, RE::FormType::AlchemyItem)) {
            if (Actor::HasItem(player, _form)) {
                auto object = _form->As<RE::TESBoundObject>();
                if (object) {
                    equipManager->EquipObject(player, object, nullptr, 1U, nullptr, _queue, _force, _sound, false);
                }
            }
        } else if (_form->Is(RE::FormType::Weapon, RE::FormType::Armor)) {
            auto dummy = GetDummyDagger();
            if (Actor::HasItem(player, _form) || (dummy && _form->formID == dummy->formID)) {
                auto object = _form->As<RE::TESBoundObject>();
                if (object) {
                    auto armor = _form->As<RE::TESObjectARMO>();
                    if (armor && armor->IsShield()) {
                        equipManager->EquipObject(player, object, _xList, 1U, nullptr, _queue, _force, _sound, false);
                    } else {
                        equipManager->EquipObject(player, object, _xList, 1U, _slot, _queue, _force, _sound, false);
                    }
                }
            }
        }
    }
}

static void UnequipItem(RE::TESForm* _form, RE::BGSEquipSlot* _slot, bool _sound, RE::ExtraDataList* _xList,
                        bool _queue = false, bool _force = false) {
    if (!_form) return;

    auto equipManager = RE::ActorEquipManager::GetSingleton();
    if (!equipManager) return;

    auto player = RE::PlayerCharacter::GetSingleton();
    if (!player) return;

    if (_form->Is(RE::FormType::Shout)) {
        // Need to figure out Unequip Shout

    } else {
        RE::TESBoundObject* object = _form->As<RE::TESBoundObject>();
        if (object) {
            equipManager->UnequipObject(player, object, _xList, 1U, _slot, _queue, _force, _sound, false, nullptr);
        }
    }
}

static bool isTwoHandedWeapon(RE::TESObjectWEAP* obj) {
    if (!obj) return false;
    if (obj->IsTwoHandedAxe() || obj->IsTwoHandedSword() || obj->IsBow() || obj->IsCrossbow()) {
        return true;
    }
    return false;
}

void NormalSet::Equip() {
    auto player = RE::PlayerCharacter::GetSingleton();
    if (!player) return;

	auto equipset = this;
	
    bool unequipLeft = false, unequipRight = false, unequipShout = false;
    bool equipLeft = false, equipRight = false, equipShout = false;
    
    std::vector<bool> unequipItems(equipset->items.size(), false);
    std::vector<bool> equipItems(equipset->items.size(), false);
    
    unequipLeft = equipset->lefthand.type == Data::DATATYPE::UNEQUIP ? true : unequipLeft;
    unequipRight = equipset->righthand.type == Data::DATATYPE::UNEQUIP ? true : unequipRight;
    unequipShout = equipset->shout.type == Data::DATATYPE::UNEQUIP ? true : unequipShout;

    RE::TESForm* equippedLeft = player->GetEquippedObject(true);
    RE::TESForm* equippedRight = player->GetEquippedObject(false);
    RE::TESForm* equippedShout = Actor::GetEquippedShout(player);

    // Toggle equip/unequip option Off & Re equip option On    
    if (!equipset->toggleEquip && equipset->reEquip) {
        if (equipset->lefthand.type != Data::DATATYPE::NOTHING &&
            equipset->lefthand.type != Data::DATATYPE::UNEQUIP) {
            unequipLeft = true;
            equipLeft = true;
        }

        if (equipset->righthand.type != Data::DATATYPE::NOTHING &&
            equipset->righthand.type != Data::DATATYPE::UNEQUIP) {
            unequipRight = true;
            equipRight = true;
        }

        if (equipset->shout.type != Data::DATATYPE::NOTHING &&
            equipset->shout.type != Data::DATATYPE::UNEQUIP) {
            unequipShout = true;
            equipShout = true;
        }

        for (int i = 0; i < equipset->items.size(); i++) {
            unequipItems[i] = true;
            equipItems[i] = true;
        }
    } else {
        // Toggle equip/unequip option Off & Re equip option Off
        if (equipset->lefthand.type != Data::DATATYPE::NOTHING &&
            equipset->lefthand.type != Data::DATATYPE::UNEQUIP) {
            if (!equippedLeft) {
                equipLeft = true;
            } else {
                const char* leftName = equippedLeft->GetName();
                if (!leftName || leftName != equipset->lefthand.name) {
                    equipLeft = true;
                }
            }
        }
        if (equipset->righthand.type != Data::DATATYPE::NOTHING &&
            equipset->righthand.type != Data::DATATYPE::UNEQUIP) {
            if (!equippedRight) {
                equipRight = true;
            } else {
                const char* rightName = equippedRight->GetName();
                if (!rightName || rightName != equipset->righthand.name) {
                    equipRight = true;
                }
            }
        }
        if (equipset->shout.type != Data::DATATYPE::NOTHING &&
            equipset->shout.type != Data::DATATYPE::UNEQUIP) {
            if (!equippedShout) {
                equipShout = true;
            } else {
                const char* shoutName = equippedShout->GetName();
                if (!shoutName || shoutName != equipset->shout.name) {
                    equipShout = true;
                }
            }
        }

        uint32_t count = 0U;
        if (equipset->items.size() > 0) {
            std::vector<std::string> itemsName;
            std::vector<bool> isEquipped(equipset->items.size(), false);

            std::vector<RE::TESForm*> items = Actor::GetAllEquippedItems();
            for (const auto& elem : items) {
                const char* elemName = elem ? elem->GetName() : nullptr;
                if (elemName) {
                    itemsName.push_back(elemName);
                } else {
                    itemsName.push_back("_NONE_");
                }
            }

            for (int i = 0; i < equipset->items.size(); i++) {
                for (int j = 0; j < itemsName.size(); ++j) {
                    if (itemsName[j] == equipset->items[i].name) {
                        isEquipped[i] = true;
                        ++count;
                    }
                }
            }

            for (int i = 0; i < equipset->items.size(); i++) {
                if (!isEquipped[i]) {
                    equipItems[i] = true;
                }
            }
        }
        if (equipset->toggleEquip) {
            // Toggle equip/unequip option On & Re equip option Off
            if (!equipRight && !equipLeft && !equipShout && count == equipset->items.size()) {
                unequipLeft = true;
                unequipRight = true;
                unequipShout = true;
                for (int i = 0; i < equipset->items.size(); i++) {
                    unequipItems[i] = true;
                }
            }

            // Toggle equip/unequip option On & Re equip option On
            else if (equipset->reEquip) {
                unequipLeft = true;
                unequipRight = true;
                unequipShout = true;
                equipLeft = true;
                equipRight = true;
                equipShout = true;
                for (int i = 0; i < equipset->items.size(); i++) {
                    unequipItems[i] = true;
                    equipItems[i] = true;
                }
            }
        }
    }

    auto DummyDagger = GetDummyDagger();
    // auto DummyShout = GetDummyShout();

    if (unequipLeft) {
        if (DummyDagger) {
            EquipItem(DummyDagger, GetLeftHandSlot(), false, nullptr, false, true);
            UnequipItem(DummyDagger, GetLeftHandSlot(), false, nullptr, false, true);
        }
    }
    if (unequipRight) {
        if (DummyDagger) {
            EquipItem(DummyDagger, GetRightHandSlot(), false, nullptr, false, true);
            UnequipItem(DummyDagger, GetRightHandSlot(), false, nullptr, false, true);
        }
    }
    /*
    if (unequipShout) {
        if (DummyShout) {
            EquipItem(DummyShout, nullptr, false, nullptr, false, true);
            UnequipItem(DummyShout, nullptr, false, nullptr, false, true);
        }
    }
    */

    for (int i = 0; i < equipset->items.size(); i++) {
        if (unequipItems[i] && equipset->items[i].form) {
            auto& armor = equipset->items[i];
            auto xList = Extra::SearchExtraDataList(armor.name, armor.enchNum, armor.enchName, armor.tempVal);
            UnequipItem(equipset->items[i].form, nullptr, equipset->equipSound, xList);
        }
    }
    if (equippedLeft != nullptr && equippedLeft->Is(RE::FormType::Weapon) && isTwoHandedWeapon(static_cast<RE::TESObjectWEAP*>(equippedLeft)) 
        && (!equippedLeft->GetName() || equippedLeft->GetName() != equipset->lefthand.name)) {
        EquipItem(DummyDagger, GetLeftHandSlot(), false, nullptr, false, true);
        UnequipItem(DummyDagger, GetLeftHandSlot(), false, nullptr, false, true);
    }
    // notice: need equip left hand weapon, otherwise left hand weapon lost charge
    equippedLeft = player->GetEquippedObject(true);
    if (equippedLeft != nullptr && equippedLeft->GetName() && equippedLeft->GetName() == equipset->lefthand.name) {
        equipLeft = false;
    }
    if (equipLeft && equipset->lefthand.form &&
        equipset->lefthand.type != Data::DATATYPE::NOTHING &&
        equipset->lefthand.type != Data::DATATYPE::UNEQUIP) {
        auto& weapon = equipset->lefthand;
        // add a name salt seem to avoid left hand weapon can`t be equip problem when left hand and right hand has same name weapon
        auto xList = Extra::SearchExtraDataList(weapon.name + "L", weapon.enchNum, weapon.enchName, weapon.tempVal);
        EquipItem(equipset->lefthand.form, GetLeftHandSlot(), equipset->equipSound, xList);
    }
    
    equippedRight = player->GetEquippedObject(false);
    if (equippedRight != nullptr && equippedRight->GetName() && equippedRight->GetName() == equipset->righthand.name) {
        equipRight = false;
    }
    if (equipRight && equipset->righthand.form &&
        equipset->righthand.type != Data::DATATYPE::NOTHING &&
        equipset->righthand.type != Data::DATATYPE::UNEQUIP) {
        auto& weapon = equipset->righthand;
        auto xList = Extra::SearchExtraDataList(weapon.name + "R", weapon.enchNum, weapon.enchName, weapon.tempVal);
        EquipItem(equipset->righthand.form, GetRightHandSlot(), equipset->equipSound, xList);
    }
    if (equipShout && equipset->shout.form &&
        equipset->shout.type != Data::DATATYPE::NOTHING &&
        equipset->shout.type != Data::DATATYPE::UNEQUIP) {
        EquipItem(equipset->shout.form, nullptr, equipset->equipSound, nullptr);
    }

    for (int i = 0; i < equipset->items.size(); i++) {
        if (equipItems[i] && equipset->items[i].form) {
            auto& armor = equipset->items[i];
            auto xList = Extra::SearchExtraDataList(armor.name, armor.enchNum, armor.enchName, armor.tempVal);
            EquipItem(equipset->items[i].form, nullptr, equipset->equipSound, xList);
        }
    }

    auto widgetHandler = WidgetHandler::GetSingleton();
    if (!widgetHandler) return;

    if (equipset->widgetIcon.enable) {
        widgetHandler->UnloadWidget(equipset->widgetID.icon);
        this->CreateWidgetIcon();
    }
}

void PotionSet::Equip() {
    auto player = RE::PlayerCharacter::GetSingleton();
    if (!player) return;

    using TYPE = DataPotion::DATATYPE;
    auto equipset = this;

    // Resolve player-crafted or dynamic potions whose FormID may have changed across brewing or reload
    auto resolvePotion = [&](DataPotion& pot) {
        if (pot.type == TYPE::NOTHING || pot.type == TYPE::POTION_AUTO_HIGHEST || pot.type == TYPE::POTION_AUTO_LOWEST) {
            return;
        }
        if ((!pot.form || !Actor::HasItem(player, pot.form)) && !pot.name.empty()) {
            auto inv = player->GetInventory();
            for (const auto& [item, data] : inv) {
                const auto& [numItem, entry] = data;
                if (numItem > 0 && item && item->Is(RE::FormType::AlchemyItem)) {
                    const char* itemName = item->GetName();
                    if (itemName && itemName == pot.name) {
                        pot.form = item->As<RE::TESForm>();
                        break;
                    }
                }
            }
        }
    };

    resolvePotion(equipset->health);
    resolvePotion(equipset->magicka);
    resolvePotion(equipset->stamina);
    for (auto& item : equipset->items) {
        resolvePotion(item);
    }

    if (equipset->health.type != TYPE::NOTHING) {
        if (equipset->health.form) {
            EquipItem(equipset->health.form, nullptr, equipset->equipSound, nullptr);
        }
    }
    if (equipset->magicka.type != TYPE::NOTHING) {
        if (equipset->magicka.form &&
            equipset->magicka.form != equipset->health.form) {
            EquipItem(equipset->magicka.form, nullptr, equipset->equipSound, nullptr);
        }
    }
    if (equipset->stamina.type != TYPE::NOTHING) {
        if (equipset->stamina.form &&
            equipset->stamina.form != equipset->health.form &&
            equipset->stamina.form != equipset->magicka.form) {
            EquipItem(equipset->stamina.form, nullptr, equipset->equipSound, nullptr);
        }
    }

    for (int i = 0; i < equipset->items.size(); i++) {
        if (equipset->items[i].form) {
            EquipItem(equipset->items[i].form, nullptr, equipset->equipSound, nullptr);
        }
    }

    auto widgetHandler = WidgetHandler::GetSingleton();
    if (!widgetHandler) return;

    if (equipset->widgetIcon.enable) {
        widgetHandler->UnloadWidget(equipset->widgetID.icon);
        this->CreateWidgetIcon();
    }

    if (equipset->widgetAmount.enable) {
        auto amount = equipset->GetPotionAmount();
        widgetHandler->SetText(equipset->widgetID.text2, amount);
    }
}

void CycleSet::Equip() {
    bool test = false;
    if(this->cycleIndex == 1) {
        test = true;
    }

    auto player = RE::PlayerCharacter::GetSingleton();
    if (!player) return;

    auto manager = EquipsetManager::GetSingleton();
    if (!manager) return;

    auto cycleset = this;
    if (cycleset->items.empty()) return;

    bool doNext = true;
    if (cycleset->cyclePersist && isCycleInit) {
        uint32_t prevIndex = cycleset->cycleIndex == 0 ? cycleset->items.size() - 1U : cycleset->cycleIndex - 1U;

        auto prevset = manager->SearchEquipsetByName(cycleset->items[prevIndex]);
        if (prevset && prevset->type == Equipset::TYPE::NORMAL) {
            auto prevNormalset = static_cast<NormalSet*>(prevset);

            bool IsChanged = false;
            std::vector<RE::TESForm*> items = Actor::GetAllEquippedItems();
            std::vector<RE::FormID> itemsID;

            for (const auto& elem : items) {
                if (elem) {
                    itemsID.push_back(elem->GetFormID());
                } else {
                    itemsID.push_back(0);
                }
            }

            if (prevNormalset->lefthand.type != Data::DATATYPE::NOTHING &&
                prevNormalset->lefthand.type != Data::DATATYPE::UNEQUIP) {
                RE::TESForm* form = player->GetEquippedObject(true);
                auto left = prevNormalset->lefthand.form;

                if (!form) {
                    IsChanged = true;
                }

                else if (left && form->GetFormID() != left->GetFormID()) {
                    IsChanged = true;
                }
            }

            if (prevNormalset->righthand.type != Data::DATATYPE::NOTHING &&
                prevNormalset->righthand.type != Data::DATATYPE::UNEQUIP &&
                !IsChanged) {
                RE::TESForm* form = player->GetEquippedObject(false);
                auto right = prevNormalset->righthand.form;

                if (!form) {
                    IsChanged = true;
                }

                else if (right && form->GetFormID() != right->GetFormID()) {
                    IsChanged = true;
                }
            }

            if (prevNormalset->shout.type != Data::DATATYPE::NOTHING &&
                prevNormalset->shout.type != Data::DATATYPE::UNEQUIP &&
                !IsChanged) {
                RE::TESForm* form = Actor::GetEquippedShout(player);
                auto shout = prevNormalset->shout.form;

                if (!form) {
                    IsChanged = true;
                }

                else if (shout && form->GetFormID() != shout->GetFormID()) {
                    IsChanged = true;
                }
            }

            if (!IsChanged) {
                for (const auto& item : prevNormalset->items) {
                    uint32_t count = 0U;

                    for (int i = 0; i < items.size(); i++) {
                        if (item.form && itemsID[i] == item.form->GetFormID()) {
                            ++count;
                        }
                    }
                    if (count == 0U) {
                        IsChanged = true;
                        break;
                    }
                }
            }

            // Equipped weapons, spells, shout, items Changed.
            if (IsChanged) {
                doNext = false;
            }
        }
    }

    if (doNext) {
        if (cycleIndex < cycleset->items.size()) {
            auto equipset = manager->SearchEquipsetByName(cycleset->items[cycleIndex]);
            if (equipset) equipset->Equip();

            cycleset->cycleIndex = cycleset->cycleIndex == cycleset->items.size() - 1 ? 0 : cycleset->cycleIndex + 1;
        }
    } else {
        uint32_t prevIndex = cycleset->cycleIndex == 0 ? cycleset->items.size() - 1U : cycleset->cycleIndex - 1U;

        if (prevIndex < cycleset->items.size()) {
            auto equipset = manager->SearchEquipsetByName(cycleset->items[prevIndex]);
            if (equipset) equipset->Equip();
        }
    }

    isCycleInit = true;

    auto widgetHandler = WidgetHandler::GetSingleton();
    if (!widgetHandler) return;

    if (cycleset->widgetIcon.enable) {
        widgetHandler->UnloadWidget(cycleset->widgetID.icon);
        this->CreateWidgetIcon();
    }

    if (cycleset->widgetName.enable && cycleset->cycleIndex < cycleset->items.size()) {
        widgetHandler->SetText(cycleset->widgetID.text1, cycleset->items[cycleset->cycleIndex]);
    }
}

CycleSet::~CycleSet() {
    CloseExpireTimer();
    CloseResetTimer();
    if (expire_future.valid()) {
        expire_future.wait_for(std::chrono::milliseconds(50));
    }
    if (reset_future.valid()) {
        reset_future.wait_for(std::chrono::milliseconds(50));
    }
}

void CycleSet::SetExpireProgress(const float& _amount) {
    cycleExpireProgress.store(_amount);
}

void CycleSet::StartExpireTimer() {
    if (cycleExpireProgress.load() != 0.0f && cycleExpireProgress.load() < cycleExpire) return;

    cycleExpireProgress.store(0.01f);
    expire_future = std::async(std::launch::async, &CycleSet::ExpireFunc, this);
}

void CycleSet::CloseExpireTimer() {
    if (cycleExpireProgress.load() == 0.0f) return;

    shouldCloseExpire.store(true);
}

bool CycleSet::ExpireFunc() {
    auto lastTime = std::chrono::steady_clock::now();
    while (!shouldCloseExpire.load() && cycleExpireProgress.load() < cycleExpire) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<float> elapsed = now - lastTime;
        lastTime = now;
        cycleExpireProgress.fetch_add(elapsed.count());
    }

    if (!shouldCloseExpire.load()) {
        this->cycleIndex = 0;
        this->isCycleInit = false;
    }
    shouldCloseExpire.store(false);
    cycleExpireProgress.store(0.0f);

    return true;
}

void CycleSet::SetResetProgress(const float& _amount) {
    cycleResetProgress.store(_amount);
}

void CycleSet::StartResetTimer() {
    if (cycleResetProgress.load() != 0.0f && cycleResetProgress.load() < cycleReset) return;

    cycleResetProgress.store(0.01f);
    reset_future = std::async(std::launch::async, &CycleSet::ResetFunc, this);
}

void CycleSet::CloseResetTimer() {
    if (cycleResetProgress.load() == 0.0f) return;
    
    shouldCloseReset.store(true);
}

bool CycleSet::ResetFunc() {
    auto lastTime = std::chrono::steady_clock::now();
    while (!shouldCloseReset.load() && cycleResetProgress.load() < cycleReset) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<float> elapsed = now - lastTime;
        lastTime = now;
        cycleResetProgress.fetch_add(elapsed.count());
    }

    if (!shouldCloseReset.load()) {
        this->cycleIndex = 0;
        this->isCycleInit = false;

        auto task = SKSE::GetTaskInterface();
        if (!task) logger::error("Failed to get task interface.");
        else {
            task->AddTask([this]() {
                this->Equip();
            });
        }
    }
    shouldCloseReset.store(false);
    cycleResetProgress.store(0.0f);

    return true;
}

void Equipset::CreateWidgetBackground() {
    auto widgetHandler = WidgetHandler::GetSingleton();
    if (!widgetHandler) return;

    auto config = ConfigHandler::GetSingleton();
    if (!config) return;

    float resScale = config->Widget.General.autoResolutionScale ? Utility::GetResolutionScale() : 1.0f;

    if (this->type == Equipset::TYPE::NORMAL) {
        auto equipset = static_cast<NormalSet*>(this);
        if (!equipset) return;

        if (equipset->widgetIcon.enable) {
            auto id = equipset->widgetID.background;
            auto path = config->GetWidgetPath(config->Widget.Equipset.Normal.bgType);
            auto offsetX = equipset->widgetIcon.offsetX;
            auto offsetY = equipset->widgetIcon.offsetY;
            auto width = static_cast<int32_t>(1.3f * (float)config->Widget.Equipset.Normal.bgSize * resScale);
            auto height = static_cast<int32_t>(1.3f * (float)config->Widget.Equipset.Normal.bgSize * resScale);
            auto alpha = config->Widget.Equipset.Normal.bgAlpha;
            
            widgetHandler->LoadWidget(id, path, offsetX, offsetY, width, height, alpha);
        }
    } else if (this->type == Equipset::TYPE::POTION) {
        auto equipset = static_cast<PotionSet*>(this);
        if (!equipset) return;

        if (equipset->widgetIcon.enable) {
            auto id = equipset->widgetID.background;
            auto path = config->GetWidgetPath(config->Widget.Equipset.Potion.bgType);
            auto offsetX = equipset->widgetIcon.offsetX;
            auto offsetY = equipset->widgetIcon.offsetY;
            auto width = static_cast<int32_t>(1.3f * (float)config->Widget.Equipset.Potion.bgSize * resScale);
            auto height = static_cast<int32_t>(1.3f * (float)config->Widget.Equipset.Potion.bgSize * resScale);
            auto alpha = config->Widget.Equipset.Potion.bgAlpha;

            widgetHandler->LoadWidget(id, path, offsetX, offsetY, width, height, alpha);
        }
    } else if (this->type == Equipset::TYPE::CYCLE) {
        auto equipset = static_cast<CycleSet*>(this);
        if (!equipset) return;

        if (equipset->widgetIcon.enable) {
            auto id = equipset->widgetID.background;
            auto path = config->GetWidgetPath(config->Widget.Equipset.Cycle.bgType);
            auto offsetX = equipset->widgetIcon.offsetX;
            auto offsetY = equipset->widgetIcon.offsetY;
            auto width = static_cast<int32_t>(1.3f * (float)config->Widget.Equipset.Cycle.bgSize * resScale);
            auto height = static_cast<int32_t>(1.3f * (float)config->Widget.Equipset.Cycle.bgSize * resScale);
            auto alpha = config->Widget.Equipset.Cycle.bgAlpha;
            
            widgetHandler->LoadWidget(id, path, offsetX, offsetY, width, height, alpha);
        }
    }
}

void Equipset::CreateWidgetIcon() {
    auto widgetHandler = WidgetHandler::GetSingleton();
    if (!widgetHandler) return;

    auto config = ConfigHandler::GetSingleton();
    if (!config) return;

    float resScale = config->Widget.General.autoResolutionScale ? Utility::GetResolutionScale() : 1.0f;

    if (this->type == Equipset::TYPE::NORMAL) {
        auto equipset = static_cast<NormalSet*>(this);
        if (!equipset) return;

        if (equipset->widgetIcon.enable) {
            auto id = equipset->widgetID.icon;
            auto path = config->GetWidgetPath(equipset->widgetIcon.type);
            auto offsetX = equipset->widgetIcon.offsetX;
            auto offsetY = equipset->widgetIcon.offsetY;
            auto width = static_cast<int32_t>((float)config->Widget.Equipset.Normal.widgetSize * resScale);
            auto height = static_cast<int32_t>((float)config->Widget.Equipset.Normal.widgetSize * resScale);

            widgetHandler->LoadWidget(id, path, offsetX, offsetY, width, height, 100);
        }
    } else if (this->type == Equipset::TYPE::POTION) {
        auto equipset = static_cast<PotionSet*>(this);
        if (!equipset) return;

        if (equipset->widgetIcon.enable) {
            auto id = equipset->widgetID.icon;
            auto path = config->GetWidgetPath(equipset->widgetIcon.type);
            auto offsetX = equipset->widgetIcon.offsetX;
            auto offsetY = equipset->widgetIcon.offsetY;
            auto width = static_cast<int32_t>((float)config->Widget.Equipset.Potion.widgetSize * resScale);
            auto height = static_cast<int32_t>((float)config->Widget.Equipset.Potion.widgetSize * resScale);

            widgetHandler->LoadWidget(id, path, offsetX, offsetY, width, height, 100);
        }
    } else if (this->type == Equipset::TYPE::CYCLE) {
        auto manager = EquipsetManager::GetSingleton();
        if (!manager) return;

        auto equipset = static_cast<CycleSet*>(this);
        if (!equipset) return;

        if (equipset->widgetIcon.enable && equipset->items.size() > 0) {
            auto id = equipset->widgetID.icon;
            std::string path = "";
            auto offsetX = equipset->widgetIcon.offsetX;
            auto offsetY = equipset->widgetIcon.offsetY;
            auto width = static_cast<int32_t>((float)config->Widget.Equipset.Cycle.widgetSize * resScale);
            auto height = static_cast<int32_t>((float)config->Widget.Equipset.Cycle.widgetSize * resScale);

            if (equipset->cycleIndex < equipset->items.size()) {
                auto& name = equipset->items[equipset->cycleIndex];
                auto found = manager->SearchEquipsetByName(name);
                if (!found) return;

                if (found->type == Equipset::TYPE::NORMAL) {
                    auto normalset = static_cast<NormalSet*>(found);
                    if (normalset) path = config->GetWidgetPath(normalset->widgetIcon.type);
                } else if (found->type == Equipset::TYPE::POTION) {
                    auto potionset = static_cast<PotionSet*>(found);
                    if (potionset) path = config->GetWidgetPath(potionset->widgetIcon.type);
                }
            }

            widgetHandler->LoadWidget(id, path, offsetX, offsetY, width, height, 100);
        }
    }
}

void Equipset::CreateWidgetText1() {
    auto widgetHandler = WidgetHandler::GetSingleton();
    if (!widgetHandler) return;

    auto config = ConfigHandler::GetSingleton();
    if (!config) return;

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

    if (this->type == Equipset::TYPE::NORMAL) {
        auto equipset = static_cast<NormalSet*>(this);
        if (!equipset) return;

        if (equipset->widgetIcon.enable && equipset->widgetName.enable) {
            auto id = equipset->widgetID.text1;
            auto text = equipset->name;
            auto font = getFont(config);
            auto offsetX = equipset->widgetName.offsetX + equipset->widgetIcon.offsetX;
            auto offsetY = equipset->widgetName.offsetY + equipset->widgetIcon.offsetY;
            auto align = static_cast<uint32_t>(equipset->widgetName.align);
            auto size = static_cast<int32_t>(0.25f * (float)config->Widget.Equipset.Normal.fontSize * resScale);
            auto shadow = config->Widget.Equipset.Normal.fontShadow;

            widgetHandler->LoadText(id, text, font, offsetX, offsetY, align, size, 100, shadow);
        }
    } else if (this->type == Equipset::TYPE::POTION) {
        auto equipset = static_cast<PotionSet*>(this);
        if (!equipset) return;

        if (equipset->widgetIcon.enable && equipset->widgetName.enable) {
            auto id = equipset->widgetID.text1;
            auto text = equipset->GetPotionName();
            auto font = getFont(config);
            auto offsetX = equipset->widgetName.offsetX + equipset->widgetIcon.offsetX;
            auto offsetY = equipset->widgetName.offsetY + equipset->widgetIcon.offsetY;
            auto align = static_cast<uint32_t>(equipset->widgetName.align);
            auto size = static_cast<int32_t>(0.25f * (float)config->Widget.Equipset.Potion.fontSize * resScale);
            auto shadow = config->Widget.Equipset.Potion.fontShadow;

            widgetHandler->LoadText(id, text, font, offsetX, offsetY, align, size, 100, shadow);
        }
    } else if (this->type == Equipset::TYPE::CYCLE) {
        auto equipset = static_cast<CycleSet*>(this);
        if (!equipset) return;

        if (equipset->widgetIcon.enable && equipset->widgetName.enable && equipset->items.size() > 0 &&
            equipset->cycleIndex < equipset->items.size()) {
            auto id = equipset->widgetID.text1;
            auto text = equipset->items[equipset->cycleIndex];
            auto font = getFont(config);
            auto offsetX = equipset->widgetName.offsetX + equipset->widgetIcon.offsetX;
            auto offsetY = equipset->widgetName.offsetY + equipset->widgetIcon.offsetY;
            auto align = static_cast<uint32_t>(equipset->widgetName.align);
            auto size = static_cast<int32_t>(0.25f * (float)config->Widget.Equipset.Cycle.fontSize * resScale);
            auto shadow = config->Widget.Equipset.Cycle.fontShadow;

            widgetHandler->LoadText(id, text, font, offsetX, offsetY, align, size, 100, shadow);
        }
    }
}

void Equipset::CreateWidgetText2() {
    auto widgetHandler = WidgetHandler::GetSingleton();
    if (!widgetHandler) return;

    auto config = ConfigHandler::GetSingleton();
    if (!config) return;

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

    if (this->type == Equipset::TYPE::NORMAL) {
        auto equipset = static_cast<NormalSet*>(this);
        if (!equipset) return;

        if (equipset->widgetIcon.enable && equipset->widgetHotkey.enable) {
            auto id = equipset->widgetID.text2;
            const char* kn = ImGui::GetKeyName(static_cast<ImGuiKey>(equipset->hotkey));
            std::string text = kn ? kn : "";
            auto font = getFont(config);
            auto offsetX = equipset->widgetHotkey.offsetX + equipset->widgetIcon.offsetX;
            auto offsetY = equipset->widgetHotkey.offsetY + equipset->widgetIcon.offsetY;
            auto align = static_cast<uint32_t>(equipset->widgetHotkey.align);
            auto size = static_cast<int32_t>(0.25f * (float)config->Widget.Equipset.Normal.fontSize * resScale);
            auto shadow = config->Widget.Equipset.Normal.fontShadow;

            widgetHandler->LoadText(id, text, font, offsetX, offsetY, align, size, 100, shadow);
        }
    } else if (this->type == Equipset::TYPE::POTION) {
        auto equipset = static_cast<PotionSet*>(this);
        if (!equipset) return;

        if (equipset->widgetIcon.enable && equipset->widgetAmount.enable) {
            auto id = equipset->widgetID.text2;
            auto text = equipset->GetPotionAmount();
            auto font = getFont(config);
            auto offsetX = equipset->widgetAmount.offsetX + equipset->widgetIcon.offsetX;
            auto offsetY = equipset->widgetAmount.offsetY + equipset->widgetIcon.offsetY;
            auto align = static_cast<uint32_t>(equipset->widgetAmount.align);
            auto size = static_cast<int32_t>(0.25f * (float)config->Widget.Equipset.Potion.fontSize * resScale);
            auto shadow = config->Widget.Equipset.Potion.fontShadow;

            widgetHandler->LoadText(id, text, font, offsetX, offsetY, align, size, 100, shadow);
        }
    } else if (this->type == Equipset::TYPE::CYCLE) {
        auto equipset = static_cast<CycleSet*>(this);
        if (!equipset) return;

        if (equipset->widgetIcon.enable && equipset->widgetHotkey.enable) {
            auto id = equipset->widgetID.text2;
            const char* kn = ImGui::GetKeyName(static_cast<ImGuiKey>(equipset->hotkey));
            std::string text = kn ? kn : "";
            auto font = getFont(config);
            auto offsetX = equipset->widgetHotkey.offsetX + equipset->widgetIcon.offsetX;
            auto offsetY = equipset->widgetHotkey.offsetY + equipset->widgetIcon.offsetY;
            auto align = static_cast<uint32_t>(equipset->widgetHotkey.align);
            auto size = static_cast<int32_t>(0.25f * (float)config->Widget.Equipset.Cycle.fontSize * resScale);
            auto shadow = config->Widget.Equipset.Cycle.fontShadow;

            widgetHandler->LoadText(id, text, font, offsetX, offsetY, align, size, 100, shadow);
        }
    }
}

void NormalSet::CreateWidget() {
    this->CreateWidgetBackground();
    this->CreateWidgetIcon();
    this->CreateWidgetText1();
    this->CreateWidgetText2();
}

void NormalSet::RemoveWidget() {
    auto widgetHandler = WidgetHandler::GetSingleton();
    if (!widgetHandler) return;

    widgetHandler->UnloadWidget(this->widgetID.background);
    widgetHandler->UnloadWidget(this->widgetID.icon);
    widgetHandler->UnloadText(this->widgetID.text1);
    widgetHandler->UnloadText(this->widgetID.text2);
}

void PotionSet::CreateWidget() {
    this->CreateWidgetBackground();
    this->CreateWidgetIcon();
    this->CreateWidgetText1();
    this->CreateWidgetText2();
}

void PotionSet::RemoveWidget() {
    auto widgetHandler = WidgetHandler::GetSingleton();
    if (!widgetHandler) return;

    widgetHandler->UnloadWidget(this->widgetID.background);
    widgetHandler->UnloadWidget(this->widgetID.icon);
    widgetHandler->UnloadText(this->widgetID.text1);
    widgetHandler->UnloadText(this->widgetID.text2);
}

void CycleSet::CreateWidget() {
    this->CreateWidgetBackground();
    this->CreateWidgetIcon();
    this->CreateWidgetText1();
    this->CreateWidgetText2();
}

void CycleSet::RemoveWidget() {
    auto widgetHandler = WidgetHandler::GetSingleton();
    if (!widgetHandler) return;

    widgetHandler->UnloadWidget(this->widgetID.background);
    widgetHandler->UnloadWidget(this->widgetID.icon);
    widgetHandler->UnloadText(this->widgetID.text1);
    widgetHandler->UnloadText(this->widgetID.text2);
}

void Equipset::SyncEquipset(const std::string& _prevName, const std::string& _curName) {
    auto manager = EquipsetManager::GetSingleton();
    if (!manager) return;

    for (auto elem : manager->equipsetVec) {
        if (elem->type != Equipset::TYPE::CYCLE) continue;

        auto cycleset = static_cast<CycleSet*>(elem);
        for (auto& item : cycleset->items) {
            if (item == _prevName) {
                item = _curName;
                break;
            }
        }
    }
}

void Equipset::SyncWidget() {
    auto manager = EquipsetManager::GetSingleton();
    if (!manager) return;

    auto widgetHandler = WidgetHandler::GetSingleton();
    if (!widgetHandler) return;

    for (auto elem : manager->equipsetVec) {
        if (elem->type != Equipset::TYPE::CYCLE) continue;

        auto cycleset = static_cast<CycleSet*>(elem);
        if (!cycleset) continue;

        cycleset->RemoveWidget();
        cycleset->CreateWidget();
    }
}

static RE::TESForm* GetMinMaxPotion(bool _isMax, bool _calcDuration, const std::vector<RE::AlchemyItem*>& _potion,
                                    const std::vector<float>& _magnitude, const std::vector<uint32_t>& _duration) {
    if (_potion.empty() || _potion.size() != _magnitude.size()) {
        return nullptr;
    }

    int selected_index = -1;
    float best_value = 0.0f;

    for (size_t i = 0; i < _potion.size(); ++i) {
        if (!_potion[i]) continue;

        float magnitude = _magnitude[i];
        float val = magnitude;
        if (_calcDuration) {
            uint32_t dur = (i < _duration.size() && _duration[i] > 0U) ? _duration[i] : 1U;
            val = magnitude * static_cast<float>(dur);
        }

        if (selected_index == -1) {
            best_value = val;
            selected_index = static_cast<int>(i);
        } else if (_isMax) {
            if (val > best_value) {
                best_value = val;
                selected_index = static_cast<int>(i);
            }
        } else {
            if (val < best_value) {
                best_value = val;
                selected_index = static_cast<int>(i);
            }
        }
    }

    if (selected_index >= 0 && static_cast<size_t>(selected_index) < _potion.size() && _potion[selected_index]) {
        return _potion[selected_index]->As<RE::TESForm>();
    }

    return nullptr;
}

void PotionSet::AssignAutoPotion() {
    auto player = RE::PlayerCharacter::GetSingleton();
    if (!player) return;

    auto config = ConfigHandler::GetSingleton();
    if (!config) return;

    auto TESDataHandler = RE::TESDataHandler::GetSingleton();
    if (!TESDataHandler) return;

    std::vector<RE::AlchemyItem*> health;
    std::vector<RE::AlchemyItem*> magicka;
    std::vector<RE::AlchemyItem*> stamina;
    std::vector<float> health_magnitude;
    std::vector<float> magicka_magnitude;
    std::vector<float> stamina_magnitude;
    std::vector<uint32_t> health_duration;
    std::vector<uint32_t> magicka_duration;
    std::vector<uint32_t> stamina_duration;

    auto inv = player->GetInventory();
    for (const auto& [item, data] : inv) {
        const auto& [numItem, entry] = data;
        if (numItem > 0 && item && item->Is(RE::FormType::AlchemyItem)) {
            auto potion = item->As<RE::AlchemyItem>();
            if (!potion) continue;

            for (auto effect : potion->effects) {
                if (!effect) continue;
                auto baseEffect = effect->baseEffect;
                if (!baseEffect) continue;

                bool isHealth = false;
                bool isMagicka = false;
                bool isStamina = false;

                auto formid = baseEffect->GetFormID();
                for (const auto& elem : config->healthVec) {
                    auto compare = TESDataHandler->LookupFormID(elem.formid, elem.modname);
                    if (formid == compare) {
                        isHealth = true;
                        break;
                    }
                }
                for (const auto& elem : config->magickaVec) {
                    auto compare = TESDataHandler->LookupFormID(elem.formid, elem.modname);
                    if (formid == compare) {
                        isMagicka = true;
                        break;
                    }
                }
                for (const auto& elem : config->staminaVec) {
                    auto compare = TESDataHandler->LookupFormID(elem.formid, elem.modname);
                    if (formid == compare) {
                        isStamina = true;
                        break;
                    }
                }

                // Universal fallback: Check ActorValue and Archetype for beneficial restoration/fortification
                // This detects player-crafted potions and modded potions (e.g. CACO, Apothecary)
                if (!isHealth && !isMagicka && !isStamina) {
                    auto av = baseEffect->data.primaryAV;
                    auto arch = baseEffect->GetArchetype();
                    bool isBeneficial = !baseEffect->data.flags.any(RE::EffectSetting::EffectSettingData::Flag::kHostile,
                                                                   RE::EffectSetting::EffectSettingData::Flag::kDetrimental);
                    if (isBeneficial && (arch == RE::EffectSetting::Archetype::kValueModifier ||
                                         arch == RE::EffectSetting::Archetype::kPeakValueModifier)) {
                        if (av == RE::ActorValue::kHealth) isHealth = true;
                        else if (av == RE::ActorValue::kMagicka) isMagicka = true;
                        else if (av == RE::ActorValue::kStamina) isStamina = true;
                    }
                }

                if (isHealth) {
                    health.push_back(potion);
                    health_magnitude.push_back(effect->GetMagnitude());
                    health_duration.push_back(effect->GetDuration());
                }
                if (isMagicka) {
                    magicka.push_back(potion);
                    magicka_magnitude.push_back(effect->GetMagnitude());
                    magicka_duration.push_back(effect->GetDuration());
                }
                if (isStamina) {
                    stamina.push_back(potion);
                    stamina_magnitude.push_back(effect->GetMagnitude());
                    stamina_duration.push_back(effect->GetDuration());
                }
            }
        }
    }

    if (this->health.type == Data::DATATYPE::POTION_AUTO_HIGHEST) {
        this->health.form = GetMinMaxPotion(true, this->calcDuration, health, health_magnitude, health_duration);
    } else if (this->health.type == Data::DATATYPE::POTION_AUTO_LOWEST) {
        this->health.form = GetMinMaxPotion(false, this->calcDuration, health, health_magnitude, health_duration);
    }

    if (this->magicka.type == Data::DATATYPE::POTION_AUTO_HIGHEST) {
        this->magicka.form = GetMinMaxPotion(true, this->calcDuration, magicka, magicka_magnitude, magicka_duration);
    } else if (this->magicka.type == Data::DATATYPE::POTION_AUTO_LOWEST) {
        this->magicka.form = GetMinMaxPotion(false, this->calcDuration, magicka, magicka_magnitude, magicka_duration);
    }

    if (this->stamina.type == Data::DATATYPE::POTION_AUTO_HIGHEST) {
        this->stamina.form = GetMinMaxPotion(true, this->calcDuration, stamina, stamina_magnitude, stamina_duration);
    } else if (this->stamina.type == Data::DATATYPE::POTION_AUTO_LOWEST) {
        this->stamina.form = GetMinMaxPotion(false, this->calcDuration, stamina, stamina_magnitude, stamina_duration);
    }
}

static std::string GetAmount(RE::TESForm* _item, const std::string& _fallbackName = "") {
    std::string result = "0";

    auto player = RE::PlayerCharacter::GetSingleton();
    if (!player) return result;

    auto inv = player->GetInventory();
    if (_item) {
        for (const auto& [item, data] : inv) {
            const auto& [numItem, entry] = data;
            if (numItem > 0 && item->Is(RE::FormType::AlchemyItem)) {
                if (item->GetFormID() == _item->GetFormID()) {
                    return std::to_string(numItem);
                }
            }
        }
    }

    // Fallback: If not found by FormID, match by name for dynamic player-crafted potions
    if (!_fallbackName.empty()) {
        for (const auto& [item, data] : inv) {
            const auto& [numItem, entry] = data;
            if (numItem > 0 && item && item->Is(RE::FormType::AlchemyItem)) {
                const char* itemName = item->GetName();
                if (itemName && itemName == _fallbackName) {
                    return std::to_string(numItem);
                }
            }
        }
    }

    return result;
}

std::string PotionSet::GetPotionName() {
    std::string result = this->name;
    if (this->health.type == Data::DATATYPE::POTION_AUTO_HIGHEST ||
        this->health.type == Data::DATATYPE::POTION_AUTO_LOWEST) {
        if (this->health.form) {
            const char* n = this->health.form->GetName();
            if (n) result = n;
        }
    } else if (this->magicka.type == Data::DATATYPE::POTION_AUTO_HIGHEST ||
               this->magicka.type == Data::DATATYPE::POTION_AUTO_LOWEST) {
        if (this->magicka.form) {
            const char* n = this->magicka.form->GetName();
            if (n) result = n;
        }
    } else if (this->stamina.type == Data::DATATYPE::POTION_AUTO_HIGHEST ||
               this->stamina.type == Data::DATATYPE::POTION_AUTO_LOWEST) {
        if (this->stamina.form) {
            const char* n = this->stamina.form->GetName();
            if (n) result = n;
        }
    }
    return result;
}

std::string PotionSet::GetPotionAmount() {
    std::string result = "0";
    if (this->health.type != Data::DATATYPE::NOTHING) {
        result = GetAmount(this->health.form, this->health.name);
    } else if (this->magicka.type != Data::DATATYPE::NOTHING) {
        result = GetAmount(this->magicka.form, this->magicka.name);
    } else if (this->stamina.type != Data::DATATYPE::NOTHING) {
        result = GetAmount(this->stamina.form, this->stamina.name);
    } else if (this->items.size() > 0 && this->items[0].type != Data::DATATYPE::NOTHING) {
        result = GetAmount(this->items[0].form, this->items[0].name);
    }

    return result;
}