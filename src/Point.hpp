#pragma once

#include "godot_cpp/classes/node2d.hpp"
#include "godot_cpp/classes/sprite2d.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/variant/string.hpp"

class Point : public godot::Node2D {
	GDCLASS(Point, godot::Node2D)

public:
	void _physics_process(double delta) override;
	void _ready() override;

protected:
	static void _bind_methods();

private:
	godot::NodePath sprite_path;
	godot::Sprite2D *sprite = nullptr;
	double position = 0.0;

	void set_sprite_path(const godot::NodePath new_path) {
		sprite_path = new_path;
	}

	godot::NodePath get_sprite_path() const {
		return sprite_path;
	}
};
