#ifndef GUITARSYNTH_HPP
#define GUITARSYNTH_HPP

#include "Synth.hpp"
#include "StringSynth.hpp"

#define NO_STDIO_REDIRECT

#include "SDL2/SDL.h"

namespace geiger {
	namespace midi {

		class GuitarSynth : public Synth
		{
			public:
				GuitarSynth();
				GuitarSynth(float length_meters, float linear_density = 0.002f, float damping_ratio = 1.0f);
				virtual ~GuitarSynth();

				void SetStringLength(float length_meters);
				void SetStringDensity(float linear_density);
				void SetDampingRatio(float damp);

				float GetStringLength() const;
				float GetStringDensity() const;
				float GetDampingRatio() const;

				void Strum(float time_offset_seconds = 0.0f);
				void PluckString(uint32_t string_, float time_offset_seconds = 0.0f);
				void StopString(uint32_t string_);
				void FretString(Note n, uint32_t string_);
				void OpenString(uint32_t string_);

				virtual float Value(float t) override;

				virtual SoundSample GenerateSample(uint32_t sample_rate, uint32_t duration_milliseconds, int32_t offset_milliseconds) override;

				virtual void PlayNote(Note n) override;
				void PlayChord(Chord c);

				virtual void Pause() override;
				virtual void Unpause() override;

				virtual void Play() override;
				virtual void Stop() override;

				virtual void SetVolume(float percent) override;
				virtual float GetVolume() const override;

			private:
				float GetLengthForNote(uint32_t string_, Note n);

				StringSynth strings[6];
				float time_offsets[6];
				float time_elapsed;
				float volume;
				float max_amplitude;

				float max_length;
				float string_density;
				float damping_ratio;

				bool paused;
				bool stopped;

				std::atomic<bool> parameter_change;

				SDL_AudioDeviceID device_ID;
				SDL_AudioSpec specification;

				friend void guitar_callback(void* synth_, Uint8* stream_, int len_);
		};

		void guitar_callback(void* synth_, Uint8* stream_, int len_);

	}
}

#endif // GUITARSYNTH_HPP
