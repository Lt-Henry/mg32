
gcc = meson.get_compiler('cpp')
libm = gcc.find_library('m')
sdl2 = dependency('sdl2')
#sdl2_image = dependency('SDL2_image')
#sdl2_ttf = dependency('SDL2_ttf')
lua = dependency('lua54-c++')

executable('mg32', ['main.cpp'],
    cpp_args:['-std=c++11','-Wfatal-errors'],
    dependencies: [libm,sdl2,lua]
    )
