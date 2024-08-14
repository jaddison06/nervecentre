#pragma once

#include <stdbool.h>

typedef struct {
    int x, y;
} Vec2;

bool V2ContainedByBox(Vec2 point, Vec2 boxPos, Vec2 boxSize);

typedef struct {
    int r, g, b;
} Colour;

#define V2(X, Y) (Vec2){.x = X, .y = Y}
#define COL(R, G, B) (Colour){.r = R, .g = G, .b = B}