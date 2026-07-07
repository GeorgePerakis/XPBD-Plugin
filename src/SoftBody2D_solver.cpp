#include "SoftBody2D.hpp"
#include "consts.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include "structs/Particle.hpp"

#include <chrono>
#include <cmath>

using namespace godot;

void SoftBody2D::spawn_rectangle(double particle_distance, double mass, double stiffness) {
    Vector2 origin = get_global_position();
    double ox = origin.x;
    double oy = origin.y;

    Particles = {
        Particle(Vec2(ox, oy), 1/mass),
        Particle(Vec2(ox + particle_distance, oy), 1/mass),
        Particle(Vec2(ox + particle_distance, oy + particle_distance), 1/mass),
        Particle(Vec2(ox, oy + particle_distance), 1/mass),
    };

    Distance_Constraints = {
        Distance_Constraint(0, 1, particle_distance, stiffness),
        Distance_Constraint(1, 2, particle_distance, stiffness),
        Distance_Constraint(2, 3, particle_distance, stiffness),
        Distance_Constraint(3, 0, particle_distance, stiffness),
        Distance_Constraint(0, 2, std::sqrt(2.0) * particle_distance, stiffness),
        Distance_Constraint(1, 3, std::sqrt(2.0) * particle_distance, stiffness),
    };

    metric_particle_count = (int)Particles.size();
    metric_constraint_count = (int)Distance_Constraints.size();
}

void SoftBody2D::spawn_grid(int width, int height, double spacing, double mass, double stiffness) {
    Vector2 origin = get_global_position();
    double ox = origin.x;
    double oy = origin.y;

    Particles.clear();
    Distance_Constraints.clear();

    // Create particles in a grid
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Particles.push_back(Particle(
                Vec2(ox + x * spacing, oy + y * spacing),
                1.0 / mass
            ));
        }
    }

    // Structural constraints (horizontal + vertical) = priority 0
    // Diagonal shear = priority 1
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * width + x;

            // Horizontal (priority 0 - structural)
            if (x < width - 1) {
                Distance_Constraints.push_back(
                    Distance_Constraint(idx, idx + 1, spacing, stiffness, 0));
            }
            // Vertical (priority 0 - structural)
            if (y < height - 1) {
                Distance_Constraints.push_back(
                    Distance_Constraint(idx, idx + width, spacing, stiffness, 0));
            }
            // Diagonal shear (priority 1)
            double diag = std::sqrt(2.0) * spacing;
            if (x < width - 1 && y < height - 1) {
                Distance_Constraints.push_back(
                    Distance_Constraint(idx, idx + width + 1, diag, stiffness, 1));
            }
            if (x > 0 && y < height - 1) {
                Distance_Constraints.push_back(
                    Distance_Constraint(idx, idx + width - 1, diag, stiffness, 1));
            }
        }
    }

    metric_particle_count = (int)Particles.size();
    metric_constraint_count = (int)Distance_Constraints.size();

    build_priority_groups();
}

void SoftBody2D::build_priority_groups() {
    priority_groups.clear();
    int max_pri = 0;
    for (auto &c : Distance_Constraints) {
        if (c.priority > max_pri) max_pri = c.priority;
    }
    priority_groups.resize(max_pri + 1);
    for (int i = 0; i < (int)Distance_Constraints.size(); i++) {
        priority_groups[Distance_Constraints[i].priority].push_back(i);
    }
}

double SoftBody2D::compute_constraint_error() {
    double total_error = 0.0;
    for (int i = 0; i < Distance_Constraints.size(); i++) {
        Distance_Constraint &c = Distance_Constraints[i];
        Vec2 diff = Particles[c.particle_b].position - Particles[c.particle_a].position;
        double distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);
        total_error += std::abs(distance - c.rest_length);
    }
    return total_error;
}

void SoftBody2D::solve_flat(double sub_dt) {
    for (int i = 0; i < Distance_Constraints.size(); i++) {
        Distance_Constraint &c = Distance_Constraints[i];

        Vec2 difference = Particles[c.particle_b].position - Particles[c.particle_a].position;
        double distance = std::sqrt(difference.x * difference.x + difference.y * difference.y);

        if (distance < 1e-7) continue;

        double C = distance - c.rest_length;
        Vec2 C_gradient = difference / distance;

        double w_a = Particles[c.particle_a].inverse_mass;
        double w_b = Particles[c.particle_b].inverse_mass;
        double alpha_tilde = (1.0 / c.stiffness) / (sub_dt * sub_dt);
        double delta_lambda = -(C + alpha_tilde * c.lambda) / (w_a + w_b + alpha_tilde);

        c.lambda += delta_lambda;

        Particles[c.particle_a].position -= C_gradient * (delta_lambda * w_a);
        Particles[c.particle_b].position += C_gradient * (delta_lambda * w_b);
    }
}

void SoftBody2D::solve_hierarchical(double sub_dt) {
    // Solve constraints by priority level (0 = highest priority first)
    for (int level = 0; level < (int)priority_groups.size(); level++) {
        for (int idx : priority_groups[level]) {
            Distance_Constraint &c = Distance_Constraints[idx];

            Vec2 difference = Particles[c.particle_b].position - Particles[c.particle_a].position;
            double distance = std::sqrt(difference.x * difference.x + difference.y * difference.y);

            if (distance < 1e-7) continue;

            double C = distance - c.rest_length;
            Vec2 C_gradient = difference / distance;

            double w_a = Particles[c.particle_a].inverse_mass;
            double w_b = Particles[c.particle_b].inverse_mass;
            double alpha_tilde = (1.0 / c.stiffness) / (sub_dt * sub_dt);
            double delta_lambda = -(C + alpha_tilde * c.lambda) / (w_a + w_b + alpha_tilde);

            c.lambda += delta_lambda;

            Particles[c.particle_a].position -= C_gradient * (delta_lambda * w_a);
            Particles[c.particle_b].position += C_gradient * (delta_lambda * w_b);
        }
    }
}

void SoftBody2D::step(double delta) {
    auto start = std::chrono::high_resolution_clock::now();

	double sub_dt = delta / num_substeps;

	for (int substep = 0; substep < num_substeps; substep++) {
		// Predict positions
		for (int i = 0; i < Particles.size(); i++) {
			Particle &p = Particles[i];

			Vec2 velocity = p.position - p.prev_position;
			p.prev_position = p.position;
			p.position = p.position + velocity;
			p.position.y += GRAVITY * sub_dt * sub_dt;
		}

		// Reset Lagrange multipliers
		for (int i = 0; i < Distance_Constraints.size(); i++) {
			Distance_Constraints[i].lambda = 0.0;
		}

		// Solve constraints based on solver mode
		if (solver_mode == SOLVER_HIERARCHICAL) {
			solve_hierarchical(sub_dt);
		} else {
			solve_flat(sub_dt);
		}

		// Floor collision
		for (int i = 0; i < Particles.size(); i++) {
			Particle &p = Particles[i];
			if (p.position.y > floor_y) {
				p.position.y = floor_y;
			}
		}
	}

    auto end = std::chrono::high_resolution_clock::now();
    metric_step_time_ms = std::chrono::duration<double, std::milli>(end - start).count();

    // Update running metrics
    metric_frame_count++;
    metric_total_step_time_ms += metric_step_time_ms;
    metric_avg_step_time_ms = metric_total_step_time_ms / metric_frame_count;
    if (metric_step_time_ms > metric_max_step_time_ms) {
        metric_max_step_time_ms = metric_step_time_ms;
    }

    // Compute constraint residual error
    metric_constraint_error = compute_constraint_error();

	// Update sprite positions
	for (int i = 0; i < Particles.size(); i++) {
		particle_sprites[i]->set_global_position(Vector2(Particles[i].position.x, Particles[i].position.y));
	}

    queue_redraw();
}
