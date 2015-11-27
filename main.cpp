#include <iostream>

#define NO_STDIO_REDIRECT

#include "SDL2/SDL.h"
#include "SDL2/SDL_main.h"

#include "WaveSynth.hpp"
#include "StringSynth.hpp"
#include <thread>
#include <chrono>
#include <fstream>

using namespace geiger::midi;

int main(int argc, char* argv[])
{
	SDL_SetMainReady();

	SDL_Init(SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_TIMER | SDL_INIT_VIDEO);

	SDL_Window* win = SDL_CreateWindow("MIDI Player", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 460, 460, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);

	//equivalent to a 60.7cm guitar string, 2g/m linear density and 117.86N of tension
	StringSynth synth{0.6069f, 117.86f, .002f, 1.0f};
	synth.SetHarmonicCount(20);
	synth.SetVolume(0.5f);

	//pluck a little under 1/4th of the way along the string, pulling it 5cm out
	synth.Pluck(0.23f * 0.6069f, 0.02);
	synth.Play();

	SDL_Event evt;
	bool quit = false;
	bool paused = false;
	bool stopped = false;

	while(!quit) {
		SDL_PollEvent(&evt);
		if(evt.type == SDL_KEYDOWN) {
			if(evt.key.keysym.sym == SDLK_q || evt.key.keysym.sym == SDLK_ESCAPE) {
				quit = true;
			} else if(evt.key.keysym.sym == SDLK_p) {
				paused = !paused;

				if(paused) {
					synth.Pause();
				} else {
					synth.Unpause();
				}
			} else if(evt.key.keysym.sym == SDLK_s) {
				stopped = !stopped;

				if(stopped) {
					synth.Stop();
				} else {
					synth.Pluck(0.23f * 0.6069f, 0.05);
					synth.Play();
				}
			} else if(evt.key.keysym.sym == SDLK_EQUALS) {
				float vol = synth.GetVolume();
				synth.SetVolume(vol + 0.01f);
			} else if(evt.key.keysym.sym == SDLK_MINUS) {
				float vol = synth.GetVolume();
				synth.SetVolume(vol - 0.01f);
			}
		} else if(evt.type == SDL_QUIT) {
			quit = true;
		}
	}

	SDL_DestroyWindow(win);
	SDL_Quit();

    return 0;
}
