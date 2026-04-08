#include "SoftBody2D.hpp"
#include "consts.hpp"
#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/classes/viewport.hpp"
#include "structs/Particle.hpp"

using namespace godot;

void SoftBody2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_sprite_path", "new_path"), &SoftBody2D::set_sprite_path);
	ClassDB::bind_method(D_METHOD("get_sprite_path"), &SoftBody2D::get_sprite_path);

	ADD_PROPERTY(make_property_info(godot::Variant::NODE_PATH, "sprite_path"), "set_sprite_path", "get_sprite_path");
}

void SoftBody2D::spawn_rectangle(double particle_distance) {

    Vector2 origin = get_global_position();
    double ox = origin.x;
    double oy = origin.y;

    Particles = {
        Particle(Vec2(ox, oy), 1.0f),
        Particle(Vec2(ox + particle_distance, oy), 1.0f),
        Particle(Vec2(ox + particle_distance, oy + particle_distance), 1.0f),
        Particle(Vec2(ox, oy + particle_distance), 1.0f),
    };

    Distance_Constraints = {
        Distance_Constraint(0, 1, particle_distance),
        Distance_Constraint(1, 2, particle_distance),
        Distance_Constraint(2, 3, particle_distance),
        Distance_Constraint(3, 0, particle_distance),
        Distance_Constraint(0, 2, std::sqrt(2.0) * particle_distance),
        Distance_Constraint(1, 3, std::sqrt(2.0) * particle_distance),
    };
}

void SoftBody2D::step(double delta) {
	for (int i = 0; i < Particles.size(); i++) {
		Particle &p = Particles[i];

		Vec2 velocity = p.position - p.prev_position;
		p.prev_position = p.position;
		p.position = p.position + velocity;
		p.position.y += GRAVITY * delta * delta;
	}

    for (int i = 0; i < Distance_Constraints.size(); i++) {
        Distance_Constraint &c = Distance_Constraints[i];
        
        Vec2 diff = Particles[c.particle_b].position - Particles[c.particle_a].position;
        double dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);

        if (dist < 1e-7) continue;

        double C = dist - c.rest_length;
        Vec2 n = diff / dist;

        double w_a = Particles[c.particle_a].inverse_mass;
        double w_b = Particles[c.particle_b].inverse_mass;
        double lambda = -C / (w_a + w_b);

        Particles[c.particle_a].position -= n * (lambda * w_a);
        Particles[c.particle_b].position += n * (lambda * w_b);
	}

    for (int i = 0; i < Particles.size(); i++) {
		Particle &p = Particles[i];

        if (p.position.y > floor_y) {
            p.position.y = floor_y;
        }

		particle_sprites[i]->set_global_position(Vector2(p.position.x, p.position.y));
	}
}

void SoftBody2D::create_particle_sprites() {
	godot::Sprite2D *template_sprite = get_node<godot::Sprite2D>(sprite_path);
	if (template_sprite != nullptr) {
		particle_texture = template_sprite->get_texture();
		template_sprite->set_visible(false);
	}

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
	spawn_rectangle(500.0);
	create_particle_sprites();
}

void SoftBody2D::_physics_process(double delta) {
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	step(delta);
}
