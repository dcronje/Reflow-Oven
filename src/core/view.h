#pragma once

#include <lvgl.h>

class View {
public:
    View(int x = 0, int y = 0, int w = LV_SIZE_CONTENT, int h = LV_SIZE_CONTENT);
    virtual ~View() = default;

    virtual void render(lv_obj_t* parent) = 0;

protected:
    int x;
    int y;
    int width;
    int height;
};
