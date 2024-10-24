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

static int f_set_clip_rect(lua_State *L) {
    Rect rect;
    rect.x = luaL_checkinteger(L, 1);
    rect.y = luaL_checkinteger(L, 2);
    rect.width = luaL_checkinteger(L, 3);
    rect.height = luaL_checkinteger(L, 4);
    ren_set_clip_rect(rect);
    return 0;
}

static int f_update(lua_State *L) {
    ren_update();
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

static const luaL_Reg lib[] = {
    { "update", f_update },
    { "set_clip_rect", f_set_clip_rect },
    { "draw_rect", f_draw_rect },
    { NULL, NULL }
};

int luaopen_renderer(lua_State *L) {
    luaL_newlib(L, lib);
    return 1;
}
