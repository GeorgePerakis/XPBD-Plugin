#include "SoftBody2D.hpp"
#include "consts.hpp"
#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/classes/viewport.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include "structs/Particle.hpp"

using namespace godot;

void SoftBody2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_particle_texture", "texture"), &SoftBody2D::set_particle_texture);
	ClassDB::bind_method(D_METHOD("get_particle_texture"), &SoftBody2D::get_particle_texture);
	ADD_PROPERTY(make_property_info(Variant::OBJECT, "particle_texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_particle_texture", "get_particle_texture");

	ClassDB::bind_method(D_METHOD("set_num_substeps", "value"), &SoftBody2D::set_num_substeps);
	ClassDB::bind_method(D_METHOD("get_num_substeps"), &SoftBody2D::get_num_substeps);
	ADD_PROPERTY(make_property_info(Variant::INT, "num_substeps"), "set_num_substeps", "get_num_substeps");

	ClassDB::bind_method(D_METHOD("set_grid_width", "value"), &SoftBody2D::set_grid_width);
	ClassDB::bind_method(D_METHOD("get_grid_width"), &SoftBody2D::get_grid_width);
	ADD_PROPERTY(make_property_info(Variant::INT, "grid_width"), "set_grid_width", "get_grid_width");

	ClassDB::bind_method(D_METHOD("set_grid_height", "value"), &SoftBody2D::set_grid_height);
	ClassDB::bind_method(D_METHOD("get_grid_height"), &SoftBody2D::get_grid_height);
	ADD_PROPERTY(make_property_info(Variant::INT, "grid_height"), "set_grid_height", "get_grid_height");

	ClassDB::bind_method(D_METHOD("set_grid_spacing", "value"), &SoftBody2D::set_grid_spacing);
	ClassDB::bind_method(D_METHOD("get_grid_spacing"), &SoftBody2D::get_grid_spacing);
	ADD_PROPERTY(make_property_info(Variant::FLOAT, "grid_spacing"), "set_grid_spacing", "get_grid_spacing");

	ClassDB::bind_method(D_METHOD("set_grid_mass", "value"), &SoftBody2D::set_grid_mass);
	ClassDB::bind_method(D_METHOD("get_grid_mass"), &SoftBody2D::get_grid_mass);
	ADD_PROPERTY(make_property_info(Variant::FLOAT, "grid_mass"), "set_grid_mass", "get_grid_mass");

	ClassDB::bind_method(D_METHOD("set_grid_stiffness", "value"), &SoftBody2D::set_grid_stiffness);
	ClassDB::bind_method(D_METHOD("get_grid_stiffness"), &SoftBody2D::get_grid_stiffness);
	ADD_PROPERTY(make_property_info(Variant::FLOAT, "grid_stiffness"), "set_grid_stiffness", "get_grid_stiffness");

	// Read-only metrics
	ClassDB::bind_method(D_METHOD("get_metric_step_time_ms"), &SoftBody2D::get_metric_step_time_ms);
	ClassDB::bind_method(D_METHOD("get_metric_constraint_error"), &SoftBody2D::get_metric_constraint_error);
	ClassDB::bind_method(D_METHOD("get_metric_particle_count"), &SoftBody2D::get_metric_particle_count);
	ClassDB::bind_method(D_METHOD("get_metric_constraint_count"), &SoftBody2D::get_metric_constraint_count);
	ClassDB::bind_method(D_METHOD("get_metric_avg_step_time_ms"), &SoftBody2D::get_metric_avg_step_time_ms);
	ClassDB::bind_method(D_METHOD("get_metric_max_step_time_ms"), &SoftBody2D::get_metric_max_step_time_ms);
}

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

    // Structural constraints (horizontal + vertical + diagonal shear)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * width + x;

            // Horizontal
            if (x < width - 1) {
                Distance_Constraints.push_back(
                    Distance_Constraint(idx, idx + 1, spacing, stiffness));
            }
            // Vertical
            if (y < height - 1) {
                Distance_Constraints.push_back(
                    Distance_Constraint(idx, idx + width, spacing, stiffness));
            }
            // Diagonal shear
            double diag = std::sqrt(2.0) * spacing;
            if (x < width - 1 && y < height - 1) {
                Distance_Constraints.push_back(
                    Distance_Constraint(idx, idx + width + 1, diag, stiffness));
            }
            if (x > 0 && y < height - 1) {
                Distance_Constraints.push_back(
                    Distance_Constraint(idx, idx + width - 1, diag, stiffness));
            }
        }
    }

    metric_particle_count = (int)Particles.size();
    metric_constraint_count = (int)Distance_Constraints.size();
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

		// Solve constraints with accumulation
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

void SoftBody2D::create_particle_sprites() {
	for (Particle &p : Particles) {
		godot::Sprite2D *s = memnew(godot::Sprite2D);
		if (particle_texture.is_valid()) {
			s->set_texture(particle_texture);
		}
		add_child(s);
		s->set_global_position(Vector2(p.position.x, p.position.y));
		particle_sprites.push_back(s);
	}
}

void SoftBody2D::_ready() {
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	floor_y = get_viewport()->get_visible_rect().size.y;
	spawn_grid(grid_width, grid_height, grid_spacing, grid_mass, grid_stiffness);
	create_particle_sprites();

    UtilityFunctions::print("[XPBD] Stress test started: ",
        metric_particle_count, " particles, ",
        metric_constraint_count, " constraints, ",
        num_substeps, " substeps");
}

void SoftBody2D::_physics_process(double delta) {
	if (Engine::get_singleton()->is_editor_hint()) {
        queue_redraw();
		return;
	}
	step(delta);

    // Log metrics every 60 frames
    if (metric_frame_count % 60 == 0) {
        UtilityFunctions::print("[XPBD Metrics] frame=", metric_frame_count,
            " | step_time=", metric_step_time_ms, "ms",
            " | avg=", metric_avg_step_time_ms, "ms",
            " | max=", metric_max_step_time_ms, "ms",
            " | error=", metric_constraint_error,
            " | particles=", metric_particle_count,
            " | constraints=", metric_constraint_count);
    }
}

void SoftBody2D::_draw() {
    for (Distance_Constraint &c : Distance_Constraints) {
        Vec2 pos_a = Particles[c.particle_a].position;
        Vec2 pos_b = Particles[c.particle_b].position;
        Vector2 local_a = to_local(Vector2(pos_a.x, pos_a.y));
        Vector2 local_b = to_local(Vector2(pos_b.x, pos_b.y));
        draw_line(local_a, local_b, Color(1, 0, 0), 3.0);
    }
}
