#pragma once
#include <cmath>

struct Vec2 {
    double x;
    double y;

    Vec2()
    {
        x = 0.0;
        y = 0.0;
    }

    Vec2(double x_value, double y_value)
    {
        x = x_value;
        y = y_value;
    }

    Vec2 operator-(double other) const
    {
        return Vec2(x - other, y - other);
    }

    Vec2 operator+(double other) const
    {
        return Vec2(x + other, y + other);
    }

    Vec2 operator*(double other) const
    {
        return Vec2(x * other, y * other);
    }

    Vec2 operator/(double other) const
    {
        return Vec2(x / other, y / other);
    }

    Vec2& operator+=(double other)
    {
        x += other;
        y += other;
        return *this;
    }

    Vec2& operator-=(double other)
    {
        x -= other;
        y -= other;
        return *this;
    }

    Vec2& operator*=(double other)
    {
        x *= other;
        y *= other;
        return *this;
    }

    Vec2& operator/=(double other)
    {
        x /= other;
        y /= other;
        return *this;
    }


    Vec2 operator-(const Vec2 &other) const
    {
        return Vec2(x - other.x, y - other.y);
    }

    Vec2 operator+(const Vec2 &other) const
    {
        return Vec2(x + other.x, y + other.y);
    }

    Vec2 operator*(const Vec2 &other) const
    {
        return Vec2(x * other.x, y * other.y);
    }

    Vec2 operator/(const Vec2 &other) const
    {
        return Vec2(x / other.x, y / other.y);
    }

    Vec2& operator+=(const Vec2 &other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    Vec2& operator-=(const Vec2 &other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    Vec2& operator*=(const Vec2 &other)
    {
        x *= other.x;
        y *= other.y;
        return *this;
    }

    Vec2& operator/=(const Vec2 &other)
    {
        x /= other.x;
        y /= other.y;
        return *this;
    }

    static Vec2 abs(const Vec2 &v)
    {
        return Vec2(std::abs(v.x), std::abs(v.y));
    }

    static Vec2 sqr(const Vec2 &v)
    {
        return Vec2(v.x * v.x, v.y * v.y);
    }
};

inline Vec2 operator*(double lhs, const Vec2 &rhs)
{
    return Vec2(lhs * rhs.x, lhs * rhs.y);
}

inline Vec2 operator/(double lhs, const Vec2 &rhs)
{
    return Vec2(lhs / rhs.x, lhs / rhs.y);
}

inline Vec2 operator+(double lhs, const Vec2 &rhs)
{
    return Vec2(lhs + rhs.x, lhs + rhs.y);
}

inline Vec2 operator-(double lhs, const Vec2 &rhs)
{
    return Vec2(lhs - rhs.x, lhs - rhs.y);
}
