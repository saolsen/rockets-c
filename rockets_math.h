#ifndef _rockets_math_h
#define _rockets_math_h

#include <math.h>

// Math
#define CLAMP(x,xmin,xmax)  ((x) < (xmin) ? (xmin) : (x) > (xmax) ? (xmax) : (x))

float
deg_to_rad(const float deg)
{
    return deg * M_PI / 180.0;
}

bool
bounds_contains(float top_leftx, float top_lefty,
                float bottom_rightx, float bottom_righty,
                float x, float y)
{
    return (x > top_leftx && x < bottom_rightx &&
            y > top_lefty && y < bottom_righty);
}

// Vectors
typedef union {
    struct {float x; float y;};
    float arr[2];
} V2;

V2
v2(float x, float y)
{
    return (V2){.x = x, .y = y};
}


V2
v2_plus(const V2 i, const V2 j)
{
    return v2(i.x + j.x,
              i.y + j.y);
}


V2
v2_minus(const V2 i, const V2 j)
{
    return v2(i.x - j.x,
              i.y - j.y);
}


V2
v2_scale(const V2 v, const float f)
{
    return v2(v.x * f,
              v.y * f);
}


V2
v2_rotate(const V2 v, const float radians)
{
    float nx = v.x * cos(radians) - v.y * sin(radians);
    float ny = v.x * sin(radians) + v.y * cos(radians);

    return v2(nx, ny);
}

// Bounding Box
typedef struct BoundingBox {
    V2 top_left;
    V2 bottom_right;
} BoundingBox;

/* Bounding Box */

BoundingBox
boundingBox(V2 top_left, float width, float height)
{
    return (BoundingBox){top_left, v2_plus(top_left, v2(width, height))};
}


bool
bb_contains(BoundingBox bb, float x, float y)
{
    return bounds_contains(bb.top_left.x,
                           bb.top_left.y,
                           bb.bottom_right.x,
                           bb.bottom_right.y, x, y);
}


V2
bb_center(BoundingBox bb)
{
    float centerx = ((bb.bottom_right.x - bb.top_left.x) / 2)
        + bb.top_left.x;
    float centery = ((bb.bottom_right.y - bb.top_left.y) / 2)
        + bb.top_left.y;

    return v2(centerx, centery);
}


// Calculate width and height of bb
V2
bb_size(BoundingBox bb)
{
    return v2(bb.bottom_right.x - bb.top_left.x,
              bb.bottom_right.y - bb.top_left.y);
}


// up the size of a bb by 1
BoundingBox
bb_blow_up(BoundingBox bb)
{
    bb.top_left.x -= 1;
    bb.top_left.y -= 1;
    bb.bottom_right.x += 2;
    bb.bottom_right.y += 2;
    return bb;
}

#endif
