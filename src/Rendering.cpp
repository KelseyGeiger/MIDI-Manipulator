#include "Rendering.h"
#include <iostream>

void DrawSampleWaveform(SDL_Renderer* renderer, const geiger::midi::SoundSample& sample) {
	static float max_amp = 0.0f;

	int width, height;

	SDL_GetRendererOutputSize(renderer, &width, &height);

    float x_scale = (float)(width) / (float)(sample.buffer_length);

    for(size_t i = 0; i < sample.buffer_length; i++) {
		if(std::abs(sample[i]) > max_amp) {
            max_amp = std::abs(sample[i]);
		}
    }

	float y_scale;

	if(max_amp > 0.0f) {
		y_scale = ((float)(height) / (float)(max_amp)) / 3.0f;
	} else {
		y_scale = 1.0f;
	}

    SDL_Point* points = new SDL_Point[sample.buffer_length];

	int out_of_bounds_idx = -1;
	int in_bounds_idx = 0;

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

	SDL_RenderDrawLine(renderer, 0, (height/6), width, (height/6));
	SDL_RenderDrawLine(renderer, 0, (5*height/6), width, (5*height/6));

	SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
	SDL_RenderDrawLine(renderer, 0, (height/2), width, (height/2));

	if(sample[0] < -1.0f || sample[0] > 1.0f) {
		SDL_SetRenderDrawColor(renderer, 255, 64, 64, 255);
		out_of_bounds_idx = 0;
	} else {
		SDL_SetRenderDrawColor(renderer, 64, 128, 64, 255);
	}

	for(size_t i = 0; i < sample.buffer_length; i++) {
		SDL_Point p;
	    p.x = (int)(i * x_scale);

	    //the sample is negated because SDL draws with y=0 at the top of the window
	    p.y = (int)(-sample[i] * y_scale) + (height / 2);

		if(p.x >= width) {
			p.x = width;
		} else if(p.x < 0) {
			p.x = 0;
		}

		points[i] = p;

		if((p.y > height * y_scale || p.y < height/4) && out_of_bounds_idx == -1) {
			out_of_bounds_idx = i;
			SDL_RenderDrawLines(renderer, (points + in_bounds_idx), out_of_bounds_idx - in_bounds_idx);
			SDL_SetRenderDrawColor(renderer, 255, 64, 64, 255);
		} else {
			if(out_of_bounds_idx != -1) {
				in_bounds_idx = i;
				SDL_RenderDrawLines(renderer, (points + out_of_bounds_idx), in_bounds_idx - out_of_bounds_idx);
				out_of_bounds_idx = -1;
				SDL_SetRenderDrawColor(renderer, 64, 128, 64, 255);
			}
		}
	}

	if(out_of_bounds_idx != -1) {
		SDL_RenderDrawLines(renderer, (points + in_bounds_idx), sample.buffer_length - out_of_bounds_idx);
	} else {
		SDL_RenderDrawLines(renderer, (points + out_of_bounds_idx + 1), sample.buffer_length - in_bounds_idx);
	}

	delete[] points;

	SDL_RenderPresent(renderer);

}
