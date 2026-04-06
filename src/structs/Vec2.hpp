#pragma once

struct Vec2 {
    double x;
    double y;

    Vec2()
    {
        x = 0.0;
        y = 0.0;
    }

    Vec2(float x_value, float y_value)
    {
        x = x_value;
        y = y_value;
    }
};
