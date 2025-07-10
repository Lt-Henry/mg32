
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
#include <filesystem>

using namespace std;

map<int,mg32::Bank*> banks;
map<int,mg32::Sample*> samples;

vector<uint8_t> keyboard;
vector<uint8_t> keyboard_last;

float mouse_x;
float mouse_y;

#define MAX_MOUSE_BUTTONS 8
vector<uint8_t> mouse_buttons;
vector<uint8_t> mouse_buttons_last;

map<int, mg32::Gamepad*> gamepads;

SDL_Window* window;
SDL_Renderer* renderer;

vector<mg32::DrawCommand> commands;

SDL_AudioDeviceID pb_device_id;
SDL_AudioSpec pbspec;

#define MAX_STREAMS 16
mg32::Stream streams[MAX_STREAMS];

int load_bank(lua_State* L)
{
    int id = lua_tonumber(L, 1);
    const char* filename = lua_tostring(L, 2);
    int tw = lua_tonumber(L, 3);
    int th = lua_tonumber(L, 4);

    if (banks[id]!=nullptr) {
        delete banks[id];
        banks[id] = nullptr;
    }

    banks[id] = new mg32::Bank(renderer, filename, tw, th);

    lua_pushinteger(L, id);

    return 1;
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

int load_sample(lua_State* L)
{
    int id = lua_tonumber(L, 1);
    const char* filename = lua_tostring(L, 2);

    if (samples[id] != nullptr) {
        delete samples[id];
        samples[id] = nullptr;
    }

    samples[id] = new mg32::Sample(filename, pbspec);

    lua_pushinteger(L, id);

    return 1;
}

int play_sample(lua_State* L)
{
    int id = lua_tonumber(L, 1);

    mg32::Sample* sample = samples[id];

    if (sample) {
        for (size_t n=0;n<MAX_STREAMS;n++) {
            if (!streams[n].sample) {
                streams[n].flags = 0;
                streams[n].pos = 0;
                streams[n].sample = sample;
                break;
            }
        }
    }

    return 0;
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

    lua_pushboolean(L, mouse_buttons[btn]);

    return 1;
}

int buttondown(lua_State* L)
{
    int btn = lua_tonumber(L, 1);

    lua_pushboolean(L, mouse_buttons[btn] > mouse_buttons_last[btn]);

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

int gamepad_button(lua_State* L)
{
    int id = lua_tonumber(L, 1);
    int what = lua_tonumber(L, 2);
    
    mg32::Gamepad* gp = gamepads[id];
    
    int value = 0;
    
    if (gp) {
        value = gp->button(what);
    }
    
    lua_pushboolean(L,value);
    
    return 1;
}

int gamepad_buttondown(lua_State* L)
{
    int id = lua_tonumber(L, 1);
    int what = lua_tonumber(L, 2);
    
    mg32::Gamepad* gp = gamepads[id];
    
    int value = 0;
    
    if (gp) {
        value = gp->buttondown(what);
    }
    
    lua_pushboolean(L,value);
    
    return 1;
}

int gamepad_joystick(lua_State* L)
{
    int id = lua_tonumber(L, 1);
    int axis = lua_tonumber(L, 2);
    mg32::Gamepad* gp = gamepads[id];

    if (gp) {
        double x,y;

        gp->get_axis(axis,x,y);

        lua_pushnumber(L,x);
        lua_pushnumber(L,y);

        return 2;
    }

    lua_pushnumber(L,0);
    lua_pushnumber(L,0);
    return 2;
}


int gamepad_rumble(lua_State* L)
{
    int id = lua_tonumber(L, 1);

    mg32::Gamepad* gp = gamepads[id];

    if (gp) {
        gp->rumble();
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

    for (size_t n = 0;n < SDL_NUM_SCANCODES; n++) {
        keyboard_last[n] = keyboard[n];
    }

    for (size_t n = 0;n < MAX_MOUSE_BUTTONS; n++) {
        mouse_buttons_last[n] = mouse_buttons[n];
    }

    for (auto p:gamepads) {
        if (p.second) {
            p.second->frame();
        }
    }

    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                exit(0);
            break;
            
            case SDL_CONTROLLERDEVICEADDED:
                
                gamepads[event.cdevice.which] = new mg32::Gamepad(event.cdevice.which);
                clog<<"gamepad added:"<<event.cdevice.which<<endl;

            break;
            
            case SDL_CONTROLLERDEVICEREMOVED:
                clog<<"gamepad removed"<<endl;
                delete gamepads[event.cdevice.which];
                gamepads[event.cdevice.which] = nullptr;
            break;
            
            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
                //clog<<"button event"<<endl;
                gamepads[event.cbutton.which]->update(event);
            break;
            
            case SDL_KEYDOWN:
                keyboard[event.key.keysym.scancode] = 1;
            break;
            
            case SDL_KEYUP:
                keyboard[event.key.keysym.scancode] = 0;
            break;
            
            case SDL_MOUSEMOTION:
                mouse_x = event.motion.x;
                mouse_y = event.motion.y;
            break;
            
            case SDL_MOUSEBUTTONDOWN:
                mouse_buttons[event.button.button] = 1;
                mouse_x = event.button.x;
                mouse_y = event.button.y;
            break;
            
            case SDL_MOUSEBUTTONUP:
                mouse_buttons[event.button.button] = 0;
                mouse_x = event.button.x;
                mouse_y = event.button.y;
            break;
        }
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
                SDL_RenderCopyEx(renderer,q->texture,&q->src,&q->dst,q->angle,&q->pivot,static_cast<SDL_RendererFlip>(q->flip));
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

    SDL_RenderGetLogicalSize(renderer,&w,&h);
    lua_pushinteger(L,w);
    lua_pushinteger(L,h);

    return 2;
}

int mg32_set_screen_size(lua_State* L)
{
    int w = lua_tonumber(L,1);
    int h = lua_tonumber(L,2);
    
    //SDL_GetRendererOutputSize(renderer,&w,&h);
    SDL_RenderSetLogicalSize(renderer, w,h);
    
    return 0;
}

int mg32_set_screen_color(lua_State* L)
{
    int r = lua_tonumber(L,1);
    int g = lua_tonumber(L,2);
    int b = lua_tonumber(L,3);
    SDL_SetRenderDrawColor(renderer,r,g,b,255);

    return 0;
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
        cmd.flip = flip;
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

void audio_pb_cb(void* userdata, uint8_t* stream, int len )
{

    for (int n=0;n<len;n+=4) {

        int32_t left = 0;
        int32_t right = 0;

        for (int m=0;m<MAX_STREAMS;m++) {
            if (streams[m].sample) {
                if (streams[m].pos >= streams[m].sample->size) {
                    streams[m].sample = nullptr;
                }
                else {
                    int16_t* sptr = (int16_t *) (streams[m].sample->buffer + streams[m].pos);
                    left = left + sptr[0];
                    right = right + sptr[1];

                    streams[m].pos = streams[m].pos + 4; //s16 left and right channels
                }
            }
        }

        int16_t* ptr = (int16_t*)(stream + n);
        ptr[0] = std::min(std::max(left, -32768), 32767);
        ptr[1] = std::min(std::max(right, -32768), 32767);
    }
}

int main(int argc, char* argv[])
{
    if (argc<2) {
        return 0;
    }

    int status;

    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    string core_version="1";
    string core_path = "core.lua";
    
    if (!std::filesystem::exists(core_path)) {
        core_path = "/usr/local/lib/mg32/r" + core_version + "/core.lua";
        
        if (!std::filesystem::exists(core_path)) {
            core_path = "/usr/lib/mg32/r" + core_version + "/core.lua";
            
            if (!std::filesystem::exists(core_path)) {
                cerr<<"Could not find core.lua"<<endl;
                return -1;
            }
        }
    }
    
    status = luaL_loadfile(L, core_path.c_str());
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

    lua_pushcfunction(L, load_sample);
    lua_setglobal(L, "load_sample");

    lua_pushcfunction(L, play_sample);
    lua_setglobal(L, "play_sample");

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
    
    lua_pushcfunction(L, gamepad_button);
    lua_setglobal(L, "gamepad_button");
    
    lua_pushcfunction(L, gamepad_buttondown);
    lua_setglobal(L, "gamepad_buttondown");

    lua_pushcfunction(L, gamepad_joystick);
    lua_setglobal(L, "gamepad_joystick");

    lua_pushcfunction(L, gamepad_rumble);
    lua_setglobal(L, "gamepad_rumble");

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

    lua_pushcfunction(L, mg32_set_screen_color);
    lua_setglobal(L, "mg32_set_screen_color");

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
                              640*2,360*2,SDL_WINDOW_FULLSCREEN_DESKTOP);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_RenderSetLogicalSize(renderer, 640,360);
    
    SDL_StopTextInput();

    //SDL_PumpEvents();
    //keyboard = SDL_GetKeyboardState(nullptr);
    keyboard.reserve(SDL_NUM_SCANCODES);
    keyboard_last.reserve(SDL_NUM_SCANCODES);
    
    for (size_t n = 0;n < SDL_NUM_SCANCODES; n++) {
        keyboard_last[n] = 0;
        keyboard[n] = 0;
    }
    
    
    mouse_buttons.reserve(MAX_MOUSE_BUTTONS);
    mouse_buttons_last.reserve(MAX_MOUSE_BUTTONS);
    
    for (size_t n = 0; n < MAX_MOUSE_BUTTONS; n++) {
        mouse_buttons[n] = 0;
        mouse_buttons_last[n] = 0;
    }
    
    commands.reserve(1024);

    // audio setup

    pbspec.freq = 44100;
    pbspec.format = AUDIO_S16;
    pbspec.channels = 2;
    pbspec.samples = 4096;
    pbspec.callback = audio_pb_cb;

    pb_device_id = SDL_OpenAudioDevice( nullptr, SDL_FALSE, &pbspec, &pbspec, SDL_AUDIO_ALLOW_FORMAT_CHANGE );

    if (pb_device_id == 0) {
        cerr<<"Failed to open audio playback device"<<endl;
    }

    SDL_PauseAudioDevice(pb_device_id,0);

    lua_getglobal(L, "main");

    if (lua_pcall(L, 0, 0, 0) != 0) {
        cerr<<"Error running main:"<<lua_tostring(L, -1)<<endl;
        return -1;
    }

    return 0;
}
