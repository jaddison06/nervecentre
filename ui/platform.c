#include "platform.h"

Platform* _platform = 0;

#ifdef NC_PLATFORM_SDL

#define COL_TO_SDL(colour) (SDL_Color){colour.r, colour.g, colour.b, 255}
#define POSSIZE_TO_SDL(pos, size) (SDL_Rect){pos.x, pos.y, size.x, size.y}

#ifdef _WIN32
#include <windows.h>
#endif

// If this returns 0 you can call SDL_GetError() for error message
Platform* InitPlatform(char* windowTitle, bool fullscreen) {
    if (_platform != 0) return _platform;
#ifdef _WIN32
    SetProcessDPIAware();
#endif

    if (SDL_Init(SDL_INIT_VIDEO) != 0) return 0;

    _platform = malloc(sizeof(Platform));

    uint32_t flags = 0;
    if (fullscreen) flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    else flags |= SDL_WINDOW_RESIZABLE;

#define EndItAll() { DestroyPlatform(); return 0; }

    _platform->win = SDL_CreateWindow(windowTitle, 40, 40, 350, 350, flags);
    if (_platform->win == 0) EndItAll()

    _platform->ren = SDL_CreateRenderer(_platform->win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (_platform->ren == 0) EndItAll()

    if (TTF_Init() != 0) EndItAll()

    _platform->event.text = malloc(32);
    SDL_StartTextInput();

#undef EndItAll

    return _platform;
}

void DestroyPlatform() {
    TTF_Quit();
    SDL_DestroyRenderer(_platform->ren);
    SDL_DestroyWindow(_platform->win);
    SDL_Quit();
    free(_platform->event.text);
    free(_platform);
    _platform = 0;
}

static MouseButton TranslateMouseButton(uint8_t sdlButton) {
    switch (sdlButton) {
        case SDL_BUTTON_LEFT: return Mouse_Left;
        case SDL_BUTTON_MIDDLE: return Mouse_Middle;
        case SDL_BUTTON_RIGHT: return Mouse_Right;
    }
}

extern inline Event* GetEvent() { return &_platform->event; }

Event* NextEvent() {
    if (SDL_PollEvent(&_platform->eventRaw) == 0) return 0;
    switch (_platform->eventRaw.type) {
        case SDL_QUIT: {
            _platform->event.class = Event_HandleAtTopLevel;
            _platform->event.tlType = EventType_Quit;
            return &_platform->event;
        }
        // todo: does this work at all?
        case SDL_TEXTINPUT: {
            _platform->event.class = Event_HandleAtTopLevel;
            _platform->event.tlType = EventType_TextInput;
            strcpy(_platform->event.text, _platform->eventRaw.text.text);
            return &_platform->event;
        }
        case SDL_WINDOWEVENT: {
            if (_platform->eventRaw.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                _platform->event.class = Event_HandleAtTopLevel;
                _platform->event.tlType = EventType_WindowResized;
                _platform->event.size.x = _platform->eventRaw.window.data1;
                _platform->event.size.y = _platform->eventRaw.window.data2;
                return &_platform->event;
            }
        }
        case SDL_MOUSEMOTION: {
            _platform->event.class = Event_Passthrough;
            _platform->event.ui.type = EventType_MouseMotion;
            _platform->event.ui.pos.x = _platform->eventRaw.motion.x;
            _platform->event.ui.pos.y = _platform->eventRaw.motion.y;
            return &_platform->event;
        }
        case SDL_MOUSEBUTTONDOWN: {
            _platform->event.class = Event_Passthrough;
            _platform->event.ui.type = EventType_MouseDown;
            _platform->event.ui.pos.x = _platform->eventRaw.button.x;
            _platform->event.ui.pos.y = _platform->eventRaw.button.y;
            _platform->event.ui.button = TranslateMouseButton(_platform->eventRaw.button.button);
            return &_platform->event;
        }
        case SDL_MOUSEBUTTONUP: {
            _platform->event.class = Event_Passthrough;
            _platform->event.ui.type = EventType_MouseUp;
            _platform->event.ui.pos.x = _platform->eventRaw.button.x;
            _platform->event.ui.pos.y = _platform->eventRaw.button.y;
            _platform->event.ui.button = TranslateMouseButton(_platform->eventRaw.button.button);
            return &_platform->event;
        }
        default: {
            _platform->event.class = Event_None;
            return &_platform->event;
        }
    }

    return 0;
}

UIEvent GetUIEvent() { return _platform->event.ui; }

Vec2 GetWindowSize() {
    Vec2 out;
    SDL_GetWindowSize(_platform->win, &out.x, &out.y);
    return out;
}

void SetDrawColour(Colour col) {
    SDL_SetRenderDrawColor(_platform->ren, col.r, col.g, col.b, 255);
}

void FlushDisplay() {
    SDL_RenderPresent(_platform->ren);
}

static SDL_Texture* GetTextTexture(Font* font, char* text, Vec2* outDimensions) {
    SDL_Color col;
    SDL_GetRenderDrawColor(_platform->ren, &col.r, &col.g, &col.b, &col.a);
    SDL_Surface* textSurface = TTF_RenderUTF8_Blended(font, text, col);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(_platform->ren, textSurface);
    outDimensions->x = textSurface->w;
    outDimensions->y = textSurface->h;
    SDL_FreeSurface(textSurface);
    return textTexture;
}

static void RenderTexture(SDL_Texture* texture, Vec2 pos, Vec2 size) {
    SDL_Rect rect = POSSIZE_TO_SDL(pos, size);
    SDL_RenderCopy(_platform->ren, texture, 0, &rect);
}

void DrawText(Font* font, char* text, Vec2 pos) {
    Vec2 size;
    SDL_Texture* textTexture = GetTextTexture(font, text, &size);
    RenderTexture(textTexture, pos, size);
    SDL_DestroyTexture(textTexture);
}

void DrawRect(Vec2 pos, Vec2 size) {
    SDL_Rect rect = POSSIZE_TO_SDL(pos, size);
    SDL_RenderDrawRect(_platform->ren, &rect);
}

void FillRect(Vec2 pos, Vec2 size) {
    SDL_Rect rect = POSSIZE_TO_SDL(pos, size);
    SDL_RenderFillRect(_platform->ren, &rect);
}

Font* CreateFont(char* family, int size) {
    Font* out = TTF_OpenFont(family, size);
    return out;
}

void DestroyFont(Font* self) {
    TTF_CloseFont(self);
}

Vec2 GetTextSize(Font* self, char* text) {
    Vec2 out;
    TTF_SizeText(self, text, &out.x, &out.y);
    return out;
}

#endif