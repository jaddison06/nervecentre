#pragma once

typedef struct {
    int x, y;
} Vec2;

typedef struct {
    int r, g, b;
} Colour;

#define V2(X, Y) (Vec2){.x = X, .y = Y}
#define COL(R, G, B) (Colour){.r = R, .g = G, .b = B}