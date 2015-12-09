#include "WaveSynth.hpp"
#include <limits>

namespace geiger {
	namespace midi {

		WaveSynth::WaveSynth() : volume_adjusting{false}
		{
			paused = false;
			stopped = true;

			wave = SIN;
			frequency = 440.0f;
			amplitude = 0.5f;
		}

		WaveSynth::WaveSynth(WAVE_TYPE type, float freq, float volume) {
			paused = false;
			stopped = true;

			wave = type;
			frequency = freq;
			amplitude = volume;

		}

		WaveSynth::~WaveSynth()
		{
			Stop();
		}

		void WaveSynth::SetWaveType(WAVE_TYPE type) {
			wave = type;
		}

		void WaveSynth::SetFrequency(float freq) {
			frequency = freq;
		}

		void WaveSynth::PlayWave(WAVE_TYPE type, float freq, float volume, uint32_t samp_rate, int32_t dur_milli) {

			if(!stopped) {
				Stop();
			}

			float dur_second = (samp_rate / 1000.0f) * (float)(dur_milli);

			wave = type;
			frequency = freq;
			amplitude = volume;

			Play();

			//this gets modified in another thread through a pointer
			while(time_elapsed < dur_second);

			Stop();
		}

		float WaveSynth::Value(float t) {
            switch(wave) {
				case SIN: {
					float omega = 2 * M_PI * frequency;
					return amplitude * std::sin(omega * t);
				}

				case SQR: {
					float omega = 2 * M_PI * frequency;
					return amplitude * sign(std::sin(omega * t));
				}

				case TRI: {
					float period = 1.0f/frequency;
					float a = period / 2;
					float sawtooth = 2 * ((t/a) - (int32_t)((t/a) + 1.0f/2.0f));
					return (amplitude * (2 * std::abs(sawtooth) - 1.0f));
				}

				case SAW: {
					float period = 1.0f/frequency;
					float a = period / 2;
					float sawtooth = 2 * ((t/a) - (int32_t)(1.0f/2.0f + t/a));
					return amplitude * sawtooth;
				}

				default: {
					float omega = 2 * M_PI * frequency;
					return amplitude * std::sin(omega * t);
				}
			}
		}

		SoundSample WaveSynth::GenerateSample(uint32_t samp_rate, uint32_t duration_milliseconds, int32_t offset_milliseconds) {
            SoundSample sample{samp_rate, duration_milliseconds};

			float offset_t = (float)(offset_milliseconds) / (float)(samp_rate);
			float dt = 1.0f / (float)(samp_rate);

			switch(wave) {
				case SIN: {
					float omega = 2 * M_PI * frequency;
					for(size_t i = 0; i < sample.buffer_length; i++) {
						float t = (i * dt) + offset_t;
						sample.audio_buffer[i] = (amplitude * std::sin(omega * t));
					}
					break;
				}

				case SQR: {
					float omega = 2 * M_PI * frequency;
					for(size_t i = 0; i < sample.buffer_length; i++) {
						float t = (i * dt) + offset_t;
						sample.audio_buffer[i] = (amplitude * sign(std::sin(omega * t)));
					}
					break;
				}

				case TRI: {
					float period = 1.0f/frequency;
					float a = period / 2;
					for(size_t i = 0; i < sample.buffer_length; i++) {
						float t = (i * dt) + offset_t;
						float sawtooth = 2 * ((t/a) - (int32_t)((t/a) + 1.0f/2.0f));
						sample.audio_buffer[i] = (amplitude * (2 * std::abs(sawtooth) - 1.0f));
					}
					break;
				}

				case SAW: {
					float period = 1.0f/frequency;
					float a = period / 2;
					for(size_t i = 0; i < sample.buffer_length; i++) {
						float t = (i * dt) + offset_t;
						float sawtooth = 2 * ((t/a) - (int32_t)(1.0f/2.0f + t/a));
						sample.audio_buffer[i] = (amplitude * sawtooth);
					}
					break;
				}

				default: {
					float omega = 2 * M_PI * frequency;
					for(size_t i = 0; i < sample.buffer_length; i++) {
						float t = (i * dt) + offset_t;
						sample.audio_buffer[i] = (amplitude * std::sin(omega * t));
					}
					break;
				}
			}

            return sample;
		}

		void WaveSynth::PlayNote(Note n) {

			float frequency = NoteToFrequency(n);

            PlayWave(SIN, frequency, amplitude);
		}

		void WaveSynth::Pause() {
			if(!stopped) {
				SDL_PauseAudioDevice(device_ID, 1);
				paused = true;
			}
		}

		void WaveSynth::Unpause() {
			if(!stopped) {
				SDL_PauseAudioDevice(device_ID, 0);
				paused = false;
			}
		}

		void WaveSynth::Play() {
			SDL_AudioSpec want;
			want.freq = 44100;
			want.format = AUDIO_F32SYS;
			want.channels = 1;
			want.samples = 4096;
			want.callback = wavesynth_callback;
			want.userdata = (void*)(this);

			device_ID = SDL_OpenAudioDevice(NULL, 0, &want, &specification, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE);
			SDL_PauseAudioDevice(device_ID, 0);
			paused = false;
			stopped = false;
		}

		void WaveSynth::Stop() {
			SDL_PauseAudioDevice(device_ID, 1);
			SDL_CloseAudioDevice(device_ID);
			stopped = true;
			paused = false;
		}

		void WaveSynth::SetVolume(float percent) {

			if(percent < 0.0f) {
				percent = 0.0f;
			}

			if(percent > 1.0f) {
				percent = 1.0f;
			}

			while(volume_adjusting.exchange(true));

			amplitude = percent;

			volume_adjusting.store(false);

		}

		float WaveSynth::GetVolume() const {
			return amplitude;
		}

		void wavesynth_callback(void* synth_, Uint8* stream_, int len_) {
			WaveSynth* synth = (WaveSynth*)(synth_);

			float dt = 1.0f / (float)(synth->specification.freq);

			if(synth->specification.format != AUDIO_F32SYS) {
				if(synth->specification.format == AUDIO_S16SYS) {
					uint16_t* stream = (uint16_t*)(stream_);

					len_ = len_ / sizeof(uint16_t);

					while(synth->volume_adjusting.exchange(true));

					for(int i = 0; i < len_; i++) {
						stream[i] = (int16_t)(synth->Value(synth->time_elapsed) * std::numeric_limits<int16_t>::max());
						synth->time_elapsed += dt;
					}

					synth->volume_adjusting.store(false);
				}
			}

			float* stream = (float*)(stream_);

			len_ = len_ / sizeof(float);

			while(synth->volume_adjusting.exchange(true));

			for(int i = 0; i < len_; i++) {
				stream[i] = synth->Value(synth->time_elapsed);
				synth->time_elapsed += dt;
			}

			synth->volume_adjusting.store(false);

		}
	}
}
