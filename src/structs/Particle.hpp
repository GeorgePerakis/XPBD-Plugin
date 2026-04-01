#pragma once

#include "Vec2.hpp"

struct Particle {
    Vec2 position;
    Vec2 old_position;
    float inverse_mass;

    Particle(Vec2 pos, float inv_mass) : position(pos), old_position(pos), inverse_mass(inv_mass) {}
};
