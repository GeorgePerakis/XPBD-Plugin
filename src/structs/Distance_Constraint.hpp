#pragma once

struct Distance_Constraint {
    int particle_a;
    int particle_b;
    double rest_length;
    double stiffness;
    double lambda;

    Distance_Constraint(int a, int b, double rest_len, double stiff) : particle_a(a), particle_b(b), rest_length(rest_len), stiffness(stiff), lambda(0.0) {}
};
