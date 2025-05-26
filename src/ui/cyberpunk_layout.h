#pragma once

#include "lvgl.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>

class CyberpunkLayout {
public:
    // Callback type for animation completion
    using AnimationCallback = std::function<void()>;

    CyberpunkLayout(lv_obj_t* parent);
    ~CyberpunkLayout();

    // Set the bottom tags (up to 3)
    void setBottomTags(const std::vector<std::string>& tags);
    
    // Control visibility of all bottom tags
    void setBottomTagsVisible(bool visible);
    
    // Set encoder tag text and visibility
    void setEncoderTag(const std::string& tag, bool showScrollIcon);
    void setEncoderTagVisible(bool visible);
    
    // Set visual feedback for tag press (0-2)
    // If duration_ms is specified, will automatically toggle back after that duration
    // If callback is provided, it will be called after the animation completes
    void setTagPressed(size_t index, bool pressed, uint32_t duration_ms = 0, AnimationCallback callback = nullptr);
    
    // Set visual feedback for encoder tag press
    // If duration_ms is specified, will automatically toggle back after that duration
    // If callback is provided, it will be called after the animation completes
    void setEncoderTagPressed(bool pressed, uint32_t duration_ms = 0, AnimationCallback callback = nullptr);
    
    // Get the content area for adding widgets
    lv_obj_t* getContentArea();
    
    // Animation control
    void animateIn();
    void animateOut();

private:
    lv_obj_t* container = nullptr;
    lv_obj_t* contentArea = nullptr;
    lv_obj_t* bottomBar = nullptr;
    lv_obj_t* encoderTagContainer = nullptr;
    lv_obj_t* encoderTag = nullptr;
    
    // Store tag containers and labels
    std::vector<lv_obj_t*> tagContainers;
    std::vector<lv_obj_t*> tagLabels;
    std::vector<lv_obj_t*> encoderCharLabels;

    void updateBottomTags(const std::vector<std::string>& tags);
};
