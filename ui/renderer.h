#pragma once

#include <stdbool.h>
#include "ui_types.h"

#define NC_RENDERER_SDL

#ifdef NC_RENDERER_SDL

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

typedef struct {
    SDL_Window* win;
    SDL_Renderer* ren;
} Renderer;

typedef TTF_Font Font;

#endif

Renderer* CreateRenderer(char* windowTitle, bool fullscreen);
void DestroyRenderer(Renderer* self);

Vec2 GetSize(Renderer* self);
void SetDrawColour(Renderer* self, Colour col);
void Flush(Renderer* self, Colour col);
void DrawText(Renderer* self, Font* font, char* text, Vec2 pos);
void DrawRect(Renderer* self, Vec2 pos, Vec2 size);
void FillRect(Renderer* self, Vec2 pos, Vec2 size);

Font* CreateFont(char* family, int size);
void DestroyFont(Font* self);

Vec2 GetTextSize(Font* self, char* text);