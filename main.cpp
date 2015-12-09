#include <iostream>

#define NO_STDIO_REDIRECT

#include "SDL2/SDL.h"
#include "SDL2/SDL_main.h"

#include "WaveSynth.hpp"
#include "StringSynth.hpp"
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
		return 1;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

	if(!renderer) {
		return 2;
	}

	GuitarSynth synth;
	synth.SetVolume(1.0f);

	synth.Play();

	SDL_Event evt;
	bool quit = false;
	bool paused = false;
	bool stopped = false;
	bool should_draw = true;

	bool string_plucked[6] = { false };

	int32_t d_time = 0;
	std::chrono::steady_clock::time_point prev = std::chrono::steady_clock::now();

	while(!quit) {

		auto time = std::chrono::steady_clock::now();
		auto diff = time - prev;
		prev = time;

        d_time += std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();

		if(should_draw) {
			SoundSample samp = synth.GenerateSample(48000, 25, d_time);
			DrawSampleWaveform(renderer, samp);
		} else {
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			SDL_RenderClear(renderer);
			SDL_RenderPresent(renderer);
		}

		Chord chord;

		while(SDL_PollEvent(&evt)) {
			if(evt.type == SDL_KEYDOWN) {
				if(evt.key.keysym.sym == SDLK_ESCAPE) {
					quit = true;
				} else if(evt.key.keysym.sym == SDLK_d) {
					should_draw = !should_draw;
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
						synth.Play();
					}
				} else if(evt.key.keysym.sym == SDLK_SPACE) {
					synth.Strum();
				} else if(evt.key.keysym.sym == SDLK_o) {
					for(uint32_t i = 1; i <= 6; i++) {
						synth.OpenString(i);
					}
				} else if(evt.key.keysym.sym == SDLK_q && !string_plucked[0]) {
					string_plucked[0] = true;
					chord.AddNote(Note(Note::C, Note::SHARP, 5));
				} else if(evt.key.keysym.sym == SDLK_w && !string_plucked[1]) {
					string_plucked[1] = true;
					chord.AddNote(Note(Note::A, Note::FLAT, 4));
				} else if(evt.key.keysym.sym == SDLK_e && !string_plucked[2]) {
					string_plucked[2] = true;
					chord.AddNote(Note(Note::E, Note::NATURAL, 4));
				} else if(evt.key.keysym.sym == SDLK_r && !string_plucked[3]) {
					string_plucked[3] = true;
					chord.AddNote(Note(Note::B, Note::NATURAL, 3));
				} else if(evt.key.keysym.sym == SDLK_t && !string_plucked[4]) {
					string_plucked[4] = true;
					chord.AddNote(Note(Note::F, Note::SHARP, 3));
				} else if(evt.key.keysym.sym == SDLK_y && !string_plucked[5]) {
					string_plucked[5] = true;
					chord.AddNote(Note(Note::C, Note::SHARP, 3));
				} else if(evt.key.keysym.sym == SDLK_EQUALS) {
					float vol = synth.GetVolume();
					synth.SetVolume(vol + 0.01f);
				} else if(evt.key.keysym.sym == SDLK_MINUS) {
					float vol = synth.GetVolume();
					synth.SetVolume(vol - 0.01f);
				}
			} else if(evt.type == SDL_KEYUP) {
				if(evt.key.keysym.sym == SDLK_q && string_plucked[0]) {
					string_plucked[0] = false;
				} else if(evt.key.keysym.sym == SDLK_w && string_plucked[1]) {
					string_plucked[1] = false;
				} else if(evt.key.keysym.sym == SDLK_e && string_plucked[2]) {
					string_plucked[2] = false;
				} else if(evt.key.keysym.sym == SDLK_r && string_plucked[3]) {
					string_plucked[3] = false;
				} else if(evt.key.keysym.sym == SDLK_t && string_plucked[4]) {
					string_plucked[4] = false;
				} else if(evt.key.keysym.sym == SDLK_y && string_plucked[5]) {
					string_plucked[5] = false;
				}
			} else if(evt.type == SDL_QUIT) {
				quit = true;
			}
		}

		synth.PlayChord(chord);
	}

	SDL_DestroyWindow(win);
	SDL_Quit();

    return 0;
}
