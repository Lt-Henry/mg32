
#include "mg32.hpp"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <SDL2/SDL.h>

#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <map>
#include <vector>
#include <sstream>

using namespace std;

map<int,mg32::Bank*> banks;

const uint8_t* keyboard;
vector<uint8_t> keyboard_last;

int mouse_x;
int mouse_y;
uint32_t mouse_buttons;
uint32_t mouse_buttons_last;

SDL_Window* window;
SDL_Renderer* renderer;

int load_bank(lua_State* L)
{
    int id = lua_tonumber(L, 1);
    int tw = lua_tonumber(L, 2);
    int th = lua_tonumber(L, 3);

    stringstream ss;

    ss<<id<<".png";

    banks[id] = new mg32::Bank(renderer, ss.str(), tw, th);

    return 0;
}

int get_bank_info(lua_State* L)
{
    int id = lua_tonumber(L, 1);

    mg32::Bank* bank = banks[id];

    lua_pushinteger(L,bank->width);
    lua_pushinteger(L,bank->height);
    lua_pushinteger(L,bank->tile_width);
    lua_pushinteger(L,bank->tile_height);

    return 4;
}

int key(lua_State* L)
{
    int key = lua_tonumber(L, 1);

    if (key < SDL_NUM_SCANCODES) {
        lua_pushboolean(L,keyboard[key]);
        return 1;
    }

    return 0;
}

int keydown(lua_State* L)
{
    int key = lua_tonumber(L, 1);

    if (key < SDL_NUM_SCANCODES) {
        lua_pushboolean(L,keyboard[key] > keyboard_last[key]);
        return 1;
    }

    return 0;
}

int button(lua_State* L)
{
    int btn = lua_tonumber(L, 1);

    lua_pushboolean(L,SDL_BUTTON(btn) & mouse_buttons);

    return 1;
}

int buttondown(lua_State* L)
{
    int btn = lua_tonumber(L, 1);

    uint32_t last = SDL_BUTTON(btn) & mouse_buttons_last;
    uint32_t now = SDL_BUTTON(btn) & mouse_buttons;
    lua_pushboolean(L,now > last);

    return 1;
}

int sleep(lua_State* L)
{
    int ms = lua_tonumber(L, 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    return 0;
}

int mg32_start_frame(lua_State* L)
{
    SDL_RenderClear(renderer);

    for (size_t n = 0;n < SDL_NUM_SCANCODES; n++) {
        keyboard_last[n] = keyboard[n];
    }

    mouse_buttons_last = mouse_buttons;

    SDL_PumpEvents();
    keyboard = SDL_GetKeyboardState(nullptr);

    mouse_buttons = SDL_GetMouseState(&mouse_x, &mouse_y);

    if (SDL_HasEvent(SDL_QUIT)) {
        exit(0);
    }

    return 0;
}

int mg32_end_frame(lua_State* L)
{
    SDL_RenderPresent(renderer);

    return 0;
}

int mg32_exit(lua_State* L)
{
    exit(0);

    return 0;
}

int mg32_get_screen_size(lua_State* L)
{
    int w,h;

    SDL_GetRendererOutputSize(renderer,&w,&h);
    lua_pushinteger(L,w);
    lua_pushinteger(L,h);

    return 2;
}

int mg32_draw_texture(lua_State* L)
{
    int bank_id = lua_tonumber(L, 1);
    int texture_id = lua_tonumber(L, 2);
    int x = lua_tonumber(L, 3);
    int y = lua_tonumber(L, 4);

    mg32::Bank* bank = banks[bank_id];

    if (bank) {

        int tw = bank->tile_width;
        int th = bank->tile_height;

        int row = texture_id / tw;
        int col = texture_id % th;

        SDL_Rect srect;

        srect.x = col * tw;
        srect.y = row * th;
        srect.w = tw;
        srect.h = th;

        SDL_Rect drect;

        drect.x = x;
        drect.y = y;
        drect.w = tw;
        drect.h = th;

        SDL_RenderCopy(renderer,bank->data,&srect,&drect);
    }
    return 0;
}

int main(int argc, char* argv[])
{
    if (argc<2) {
        return 0;
    }

    int status;

    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    status = luaL_loadfile(L, "core.lua");
    if(status != LUA_OK) {
        cerr<<"Failed to load core"<<endl;
        return -1;
    }

    if (lua_pcall(L, 0, 0, 0) != 0) {
        cerr<<"Error compiling core:"<<lua_tostring(L, -1)<<endl;
        return -1;
    }

    status = luaL_loadfile(L, argv[1]);
    if(status != LUA_OK) {
        cerr<<"Failed to load file "<<argv[1]<<endl;
        return -1;
    }

    if (lua_pcall(L, 0, 0, 0) != 0) {
        cerr<<"Error compiling main:"<<lua_tostring(L, -1)<<endl;
        return -1;
    }

    lua_pushcfunction(L, load_bank);
    lua_setglobal(L, "load_bank");

    lua_pushcfunction(L, get_bank_info);
    lua_setglobal(L, "get_bank_info");

    lua_pushcfunction(L, key);
    lua_setglobal(L, "key");

    lua_pushcfunction(L, keydown);
    lua_setglobal(L, "keydown");

    lua_pushcfunction(L, button);
    lua_setglobal(L, "button");

    lua_pushcfunction(L, buttondown);
    lua_setglobal(L, "buttondown");

    lua_pushcfunction(L, sleep);
    lua_setglobal(L, "sleep");

    lua_pushcfunction(L, mg32_start_frame);
    lua_setglobal(L, "mg32_start_frame");

    lua_pushcfunction(L, mg32_end_frame);
    lua_setglobal(L, "mg32_end_frame");

    lua_pushcfunction(L, mg32_exit);
    lua_setglobal(L, "mg32_exit");

    lua_pushcfunction(L, mg32_get_screen_size);
    lua_setglobal(L, "mg32_get_screen_size");

    lua_pushcfunction(L, mg32_draw_texture);
    lua_setglobal(L, "mg32_draw_texture");

    SDL_Init(SDL_INIT_EVERYTHING);

    window = SDL_CreateWindow("MG32",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              640,480, 0/*SDL_WINDOW_FULLSCREEN_DESKTOP*/);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_PumpEvents();
    keyboard = SDL_GetKeyboardState(nullptr);
    keyboard_last.reserve(SDL_NUM_SCANCODES);

    lua_getglobal(L, "main");

    if (lua_pcall(L, 0, 0, 0) != 0) {
        cerr<<"Error running main:"<<lua_tostring(L, -1)<<endl;
        return -1;
    }

    return 0;
}
