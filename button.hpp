#pragma once
#include "include\raylib.h"

class Button
{
    public:
        float width = 1280 / 2;
        float height = 720 / 4;
        Vector2 position;
        Button();
        void draw(Vector2 mousePoint);
        bool isHovered(Vector2 mousePoint);
        bool isPressed(Vector2 mousePoint);
        bool isReleased(Vector2 mousePoint);
        void changeColor(Vector2 mousePoint);
    private:
        int state = 0;
        Color color = GREEN;
};