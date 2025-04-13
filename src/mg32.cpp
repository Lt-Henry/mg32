#include "mg32.hpp"

#include <SDL2/SDL_image.h>

#include <iostream>

using namespace mg32;

using namespace std;

Bank::Bank(SDL_Renderer* renderer, string filename,int tw, int th) : tile_width(tw), tile_height(th)
{
    SDL_Surface* surface = IMG_Load(filename.c_str());

    if (!surface) {
        cerr<<"Failed to load image:"<<filename<<endl;
    }

    data = SDL_CreateTextureFromSurface(renderer, surface);

    width = surface->w;
    height = surface->h;

    SDL_FreeSurface(surface);
}

Bank::~Bank()
{

}

Gamepad::Gamepad(int which) : id(which)
{
    for (size_t n=0;n<32;n++) {
        buttons[n] = 0;
        buttons_last[n] = 0;
    }
    
    gc = SDL_GameControllerOpen(id);
}

Gamepad::~Gamepad()
{
}

int Gamepad::button(int btn)
{
    return buttons[btn];
}

int Gamepad::buttondown(int btn)
{
    return buttons[btn] > buttons_last[btn];
}

void Gamepad::update(SDL_Event& event)
{
    if (event.type == SDL_CONTROLLERBUTTONDOWN or event.type == SDL_CONTROLLERBUTTONUP) {
        buttons_last[event.cbutton.button] = buttons[event.cbutton.button];
        buttons[event.cbutton.button] = event.cbutton.state;
    }
}
