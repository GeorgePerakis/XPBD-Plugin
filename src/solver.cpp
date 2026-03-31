#pragma once

#include "structs/Distance_Constraint.hpp"
#include "structs/particle.hpp"
#include <vector>

class Solver {
    std::pmr::vector<Particle> particles;
    std::pmr::vector<Distance_Constraint> constraints;

    void step(double delta_time) {
        
    }

};
