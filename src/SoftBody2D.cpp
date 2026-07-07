#include "SoftBody2D.hpp"
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
	ClassDB::bind_method(D_METHOD("start_metrics_capture", "duration_s", "sample_ms"), &SoftBody2D::start_metrics_capture);

	ClassDB::bind_method(D_METHOD("set_auto_start_capture", "value"), &SoftBody2D::set_auto_start_capture);
	ClassDB::bind_method(D_METHOD("get_auto_start_capture"), &SoftBody2D::get_auto_start_capture);
	ADD_PROPERTY(make_property_info(Variant::BOOL, "auto_start_capture"), "set_auto_start_capture", "get_auto_start_capture");

	ClassDB::bind_method(D_METHOD("set_capture_duration", "value"), &SoftBody2D::set_capture_duration);
	ClassDB::bind_method(D_METHOD("get_capture_duration"), &SoftBody2D::get_capture_duration);
	ADD_PROPERTY(make_property_info(Variant::FLOAT, "capture_duration"), "set_capture_duration", "get_capture_duration");

	ClassDB::bind_method(D_METHOD("set_sampling_interval", "value"), &SoftBody2D::set_sampling_interval);
	ClassDB::bind_method(D_METHOD("get_sampling_interval"), &SoftBody2D::get_sampling_interval);
	ADD_PROPERTY(make_property_info(Variant::FLOAT, "sampling_interval"), "set_sampling_interval", "get_sampling_interval");

	ClassDB::bind_method(D_METHOD("set_solver_mode", "value"), &SoftBody2D::set_solver_mode);
	ClassDB::bind_method(D_METHOD("get_solver_mode"), &SoftBody2D::get_solver_mode);
	ADD_PROPERTY(make_property_info(Variant::INT, "solver_mode", PROPERTY_HINT_ENUM, "Flat XPBD,Hierarchical"), "set_solver_mode", "get_solver_mode");
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

    if (auto_start_capture) {
        start_metrics_capture(capture_duration_s, sampling_interval_ms);
    }
}

void SoftBody2D::_physics_process(double delta) {
	if (Engine::get_singleton()->is_editor_hint()) {
        queue_redraw();
		return;
	}
	step(delta);

    // Process metrics capture (if active)
    process_metrics_capture(delta * 1000.0);
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
