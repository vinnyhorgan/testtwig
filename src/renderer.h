#ifndef RENDERER_H
#define RENDERER_H

#include <stdint.h>

typedef union { struct { uint8_t b, g, r, a; }; uint32_t w; } Color;
typedef struct { int x, y, w, h; } kit_Rect;
typedef struct { int x, y, width, height; } Rect;
typedef struct { Color *pixels; int width, height; } Image;
typedef struct { Rect rect; int xadv; } Glyph;
typedef struct { Image *image; Glyph glyphs[256]; } Font;

void ren_init();
void ren_update();

Image* ren_create_image(int width, int height);
void ren_destroy_image(Image *image);

void ren_clear(Color color);
void ren_set_clip(Rect rect);
void ren_draw_point(int x, int y, Color color);
void ren_draw_rect(Rect rect, Color color);

#endif
