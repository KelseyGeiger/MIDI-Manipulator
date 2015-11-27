#include "StringSynth.hpp"

namespace geiger {
	namespace midi {

		StringSynth::StringSynth(float L, float ten, float mu, float gamma) : parameter_change(false)
		{
			tension = ten;
			length = L;
			linear_density = mu;
			damping_ratio = gamma;

			mass = linear_density * length;
			velocity = std::sqrt(tension / linear_density);
			fundamental_frequency = velocity / (2 * length);
			float natural_frequency = 2 * M_PI * fundamental_frequency;
			spring_constant = (natural_frequency * natural_frequency * mass);

			volume = 0.1f;
			distance_struck = 0.0f;
			initial_offset = 0.0f;
			number_of_harmonics = 12;
			time_elapsed = 0.0f;
		}

		StringSynth::~StringSynth()
		{
			Stop();
		}

		void StringSynth::SetHarmonicCount(uint32_t harmonics) {
			while(parameter_change.exchange(true));

			number_of_harmonics = harmonics;

			parameter_change.store(false);
		}

		void StringSynth::Pluck(float dist, float offset) {
			while(parameter_change.exchange(true));

			distance_struck = dist;
			initial_offset = offset;
			time_elapsed = 0.0f;

			parameter_change.store(false);
		}

		void StringSynth::Strike(float dist, float force) {
			while(parameter_change.exchange(true));

			distance_struck = dist;
			initial_offset = (force / spring_constant);
			time_elapsed = 0.0f;

			parameter_change.store(false);
		}

		float StringSynth::Amplitude(float t) {

			if(t < 0.0f) {
				return 0.0f;
			}

			float total_amplitude = 0.0f;

			for(uint32_t i = 0; i < number_of_harmonics; i++) {
				uint32_t j = i + 1;
				total_amplitude += HarmonicAmplitude(j) * std::cos(2.0f * M_PI * HarmonicFrequency(j) * t) * std::exp(-damping_ratio * j * t);
			}

			return volume * (total_amplitude / initial_offset);
		}

		SoundSample StringSynth::GenerateSample(uint32_t sample_rate, uint32_t duration_milliseconds, int32_t offset_milliseconds) {
			SoundSample sample{sample_rate, duration_milliseconds};

			float dt = 1.0f / (float)(sample_rate);
			float offset_t = ((float)(sample_rate) / 1000.0f) * offset_milliseconds;

			for(int i = 0; i < sample.buffer_length; i++) {
				sample.audio_buffer[i] = Amplitude(i*dt + offset_t);
			}

			return sample;
		}

		void StringSynth::Pause() {
			if(!stopped) {
				SDL_PauseAudioDevice(device_ID, 1);
				paused = true;
			}
		}

		void StringSynth::Unpause() {
			if(!stopped) {
				SDL_PauseAudioDevice(device_ID, 0);
				paused = false;
			}
		}

		void StringSynth::Play() {
			SDL_AudioSpec want;
			want.freq = 44100;
			want.format = AUDIO_F32SYS;
			want.channels = 1;
			want.samples = 4096;
			want.callback = stringsynth_callback;
			want.userdata = (void*)(this);

			device_ID = SDL_OpenAudioDevice(NULL, 0, &want, &specification, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE);
			SDL_PauseAudioDevice(device_ID, 0);
			paused = false;
			stopped = false;
		}

		void StringSynth::Stop() {
			SDL_PauseAudioDevice(device_ID, 1);
			SDL_CloseAudioDevice(device_ID);
			stopped = true;
			paused = false;
			time_elapsed = 0.0f;
			distance_struck = 0.0f;
			initial_offset = 0.0f;
		}

		void StringSynth::SetVolume(float percent) {
			volume = percent;
		}

		float StringSynth::GetVolume() const {
			return volume;
		}

		float StringSynth::HarmonicAmplitude(uint32_t harmonic) {
			float numer = 2.0f * initial_offset * (length * length);
			float denom = (M_PI * M_PI) * (harmonic * harmonic) * (distance_struck * (length - distance_struck));
			float factor = std::sin(harmonic * M_PI * (distance_struck / length));

			return (numer / denom) * factor;
		}

		float StringSynth::HarmonicFrequency(uint32_t harmonic) {
			return harmonic * fundamental_frequency;
		}

		void stringsynth_callback(void* synth_, Uint8* stream_, int len_) {
			StringSynth* synth = (StringSynth*)(synth_);

			float dt = 1.0f / (float)(synth->specification.freq);

			if(synth->specification.format != AUDIO_F32SYS) {
				if(synth->specification.format == AUDIO_S16SYS) {
					uint16_t* stream = (uint16_t*)(stream_);

					len_ = len_ / 2;

					for(int i = 0; i < len_; i++) {
						stream[i] = (int16_t)(synth->Amplitude(synth->time_elapsed) * std::numeric_limits<int16_t>::max());
						synth->time_elapsed += dt;
					}
				}
			}

			float* stream = (float*)(stream_);

			len_ = len_ / sizeof(float);

			while(synth->parameter_change.exchange(true));

			for(int i = 0; i < len_; i++) {
				stream[i] = synth->Amplitude(synth->time_elapsed);
				synth->time_elapsed += dt;
			}

			synth->parameter_change.store(false);
		}

	}
}
