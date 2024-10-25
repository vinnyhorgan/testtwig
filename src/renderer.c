#include "renderer.h"

#include <SDL3/SDL.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "lib/stb/stb_image.h"

#define ren_max(a, b) ((a) > (b) ? (a) : (b))
#define ren_min(a, b) ((a) < (b) ? (a) : (b))

extern SDL_Window *window;
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

static void* read_file(const char *filename, int *len) {
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

static bool check_column(Image *img, int x, int y, int h) {
    while (h > 0) {
        if (img->pixels[x + y * img->width].a) {
            return true;
        }
        y++; h--;
    }
    return false;
}

static Font* load_font_from_image(Image *img) {
    if (!img) { return NULL; }
    Font *font = malloc(sizeof(Font));
    font->image = img;

    // init glyphs
    for (int i = 0; i < 256; i++) {
        Glyph *g = &font->glyphs[i];
        Rect r = {
            (img->width / 16) * (i % 16),
            (img->height / 16) * (i / 16),
            img->width / 16,
            img->height / 16
        };
        // right-trim rect
        for (int x = r.x + r.width - 1; x >= r.x; x--) {
            if (check_column(font->image, x, r.y, r.height)) { break; }
            r.width--;
        }
        // left-trim rect
        for (int x = r.x; x < r.x + r.width; x++) {
            if (check_column(font->image, x, r.y, r.height)) { break; }
            r.x++;
            r.width--;
        }
        // set xadvance and rect
        g->xadv = r.width + 1;
        g->rect = r;
    }

    font->glyphs[' '].rect = (Rect) {0};
    font->glyphs[' '].xadv = font->glyphs['a'].xadv;

    return font;
}

static void *font_png_data;
static int   font_png_size;

void ren_init() {
    clip = ren_rect(0, 0, screen_image->width, screen_image->height);

    for (int y = 0; y < screen_image->height; y++) {
        for (int x = 0; x < screen_image->width; x++) {
            screen_image->pixels[x + y * screen_image->width] = ren_rgb(0, 0, 0); // investigate why this is necessary
        }
    }

    set_font = ren_load_font_mem(font_png_data, font_png_size);
}

void ren_update() {
    SDL_UpdateTexture(screen_texture, NULL, screen_image->pixels, screen_image->width * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, screen_texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    static bool first = true;
    if (first) {
        SDL_ShowWindow(window);
        first = false;
    }
}

Image* ren_create_image(int width, int height) {
    assert(width > 0 && height > 0);
    Image *image = calloc(1, sizeof(Image) + width * height * sizeof(Color));
    image->pixels = (void*) (image + 1);
    image->width = width;
    image->height = height;
    return image;
}

Image* ren_load_image_file(const char *filename) {
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

Font* ren_load_font_file(const char *filename) {
    return load_font_from_image(ren_load_image_file(filename));
}

Font* ren_load_font_mem(void *data, int len) {
    return load_font_from_image(ren_load_image_mem(data, len));
}

void ren_destroy_font(Font *font) {
    free(font->image);
    free(font);
}

int ren_text_width(Font *font, const char *text) {
    int x = 0;
    for (uint8_t *p = (void*) text; *p; p++) {
        x += font->glyphs[*p].xadv;
    }
    return x;
}

int ren_text_height(Font *font) {
    return font->image->height / 16;
}

int ren_text_width_default(const char *text) {
    int x = 0;
    for (uint8_t *p = (void*) text; *p; p++) {
        x += set_font->glyphs[*p].xadv;
    }
    return x;
}

int ren_text_height_default() {
    return set_font->image->height / 16;
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

void ren_draw_line(int x1, int y1, int x2, int y2, Color color) {
    int dx = abs(x2-x1);
    int sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1);
    int sy = y1 < y2 ? 1 : -1;
    int err = dx + dy;
    for (;;) {
        ren_draw_point(x1, y1, color);
        if (x1 == x2 && y1 == y2) { break; }
        int e2 = err << 1;
        if (e2 >= dy) { err += dy; x1 += sx; }
        if (e2 <= dx) { err += dx; y1 += sy; }
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

int ren_draw_text(const char *text, int x, int y, Color color) {
    return ren_draw_text2(set_font, text, x, y, color);
}

static const char* utf8_to_codepoint(const char *p, unsigned *dst) {
  unsigned res, n;
  switch (*p & 0xf0) {
    case 0xf0 :  res = *p & 0x07;  n = 3;  break;
    case 0xe0 :  res = *p & 0x0f;  n = 2;  break;
    case 0xd0 :
    case 0xc0 :  res = *p & 0x1f;  n = 1;  break;
    default   :  res = *p;         n = 0;  break;
  }
  while (n--) {
    res = (res << 6) | (*(++p) & 0x3f);
  }
  *dst = res;
  return p + 1;
}

Glyph* find_glyph(Font *font, unsigned  codepoint) {
    if (codepoint < 256) {
        return &font->glyphs[codepoint];
    }
    return &font->glyphs['?'];
}

int ren_draw_text2(Font *font, const char *text, int x, int y, Color color) {
    const char *p = text;
    unsigned codepoint;
    while (*p) {
        p = utf8_to_codepoint(p, &codepoint);
        Glyph *g = find_glyph(font, codepoint);
        ren_draw_image2(font->image, x, y, g->rect, color);
        x += g->xadv;
    }
}

static char font_png[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00,
    0x0d, 0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
    0x00, 0x80, 0x01, 0x03, 0x00, 0x00, 0x00, 0xf9, 0xf0, 0xf3, 0x88,
    0x00, 0x00, 0x00, 0x06, 0x50, 0x4c, 0x54, 0x45, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xa5, 0xd9, 0x9f, 0xdd, 0x00, 0x00, 0x00, 0x01,
    0x74, 0x52, 0x4e, 0x53, 0x00, 0x40, 0xe6, 0xd8, 0x66, 0x00, 0x00,
    0x03, 0xb6, 0x49, 0x44, 0x41, 0x54, 0x48, 0xc7, 0xed, 0x55, 0xbd,
    0x8a, 0x1c, 0x47, 0x10, 0x2e, 0x3b, 0xb8, 0xa8, 0x91, 0x2f, 0x6c,
    0xd0, 0xa2, 0xc4, 0xb1, 0xa1, 0xf1, 0xd9, 0xb8, 0x31, 0xc3, 0x19,
    0x14, 0xf9, 0x31, 0x1a, 0x59, 0xb4, 0x14, 0x34, 0xf6, 0x46, 0x43,
    0x81, 0x9b, 0x39, 0x3f, 0x80, 0x1f, 0xc0, 0x0e, 0x15, 0xf8, 0x0d,
    0x1c, 0xd8, 0x81, 0xa1, 0xe1, 0xa0, 0xa3, 0xe2, 0x9c, 0x09, 0x83,
    0x96, 0x63, 0xa3, 0x8d, 0x8c, 0xd8, 0xe8, 0x98, 0xa0, 0x19, 0xf9,
    0xab, 0x9e, 0xe3, 0xe4, 0xb3, 0xfc, 0x08, 0xaa, 0xdd, 0xd9, 0x9e,
    0xf9, 0xb6, 0xba, 0x7e, 0xbe, 0xae, 0xaa, 0x21, 0x7a, 0x2f, 0xf7,
    0xc5, 0xa6, 0xe4, 0xc8, 0x07, 0x1b, 0xfa, 0xd3, 0x09, 0xd1, 0x90,
    0x52, 0xae, 0x29, 0x38, 0x17, 0x1d, 0x00, 0x03, 0x80, 0x96, 0x22,
    0xbe, 0x04, 0x3b, 0x28, 0x60, 0xf5, 0x9b, 0xd8, 0x8e, 0x14, 0xec,
    0x9b, 0x46, 0x8d, 0x9c, 0x02, 0x8b, 0x71, 0x3b, 0xba, 0xd5, 0xe8,
    0x96, 0xd2, 0x1c, 0x85, 0xd4, 0x86, 0x23, 0x57, 0xfa, 0x16, 0x57,
    0x47, 0x52, 0x2f, 0x00, 0x7e, 0xb8, 0x17, 0x41, 0xd7, 0xcf, 0x8e,
    0x79, 0xd3, 0x7c, 0x63, 0x56, 0xe5, 0xc0, 0x54, 0xb7, 0x22, 0x43,
    0x09, 0x22, 0xe2, 0xd4, 0x86, 0xd0, 0x95, 0x33, 0x26, 0xcd, 0xc5,
    0x28, 0x10, 0x9a, 0x35, 0x74, 0xed, 0xbc, 0x17, 0x33, 0x5b, 0xce,
    0x44, 0x85, 0x8c, 0xa5, 0x1b, 0x17, 0xcc, 0x62, 0xc4, 0x89, 0x51,
    0xa3, 0xd6, 0xd1, 0xc1, 0x89, 0xa8, 0xbe, 0x58, 0xd8, 0x68, 0x88,
    0x2d, 0xb7, 0xc6, 0x8f, 0x98, 0x1d, 0x6f, 0x9d, 0x83, 0x17, 0xf7,
    0x4e, 0x18, 0x88, 0xa3, 0x0d, 0xf3, 0xb2, 0x0c, 0xc2, 0x8f, 0x0e,
    0xc7, 0x5a, 0x3d, 0x55, 0x8e, 0x31, 0x3d, 0x79, 0x12, 0xc5, 0x99,
    0x18, 0x5e, 0x1f, 0x12, 0x5d, 0x4b, 0x2c, 0xf1, 0xf9, 0xf3, 0x02,
    0x20, 0x85, 0xe5, 0xa6, 0x02, 0xc8, 0x25, 0x32, 0x97, 0xe6, 0x0c,
    0x87, 0xe5, 0x1a, 0x40, 0xeb, 0x1a, 0x57, 0x70, 0x9c, 0x9e, 0xbc,
    0xbc, 0xaa, 0x54, 0x44, 0x6d, 0x04, 0xd8, 0xc0, 0x4d, 0xad, 0x89,
    0x58, 0xd4, 0xcb, 0xf1, 0x5c, 0x98, 0x0f, 0x8b, 0x7a, 0x79, 0x47,
    0x1a, 0xe3, 0x23, 0x52, 0xab, 0x2c, 0x5c, 0xf8, 0x94, 0x28, 0x4a,
    0x94, 0x5f, 0x3b, 0x50, 0x43, 0xb0, 0xbe, 0x03, 0x7b, 0xe4, 0x57,
    0x93, 0xfc, 0x18, 0x9c, 0x4d, 0x08, 0x5d, 0xf2, 0x16, 0xc0, 0x4b,
    0xcf, 0x36, 0x58, 0x5b, 0x91, 0xd1, 0x2e, 0x6d, 0x00, 0x2c, 0xde,
    0x7d, 0x1a, 0x40, 0x0f, 0x00, 0xd6, 0x10, 0xf8, 0x75, 0x72, 0x31,
    0x9c, 0x28, 0x70, 0xdc, 0x1c, 0x98, 0x9b, 0xab, 0x95, 0x17, 0xfe,
    0x90, 0xdf, 0x8d, 0xe2, 0x0d, 0x4e, 0x6b, 0x4f, 0x1b, 0xf2, 0xb4,
    0x77, 0x66, 0xbf, 0x55, 0xc8, 0x81, 0x28, 0x43, 0x49, 0xe9, 0x5a,
    0xf9, 0xb3, 0x1c, 0x18, 0x89, 0x8f, 0x69, 0x6b, 0xa2, 0x93, 0x19,
    0x56, 0x0c, 0x22, 0x91, 0xa3, 0x8c, 0x20, 0xc8, 0x2d, 0x22, 0x88,
    0x2c, 0x16, 0x69, 0x41, 0x22, 0x08, 0x5a, 0x01, 0xe4, 0x22, 0x25,
    0x64, 0xe8, 0x27, 0xf7, 0x52, 0x81, 0x71, 0xc7, 0x23, 0x1f, 0xcd,
    0x81, 0xe5, 0xc0, 0x55, 0xee, 0x22, 0x99, 0x89, 0xfe, 0x15, 0xd5,
    0xe9, 0xba, 0x6c, 0xec, 0x7e, 0x5c, 0xef, 0x6e, 0xcf, 0xcf, 0x59,
    0x14, 0x1f, 0xed, 0xc6, 0x5d, 0xce, 0x9d, 0xa0, 0x06, 0x80, 0x94,
    0xa0, 0xb1, 0x28, 0x1f, 0x49, 0x7e, 0xd2, 0x1c, 0x3a, 0x63, 0xdc,
    0xf9, 0x40, 0x05, 0x75, 0x8d, 0x9c, 0x83, 0xf9, 0x4c, 0x78, 0x49,
    0xf9, 0xe9, 0x0a, 0x04, 0x73, 0x9c, 0xed, 0xe8, 0x52, 0x35, 0x0d,
    0x5e, 0x94, 0xa0, 0x47, 0x6f, 0x23, 0x79, 0x2f, 0xff, 0x3b, 0x3f,
    0x50, 0x8b, 0x62, 0xcf, 0x65, 0x18, 0xf0, 0x30, 0x4c, 0x0a, 0x24,
    0x00, 0xdf, 0xd0, 0xe3, 0x74, 0x46, 0xf4, 0x58, 0x55, 0xa6, 0xa7,
    0x95, 0xa1, 0xf5, 0x73, 0x8a, 0x44, 0xbf, 0xeb, 0x96, 0x72, 0xcc,
    0x0d, 0x25, 0xf4, 0xe2, 0x5c, 0x1a, 0xfd, 0xd2, 0x81, 0x90, 0xb4,
    0x84, 0x5f, 0x50, 0x34, 0xab, 0xc6, 0x74, 0xc8, 0x4d, 0xb7, 0xd0,
    0x99, 0x59, 0x35, 0x6c, 0xab, 0xce, 0xca, 0x6a, 0xaf, 0xff, 0x58,
    0x3d, 0x7e, 0x26, 0xf5, 0xaa, 0x3f, 0xde, 0x7a, 0x6f, 0xe9, 0x02,
    0x45, 0xe2, 0xe9, 0xf2, 0xb2, 0x38, 0x4a, 0xd6, 0x18, 0x47, 0xaf,
    0xa0, 0x95, 0x50, 0x13, 0x1f, 0x63, 0x22, 0x4d, 0xd6, 0x52, 0x04,
    0x70, 0x9a, 0x50, 0x7d, 0x15, 0x43, 0xcf, 0x3a, 0x43, 0xf1, 0x7b,
    0x4b, 0xde, 0x9f, 0xfd, 0x71, 0xfd, 0x09, 0x0a, 0xda, 0x0e, 0x9e,
    0xe2, 0x43, 0x6d, 0x8c, 0xf8, 0xc5, 0xd9, 0xab, 0xb2, 0x26, 0x98,
    0x1f, 0xaa, 0x2f, 0xf9, 0x2e, 0x7e, 0x8e, 0x3a, 0x9d, 0xb4, 0x48,
    0x1e, 0xea, 0x54, 0xa4, 0x3f, 0xe5, 0xd9, 0x6d, 0x6d, 0x96, 0xb5,
    0x4a, 0xcb, 0x47, 0x1f, 0x10, 0xed, 0x1f, 0xf8, 0xb1, 0xba, 0x73,
    0xde, 0x6f, 0x5c, 0xc2, 0x25, 0x3a, 0x0a, 0x77, 0x1e, 0x31, 0x10,
    0x7d, 0x4d, 0xfd, 0xf2, 0xe8, 0x5d, 0x30, 0x54, 0x1a, 0x04, 0xd3,
    0x8e, 0x09, 0xf3, 0x36, 0xa3, 0x01, 0x25, 0x40, 0x9c, 0x63, 0x47,
    0x15, 0x82, 0x36, 0x61, 0x15, 0xa7, 0xd3, 0x6f, 0x81, 0xd4, 0x26,
    0x76, 0xd5, 0x70, 0xab, 0x46, 0x95, 0x2b, 0xb3, 0xda, 0xe0, 0xbb,
    0x1e, 0x79, 0xdb, 0x2c, 0xad, 0x6d, 0x1f, 0x0c, 0xe3, 0x25, 0x7d,
    0x89, 0x45, 0x1e, 0x1c, 0x91, 0x91, 0x66, 0xb5, 0xc3, 0xb4, 0xef,
    0xb7, 0x18, 0xb0, 0x37, 0x62, 0x41, 0xc8, 0xe0, 0xaf, 0x62, 0x44,
    0x3f, 0x66, 0xa1, 0x9b, 0xbf, 0x87, 0x61, 0xc8, 0xf1, 0xf4, 0x3a,
    0x46, 0x74, 0x4a, 0x9c, 0x29, 0x36, 0xdc, 0xd5, 0xe8, 0x6f, 0x14,
    0x18, 0x30, 0xcf, 0xe3, 0x0e, 0x1a, 0x15, 0x2f, 0x08, 0x05, 0x6c,
    0x9e, 0xa9, 0xa9, 0x8d, 0x6c, 0xe9, 0x37, 0xe0, 0x98, 0x3a, 0xe5,
    0x3f, 0xb5, 0x71, 0x2c, 0xb4, 0xdf, 0x4c, 0xa3, 0xa0, 0xb5, 0xb0,
    0xca, 0xde, 0x67, 0xe5, 0xe3, 0x72, 0xb7, 0x36, 0xee, 0x25, 0x2e,
    0x4c, 0x29, 0xe6, 0x01, 0x19, 0x5c, 0xb0, 0xae, 0xdb, 0xad, 0xdf,
    0x92, 0x31, 0x27, 0x06, 0x52, 0x04, 0x53, 0xa2, 0x37, 0x6a, 0xce,
    0xe7, 0x39, 0xe7, 0x0b, 0x30, 0x36, 0xb5, 0x0e, 0xe8, 0x3f, 0x10,
    0x2e, 0x25, 0x94, 0x0e, 0x4c, 0xd3, 0x57, 0xd3, 0x34, 0x5d, 0x98,
    0xd5, 0xd6, 0x70, 0x47, 0xc3, 0xdb, 0xc1, 0xfa, 0x97, 0xb2, 0x33,
    0xea, 0x49, 0xde, 0x92, 0xb4, 0x9d, 0xf7, 0x1b, 0xd9, 0x89, 0x3d,
    0xc1, 0x22, 0x1b, 0x92, 0xae, 0x71, 0x5b, 0x4c, 0xeb, 0xdb, 0xc0,
    0xcc, 0xfd, 0x04, 0xa6, 0x6f, 0xd5, 0x17, 0x26, 0x1a, 0x06, 0x6c,
    0x17, 0x9a, 0xfa, 0x02, 0x3e, 0xd6, 0x67, 0xb1, 0x63, 0x57, 0xc8,
    0x19, 0x73, 0xbd, 0x6f, 0xb1, 0x03, 0xa2, 0x99, 0x4c, 0x30, 0xf7,
    0x5f, 0x2e, 0x33, 0xcd, 0xff, 0x00, 0xf0, 0x33, 0x22, 0x99, 0xe0,
    0x7e, 0x00, 0x87, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44,
    0xae, 0x42, 0x60, 0x82
};

static void *font_png_data = font_png;
static int   font_png_size = sizeof(font_png);
