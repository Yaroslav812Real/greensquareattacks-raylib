#include "button.hpp"
#include "iostream"

Button::Button()
{

}

void Button::draw(Vector2 mousePoint)
{
    DrawRectangleV({position}, {(float)buttonWidth, (float)buttonHeight}, color);
    changeColor(mousePoint);
}

bool Button::isHovered(Vector2 mousePoint)
{
    Rectangle rect = {position.x, position.y, (float)buttonWidth, (float)buttonHeight};
    if (CheckCollisionPointRec(mousePoint, rect)) return true;
    return false;
}

bool Button::isPressed(Vector2 mousePoint)
{
    if (isHovered(mousePoint) && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) return true;
    return false;
}

bool Button::isReleased(Vector2 mousePoint)
{
    if (isHovered(mousePoint) && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) return true;
    return false;
}

void Button::changeColor(Vector2 mousePoint)
{
    if (!isHovered(mousePoint)) state = 0;
    if (isHovered(mousePoint)) state = 1;
    if (isHovered(mousePoint) and IsMouseButtonDown(MOUSE_BUTTON_LEFT)) state = 2;
    switch(state)
    {
        case 0:
            color = GREEN;
        break;

        case 1:
            color = YELLOW;
        break;

        case 2:
            color = RED;
        break;
        
        default: break;
    }
}