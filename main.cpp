#include <iostream>

#define NO_STDIO_REDIRECT

#include "SDL2/SDL.h"
#include "SDL2/SDL_main.h"
#include "MIDI_VLQ.hpp"
#include "MIDI_Chunk.hpp"
#include "GuitarSynth.hpp"
#include "Rendering.h"
#include <thread>
#include <chrono>
#include <fstream>
#include <cstdlib>

using namespace geiger::midi;

int main(int argc, char* argv[])
{
	SDL_Init(SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_TIMER | SDL_INIT_VIDEO);

	SDL_Window* win = SDL_CreateWindow("MIDI Player", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 960, 540, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);

	if(!win) {
		return -1;
	}

	std::fstream some_midi;
    some_midi.open("./assets/BringItIn,Guys!.mid", std::ios::in | std::ios::binary);
    std::fstream midi_copy;
    midi_copy.open("./test/midi_copy.mid", std::ios::out | std::ios::trunc | std::ios::binary);

    if(!some_midi) {
		return -2;
    }

	MIDI_Chunk header_chunk;
	some_midi >> header_chunk;
    midi_copy << header_chunk;
    midi_copy.flush();

	std::vector<MIDI_Chunk> tracks;

	for(uint32_t i = 0; i < header_chunk.GetHeader().track_count; i++) {
		MIDI_Chunk tmp{};
		some_midi >> tmp;
		midi_copy << tmp;
		midi_copy.flush();
		tracks.push_back(tmp);
	}

	SDL_Event evt;
	bool quit = false;

	int32_t d_time = 0;
	std::chrono::steady_clock::time_point prev = std::chrono::steady_clock::now();

	while(!quit) {

		auto time = std::chrono::steady_clock::now();
		auto diff = time - prev;
		prev = time;

        d_time += std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();

		while(SDL_PollEvent(&evt)) {
			if(evt.type == SDL_KEYDOWN) {
				if(evt.key.keysym.sym == SDLK_ESCAPE) {
					quit = true;
				}
			} else if(evt.type == SDL_QUIT) {
				quit = true;
			}
		}
	}
	SDL_DestroyWindow(win);
	SDL_Quit();

    return 0;
}
