#pragma once

#include <stdbool.h>
#include "ui_types.h"

#define NC_PLATFORM_SDL

// I can do what the fuck I want it's my software
typedef enum {
    Event_HandleAtTopLevel,
    Event_Passthrough,
    Event_None
} EventClass;

typedef enum {
    EventType_Quit,
    EventType_TextInput,
    EventType_WindowResized,
} TopLevelEvent;

typedef enum {
    EventType_MouseMotion,
    EventType_MouseDown,
    EventType_MouseUp
} PassthroughEvent;

typedef enum {
    Mouse_Left,
    Mouse_Middle,
    Mouse_Right
} MouseButton;

typedef struct {
    PassthroughEvent type;
    Vec2 pos;
    MouseButton button;
} UIEvent;

typedef struct {
    EventClass class;
    char* text;
    Vec2 size;
    TopLevelEvent tlType;
    UIEvent ui;
} Event;

#ifdef NC_PLATFORM_SDL

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

typedef struct {
    SDL_Window* win;
    SDL_Renderer* ren;
    SDL_Event eventRaw;
    Event event;
} Platform;

typedef TTF_Font Font;

#endif

Platform* CreatePlatform(char* windowTitle, bool fullscreen);
void DestroyPlatform(Platform* self);

// Null if none left in queue
Event* GetEvent(Platform* self);
UIEvent GetUIEvent(Platform* self);

Vec2 GetSize(Platform* self);
void SetDrawColour(Platform* self, Colour col);
void FlushDisplay(Platform* self, Colour col);
void DrawText(Platform* self, Font* font, char* text, Vec2 pos);
void DrawRect(Platform* self, Vec2 pos, Vec2 size);
void FillRect(Platform* self, Vec2 pos, Vec2 size);

Font* CreateFont(char* family, int size);
void DestroyFont(Font* self);

Vec2 GetTextSize(Font* self, char* text);