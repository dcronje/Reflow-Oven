#pragma once
#include "lvgl.h"
#include <vector>
#include <string>

class CyberpunkLayout {
public:
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
    void setTagPressed(size_t index, bool pressed, uint32_t duration_ms = 0);
    
    // Set visual feedback for encoder tag press
    // If duration_ms is specified, will automatically toggle back after that duration
    void setEncoderTagPressed(bool pressed, uint32_t duration_ms = 0);
    
    // Get the content area for adding widgets
    lv_obj_t* getContentArea();
    
    // Animation control
    void animateIn();
    void animateOut();

private:
    lv_obj_t* container = nullptr;
    lv_obj_t* contentArea = nullptr;
    lv_obj_t* bottomBar = nullptr;
    lv_obj_t* encoderTag = nullptr;
    
    // Store tag containers and labels
    std::vector<lv_obj_t*> tagContainers;
    std::vector<lv_obj_t*> tagLabels;

    // Animation callback for auto-release
    static void autoReleaseAnimCb(lv_anim_t* a);
    static void autoReleaseEncoderAnimCb(lv_anim_t* a);

    void updateBottomTags(const std::vector<std::string>& tags);
};
