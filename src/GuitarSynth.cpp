#include "GuitarSynth.hpp"

namespace geiger {
	namespace midi {

		GuitarSynth::GuitarSynth() {
			max_length = 0.6477f;
			string_density = 0.002f;
			damping_ratio = 1.5f;
			max_amplitude = 1.0f;
            time_elapsed = 0.0f;
            volume = 1.0f;

			for(uint32_t i = 0; i < 6; i++) {
				strings[i].SetActiveLength(max_length);
				strings[i].SetLinearDensity(string_density);
				strings[i].SetDampingRatio(damping_ratio);
				time_offsets[i] = 0.0f;
            }

			strings[0].TuneToFrequency(329.63f);
			strings[1].TuneToFrequency(246.94f);
			strings[2].TuneToFrequency(196.00f);
			strings[3].TuneToFrequency(146.83f);
			strings[4].TuneToFrequency(110.00f);
            strings[5].TuneToFrequency(82.41f);


            Strum();
            this->max_amplitude = Value(0.0f) / volume;

            for(uint32_t i = 0; i < 6; i++) {
				strings[i].Silence();
            }

            paused = false;
            stopped = true;
		}

		GuitarSynth::GuitarSynth(float length_meters, float linear_density, float damping) {
			max_length = length_meters;
			string_density = linear_density;
			damping_ratio = damping;
			max_amplitude = 1.0f;
            volume = 1.0f;
            time_elapsed = 0.0f;

			for(uint32_t i = 0; i < 6; i++) {
				strings[i].SetActiveLength(max_length);
				strings[i].SetLinearDensity(string_density);
				strings[i].SetDampingRatio(damping_ratio);
				time_offsets[i] = 0.0f;
            }

			strings[0].TuneToFrequency(329.63f);
			strings[1].TuneToFrequency(246.94f);
			strings[2].TuneToFrequency(196.00f);
			strings[3].TuneToFrequency(146.83f);
			strings[4].TuneToFrequency(110.00f);
            strings[5].TuneToFrequency(82.41f);

            Strum();
            this->max_amplitude = Value(0.0f) / volume;

            for(uint32_t i = 0; i < 6; i++) {
				strings[i].Silence();
            }

            paused = false;
            stopped = true;
		}

		GuitarSynth::~GuitarSynth() {
			Stop();
		}

		void GuitarSynth::SetStringLength(float length_meters) {

			max_length = length_meters;

			for(uint32_t i = 0; i < 6; i++) {
				strings[i].SetActiveLength(max_length);
			}

		}

		void GuitarSynth::SetStringDensity(float linear_density) {

			string_density = linear_density;

			for(uint32_t i = 0; i < 6; i++) {
				strings[i].SetLinearDensity(string_density);
			}

		}

		void GuitarSynth::SetDampingRatio(float damp) {

			damping_ratio = damp;

			for(uint32_t i = 0; i < 6; i++) {
				strings[i].SetDampingRatio(damping_ratio);
			}

		}

		float GuitarSynth::GetStringLength() const {
			return max_length;
		}

		float GuitarSynth::GetStringDensity() const {
			return string_density;
		}

		float GuitarSynth::GetDampingRatio() const {
			return damping_ratio;
		}

		void GuitarSynth::Strum(float time_offset_seconds) {

			for(uint32_t i = 0; i < 6; i++) {
				strings[i].Pluck(0.23f * max_length, 0.01f);
				time_offsets[i] = time_elapsed + time_offset_seconds;
			}

		}

		void GuitarSynth::PluckString(uint32_t string_, float time_offset_seconds) {
			if(string_ > 6 || string_ == 0) {
				return;
			}

			uint32_t idx = string_-1;

            strings[idx].Pluck(0.23f * max_length, 0.01f);
            time_offsets[idx] = time_elapsed + time_offset_seconds;

		}

		void GuitarSynth::StopString(uint32_t string_) {
			if(string_ > 6 || string_ == 0) {
				return;
			}

			uint32_t idx = string_-1;

			strings[idx].Silence();
		}

		void GuitarSynth::FretString(Note n, uint32_t string_) {
			if(string_ > 6 || string_ == 0) {
				return;
			}

			float length = GetLengthForNote(string_, n);

			if(length < max_length && length > 0.23 * max_length) {
				strings[string_-1].SetActiveLength(length);
			}
		}

		void GuitarSynth::OpenString(uint32_t string_) {
			if(string_ > 6 || string_ == 0) {
				return;
			}

			strings[string_-1].SetActiveLength(max_length);
		}

		float GuitarSynth::Value(float t) {

			if(t < 0.0f) {
				return 0.0f;
			}

			float total_amplitude = 0.0f;

			for(uint32_t i = 0; i < 6; i++) {
				float string_amp = (strings[i].Value(t - time_offsets[i]));
				total_amplitude += string_amp;
			}

			if(total_amplitude > max_amplitude) {
				this->max_amplitude = total_amplitude;
			}

			float result = volume * (total_amplitude / this->max_amplitude);

			return result;
		}

		SoundSample GuitarSynth::GenerateSample(uint32_t sample_rate, uint32_t duration_milliseconds, int32_t offset_milliseconds) {
            SoundSample complete(sample_rate, duration_milliseconds);

			float dt = 1.0f / (float)(sample_rate);
			float offset_t = ((float)(sample_rate) / 1000.0f) * offset_milliseconds;
			offset_t += (time_elapsed * 1000.0f);

			for(uint32_t i = 0; i < complete.buffer_length; i++) {
				complete.audio_buffer[i] += Value(i*dt + time_elapsed);
			}

            return complete;
		}

		void GuitarSynth::PlayNote(Note n) {
			const int min_semitone = (2 * 12) + (int)(Note::E);
			const int max_semitone = (5 * 12) + (int)(Note::B);

			int semitone = (n.octave * 12) + (int)(n.note) + (int)(n.acc);

			if(semitone < min_semitone || semitone > max_semitone) {
				return;
			}

			float length = 0.0f;
			uint32_t string_ = 0;

			for(uint32_t i = 1; i <= 6; i++) {
				length = GetLengthForNote(i, n);

				if(length < max_length && length > 0.23 * max_length) {
					string_ = i;
					break;
				}
			}

			//sort of simulates holding down the string on a fret
            strings[string_-1].SetActiveLength(length);
            PluckString(string_);
		}

		void GuitarSynth::PlayChord(Chord c) {
			const int min_semitone = (2 * 12) + (int)(Note::E);
			const int max_semitone = (5 * 12) + (int)(Note::B);

			for(const Note& n : c.notes) {

				int semitone = (n.octave * 12) + (int)(n.note) + (int)(n.acc);

				if(semitone < min_semitone || semitone > max_semitone || c.notes.size() > 6) {
					return;
				}
			}

			bool string_taken[6] = { false };
			float lengths[6] = { 0.0f };
			uint32_t string_indices[6] = { 0 };
			uint32_t strings_used = 0;

			for(uint32_t i = 0; i < c.notes.size(); i++) {

				Note n = c.notes[i];

				for(uint32_t j = 1; j <= 6; j++) {
					lengths[strings_used] = GetLengthForNote(j, n);

					if(lengths[strings_used] < max_length && lengths[strings_used] > 0.23 * max_length && !string_taken[j-1]) {
						string_indices[strings_used] = j;
						string_taken[j-1] = true;
						strings_used++;
						break;
					}
				}

				if(strings_used <= i) {
					return;
				}
			}

			for(uint32_t i = 0; i < strings_used; i++) {
				uint32_t string_ = string_indices[i];

				//sort of simulates holding down the string on a fret
				strings[string_-1].SetActiveLength(lengths[i]);
				PluckString(string_);
			}
		}

		void GuitarSynth::Pause() {
			if(!stopped) {
				SDL_PauseAudioDevice(device_ID, 1);
				paused = true;
			}
		}

		void GuitarSynth::Unpause() {
			if(!stopped) {
				SDL_PauseAudioDevice(device_ID, 0);
				paused = false;
			}
		}

		void GuitarSynth::Play() {
			SDL_AudioSpec want;
			want.freq = 44100;
			want.format = AUDIO_F32SYS;
			want.channels = 1;
			want.samples = 8192;
			want.callback = guitar_callback;
			want.userdata = (void*)(this);

			device_ID = SDL_OpenAudioDevice(NULL, 0, &want, &specification, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE);
			SDL_PauseAudioDevice(device_ID, 0);
			paused = false;
			stopped = false;
		}

		void GuitarSynth::Stop() {
			SDL_PauseAudioDevice(device_ID, 1);
			SDL_CloseAudioDevice(device_ID);
			stopped = true;
			paused = false;

			for(uint32_t i = 0; i < 6; i++) {
				strings[i].Silence();
			}

			time_elapsed = 0.0f;
		}

		void GuitarSynth::SetVolume(float percent) {
			if(percent >= 0.0f) {
				volume = percent;
			} else {
				volume = -percent;
			}

			for(uint32_t i = 0; i < 6; i++) {
				strings[i].SetVolume(volume);
			}
		}

		float GuitarSynth::GetVolume() const {
			return volume;
		}

		float GuitarSynth::GetLengthForNote(uint32_t string_, Note n) {

			if(string_ > 6 || string_ == 0) {
				return -1.0f;
			}

			float velocity = std::sqrt(strings[string_-1].GetTension() / string_density);

			float desired_frequency = NoteToFrequency(n);

            return velocity / (2.0f * desired_frequency);
		}

		void guitar_callback(void* synth_, Uint8* stream_, int len_) {
			GuitarSynth* synth = (GuitarSynth*)(synth_);

			float dt = 1.0f / (float)(synth->specification.freq);

			if(synth->time_elapsed < 0.0f) {
				dt = 0.0f;
			}

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
