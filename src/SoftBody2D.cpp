#include "SoftBody2D.hpp"
#include "consts.hpp"
#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/classes/viewport.hpp"
#include "structs/Particle.hpp"

using namespace godot;

void SoftBody2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_particle_texture", "texture"), &SoftBody2D::set_particle_texture);
	ClassDB::bind_method(D_METHOD("get_particle_texture"), &SoftBody2D::get_particle_texture);

	ADD_PROPERTY(make_property_info(godot::Variant::OBJECT, "particle_texture", godot::PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_particle_texture", "get_particle_texture");
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
        
        Vec2 difference = Particles[c.particle_b].position - Particles[c.particle_a].position;
        double distance = std::sqrt(difference.x * difference.x + difference.y * difference.y);

        if (distance < 1e-7) continue;

        double C = distance - c.rest_length;
        Vec2 C_gradient = difference / distance;

        double w_a = Particles[c.particle_a].inverse_mass;
        double w_b = Particles[c.particle_b].inverse_mass;
        double compliance = 1.0 / c.stiffness;
        double lambda = -C / (w_a + w_b + compliance / (delta * delta));

        Particles[c.particle_a].position -= C_gradient * (lambda * w_a);
        Particles[c.particle_b].position += C_gradient * (lambda * w_b);
	}

    for (int i = 0; i < Particles.size(); i++) {
		Particle &p = Particles[i];

        if (p.position.y > floor_y) {
            p.position.y = floor_y;
        }

		particle_sprites[i]->set_global_position(Vector2(p.position.x, p.position.y));
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
	spawn_rectangle(700.0, 1.0, 100.0);
	create_particle_sprites();
}

void SoftBody2D::_physics_process(double delta) {
	if (Engine::get_singleton()->is_editor_hint()) {
        queue_redraw();
		return;
	}
	step(delta);
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
