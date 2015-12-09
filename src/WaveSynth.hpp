#ifndef WAVESYNTH_HPP
#define WAVESYNTH_HPP

#include "Synth.hpp"

namespace geiger {
	namespace midi {

		class WaveSynth : public Synth
		{
			public:

				enum WAVE_TYPE {
					SIN=0,
					SQR,
					TRI,
					SAW
				};

				WaveSynth();
				WaveSynth(WAVE_TYPE type, float freq, float volume);
				virtual ~WaveSynth();

				void SetWaveType(WAVE_TYPE type);
				void SetFrequency(float freq);

				virtual float Value(float t);

				void PlayWave(WAVE_TYPE type, float freq, float volume, uint32_t samp_rate = 44100, int32_t dur_milli = -1);

				virtual SoundSample GenerateSample(uint32_t sample_rate, uint32_t duration_milliseconds, int32_t offset_milliseconds) override;

				virtual void PlayNote(Note n) override;

				virtual void Pause() override;
				virtual void Unpause() override;

				virtual void Play() override;
				virtual void Stop() override;

				virtual void SetVolume(float percent) override;
				virtual float GetVolume() const override;

			private:
				friend void wavesynth_callback(void* synth_, Uint8* stream_, int len_);

				WAVE_TYPE wave;
				float frequency;
				float amplitude;

				float time_elapsed;

				bool paused;
				bool stopped;

				SDL_AudioDeviceID device_ID;
				SDL_AudioSpec specification;

				std::atomic<bool> volume_adjusting;
		};

		void wavesynth_callback(void* synth_, Uint8* stream_, int len_);

		template<typename T>
		int sign(T number) {
			if(number > 0) {
				return 1;
			} else if(number < 0) {
				return -1;
			} else {
				return 0;
			}
		}
	}
}

#endif // WAVESYNTH_HPP
