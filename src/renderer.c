#include "renderer.h"

#include <SDL3/SDL.h>
#include <stdlib.h>
#include <assert.h>

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

extern SDL_Renderer *renderer;
extern SDL_Texture *screen_texture;
extern Image *screen_image;

static Rect clip;

static Rect intersect_rects(Rect a, Rect b) {
    int x1 = max(a.x, b.x);
    int y1 = max(a.y, b.y);
    int x2 = min(a.x + a.width, b.x + b.width);
    int y2 = min(a.y + a.height, b.y + b.height);
    return (Rect) { x1, y1, x2 - x1, y2 - y1 };
}

static inline Color blend_pixel(Color dst, Color src) {
    Color res;
    res.w = (dst.w & 0xff00ff) + ((((src.w & 0xff00ff) - (dst.w & 0xff00ff)) * src.a) >> 8);
    res.g = dst.g + (((src.g - dst.g) * src.a) >> 8);
    res.a = dst.a;
    return res;
}


void ren_init() {
    ren_set_clip((Rect) { 0, 0, 320, 180 });
}

void ren_update() {
    SDL_UpdateTexture(screen_texture, NULL, screen_image->pixels, 320 * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, screen_texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

Image* ren_create_image(int width, int height) {
    assert(width > 0 && height > 0);
    Image *image = malloc(sizeof(Image) + width * height * sizeof(Color));
    image->pixels = (void*) (image + 1);
    image->width = width;
    image->height = height;
    return image;
}

void ren_destroy_image(Image *image) {
    free(image);
}

void ren_clear(Color color) {
    ren_draw_rect((Rect) {0, 0, 0xffffff, 0xffffff}, color);
}

void ren_set_clip(Rect rect) {
    Rect screen_rect = (Rect) { 0, 0, 320, 180 };
    clip = intersect_rects(rect, screen_rect);
}

void ren_draw_point(int x, int y, Color color) {
    if (color.a == 0) { return; }
    Rect r = clip;
    if (x < r.x || y < r.y || x >= r.x + r.width || y >= r.y + r.height ) {
        return;
    }
    Color *dst = &screen_image->pixels[x + y * screen_image->width];
    *dst = blend_pixel(*dst, color);
}

void ren_draw_rect(Rect rect, Color color) {
    if (color.a == 0) { return; }
    rect = intersect_rects(rect, clip);
    Color *d = &screen_image->pixels[rect.x + rect.y * screen_image->width];
    int dr = screen_image->width - rect.width;
    for (int y = 0; y < rect.height; y++) {
        for (int x = 0; x < rect.width; x++) {
            *d = blend_pixel(*d, color);
            d++;
        }
        d += dr;
    }
}
