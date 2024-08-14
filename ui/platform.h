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

// Global state (👎) = Singleton pattern (👍) get your head out your arse
extern Platform* _platform;
inline void SetPlatform(Platform* platform) { _platform = platform; }

Platform* InitPlatform(char* windowTitle, bool fullscreen);
void DestroyPlatform();

// Pull next event from queue, null if none left
Event* NextEvent();
// Returns last event pulled from queue (ie currently processing)
extern inline Event* GetEvent();
UIEvent GetUIEvent();

Vec2 GetWindowSize();
void SetDrawColour(Colour col);
void FlushDisplay();
void DrawText(Font* font, char* text, Vec2 pos);
void DrawRect(Vec2 pos, Vec2 size);
void FillRect(Vec2 pos, Vec2 size);

Font* CreateFont(char* family, int size);
void DestroyFont(Font* self);

Vec2 GetTextSize(Font* self, char* text);