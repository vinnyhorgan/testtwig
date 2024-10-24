#include "api.h"
#include "../renderer.h"

static int f_gc(lua_State *L) {
    Image **self = luaL_checkudata(L, 1, API_TYPE_IMAGE);
    if (*self) { ren_destroy_image(*self); }
    return 0;
}

static int f_load(lua_State *L) {
    Image **self = lua_newuserdata(L, sizeof(*self));
    luaL_setmetatable(L, API_TYPE_IMAGE);

    if (lua_isstring(L, 1)) {
        *self = ren_load_image_file(luaL_checkstring(L, 1));
    } else {
        int width = luaL_checkinteger(L, 1);
        int height = luaL_checkinteger(L, 2);
        *self = ren_create_image(width, height);
    }

    if (!*self) { luaL_error(L, "failed to load image"); }
    return 1;
}

static int f_get_width(lua_State *L) {
    Image **self = luaL_checkudata(L, 1, API_TYPE_IMAGE);
    lua_pushinteger(L, (*self)->width);
    return 1;
}

static int f_get_height(lua_State *L) {
    Image **self = luaL_checkudata(L, 1, API_TYPE_IMAGE);
    lua_pushinteger(L, (*self)->height);
    return 1;
}

static int f_draw(lua_State *L) {
    Image **self = luaL_checkudata(L, 1, API_TYPE_IMAGE);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    ren_draw_image(*self, x, y);
    return 0;
}

static const luaL_Reg lib[] = {
    { "__gc", f_gc},
    { "load", f_load },
    { "get_width", f_get_width },
    { "get_height", f_get_height },
    { "draw", f_draw },
    { NULL, NULL }
};

int luaopen_renderer_image(lua_State *L) {
    luaL_newmetatable(L, API_TYPE_IMAGE);
    luaL_setfuncs(L, lib, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}
