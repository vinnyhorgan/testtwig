#include "renderer.h"

#include <SDL3/SDL.h>
#include <stdlib.h>
#include <assert.h>

struct Image {
    Color *pixels;
    int width, height;
};

extern SDL_Renderer *renderer;
extern SDL_Texture *screen_texture;
extern uint32_t *screen_pixels;

static struct { int left, top, right, bottom; } clip;

void ren_set_clip_rect(Rect rect) {
    clip.left   = rect.x;
    clip.top    = rect.y;
    clip.right  = rect.x + rect.width;
    clip.bottom = rect.y + rect.height;
}

void ren_init() {
    ren_set_clip_rect((Rect) { 0, 0, 320, 180 });
}

void ren_update() {
    SDL_UpdateTexture(screen_texture, NULL, screen_pixels, 320 * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, screen_texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

Image* ren_new_image(int width, int height) {
    assert(width > 0 && height > 0);
    Image *image = malloc(sizeof(Image) + width * height * sizeof(Color));
    image->pixels = (void*) (image + 1);
    image->width = width;
    image->height = height;
    return image;
}

void ren_free_image(Image *image) {
    free(image);
}

#define rect_draw_loop(expr)            \
    for (int j = y1; j < y2; j++) {     \
        for (int i = x1; i < x2; i++) { \
            *d = expr;                  \
            d++;                        \
        }                               \
        d += dr;                        \
    }

static inline Color blend_pixel(Color dst, Color src) {
    int ia = 0xff - src.a;
    dst.r = ((src.r * src.a) + (dst.r * ia)) >> 8;
    dst.g = ((src.g * src.a) + (dst.g * ia)) >> 8;
    dst.b = ((src.b * src.a) + (dst.b * ia)) >> 8;
    return dst;
}

void ren_draw_rect(Rect rect, Color color) {
    if (color.a == 0) { return; }

    int x1 = rect.x < clip.left ? clip.left : rect.x;
    int y1 = rect.y < clip.top  ? clip.top  : rect.y;
    int x2 = rect.x + rect.width;
    int y2 = rect.y + rect.height;
    x2 = x2 > clip.right  ? clip.right  : x2;
    y2 = y2 > clip.bottom ? clip.bottom : y2;

    Color *d = (Color*) screen_pixels;
    d += x1 + y1 * 320;
    int dr = 320 - (x2 - x1);

    if (color.a == 0xff) {
        rect_draw_loop(color);
    } else {
        rect_draw_loop(blend_pixel(*d, color));
    }
}

void ren_draw_image(Image *image, Rect *sub, int x, int y, Color color) {

}
