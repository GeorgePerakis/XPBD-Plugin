#include "SoftBody2D.hpp"
#include "structs/Particle.hpp"
#include "godot_cpp/classes/engine.hpp"

using namespace godot;

void SoftBody2D::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("set_sprite_path", "new_path"), &SoftBody2D::set_sprite_path);
    ClassDB::bind_method(D_METHOD("get_sprite_path"), &SoftBody2D::get_sprite_path);

    ADD_PROPERTY(make_property_info(godot::Variant::NODE_PATH, "sprite_path"), "set_sprite_path", "get_sprite_path");
}

std::vector<Particle> SoftBody2D::spawn_rectangle(double particle_distance)
{
    return {
        Particle(Vec2(0.0,0.0), 1.0f),
        Particle(Vec2(particle_distance,0.0), 1.0f),
        Particle(Vec2(particle_distance,particle_distance), 1.0f),     
        Particle(Vec2(0.0,particle_distance), 1.0f),
    };
}

void SoftBody2D::step(double delta)     
{
    for (size_t i = 0; i < Particles.size(); i++) {
        Particle& p = Particles[i];
        
        particle_sprites[i]->set_position(Vector2(p.position.x, p.position.y));
    }
}

void SoftBody2D::_physics_process(double delta)
{
    if (Engine::get_singleton()->is_editor_hint()) {
        return;
    }

    step(delta);
}

void SoftBody2D::create_particle_sprites()
{
    godot::Sprite2D* template_sprite = get_node<godot::Sprite2D>(sprite_path);
    if (template_sprite != nullptr) {
        particle_texture = template_sprite->get_texture();
        template_sprite->set_visible(false);
    }

    for (Particle& p : Particles) {
        godot::Sprite2D* s = memnew(godot::Sprite2D);
        if (particle_texture.is_valid()) {
            s->set_texture(particle_texture);
        }
        s->set_position(Vector2(p.position.x, p.position.y));
        add_child(s);
        particle_sprites.push_back(s);
    }
}

void SoftBody2D::_ready()
{
    if (Engine::get_singleton()->is_editor_hint()) {
        return; 
    }

    Particles = spawn_rectangle(500.0);
    create_particle_sprites();
}
