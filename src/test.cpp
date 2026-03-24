#include "test.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/core/object.hpp"
#include "godot_cpp/core/type_info.hpp"
#include "godot_cpp/variant/variant.hpp"
#include <godot_cpp/core/print_string.hpp>

using namespace godot;

void Test::say_hello()
{
    print_line("Hellooo");
}

void Test::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("say_hello"), &Test::say_hello);
    ClassDB::bind_method(D_METHOD("get_my_data"),&Test::get_my_data);
    ClassDB::bind_method(D_METHOD("set_my_data", "new_data"),&Test::set_my_data);

    ADD_PROPERTY(make_property_info(godot::Variant::STRING, "my_data"), "set_my_data", "get_my_data");
}

godot::String Test::get_my_data() const
{
    return my_data;
}

void Test::set_my_data(const godot::String new_data)
{
    my_data = new_data;
}
