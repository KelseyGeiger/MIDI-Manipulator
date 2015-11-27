#ifndef STRINGSYNTH_HPP
#define STRINGSYNTH_HPP

#include "Synth.hpp"

namespace geiger {
	namespace midi {

		class StringSynth : public Synth
		{
			public:
				StringSynth(float L, float ten, float mu, float gamma);
				virtual ~StringSynth();

				void SetHarmonicCount(uint32_t harmonics);

				void Pluck(float dist, float offset);
				void Strike(float dist, float force);
				float Amplitude(float t);

				virtual SoundSample GenerateSample(uint32_t sample_rate, uint32_t duration_milliseconds, int32_t offset_milliseconds);

				virtual void Pause();
				virtual void Unpause();

				virtual void Play();
				virtual void Stop();

				virtual void SetVolume(float percent);
				virtual float GetVolume() const;

			private:

				friend void stringsynth_callback(void* synth_, Uint8* stream_, int len_);

				float HarmonicAmplitude(uint32_t harmonic);
				float HarmonicFrequency(uint32_t harmonic);

				float volume;

				bool paused;
				bool stopped;

				float tension;
				float linear_density;
				float length;
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
