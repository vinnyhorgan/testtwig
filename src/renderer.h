#ifndef RENDERER_H
#define RENDERER_H

#include <stdint.h>

typedef union { struct { uint8_t b, g, r, a; }; uint32_t w; } Color;
typedef struct { int x, y, width, height; } Rect;
typedef struct { Color *pixels; int width, height; } Image;
typedef struct { Rect rect; int xadv; } Glyph;
typedef struct { Image *image; Glyph glyphs[256]; } Font;

#define ren_rect(X, Y, W, H) ((Rect) { (X), (Y), (W), (H) })
#define ren_rgba(R, G, B, A) ((Color) { .r = (R), .g = (G), .b = (B), .a = (A) })
#define ren_rgb(R, G, B) ren_rgba(R, G, B, 0xff)

void ren_init();
void ren_update();

Image* ren_create_image(int width, int height);
Image* ren_load_image_file(const char *filename);
Image* ren_load_image_mem(void *data, int len);
void ren_destroy_image(Image *image);

Font* ren_load_font_file(const char *filename);
Font* ren_load_font_mem(void *data, int len);
void ren_destroy_font(Font *font);
int ren_text_width(Font *font, char *text);

void ren_clear(Color color);
void ren_set_clip(Rect rect);
void ren_draw_point(int x, int y, Color color);
void ren_draw_rect(Rect rect, Color color);
void ren_draw_image(Image *img, int x, int y);
void ren_draw_image2(Image *img, int x, int y, Rect src, Color color);
void ren_draw_image3(Image *img, Rect dst, Rect src, Color mul_color, Color add_color);
int  ren_draw_text(char *text, int x, int y, Color color);
int  ren_draw_text2(Font *font, char *text, int x, int y, Color color);

#endif
