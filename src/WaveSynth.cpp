#include "WaveSynth.hpp"
#include <limits>

namespace geiger {
	namespace midi {

		WaveSynth::WaveSynth() : volume_adjusting{false}
		{
			paused = false;
			stopped = true;
			audio_buffer = nullptr;
		}

		WaveSynth::WaveSynth(WAVE_TYPE type, float freq, float volume, uint32_t samp_rate, int32_t dur_milli) {
			paused = false;
			stopped = true;
			audio_buffer = nullptr;

			wave = type;

			frequency = freq;
			amplitude = (volume * std::numeric_limits<int16_t>::max());
			duration_milliseconds = dur_milli;
			sample_rate = samp_rate;
			buffer_pos = 0;

			if(duration_milliseconds > 0) {
				buffer_length = (sample_rate * duration_milliseconds / 1000.0f);
			} else {
				buffer_length = sample_rate / 10;
			}

			if(audio_buffer) {
				delete[] audio_buffer;
			}

			audio_buffer = new int16_t[buffer_length];
			buffer_pos = 0;

			FillBuffer();
		}

		WaveSynth::~WaveSynth()
		{
			Stop();
			delete[] audio_buffer;
		}

		void WaveSynth::SetWaveType(WAVE_TYPE type) {
			wave = type;

			FillBuffer();
		}

		void WaveSynth::SetFrequency(float freq) {
			frequency = freq;

			FillBuffer();
		}

		void WaveSynth::SetSampleRate(uint32_t rate) {
			sample_rate = rate;

			if(duration_milliseconds > 0) {
				buffer_length = (sample_rate * duration_milliseconds / 1000.0f);
			} else {
				buffer_length = sample_rate / 10;
			}

			if(audio_buffer) {
				delete[] audio_buffer;
			}

			audio_buffer = new int16_t[buffer_length];

			FillBuffer();
		}

		void WaveSynth::SetDuration(int32_t milliseconds) {
			duration_milliseconds = milliseconds;

			if(duration_milliseconds > 0) {
				buffer_length = (sample_rate * duration_milliseconds / 1000.0f);
			} else {
				buffer_length = sample_rate / 10;
			}

			if(audio_buffer) {
				delete[] audio_buffer;
			}

			audio_buffer = new int16_t[buffer_length];

			FillBuffer();
		}

		void WaveSynth::PlayWave(WAVE_TYPE type, float freq, float volume, uint32_t samp_rate, int32_t dur_milli) {

			if(!stopped) {
				Stop();
			}

			wave = type;

			frequency = freq;
			amplitude = (volume * std::numeric_limits<int16_t>::max());
			duration_milliseconds = dur_milli;
			sample_rate = samp_rate;
			buffer_pos = 0;

			if(dur_milli > 0) {
				buffer_length = (sample_rate * duration_milliseconds / 1000.0f);
			} else {
				buffer_length = sample_rate / 10;
			}

			if(audio_buffer) {
				delete[] audio_buffer;
			}

			audio_buffer = new int16_t[buffer_length];
			buffer_pos = 0;

			FillBuffer();

			SDL_AudioSpec want;
			want.freq = sample_rate;
			want.format = AUDIO_S16SYS;
			want.channels = 1;
			want.samples = buffer_length;
			want.callback = wavesynth_callback;
			want.userdata = (void*)(this);

			device_ID = SDL_OpenAudioDevice(NULL, 0, &want, &specification, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE);
			SDL_PauseAudioDevice(device_ID, 0);
			paused = false;
			stopped = false;
		}

		SoundSample WaveSynth::GenerateSample(uint32_t samp_rate, uint32_t duration_milliseconds, int32_t offset_milliseconds) {
            SoundSample sample{sample_rate, duration_milliseconds};

			float offset_t = (float)(offset_milliseconds) / (float)(samp_rate);
			float dt = 1.0f / (float)(samp_rate);

			switch(wave) {
				case SIN: {
					float omega = 2 * M_PI * frequency;
					for(size_t i = 0; i < sample.buffer_length; i++) {
						float t = (i * dt) + offset_t;
						sample.audio_buffer[i] = (int16_t)(amplitude * std::sin(omega * t));
					}
					break;
				}

				case SQR: {
					float omega = 2 * M_PI * frequency;
					for(size_t i = 0; i < sample.buffer_length; i++) {
						float t = (i * dt) + offset_t;
						sample.audio_buffer[i] = (int16_t)(amplitude * sign(std::sin(omega * t)));
					}
					break;
				}

				case TRI: {
					float period = 1.0f/frequency;
					float a = period / 2;
					for(size_t i = 0; i < sample.buffer_length; i++) {
						float t = (i * dt) + offset_t;
						float sawtooth = 2 * ((t/a) - (int32_t)((t/a) + 1.0f/2.0f));
						sample.audio_buffer[i] = (int16_t)(amplitude * (2 * std::abs(sawtooth) - 1.0f));
					}
					break;
				}

				case SAW: {
					float period = 1.0f/frequency;
					float a = period / 2;
					for(size_t i = 0; i < sample.buffer_length; i++) {
						float t = (i * dt) + offset_t;
						float sawtooth = 2 * ((t/a) - (int32_t)(1.0f/2.0f + t/a));
						sample.audio_buffer[i] = (int16_t)(amplitude * sawtooth);
					}
					break;
				}

				default: {
					float omega = 2 * M_PI * frequency;
					for(size_t i = 0; i < sample.buffer_length; i++) {
						float t = (i * dt) + offset_t;
						sample.audio_buffer[i] = (int16_t)(amplitude * std::sin(omega * t));
					}
					break;
				}
			}

            return sample;
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
			if(audio_buffer) {
				SDL_AudioSpec want;
				want.freq = sample_rate;
				want.format = AUDIO_S16SYS;
				want.channels = 1;
				want.samples = 4096;
				want.callback = wavesynth_callback;
				want.userdata = (void*)(this);

				device_ID = SDL_OpenAudioDevice(NULL, 0, &want, &specification, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE);
				SDL_PauseAudioDevice(device_ID, 0);
				paused = false;
				stopped = false;
			}
		}

		void WaveSynth::Stop() {
			SDL_PauseAudioDevice(device_ID, 1);
			SDL_CloseAudioDevice(device_ID);
			stopped = true;
			paused = false;
			buffer_pos = 0;
		}

		void WaveSynth::SetVolume(float percent) {

			if(percent < 0.0f) {
				percent = 0.0f;
			}

			if(percent > 1.0f) {
				percent = 1.0f;
			}

			if(amplitude >= 0.01f) {
				float amplitude_ratio = (percent * std::numeric_limits<int16_t>::max()) / amplitude;

				amplitude = (percent * std::numeric_limits<int16_t>::max());

				while(volume_adjusting.exchange(true));

				for(size_t i = 0; i < buffer_length; i++) {
					audio_buffer[i] = (int16_t)(audio_buffer[i] * amplitude_ratio);
				}

				volume_adjusting.store(false);

			} else {
				amplitude = (percent * std::numeric_limits<int16_t>::max());

				while(volume_adjusting.exchange(true));

				FillBuffer();

				volume_adjusting.store(false);
			}
		}

		float WaveSynth::GetVolume() const {
			return amplitude / (std::numeric_limits<int16_t>::max());
		}

		void WaveSynth::FillBuffer() {

			switch(wave) {
				case SIN: {
					float omega = 2 * M_PI * frequency;
					for(size_t i = 0; i < buffer_length; i++) {
						float t = (float)(i) / (float)(sample_rate);
						audio_buffer[i] = (int16_t)(amplitude * std::sin(omega * t));
					}
					break;
				}

				case SQR: {
					float omega = 2 * M_PI * frequency;
					for(size_t i = 0; i < buffer_length; i++) {
						float t = (float)(i) / (float)(sample_rate);
						audio_buffer[i] = (int16_t)(amplitude * sign(std::sin(omega * t)));
					}
					break;
				}

				case TRI: {
					float period = 1.0f/frequency;
					float a = period / 2;
					for(size_t i = 0; i < buffer_length; i++) {
						float t = (float)(i) / (float)(sample_rate);
						float sawtooth = 2 * ((t/a) - (int32_t)((t/a) + 1.0f/2.0f));
						audio_buffer[i] = (int16_t)(amplitude * (2 * std::abs(sawtooth) - 1.0f));
					}
					break;
				}

				case SAW: {
					float period = 1.0f/frequency;
					float a = period / 2;
					for(size_t i = 0; i < buffer_length; i++) {
						float t = (float)(i) / (float)(sample_rate);
						float sawtooth = 2 * ((t/a) - (int32_t)(1.0f/2.0f + t/a));
						audio_buffer[i] = (int16_t)(amplitude * sawtooth);
					}
					break;
				}

				default: {
					float omega = 2 * M_PI * frequency;
					for(size_t i = 0; i < buffer_length; i++) {
						float t = (float)(i) / (float)(sample_rate);
						audio_buffer[i] = (int16_t)(amplitude * std::sin(omega * t));
					}
					break;
				}
			}
		}

		void wavesynth_callback(void* synth_, Uint8* stream_, int len_) {
			WaveSynth* synth = (WaveSynth*)(synth_);

			len_ = len_ / 2;
			uint16_t* dest = (uint16_t*)(stream_);

			if(synth->duration_milliseconds > 0) {
				uint32_t samples_to_write = ((synth->buffer_length - synth->buffer_pos) > (uint32_t)(len_))
											? (uint32_t)(len_)
											: (synth->buffer_length - synth->buffer_pos);

				while(synth->volume_adjusting.exchange(true));

				for(uint32_t i = 0; i < samples_to_write; i++) {
					dest[i] = synth->audio_buffer[synth->buffer_pos + i];
				}

				synth->volume_adjusting.store(false);

				synth->buffer_pos += samples_to_write;

				if(synth->buffer_pos == synth->buffer_length) {
					synth->Stop();
				}

			} else {

				while(synth->volume_adjusting.exchange(true));

				for(int i = 0; i < len_; i++) {
					dest[i] = synth->audio_buffer[synth->buffer_pos%synth->buffer_length];
					synth->buffer_pos = (synth->buffer_pos+1) % synth->buffer_length;
				}

				synth->volume_adjusting.store(false);

			}
		}
	}
}
