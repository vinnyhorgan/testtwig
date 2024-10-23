#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "minilua.h"

#include "api/api.h"

#define WIDTH 320
#define HEIGHT 180

int main(int argc, char **argv) {
    SDL_Window *window;
    SDL_Renderer *renderer;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer("Twig v0.1.0", WIDTH * 3, HEIGHT * 3, SDL_WINDOW_RESIZABLE, &window, &renderer);

    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

    SDL_SetRenderLogicalPresentation(renderer, WIDTH, HEIGHT, SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);

    uint32_t pixels[WIDTH * HEIGHT];

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
        "  PATHSEP = package.config:sub(1, 1)\n"
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

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        for (int i = 0; i < WIDTH * HEIGHT; i++) {
            pixels[i] = 0xFFFFFFFF;

            if (i == 500) {
                pixels[i] = 0x000000FF;
            }
        }

        SDL_UpdateTexture(texture, NULL, pixels, WIDTH * sizeof(uint32_t));

        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
