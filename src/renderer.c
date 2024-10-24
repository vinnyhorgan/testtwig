#include "renderer.h"

#include <SDL3/SDL.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "lib/stb/stb_image.h"

#define ren_max(a, b) ((a) > (b) ? (a) : (b))
#define ren_min(a, b) ((a) < (b) ? (a) : (b))

extern SDL_Renderer *renderer;
extern SDL_Texture *screen_texture;
extern Image *screen_image;

static Rect clip;
static Font *set_font;

static Rect intersect_rects(Rect a, Rect b) {
    int x1 = ren_max(a.x, b.x);
    int y1 = ren_max(a.y, b.y);
    int x2 = ren_min(a.x + a.width, b.x + b.width);
    int y2 = ren_min(a.y + a.height, b.y + b.height);
    return (Rect) { x1, y1, x2 - x1, y2 - y1 };
}

static inline Color blend_pixel(Color dst, Color src) {
    Color res;
    res.w = (dst.w & 0xff00ff) + ((((src.w & 0xff00ff) - (dst.w & 0xff00ff)) * src.a) >> 8);
    res.g = dst.g + (((src.g - dst.g) * src.a) >> 8);
    res.a = dst.a;
    return res;
}

static inline Color blend_pixel2(Color dst, Color src, Color clr) {
    src.a = (src.a * clr.a) >> 8;
    int ia = 0xff - src.a;
    dst.r = ((src.r * clr.r * src.a) >> 16) + ((dst.r * ia) >> 8);
    dst.g = ((src.g * clr.g * src.a) >> 16) + ((dst.g * ia) >> 8);
    dst.b = ((src.b * clr.b * src.a) >> 16) + ((dst.b * ia) >> 8);
    return dst;
}


static inline Color blend_pixel3(Color dst, Color src, Color clr, Color add) {
    src.r = ren_min(255, src.r + add.r);
    src.g = ren_min(255, src.g + add.g);
    src.b = ren_min(255, src.b + add.b);
    return blend_pixel2(dst, src, clr); // signal indentation bug to rxi
}

static void* read_file(char *filename, int *len) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) { return NULL; }
    fseek(fp, 0, SEEK_END);
    int n = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *buf = malloc(n + 1);
    fread(buf, 1, n, fp);
    fclose(fp);
    if (len) { *len = n; }
    return buf;
}

void ren_init() {
    clip = ren_rect(0, 0, screen_image->width, screen_image->height);

    for (int y = 0; y < screen_image->height; y++) {
        for (int x = 0; x < screen_image->width; x++) {
            screen_image->pixels[x + y * screen_image->width] = ren_rgb(0, 0, 0); // investigate why this is necessary
        }
    }
}

void ren_update() {
    SDL_UpdateTexture(screen_texture, NULL, screen_image->pixels, screen_image->width * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, screen_texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

Image* ren_create_image(int width, int height) {
    assert(width > 0 && height > 0);
    Image *image = calloc(1, sizeof(Image) + width * height * sizeof(Color));
    image->pixels = (void*) (image + 1);
    image->width = width;
    image->height = height;
    return image;
}

Image* ren_load_image_file(char *filename) {
    int len;
    void *data = read_file(filename, &len);
    if (!data) { return NULL; }
    Image *res = ren_load_image_mem(data, len);
    free(data);
    return res;
}

Image* ren_load_image_mem(void *data, int len) {
    int width, height;
    stbi_uc *pixels = stbi_load_from_memory(data, len, &width, &height, NULL, 4);
    if (!pixels) { return NULL; }

    Image *image = ren_create_image(width, height);
    memcpy(image->pixels, pixels, width * height * sizeof(Color));
    stbi_image_free(pixels);

    uint8_t* bytes = (uint8_t*)image->pixels;
    int32_t n = image->width * image->height * sizeof(uint32_t);
    for (int32_t i = 0; i < n; i += 4) {
        uint8_t b = bytes[i];
        bytes[i] = bytes[i + 2];
        bytes[i + 2] = b;
    }

    return image;
}

void ren_destroy_image(Image *image) {
    free(image);
}

void ren_clear(Color color) {
    ren_draw_rect(ren_rect(0, 0, 0xffffff, 0xffffff), color);
}

void ren_set_clip(Rect rect) {
    Rect screen_rect = ren_rect(0, 0, screen_image->width, screen_image->height);
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

void ren_draw_image(Image *img, int x, int y) {
    Rect dst = ren_rect(x, y, img->width, img->height);
    Rect src = ren_rect(0, 0, img->width, img->height);
    ren_draw_image3(img, dst, src, ren_rgb(255, 255, 255), ren_rgb(0, 0, 0));
}


void ren_draw_image2(Image *img, int x, int y, Rect src, Color color) {
    Rect dst = ren_rect(x, y, abs(src.width), abs(src.height));
    ren_draw_image3(img, dst, src, color, ren_rgb(0, 0, 0));
}


void ren_draw_image3(Image *img, Rect dst, Rect src, Color mul_color, Color add_color) {
    if (!src.width || !src.height || !dst.width || !dst.height) { // signal bug to rxi
        return;
    }

    /* do scaled render */
    int cx1 = clip.x;
    int cy1 = clip.y;
    int cx2 = cx1 + clip.width;
    int cy2 = cy1 + clip.height;
    int stepx = (src.width << 10) / dst.width;
    int stepy = (src.height << 10) / dst.height;
    int sy = src.y << 10;

    /* vertical clipping */
    int dy = dst.y;
    if (dy < cy1) { sy += (cy1 - dy) * stepy; dy = cy1; }
    int ey = ren_min(cy2, dst.y + dst.height);

    int blend_fn = 1;
    if (mul_color.w != 0xffffffff) { blend_fn = 2; }
    if ((add_color.w & 0xffffff00) != 0xffffff00) { blend_fn = 3; }

    for (; dy < ey; dy++) {
        if (dy >= cy1 && dy < cy2) {
            int sx = src.x << 10;
            Color *srow = &img->pixels[(sy >> 10) * img->width];
            Color *drow = &screen_image->pixels[dy * screen_image->width];

            /* horizontal clipping */
            int dx = dst.x;
            if (dx < cx1) { sx += (cx1 - dx) * stepx; dx = cx1; }
            int ex = ren_min(cx2, dst.x + dst.width);

            for (; dx < ex; dx++) {
                Color *s = &srow[sx >> 10];
                Color *d = &drow[dx];
                switch (blend_fn) {
                case 1: *d = blend_pixel(*d, *s); break;
                case 2: *d = blend_pixel2(*d, *s, mul_color); break;
                case 3: *d = blend_pixel3(*d, *s, mul_color, add_color); break;
                }
                sx += stepx;
            }
        }
        sy += stepy;
    }
}


int ren_draw_text(char *text, int x, int y, Color color) {
    return ren_draw_text2(set_font, text, x, y, color);
}


int ren_draw_text2(Font *font, char *text, int x, int y, Color color) {
    for (uint8_t *p = (void*) text; *p; p++) {
        Glyph g = font->glyphs[*p];
        ren_draw_image2(font->image, x, y, g.rect, color);
        x += g.xadv;
    }
    return x;
}

