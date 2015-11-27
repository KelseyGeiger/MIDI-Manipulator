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
				WaveSynth(WAVE_TYPE type, float freq, float volume, uint32_t samp_rate = 44100, int32_t dur_milli = -1);
				virtual ~WaveSynth();

				void SetWaveType(WAVE_TYPE type);
				void SetFrequency(float freq);
				void SetSampleRate(uint32_t rate);
				void SetDuration(int32_t milliseconds);

				void PlayWave(WAVE_TYPE type, float freq, float volume, uint32_t samp_rate = 44100, int32_t dur_milli = -1);

				virtual SoundSample GenerateSample(uint32_t sample_rate, uint32_t duration_milliseconds, int32_t offset_milliseconds);

				virtual void Pause();
				virtual void Unpause();

				virtual void Play();
				virtual void Stop();

				virtual void SetVolume(float percent);
				virtual float GetVolume() const;

			private:
				void FillBuffer();

				friend void wavesynth_callback(void* synth_, Uint8* stream_, int len_);

				WAVE_TYPE wave;
				float frequency;
				float amplitude;
				int32_t duration_milliseconds;
				uint32_t sample_rate;

				bool paused;
				bool stopped;

				int16_t* audio_buffer;
				uint32_t buffer_pos;
				uint32_t buffer_length;

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
