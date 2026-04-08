#pragma once

#include "Vec2.hpp"

struct Particle {
    Vec2 position;
    Vec2 prev_position;
    double inverse_mass;

    Particle() : position(), prev_position(), inverse_mass(0.0f) {}
    Particle(Vec2 pos, float inv_mass) : position(pos), prev_position(pos), inverse_mass(inv_mass) {}
};
