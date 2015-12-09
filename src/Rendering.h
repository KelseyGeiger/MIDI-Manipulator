#ifndef RENDERING_HPP_INCLUDED
#define RENDERING_HPP_INCLUDED

#include "SDL2/SDL.h"
#include "SDL2/SDL_render.h"
#include "Synth.hpp"

void DrawSampleWaveform(SDL_Renderer* renderer, const geiger::midi::SoundSample& sample);

#endif // RENDERING_HPP_INCLUDED
