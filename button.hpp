#pragma once

#include "include\raylib.h"

extern float buttonWidth;
extern float buttonHeight;

class Button
{
    public:
        Vector2 position;
        Vector2 size = {buttonWidth, buttonHeight};
        bool toggleButton = false;
        bool toggle = false;
        Button();
        void draw(Vector2 mousePoint);
        bool isHovered(Vector2 mousePoint);
        bool isPressed(Vector2 mousePoint);
        bool isReleased(Vector2 mousePoint);
        void changeColor(Vector2 mousePoint);
    private:
        int transparency = 255;
        Color color = GREEN;
};