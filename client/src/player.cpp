#include "include/player.hpp"
#include "raylib.h"
std::pair<float, float> Player::getPosition()  {
    return {x, y};
}
void Player::setPosition(float _x, float _y) {
    x = _x;
    y = _y;
}
void Player::draw() const{

    DrawRectangle(static_cast<int>(x), static_cast<int>(y), 50, 50, BLUE);
}

Vector2 Player::getCenter() const{
    return (Vector2){x, y};
}
