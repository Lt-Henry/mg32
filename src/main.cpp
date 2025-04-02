
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
#include <sstream>

using namespace std;

map<int,mg32::Bank*> banks;

const uint8_t* keyboard;
int keyboard_numkeys;

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

int key(lua_State* L)
{
    int key = lua_tonumber(L, 1);

    if (key < keyboard_numkeys) {
        lua_pushboolean(L,keyboard[key]);
        return 1;
    }

    return 0;
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
    SDL_PumpEvents();
    keyboard = SDL_GetKeyboardState(&keyboard_numkeys);

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
        //TODO
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

    lua_pushcfunction(L, key);
    lua_setglobal(L, "key");

    lua_pushcfunction(L, sleep);
    lua_setglobal(L, "sleep");

    lua_pushcfunction(L, mg32_start_frame);
    lua_setglobal(L, "mg32_start_frame");

    lua_pushcfunction(L, mg32_end_frame);
    lua_setglobal(L, "mg32_end_frame");

    lua_pushcfunction(L, mg32_exit);
    lua_setglobal(L, "mg32_exit");

    lua_pushcfunction(L, mg32_draw_texture);
    lua_setglobal(L, "mg32_draw_texture");

    SDL_Init(SDL_INIT_EVERYTHING);

    window = SDL_CreateWindow("MG32",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              640,480, 0/*SDL_WINDOW_FULLSCREEN_DESKTOP*/);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    lua_getglobal(L, "main");

    if (lua_pcall(L, 0, 0, 0) != 0) {
        cerr<<"Error running main:"<<lua_tostring(L, -1)<<endl;
        return -1;
    }

    return 0;
}
