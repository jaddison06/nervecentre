#include "platform.h"

#ifdef NC_PLATFORM_SDL

#define COL_TO_SDL(colour) (SDL_Color){colour.r, colour.g, colour.b, 255}
#define POSSIZE_TO_SDL(pos, size) (SDL_Rect){pos.x, pos.y, size.x, size.y}

#ifdef _WIN32
#include <windows.h>
#endif

// If this returns 0 you can call SDL_GetError() for error message
Platform* CreatePlatform(char* windowTitle, bool fullscreen) {
#ifdef _WIN32
    SetProcessDPIAware();
#endif

    if (SDL_Init(SDL_INIT_VIDEO) != 0) return 0;

    Platform* out = malloc(sizeof(Platform));

    uint32_t flags = 0;
    if (fullscreen) flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    else flags |= SDL_WINDOW_RESIZABLE;

#define EndItAll() { DestroyPlatform(out); return 0; }

    out->win = SDL_CreateWindow(windowTitle, 40, 40, 350, 350, flags);
    if (out->win == 0) EndItAll()

    out->ren = SDL_CreateRenderer(out->win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (out->ren == 0) EndItAll()

    if (TTF_Init() != 0) EndItAll()

    out->event.text = malloc(32);
    SDL_StartTextInput();

#undef EndItAll

    return out;
}

void DestroyPlatform(Platform* self) {
    TTF_Quit();
    SDL_DestroyRenderer(self->ren);
    SDL_DestroyWindow(self->win);
    SDL_Quit();
    free(self->event.text);
    free(self);
}

static MouseButton TranslateMouseButton(uint8_t sdlButton) {
    switch (sdlButton) {
        case SDL_BUTTON_LEFT: return Mouse_Left;
        case SDL_BUTTON_MIDDLE: return Mouse_Middle;
        case SDL_BUTTON_RIGHT: return Mouse_Right;
    }
}

Event* GetEvent(Platform* self) {
    if (SDL_PollEvent(&self->eventRaw) == 0) return 0;
    switch (self->eventRaw.type) {
        case SDL_QUIT: {
            self->event.class = Event_HandleAtTopLevel;
            self->event.tlType = EventType_Quit;
            break;
        }
        // todo: does this work at all?
        case SDL_TEXTINPUT: {
            self->event.class = Event_HandleAtTopLevel;
            self->event.tlType = EventType_TextInput;
            strcpy(self->event.text, self->eventRaw.text.text);
            break;
        }
        case SDL_WINDOWEVENT: {
            if (self->eventRaw.window.type == SDL_WINDOWEVENT_RESIZED) {
                self->event.class = Event_HandleAtTopLevel;
                self->event.tlType = EventType_WindowResized;
                self->event.size.x = self->eventRaw.window.data1;
                self->event.size.y = self->eventRaw.window.data2;
                break;
            }
        }
        case SDL_MOUSEMOTION: {
            self->event.class = Event_Passthrough;
            self->event.ui.type = EventType_MouseMotion;
            self->event.ui.pos.x = self->eventRaw.motion.x;
            self->event.ui.pos.y = self->eventRaw.motion.y;
            break;
        }
        case SDL_MOUSEBUTTONDOWN: {
            self->event.class = Event_Passthrough;
            self->event.ui.type = EventType_MouseDown;
            self->event.ui.pos.x = self->eventRaw.button.x;
            self->event.ui.pos.y = self->eventRaw.button.y;
            self->event.ui.button = TranslateMouseButton(self->eventRaw.button.button);
            break;
        }
        case SDL_MOUSEBUTTONUP: {
            self->event.class = Event_Passthrough;
            self->event.ui.type = EventType_MouseUp;
            self->event.ui.pos.x = self->eventRaw.button.x;
            self->event.ui.pos.y = self->eventRaw.button.y;
            self->event.ui.button = TranslateMouseButton(self->eventRaw.button.button);
            break;
        }
        default: {
            self->event.class = Event_None;
            break;
        }
    }

    return &self->event;
}

UIEvent GetUIEvent(Platform* self) { return self->event.ui; }

Vec2 GetSize(Platform* self) {
    Vec2 out;
    SDL_GetWindowSize(self->win, &out.x, &out.y);
    return out;
}

void SetDrawColour(Platform* self, Colour col) {
    SDL_SetRenderDrawColor(self->ren, col.r, col.g, col.b, 255);
}

void FlushDisplay(Platform* self, Colour col) {
    SDL_RenderPresent(self->ren);
}

static SDL_Texture* GetTextTexture(Platform* self, Font* font, char* text, Vec2* outDimensions) {
    SDL_Color col;
    SDL_GetRenderDrawColor(self->ren, &col.r, &col.g, &col.b, &col.a);
    SDL_Surface* textSurface = TTF_RenderUTF8_Blended(font, text, col);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(self->ren, textSurface);
    outDimensions->x = textSurface->w;
    outDimensions->y = textSurface->h;
    SDL_FreeSurface(textSurface);
    return textTexture;
}

static void RenderTexture(Platform* self, SDL_Texture* texture, Vec2 pos, Vec2 size) {
    SDL_Rect rect = POSSIZE_TO_SDL(pos, size);
    SDL_RenderCopy(self->ren, texture, NULL, &rect);
}

void DrawText(Platform* self, Font* font, char* text, Vec2 pos) {
    Vec2 size;
    SDL_Texture* textTexture = GetTextTexture(self, font, text, &pos);
    RenderTexture(self, textTexture, pos, size);
    SDL_DestroyTexture(textTexture);
}

void DrawRect(Platform* self, Vec2 pos, Vec2 size) {
    SDL_Rect rect = POSSIZE_TO_SDL(pos, size);
    SDL_RenderDrawRect(self->ren, &rect);
}

void FillRect(Platform* self, Vec2 pos, Vec2 size) {
    SDL_Rect rect = POSSIZE_TO_SDL(pos, size);
    SDL_RenderFillRect(self->ren, &rect);
}

Font* CreateFont(char* family, int size) {
    return TTF_OpenFont(family, size);
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