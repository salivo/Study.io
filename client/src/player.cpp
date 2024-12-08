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
    DrawRectanglePro({x,y,50,50}, {25,25}, rotation, MAROON);
}

Vector2 Player::getCenter() const{
    return (Vector2){x, y};
}
void Player::rotate(float angle){
    rotation = angle;
}
