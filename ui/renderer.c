#include "renderer.h"

#ifdef NC_RENDERER_SDL

#define COL_TO_SDL(colour) (SDL_Color){colour.r, colour.g, colour.b, 255}
#define POSSIZE_TO_SDL(pos, size) (SDL_Rect){pos.x, pos.y, size.x, size.y}

#ifdef _WIN32
#include <windows.h>
#endif

// If this returns 0 you can call SDL_GetError() for error message
Renderer* CreateRenderer(char* windowTitle, bool fullscreen) {
#ifdef _WIN32
    SetProcessDPIAware();
#endif

    if (SDL_Init(SDL_INIT_VIDEO) != 0) return 0;

    Renderer* out = malloc(sizeof(Renderer));

    uint32_t flags = 0;
    if (fullscreen) flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    else flags |= SDL_WINDOW_RESIZABLE;

#define EndItAll() { DestroyRenderer(out); return 0; }

    out->win = SDL_CreateWindow(windowTitle, 40, 40, 350, 350, flags);
    if (out->win == 0) EndItAll()

    out->ren = SDL_CreateRenderer(out->win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (out->ren == 0) EndItAll()

    if (TTF_Init() != 0) EndItAll()

    // todo: setup text input

#undef EndItAll

    return out;
}

void DestroyRenderer(Renderer* ren) {
    TTF_Quit();
    SDL_DestroyRenderer(ren->ren);
    SDL_DestroyWindow(ren->win);
    SDL_Quit();
    free(ren);
}

Vec2 GetSize(Renderer* self) {
    Vec2 out;
    SDL_GetWindowSize(self->win, &out.x, &out.y);
    return out;
}

void SetDrawColour(Renderer* self, Colour col) {
    SDL_SetRenderDrawColor(self->ren, col.r, col.g, col.b, 255);
}

void Flush(Renderer* self, Colour col) {
    SDL_RenderPresent(self->ren);
}

static SDL_Texture* GetTextTexture(Renderer* self, Font* font, char* text, Vec2* outDimensions) {
    SDL_Color col;
    SDL_GetRenderDrawColor(self->ren, &col.r, &col.g, &col.b, &col.a);
    SDL_Surface* textSurface = TTF_RenderUTF8_Blended(font, text, col);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(self->ren, textSurface);
    outDimensions->x = textSurface->w;
    outDimensions->y = textSurface->h;
    SDL_FreeSurface(textSurface);
    return textTexture;
}

static void RenderTexture(Renderer* self, SDL_Texture* texture, Vec2 pos, Vec2 size) {
    SDL_Rect rect = POSSIZE_TO_SDL(pos, size);
    SDL_RenderCopy(self->ren, texture, NULL, &rect);
}

void DrawText(Renderer* self, Font* font, char* text, Vec2 pos) {
    Vec2 size;
    SDL_Texture* textTexture = GetTextTexture(self, font, text, &pos);
    RenderTexture(self, textTexture, pos, size);
    SDL_DestroyTexture(textTexture);
}

void DrawRect(Renderer* self, Vec2 pos, Vec2 size) {
    SDL_Rect rect = POSSIZE_TO_SDL(pos, size);
    SDL_RenderDrawRect(self->ren, &rect);
}

void FillRect(Renderer* self, Vec2 pos, Vec2 size) {
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