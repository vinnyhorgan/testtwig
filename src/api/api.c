#include "api.h"

int luaopen_lpeg(lua_State *L);

static const luaL_Reg libs[] = {
    { "lpeg", luaopen_lpeg },
    { NULL, NULL }
};

void api_load_libs(lua_State *L) {
    for (int i = 0; libs[i].name; i++) {
        luaL_requiref(L, libs[i].name, libs[i].func, 1);
    }
}
