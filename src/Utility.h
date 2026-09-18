#pragma once

namespace Utility {
    constexpr std::string delimiter{";##;"};

    std::vector<std::string> Split(const std::string& _str, const std::string& _delimiter);

    bool to_bool(const std::string& _str);

    std::vector<uint32_t>& acceptableKeys();

    struct ResolutionInfo {
        float screenWidth{1920.0f};
        float screenHeight{1080.0f};
        float scale{1.0f};
        float stageWidth{1280.0f};
        float stageHeight{720.0f};
    };

    ResolutionInfo GetResolutionInfo();
    float GetResolutionScale();
    float GetStageWidth();
    float GetStageHeight();
}