
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

float mouse_x;
float mouse_y;
uint32_t mouse_buttons;
uint32_t mouse_buttons_last;

SDL_Window* window;
SDL_Renderer* renderer;

vector<mg32::DrawCommand> commands;

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

int get_mouse(lua_State* L)
{

    lua_pushnumber(L,mouse_x);
    lua_pushnumber(L,mouse_y);

    return 2;
}

int show_cursor(lua_State* L)
{
    SDL_ShowCursor(SDL_ENABLE);

    return 0;
}

int hide_cursor(lua_State* L)
{
    SDL_ShowCursor(SDL_DISABLE);

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

    for (size_t n = 0;n < SDL_NUM_SCANCODES; n++) {
        keyboard_last[n] = keyboard[n];
    }

    mouse_buttons_last = mouse_buttons;

    SDL_PumpEvents();
    keyboard = SDL_GetKeyboardState(nullptr);

    int raw_mx,raw_my;
    mouse_buttons = SDL_GetMouseState(&raw_mx, &raw_my);
    SDL_RenderWindowToLogical(renderer,raw_mx,raw_my,&mouse_x,&mouse_y);

    if (SDL_HasEvent(SDL_QUIT)) {
        exit(0);
    }

    //commands.clear();

    return 0;
}

static void draw(mg32::DrawCommand* q)
{
    if (q) {

        if (q->left) {
            draw(q->left);
        }

        switch (q->command) {
            case mg32::Command::Blit:
                SDL_RenderCopy(renderer,q->texture,&q->src,&q->dst);
            break;

            case mg32::Command::BlitEx:
                SDL_RenderCopyEx(renderer,q->texture,&q->src,&q->dst,q->angle,&q->pivot,SDL_FLIP_NONE);
            break;
        }

        if (q->right) {
            draw(q->right);
        }
    }
}

int mg32_end_frame(lua_State* L)
{

    if (commands.size() > 0) {
        mg32::DrawCommand* top = &commands.data()[0];
        draw(top);
    }

    SDL_RenderPresent(renderer);
    commands.clear();

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

    //SDL_GetRendererOutputSize(renderer,&w,&h);
    SDL_RenderGetLogicalSize(renderer,&w,&h);
    lua_pushinteger(L,w);
    lua_pushinteger(L,h);

    return 2;
}

int mg32_ticks(lua_State* L)
{

    lua_pushinteger(L, SDL_GetTicks());

    return 1;
}

static void insert_command(mg32::DrawCommand* q, mg32::DrawCommand* t)
{
    if (t->z<q->z) {
        if (q->left==nullptr) {
            q->left=t;
        }
        else {
            insert_command(q->left,t);
        }
    }
    else {
        if (q->right==nullptr) {
            q->right=t;
        }
        else {
            insert_command(q->right,t);
        }
    }
}

int mg32_draw_texture(lua_State* L)
{
    int bank_id = lua_tonumber(L, 1);
    int texture_id = lua_tonumber(L, 2);
    int x = lua_tonumber(L, 3);
    int y = lua_tonumber(L, 4);
    int z = lua_tonumber(L, 5);

    mg32::Bank* bank = banks[bank_id];

    if (bank) {

        int tw = bank->tile_width;
        int th = bank->tile_height;
        int numw = bank->width/tw;

        int row = texture_id / numw;
        int col = texture_id % numw;

        mg32::DrawCommand cmd;
        cmd.command = mg32::Command::Blit;
        cmd.left = nullptr;
        cmd.right = nullptr;

        cmd.z = z;

        cmd.texture = bank->data;
        cmd.src.x = col * tw;
        cmd.src.y = row * th;
        cmd.src.w = tw;
        cmd.src.h = th;

        cmd.dst.x = x;
        cmd.dst.y = y;
        cmd.dst.w = tw;
        cmd.dst.h = th;

        commands.push_back(cmd);

        size_t size = commands.size();

        if (size > 1) {
            mg32::DrawCommand* top = &commands.data()[0];
            mg32::DrawCommand* op = &commands.data()[size-1];
            insert_command(top, op);
        }

    }
    return 0;
}

int mg32_draw_texture_ex(lua_State* L)
{
    int bank_id = lua_tonumber(L, 1);
    int texture_id = lua_tonumber(L, 2);
    int x = lua_tonumber(L, 3);
    int y = lua_tonumber(L, 4);
    int z = lua_tonumber(L, 5);
    int flip = lua_tonumber(L, 6);
    double angle = lua_tonumber(L, 7);
    int px = lua_tonumber(L, 8);
    int py = lua_tonumber(L, 9);

    mg32::Bank* bank = banks[bank_id];

    if (bank) {

        int tw = bank->tile_width;
        int th = bank->tile_height;
        int numw = bank->width/tw;

        int row = texture_id / numw;
        int col = texture_id % numw;

        mg32::DrawCommand cmd;
        cmd.command = mg32::Command::BlitEx;
        cmd.left = nullptr;
        cmd.right = nullptr;

        cmd.z = z;
        cmd.angle = angle;
        cmd.pivot.x = px;
        cmd.pivot.y = py;

        cmd.texture = bank->data;
        cmd.src.x = col * tw;
        cmd.src.y = row * th;
        cmd.src.w = tw;
        cmd.src.h = th;

        cmd.dst.x = x;
        cmd.dst.y = y;
        cmd.dst.w = tw;
        cmd.dst.h = th;

        commands.push_back(cmd);

        size_t size = commands.size();

        if (size > 1) {
            mg32::DrawCommand* top = &commands.data()[0];
            mg32::DrawCommand* op = &commands.data()[size-1];
            insert_command(top, op);
        }

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

    lua_pushcfunction(L, get_mouse);
    lua_setglobal(L, "get_mouse");

    lua_pushcfunction(L, show_cursor);
    lua_setglobal(L, "show_cursor");

    lua_pushcfunction(L, hide_cursor);
    lua_setglobal(L, "hide_cursor");

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

    lua_pushcfunction(L, mg32_ticks);
    lua_setglobal(L, "mg32_ticks");

    lua_pushcfunction(L, mg32_draw_texture);
    lua_setglobal(L, "mg32_draw_texture");

    lua_pushcfunction(L, mg32_draw_texture_ex);
    lua_setglobal(L, "mg32_draw_texture_ex");

    SDL_Init(SDL_INIT_EVERYTHING);

    window = SDL_CreateWindow("MG32",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              640*2,360*2,0/*SDL_WINDOW_FULLSCREEN_DESKTOP*/);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_RenderSetLogicalSize(renderer, 640,360);

    SDL_PumpEvents();
    keyboard = SDL_GetKeyboardState(nullptr);
    keyboard_last.reserve(SDL_NUM_SCANCODES);

    commands.reserve(1024);

    lua_getglobal(L, "main");

    if (lua_pcall(L, 0, 0, 0) != 0) {
        cerr<<"Error running main:"<<lua_tostring(L, -1)<<endl;
        return -1;
    }

    return 0;
}
