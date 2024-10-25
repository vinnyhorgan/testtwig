#include <SDL3/SDL.h>
#include <ctype.h>

#include "api.h"

extern SDL_Window *window;
extern SDL_Renderer *renderer;

static char* key_name(char *dst, int sym) {
    strcpy(dst, SDL_GetKeyName(sym));
    char *p = dst;
    while (*p) {
        *p = tolower(*p);
        p++;
    }
    return dst;
}

static const char* button_name(int button) {
    switch (button) {
        case 1  : return "left";
        case 2  : return "middle";
        case 3  : return "right";
        default : return "?";
    }
}

static bool fullscreen = false;

static int f_poll_event(lua_State *L) {
    char buf[16];
    SDL_Event e;

    if (!SDL_PollEvent(&e)) {
        return 0;
    }

    switch (e.type) {
        case SDL_EVENT_QUIT:
            lua_pushstring(L, "quit");
            return 1;

        case SDL_EVENT_KEY_DOWN:
            if (e.key.key == SDLK_RETURN && (e.key.mod & SDL_KMOD_ALT)) {
                fullscreen = !fullscreen;
                if (fullscreen) {
                    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
                } else {
                    SDL_SetWindowFullscreen(window, 0);
                }
                return 0;
            }

            lua_pushstring(L, "keypressed");
            lua_pushstring(L, key_name(buf, e.key.key));
            return 2;

        case SDL_EVENT_KEY_UP:
            lua_pushstring(L, "keyreleased");
            lua_pushstring(L, key_name(buf, e.key.key));
            return 2;

        case SDL_EVENT_TEXT_INPUT:
            lua_pushstring(L, "textinput");
            lua_pushstring(L, e.text.text);
            return 2;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if (e.button.button == 1) { SDL_CaptureMouse(1); }
            lua_pushstring(L, "mousepressed");
            lua_pushstring(L, button_name(e.button.button));
            lua_pushnumber(L, e.button.x);
            lua_pushnumber(L, e.button.y);
            lua_pushnumber(L, e.button.clicks);
            return 5;

        case SDL_EVENT_MOUSE_BUTTON_UP:
            if (e.button.button == 1) { SDL_CaptureMouse(0); }
            lua_pushstring(L, "mousereleased");
            lua_pushstring(L, button_name(e.button.button));
            lua_pushnumber(L, e.button.x);
            lua_pushnumber(L, e.button.y);
            return 4;

        case SDL_EVENT_MOUSE_MOTION:
            lua_pushstring(L, "mousemoved");
            lua_pushnumber(L, e.motion.x);
            lua_pushnumber(L, e.motion.y);
            lua_pushnumber(L, e.motion.xrel);
            lua_pushnumber(L, e.motion.yrel);
            return 5;

        case SDL_EVENT_MOUSE_WHEEL:
            lua_pushstring(L, "mousewheel");
            lua_pushnumber(L, e.wheel.y);
            return 2;
    }

    return 0;
}

static int f_get_time(lua_State *L) {
    double n = SDL_GetPerformanceCounter() / (double) SDL_GetPerformanceFrequency();
    lua_pushnumber(L, n);
    return 1;
}

static int f_sleep(lua_State *L) {
    double n = luaL_checknumber(L, 1);
    SDL_Delay(n * 1000);
    return 0;
}

static const luaL_Reg lib[] = {
    { "poll_event", f_poll_event },
    { "get_time", f_get_time },
    { "sleep", f_sleep },
    { NULL, NULL }
};

int luaopen_system(lua_State *L) {
    luaL_newlib(L, lib);
    return 1;
}
