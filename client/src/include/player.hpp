#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <raylib.h>
#include <utility>
class Player {
private:
    float x, y;
public:
    Player() = default;

    Player(float startX, float startY)
        : x(startX), y(startY) {}
    std::pair<float, float> getPosition();
    void setPosition(float x, float y);
    void draw() const;
    Vector2 getCenter() const;
};

#endif
