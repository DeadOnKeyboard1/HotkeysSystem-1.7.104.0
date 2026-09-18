#include "WidgetHandler.h"
#include "Scaleform/WidgetMenu.h"
#include "Config.h"

void WidgetHandler::AddWidgetMenuTask(WidgetTasklet a_task) {
    OpenWidgetMenu();
    Locker locker(_lock);
    _WidgetMenuTaskQueue.push_back(std::move(a_task));
}

void WidgetHandler::ProcessWidgetMenu(WidgetMenu& a_menu) {
    std::vector<WidgetTasklet> tasks;
    {
        Locker locker(_lock);
        if (!_WidgetMenuTaskQueue.empty()) {
            tasks.swap(_WidgetMenuTaskQueue);
        }
    }

    for (auto& task : tasks) {
        task(a_menu);
    }

    if (_refreshWidgetMenu) {
        a_menu.RefreshUI();
    }

    _refreshWidgetMenu = false;
}

void WidgetHandler::OpenWidgetMenu() {
    auto msgQ = RE::UIMessageQueue::GetSingleton();
    if (!msgQ) return;
    
    msgQ->AddMessage(WidgetMenu::MenuName(), RE::UI_MESSAGE_TYPE::kShow, nullptr);
}

void WidgetHandler::CloseWidgetMenu() {
    auto msgQ = RE::UIMessageQueue::GetSingleton();
    if (!msgQ) return;

    msgQ->AddMessage(WidgetMenu::MenuName(), RE::UI_MESSAGE_TYPE::kHide, nullptr);
}

void WidgetHandler::SetMenuVisible(bool _visible) {
    AddWidgetMenuTask([_visible](WidgetMenu& a_menu) {
        a_menu.SetMenuVisible(_visible); 
    });
}

void WidgetHandler::LoadWidget(uint32_t _id, std::string _path, int32_t _x, int32_t _y, int32_t _width, int32_t _height,
                               int32_t _alpha) {
    AddWidgetMenuTask([_id, _path, _x, _y, _width, _height, _alpha](WidgetMenu& a_menu) {
        a_menu.LoadWidget(_id, _path, _x, _y, _width, _height, _alpha);
    });
}

void WidgetHandler::UnloadWidget(uint32_t _id) {
    AddWidgetMenuTask([_id](WidgetMenu& a_menu) {
        a_menu.UnloadWidget(_id);
    });
}

void WidgetHandler::LoadText(uint32_t _id, std::string _text, std::string _font, int32_t _x, int32_t _y, int32_t _align,
                             int32_t _size, int32_t _alpha, bool _shadow) {
    {
        Locker locker(_lock);
        _fontMap[_id] = _font;
    }
    _text = SanitizeTextForFont(_text, _font);
    // In UIHS_Widget.swf, createTextField uses initial dimensions (0, 0, 100, 100).
    // When _align == 2 (CENTER), Flash autoSize="center" centers text around local x = 50.
    // Subtract 50 so that _x corresponds precisely to the horizontal center of the rendered text.
    if (_align == 2) {
        _x -= 50;
    }
    AddWidgetMenuTask([_id, _text, _font, _x, _y, _align, _size, _alpha, _shadow](WidgetMenu& a_menu) {
        a_menu.LoadText(_id, _text, _font, _x, _y, _align, _size, _alpha, _shadow);
    });
}

void WidgetHandler::UnloadText(uint32_t _id) {
    {
        Locker locker(_lock);
        _fontMap.erase(_id);
    }
    AddWidgetMenuTask([_id](WidgetMenu& a_menu) {
        a_menu.UnloadText(_id);
    });
}

void WidgetHandler::SetText(uint32_t _id, std::string _text) {
    std::string font = "";
    {
        Locker locker(_lock);
        auto it = _fontMap.find(_id);
        if (it != _fontMap.end()) {
            font = it->second;
        }
    }
    if (!font.empty()) {
        _text = SanitizeTextForFont(_text, font);
    }
    AddWidgetMenuTask([_id, _text](WidgetMenu& a_menu) {
        a_menu.SetText(_id, _text);
    });
}

void WidgetHandler::SetMenuAlpha(uint32_t _alpha) {
    AddWidgetMenuTask([_alpha](WidgetMenu& a_menu) {
        a_menu.SetMenuAlpha(_alpha);
    });
}

void WidgetHandler::MenuFadeIn() {
    AddWidgetMenuTask([](WidgetMenu& a_menu) {
        a_menu.MenuFadeIn();
    });
}

void WidgetHandler::MenuFadeOut() {
    AddWidgetMenuTask([](WidgetMenu& a_menu) {
        a_menu.MenuFadeOut();
    });
}

void WidgetHandler::ProcessFadeIn() {
    auto config = ConfigHandler::GetSingleton();
    if (!config) return;

    if (config->Widget.General.animType == (uint32_t)Config::AnimType::FADE) {
        this->MenuFadeIn();
    } else if (config->Widget.General.animType == (uint32_t)Config::AnimType::INSTANT) {
        this->SetMenuAlpha(100);
    }
}

void WidgetHandler::ProcessFadeOut() {
    auto config = ConfigHandler::GetSingleton();
    if (!config) return;

    if (config->Widget.General.animDelay != 0.0f &&
        config->Widget.General.displayMode == (uint32_t)Config::DisplayType::INCOMBAT) {
        if (this->expireProgress.load() != 0.0f && this->expireProgress.load() < config->Widget.General.animDelay) {
            this->SetExpireProgress(0.01f);
        } else {
            this->StartExpireTimer();
        }
    }
}

void WidgetHandler::SetExpireProgress(const float& _amount) {
    expireProgress.store(_amount);
}

void WidgetHandler::StartExpireTimer() {
    auto config = ConfigHandler::GetSingleton();
    if (!config) return;

    if (expireProgress.load() != 0.0f && expireProgress.load() < config->Widget.General.animDelay) return;

    if (expire_future.valid() && expire_future.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
        return;
    }

    expireProgress.store(0.01f);
    expire_future = std::async(std::launch::async, &WidgetHandler::ExpireFunc, this);
}

void WidgetHandler::CloseExpireTimer() {
    if (expireProgress.load() == 0.0f) return;

    shouldCloseExpire.store(true);
}

bool WidgetHandler::ExpireFunc() {
    auto config = ConfigHandler::GetSingleton();
    if (!config) return true;

    auto lastTime = std::chrono::steady_clock::now();
    while (!shouldCloseExpire.load() && expireProgress.load() < config->Widget.General.animDelay) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<float> elapsed = now - lastTime;
        lastTime = now;

        auto player = RE::PlayerCharacter::GetSingleton();
        if (config->Widget.General.displayMode == (uint32_t)Config::DisplayType::INCOMBAT &&
            player && player->IsInCombat()) {
            continue;
        }
        expireProgress.fetch_add(elapsed.count());
    }

    if (!shouldCloseExpire.load()) {
        if (config->Widget.General.animType == (uint32_t)Config::AnimType::FADE) {
            this->MenuFadeOut();
        } else if (config->Widget.General.animType == (uint32_t)Config::AnimType::INSTANT) {
            this->SetMenuAlpha(0);
        }
    }
    shouldCloseExpire.store(false);
    expireProgress.store(0.0f);

    return true;
}