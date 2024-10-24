#include <SDL3/SDL.h>

#include "api.h"

#include "../renderer.h"

static Color checkcolor(lua_State *L, int idx, int def) {
    Color color;
    if (lua_isnoneornil(L, idx)) {
        return (Color) { def, def, def, 255 };
    }
    lua_rawgeti(L, idx, 1);
    lua_rawgeti(L, idx, 2);
    lua_rawgeti(L, idx, 3);
    lua_rawgeti(L, idx, 4);
    color.r = luaL_checknumber(L, -4);
    color.g = luaL_checknumber(L, -3);
    color.b = luaL_checknumber(L, -2);
    color.a = luaL_optnumber(L, -1, 255);
    lua_pop(L, 4);
    return color;
}

static int f_update(lua_State *L) {
    ren_update();
    return 0;
}

static int f_clear(lua_State *L) {
    Color color = checkcolor(L, 1, 0);
    ren_clear(color);
    return 0;
}

static int f_set_clip(lua_State *L) {
    Rect rect;
    rect.x = luaL_checkinteger(L, 1);
    rect.y = luaL_checkinteger(L, 2);
    rect.width = luaL_checkinteger(L, 3);
    rect.height = luaL_checkinteger(L, 4);
    ren_set_clip(rect);
    return 0;
}

static int f_draw_point(lua_State *L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    Color color = checkcolor(L, 3, 255);
    ren_draw_point(x, y, color);
    return 0;
}

static int f_draw_rect(lua_State *L) {
    Rect rect;
    rect.x = luaL_checkinteger(L, 1);
    rect.y = luaL_checkinteger(L, 2);
    rect.width = luaL_checkinteger(L, 3);
    rect.height = luaL_checkinteger(L, 4);
    Color color = checkcolor(L, 5, 255);
    ren_draw_rect(rect, color);
    return 0;
}

static int f_draw_text(lua_State *L) {
    const char *text = luaL_checkstring(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    Color color = checkcolor(L, 4, 255);
    ren_draw_text((char*) text, x, y, color);
    return 0;
}

static const luaL_Reg lib[] = {
    { "update", f_update },
    { "clear", f_clear },
    { "set_clip", f_set_clip },
    { "draw_point", f_draw_point },
    { "draw_rect", f_draw_rect },
    { "draw_text", f_draw_text },
    { NULL, NULL }
};

int luaopen_renderer_image(lua_State *L);

int luaopen_renderer(lua_State *L) {
    luaL_newlib(L, lib);
    luaopen_renderer_image(L);
    lua_setfield(L, -2, "image");
    return 1;
}
