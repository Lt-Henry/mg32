#include "mg32.hpp"

#include	<sndfile.hh>

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
    clog<<"rumble:"<<SDL_GameControllerHasRumble(gc)<<endl;
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

void Gamepad::get_axis(int axis,double& x,double& y)
{
    int32_t raw1 = SDL_GameControllerGetAxis(gc,static_cast<SDL_GameControllerAxis>(axis));
    int32_t raw2 = SDL_GameControllerGetAxis(gc,static_cast<SDL_GameControllerAxis>(axis + 1));

    x = raw1/32767.0;
    y = raw2/32767.0;
}

void Gamepad::rumble()
{
    SDL_GameControllerRumble(gc,0,0xFFFF,800);
}

void Gamepad::update(SDL_Event& event)
{
    if (event.type == SDL_CONTROLLERBUTTONDOWN or event.type == SDL_CONTROLLERBUTTONUP) {
        buttons_last[event.cbutton.button] = buttons[event.cbutton.button];
        buttons[event.cbutton.button] = event.cbutton.state;
    }
}

void Gamepad::frame()
{
    for (size_t n=0;n<32;n++) {

        buttons_last[n] = buttons[n] ;
    }
}

Sample::Sample(string filename, SDL_AudioSpec spec) : size(0), buffer(nullptr)
{
    this->spec = spec;
    string extension;

    SDL_AudioCVT cvt;

    SndfileHandle file ;

    file = SndfileHandle (filename);

    if (file.frames() > 0) {

        size_t length = file.frames() * file.channels();

        // convert
        SDL_BuildAudioCVT(&cvt, AUDIO_S16, file.channels(), file.samplerate(), spec.format, spec.channels, spec.freq);

        cvt.len = length * sizeof(int16_t);
        cvt.buf = new uint8_t[cvt.len * cvt.len_mult];

        // read data
        file.read ((int16_t *)cvt.buf, length) ;

        SDL_ConvertAudio(&cvt);

        this->buffer = cvt.buf;
        this->size = cvt.len_cvt;

    }

}

Sample::~Sample()
{
    if (this->buffer) {
        delete [] this->buffer;
    }
}
