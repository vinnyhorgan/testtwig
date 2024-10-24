#ifndef RENDERER_H
#define RENDERER_H

#include <stdint.h>

typedef struct Image Image;

typedef struct { uint8_t b, g, r, a; } Color;
typedef struct { int x, y, width, height; } Rect;

void ren_init();
void ren_update();
void ren_set_clip_rect(Rect rect);

Image* ren_new_image(int width, int height);
void ren_free_image(Image *image);

void ren_draw_rect(Rect rect, Color color);
void ren_draw_image(Image *image, Rect *sub, int x, int y, Color color);

#endif
