#pragma once

#include "Vec2.hpp"

struct Particle {
    Vec2 position;
    Vec2 prev_position;
    Vec2 velocity;
    double inverse_mass;

    Particle() : position(), prev_position(), velocity(), inverse_mass(0.0f) {}
    Particle(Vec2 pos, float inv_mass) : position(pos), prev_position(pos), velocity(), inverse_mass(inv_mass) {}
};
