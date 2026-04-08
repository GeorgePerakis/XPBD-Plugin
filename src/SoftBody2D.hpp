#pragma once

#include "godot_cpp/classes/node2d.hpp"
#include "godot_cpp/classes/sprite2d.hpp"
#include "godot_cpp/classes/texture2d.hpp"
#include "structs/Distance_Constraint.hpp"
#include "structs/Particle.hpp"
#include <vector>

class SoftBody2D: public godot::Node2D {
    GDCLASS(SoftBody2D, godot::Node2D)

public:
	void _physics_process(double delta) override;
    void _ready() override;

protected:
	static void _bind_methods();

private:
    godot::NodePath sprite_path;
    godot::Ref<godot::Texture2D> particle_texture;
    
    std::vector<Particle> Particles;
    std::vector<godot::Sprite2D*> particle_sprites;
    std::vector<Distance_Constraint> Distance_Constraints;
    double floor_y = 1080.0;

    void spawn_rectangle(double particle_distance);

    void step(double delta);

    void create_particle_sprites();

    void set_sprite_path(const godot::NodePath new_path) { sprite_path = new_path; }

	godot::NodePath get_sprite_path() const { return sprite_path; }
};
