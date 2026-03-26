#include "Point.hpp"
#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/core/object.hpp"
#include "godot_cpp/core/type_info.hpp"
#include "godot_cpp/variant/variant.hpp"
#include <godot_cpp/core/print_string.hpp>

using namespace godot;

void Point::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("set_sprite_path", "new_path"), &Point::set_sprite_path);
    ClassDB::bind_method(D_METHOD("get_sprite_path"), &Point::get_sprite_path);

    ADD_PROPERTY(make_property_info(godot::Variant::NODE_PATH, "sprite_path"), "set_sprite_path", "get_sprite_path");
}

void Point::_physics_process(double delta)
{
    if (sprite == nullptr) {
        return;
    }

    if (Engine::get_singleton()->is_editor_hint()) {
        return; 
    }

    if (position > 400.0) {
        position = 400.0;
    }

    position += delta * 100.0;
    sprite->set_position(Vector2(position, 0.0));   
}

void Point::_ready()
{
    if (sprite == nullptr) {
        sprite = get_node<godot::Sprite2D>(sprite_path);
    }
}
