#include <SDL3/SDL.h>

#include "api.h"

extern SDL_Renderer *renderer;
extern SDL_Texture *screen_texture;
extern uint32_t *screen_pixels;

static int f_update(lua_State *L) {
    SDL_UpdateTexture(screen_texture, NULL, screen_pixels, 320 * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, screen_texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    return 0;
}

static const luaL_Reg lib[] = {
    { "update", f_update },
    { NULL, NULL }
};

int luaopen_renderer(lua_State *L) {
    luaL_newlib(L, lib);
    return 1;
}
