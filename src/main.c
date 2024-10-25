#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <stdlib.h>

#include "lib/minilua/minilua.h"

#include "api/api.h"
#include "renderer.h"

#define WIDTH 384
#define HEIGHT 216

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *screen_texture;
Image *screen_image;

int main(int argc, char **argv) {
    screen_image = ren_create_image(WIDTH, HEIGHT);

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer("Twig v0.1.0", WIDTH * 3, HEIGHT * 3, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN, &window, &renderer);
    SDL_SetWindowMinimumSize(window, WIDTH, HEIGHT);
    ren_init();

    SDL_HideCursor();

    SDL_StartTextInput(window); // should be enabled only when text input is needed

    SDL_SetRenderLogicalPresentation(renderer, WIDTH, HEIGHT, SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);

    screen_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
    SDL_SetTextureScaleMode(screen_texture, SDL_SCALEMODE_NEAREST);

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    api_load_libs(L);

    lua_newtable(L);
    for (int i = 0; i < argc; i++) {
        lua_pushstring(L, argv[i]);
        lua_rawseti(L, -2, i + 1);
    }
    lua_setglobal(L, "ARGS");

    lua_pushstring(L, "0.1.0");
    lua_setglobal(L, "VERSION");

    lua_pushstring(L, SDL_GetPlatform());
    lua_setglobal(L, "PLATFORM");

    luaL_dostring(L,
        "xpcall(function()\n"
        "  package.path = './data/?.lua;' .. package.path\n"
        "  package.path = './data/?/init.lua;' .. package.path\n"
        "  local core = require('core')\n"
        "  core.init()\n"
        "  core.run()\n"
        "end, function(err)\n"
        "  print('Error: ' .. tostring(err))\n"
        "  print(debug.traceback(nil, 2))\n"
        "  os.exit(1)\n"
        "end)");

    lua_close(L);

    ren_destroy_image(screen_image);

    SDL_StopTextInput(window);

    SDL_DestroyTexture(screen_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
