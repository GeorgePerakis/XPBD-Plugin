#pragma once

#include "Vec2.hpp"

struct Particle {
    Vec2 position;
    Vec2 old_position;
    float inverse_mass;
};
