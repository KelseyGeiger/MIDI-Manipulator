#include "StringSynth.hpp"

namespace geiger {
	namespace midi {

		StringSynth::StringSynth()
		{
			active_length = 0.6069f;
			linear_density = 0.002f;
			damping_ratio = 1.0f;

			mass = linear_density * active_length;

			TuneToFrequency(110.0f);

			max_amplitude = 0.0f;
			volume = 0.5f;

			distance_struck = 0.5f * active_length;
			initial_offset = 0.0f;
			number_of_harmonics = 18;
			time_elapsed = 0.0f;

			paused = false;
			stopped = true;
		}

		StringSynth::StringSynth(float L, float ten, float mu, float gamma)
		{
			tension = ten;
			active_length = L;
			linear_density = mu;
			damping_ratio = gamma;

			mass = linear_density * active_length;
			velocity = std::sqrt(tension / linear_density);
			fundamental_frequency = velocity / (2 * active_length);
			float natural_frequency = 2 * M_PI * fundamental_frequency;
			spring_constant = (natural_frequency * natural_frequency * mass);

			max_amplitude = 0.0f;
			volume = 0.5f;
			distance_struck = 0.0f;
			initial_offset = 0.0f;
			number_of_harmonics = 15;
			time_elapsed = 0.0f;

			paused = false;
			stopped = true;
		}

		StringSynth::~StringSynth()
		{
			Stop();
		}

		void StringSynth::SetHarmonicCount(uint32_t harmonics) {

			number_of_harmonics = harmonics;

		}

		void StringSynth::SetActiveLength(float len) {
			if(len <= 0.0f) {
				return;
			}

			active_length = len;

			mass = linear_density * active_length;
			fundamental_frequency = velocity / (2 * active_length);
			float natural_frequency = 2 * M_PI * fundamental_frequency;
			spring_constant = (natural_frequency * natural_frequency * mass);

		}

		void StringSynth::SetTension(float ten) {
			if(ten <= 0.0f) {
				return;
			}

			tension = ten;

			velocity = std::sqrt(tension / linear_density);

		}

		void StringSynth::SetLinearDensity(float mu) {
			if(mu <= 0.0f) {
				return;
			}

			linear_density = mu;

			mass = linear_density * active_length;
			velocity = std::sqrt(tension / linear_density);
			fundamental_frequency = velocity / (2 * active_length);
			float natural_frequency = 2 * M_PI * fundamental_frequency;
			spring_constant = (natural_frequency * natural_frequency * mass);
		}

		void StringSynth::SetDampingRatio(float gamma) {
            if(gamma < 0.0f) {
				return;
            }

			damping_ratio = gamma;
		}

		uint32_t StringSynth::GetHarmonicCount() const {

			return number_of_harmonics;
		}

		float StringSynth::GetActiveLength() const {
			return active_length;
		}

		float StringSynth::GetTension() const {
			return tension;
		}

		float StringSynth::GetLinearDensity() const {
			return linear_density;
		}

		float StringSynth::GetDampingRatio() const {
			return damping_ratio;
		}

		void StringSynth::TuneToNote(Note n) {
			TuneToFrequency(NoteToFrequency(n));
		}

		void StringSynth::TuneToFrequency(float freq) {
            fundamental_frequency = freq;
            float natural_frequency = 2 * M_PI * fundamental_frequency;
			spring_constant = (natural_frequency * natural_frequency * mass);
            velocity = 2.0f * active_length * fundamental_frequency;

            tension = linear_density * (velocity * velocity);
		}

		void StringSynth::Pluck(float dist, float offset) {
			if(dist <= 0.0f || dist >= active_length) {
				return;
			}

			distance_struck = dist;
			initial_offset = offset;
			time_elapsed = 0.0f;
		}

		void StringSynth::Strike(float dist, float force) {

			if(dist <= 0.0f || dist >= active_length) {
				return;
			}

			distance_struck = dist;
			initial_offset = (force / spring_constant);
			time_elapsed = 0.0f;
		}

		void StringSynth::Silence() {
			distance_struck = 0.5f * active_length;
			initial_offset = 0.0f;
		}

		float StringSynth::Value(float t) {

			if(t < 0.0f) {
				return 0.0f;
			}

			float total_amplitude = 0.0f;

			for(uint32_t i = 0; i < number_of_harmonics; i++) {
				uint32_t j = i + 1;
				total_amplitude += HarmonicAmplitude(j) * std::cos(2.0f * M_PI * HarmonicFrequency(j) * t) * std::exp(-damping_ratio * j * t);
			}

			if(total_amplitude > max_amplitude) {
				max_amplitude = total_amplitude;
			}

			float result = volume * (total_amplitude / max_amplitude);

			return result;
		}

		SoundSample StringSynth::GenerateSample(uint32_t sample_rate, uint32_t duration_milliseconds, int32_t offset_milliseconds) {
			SoundSample sample{sample_rate, duration_milliseconds};

			float dt = 1.0f / (float)(sample_rate);
			float offset_t = ((float)(sample_rate) / 1000.0f) * offset_milliseconds;

			for(uint32_t i = 0; i < sample.buffer_length; i++) {
				sample.audio_buffer[i] = Value(i*dt + offset_t);
			}

			return sample;
		}

		void StringSynth::PlayNote(Note n) {
			float old_freq = fundamental_frequency;

			float frequency = NoteToFrequency(n);

			TuneToFrequency(frequency);

			Play();
			Pluck(0.5f * active_length, 0.05f);

			std::this_thread::sleep_for(std::chrono::milliseconds(2000));

			Stop();

			TuneToFrequency(old_freq);
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
			time_elapsed = -1.0f;
			distance_struck = 0.0f;
			initial_offset = 0.0f;
		}

		void StringSynth::SetVolume(float percent) {
			if(percent >= 0.0f) {
				volume = percent;
			} else {
                volume = -percent;
			}
		}

		float StringSynth::GetVolume() const {
			return volume;
		}

		float StringSynth::HarmonicAmplitude(uint32_t harmonic) {
			float numer = 2.0f * initial_offset * (active_length * active_length);
			float denom = (M_PI * M_PI) * (harmonic * harmonic) * (distance_struck * (active_length - distance_struck));
			float factor = std::sin(harmonic * M_PI * (distance_struck / active_length));

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
						stream[i] = (int16_t)(synth->Value(synth->time_elapsed) * std::numeric_limits<int16_t>::max());
						synth->time_elapsed += dt;
					}
				}
			}

			float* stream = (float*)(stream_);

			len_ = len_ / sizeof(float);

			for(int i = 0; i < len_; i++) {
				stream[i] = synth->Value(synth->time_elapsed);
				synth->time_elapsed += dt;
			}
		}

	}
}
