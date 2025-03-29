#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <SDL2/SDL.h>

#include <iostream>
#include <string>
#include <chrono>
#include <thread>

using namespace std;

const uint8_t* keyboard;

int load_bank(lua_State* L)
{
    return 0;
}

int key(lua_State* L)
{


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
    SDL_PumpEvents();
    keyboard = SDL_GetKeyboardState(nullptr);

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

    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window* window;
    SDL_Renderer* renderer;

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
