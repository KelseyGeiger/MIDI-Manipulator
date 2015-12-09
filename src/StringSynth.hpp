#ifndef STRINGSYNTH_HPP
#define STRINGSYNTH_HPP

#include "Synth.hpp"

#define NO_STDIO_REDIRECT

#include "SDL2/SDL.h"
#include <thread>
#include <chrono>

namespace geiger {
	namespace midi {

		class StringSynth : public Synth
		{
			public:
				StringSynth();
				StringSynth(float L, float ten, float mu, float gamma);
				virtual ~StringSynth();

				void SetHarmonicCount(uint32_t harmonics);
				void SetActiveLength(float length);
				void SetTension(float tension);
				void SetLinearDensity(float mu);
				void SetDampingRatio(float gamma);

				uint32_t GetHarmonicCount() const;
				float GetActiveLength() const;
				float GetTension() const;
				float GetLinearDensity() const;
				float GetDampingRatio() const;

				void TuneToNote(Note n);
				void TuneToFrequency(float freq);
				void Pluck(float dist, float offset);
				void Strike(float dist, float force);
				void Silence();

				virtual float Value(float t) override;

				virtual SoundSample GenerateSample(uint32_t sample_rate, uint32_t duration_milliseconds, int32_t offset_milliseconds) override;

				virtual void PlayNote(Note n) override;

				virtual void Pause() override;
				virtual void Unpause() override;

				virtual void Play() override;
				virtual void Stop() override;

				virtual void SetVolume(float percent) override;
				virtual float GetVolume() const override;

			private:

				friend void stringsynth_callback(void* synth_, Uint8* stream_, int len_);

				float HarmonicAmplitude(uint32_t harmonic);
				float HarmonicFrequency(uint32_t harmonic);

				float max_amplitude;
				float volume;

				bool paused;
				bool stopped;

				float tension;
				float linear_density;
				float active_length;
				uint32_t number_of_harmonics;

				float mass;
				float velocity;
				float spring_constant;
				float damping_ratio;
				float fundamental_frequency;

				float distance_struck;
				float initial_offset;

				float time_elapsed;

				SDL_AudioDeviceID device_ID;
				SDL_AudioSpec specification;

				std::atomic<bool> parameter_change;
		};

		void stringsynth_callback(void* synth_, Uint8* stream_, int len_);

	}
}

#endif // STRINGSYNTH_HPP
