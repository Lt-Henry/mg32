#include <SDL2/SDL.h>

#include <string>

namespace mg32
{
    class Bank
    {
        public:

        SDL_Texture* data;
        SDL_Surface* surface;
        int width;
        int height;
        int tile_width;
        int tile_height;

        Bank(SDL_Renderer* renderer, std::string filename,int tw, int th);
        virtual ~Bank();

        uint32_t get_pixel(uint32_t x,uint32_t y);
    };

    enum class Command
    {
        Blit,
        BlitEx,
        Rectangle,
        Line
    };

    class DrawCommand
    {
        public:

        Command command;

        int z;
        int flip;
        double angle;
        double opacity;
        SDL_Point pivot;
        SDL_Color color;

        SDL_Texture* texture;
        SDL_Rect src;
        SDL_Rect dst;

        DrawCommand* left;
        DrawCommand* right;
    };
    
    class Gamepad
    {
        public:
        
        int id;
        SDL_GameController* gc;
        int buttons[32];
        int buttons_last[32];
        
        Gamepad(int which);
        virtual ~Gamepad();
        
        int button(int btn);
        int buttondown(int btn);
        void get_axis(int axis,double& x,double& y);
        void rumble();
        
        void update(SDL_Event& event);

        void frame();
    };

    class Sample
    {
        public:

        uint8_t* buffer;
        uint32_t size;
        SDL_AudioSpec spec;

        Sample(std::string filename, SDL_AudioSpec spec);
        ~Sample();
    };

    class Stream
    {
        public:

        Sample* sample;
        uint32_t pos;
        uint32_t flags;
    };
}
