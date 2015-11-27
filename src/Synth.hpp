#ifndef SYNTH_HPP
#define SYNTH_HPP

#include "SDL2/SDL_audio.h"
#include <cmath>
#include <limits>
#include <atomic>

namespace geiger {
	namespace midi {

		struct SoundSample {
            uint32_t sample_rate;
            uint32_t buffer_length;
            uint32_t duration_milliseconds;
            float* audio_buffer;

			SoundSample();
			SoundSample(uint32_t rate, uint32_t dur_milli);
			SoundSample(const SoundSample& other);
			SoundSample(SoundSample&& other);
            ~SoundSample();

            SoundSample& operator=(const SoundSample& s);
            SoundSample& operator=(SoundSample&& s);

            SoundSample operator+(SoundSample other);
            SoundSample operator-(SoundSample other);

            SoundSample& operator+=(SoundSample other);
            SoundSample& operator-=(SoundSample other);
		};

		class Synth
		{
			public:
				virtual ~Synth() {}

				virtual SoundSample GenerateSample(uint32_t sample_rate, uint32_t duration_milliseconds, int32_t offset_milliseconds) = 0;

				virtual void Pause() = 0;
				virtual void Unpause() = 0;

				virtual void Play() = 0;
				virtual void Stop() = 0;

				virtual void SetVolume(float percent) = 0;
				virtual float GetVolume() const = 0;
		};

	}
}

#endif // SYNTH_HPP
