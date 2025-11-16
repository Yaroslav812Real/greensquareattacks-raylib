#include "button.hpp"

Button::Button()
{

}

void Button::draw(Vector2 mousePoint)
{
    DrawRectangleV({position}, {(float)size.x, (float)size.y}, color);
    changeColor(mousePoint);
}

bool Button::isHovered(Vector2 mousePoint)
{
    Rectangle rect = {position.x, position.y, (float)size.x, (float)size.y};
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
    color.a = transparency;
    if (!isHovered(mousePoint) and !isPressed(mousePoint)) transparency = 255;
    if (isHovered(mousePoint)) transparency = 127;
    if (!toggleButton and isPressed(mousePoint)) transparency = 48;
    if (toggleButton and toggle) {color = GREEN; color.a = transparency;}
    if (toggleButton and !toggle) {color = RED; color.a = transparency;}
    if (toggleButton and isReleased(mousePoint)) toggle = !toggle;
}