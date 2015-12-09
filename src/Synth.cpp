#include "Synth.hpp"

namespace geiger {
	namespace midi {

		SoundSample::SoundSample() {
			audio_buffer = nullptr;
			buffer_length = 0;
			sample_rate = 0;
			duration_milliseconds = 0;
		}

		SoundSample::SoundSample(uint32_t rate, uint32_t dur_milli) {

			if(dur_milli == 0 || rate == 0) {
				audio_buffer = nullptr;
				buffer_length = 0;
				sample_rate = 0;
				duration_milliseconds = 0;
				return;
			}

			sample_rate = rate;
			duration_milliseconds = dur_milli;
			float sample_per_milli = (float)(sample_rate) / 1000.0f;
			float sample_count = sample_per_milli * dur_milli;

			buffer_length = (uint32_t)(sample_count);
			if(sample_count > buffer_length) {
				buffer_length++;
			}

			audio_buffer = new float[buffer_length];

			for(uint32_t i = 0; i < buffer_length; i++) {
				audio_buffer[i] = 0.0f;
			}
		}

		SoundSample::SoundSample(const SoundSample& other) {
			buffer_length = other.buffer_length;
			sample_rate = other.sample_rate;
			duration_milliseconds = other.duration_milliseconds;

			audio_buffer = new float[buffer_length];

			for(uint32_t i = 0; i < buffer_length; i++) {
				audio_buffer[i] = other.audio_buffer[i];
			}
		}

		SoundSample::SoundSample(SoundSample&& other) {
			buffer_length = other.buffer_length;
			sample_rate = other.sample_rate;
			duration_milliseconds = other.duration_milliseconds;

			audio_buffer = other.audio_buffer;
			other.audio_buffer = nullptr;
		}

        SoundSample::~SoundSample() {
			if(audio_buffer) {
				delete[] audio_buffer;
			}
		}

        SoundSample& SoundSample::operator=(const SoundSample& s) {
			buffer_length = s.buffer_length;
			sample_rate = s.sample_rate;
			duration_milliseconds = s.duration_milliseconds;

			audio_buffer = new float[buffer_length];

			for(uint32_t i = 0; i < buffer_length; i++) {
				audio_buffer[i] = s.audio_buffer[i];
			}

			return *this;
        }

		SoundSample& SoundSample::operator=(SoundSample&& s) {
			buffer_length = s.buffer_length;
			sample_rate = s.sample_rate;
			duration_milliseconds = s.duration_milliseconds;

			audio_buffer = s.audio_buffer;
			s.audio_buffer = nullptr;

			return *this;
		}

		SoundSample SoundSample::operator+(SoundSample other) {
			//always choose the higher quality sound
			uint32_t new_rate = (sample_rate > other.sample_rate) ? sample_rate : other.sample_rate;

			//choose the longer clip- we're adding samples together, after all
			uint32_t new_dur = (duration_milliseconds > other.duration_milliseconds) ?
                                duration_milliseconds :
								other.duration_milliseconds;

			SoundSample sum{new_rate, new_dur};

			for(uint32_t i = 0; i < sum.buffer_length; i++) {
				//convert a sample number to a specific time after the start of the sample
				float t = (float)(i) / (float)(sum.sample_rate);

				//use the calculated time in order to add together the two samples at the correct period
				uint32_t this_index = (uint32_t)(t * sample_rate);
				uint32_t other_index = (uint32_t)(t * other.sample_rate);

				sum.audio_buffer[i] = audio_buffer[this_index];

				if(other_index < other.buffer_length) {
					sum.audio_buffer[i] = other.audio_buffer[other_index];
				}
			}

			return sum;
		}

		SoundSample SoundSample::operator-(SoundSample other) {
			uint32_t new_rate = (sample_rate < other.sample_rate) ? sample_rate : other.sample_rate;
			uint32_t new_dur = (duration_milliseconds < other.duration_milliseconds) ?
                                duration_milliseconds :
								other.duration_milliseconds;

			SoundSample difference{new_rate, new_dur};

			for(uint32_t i = 0; i < difference.buffer_length; i++) {
				float t = (float)(i) / (float)(difference.sample_rate);
				uint32_t this_index = (uint32_t)(t * sample_rate);
				uint32_t other_index = (uint32_t)(t * other.sample_rate);

				difference.audio_buffer[i] = audio_buffer[this_index] - other.audio_buffer[other_index];
			}

			return difference;
		}

		SoundSample& SoundSample::operator+=(SoundSample other) {
			for(uint32_t i = 0; i < buffer_length; i++) {
				float t = (float)(i) / (float)(sample_rate);
				uint32_t other_index = (uint32_t)(t * other.sample_rate);

				audio_buffer[i] = audio_buffer[i] + other.audio_buffer[other_index];
			}

			return *this;
		}

		SoundSample& SoundSample::operator-=(SoundSample other) {
			for(uint32_t i = 0; i < buffer_length; i++) {
				float t = (float)(i) / (float)(sample_rate);
				uint32_t other_index = (uint32_t)(t * other.sample_rate);

				audio_buffer[i] = audio_buffer[i] - other.audio_buffer[other_index];
			}

			return *this;
		}

		float& SoundSample::operator[](size_t index) {
			return audio_buffer[index];
		}

		const float& SoundSample::operator[](size_t index) const {
            return audio_buffer[index];
		}

		Note::Note() : note(A), acc(NATURAL), octave(4) {}
		Note::Note(BASE_NOTE n, ACCIDENTAL a, int8_t oct) : note(n), acc(a), octave(oct) {}

		Chord::Chord() {}

		Chord::Chord(std::initializer_list<Note> notes) {
			for(Note n : notes) {
				this->notes.push_back(n);
			}
		}

		void Chord::AddNote(Note n) {
			notes.push_back(n);
		}

		float NoteToFrequency(Note n) {
            static Note STANDARD_PITCH = Note{};

            int half_step_difference = ((n.note + n.acc) - (STANDARD_PITCH.note + STANDARD_PITCH.acc));
            int octave_difference = n.octave - STANDARD_PITCH.octave;

            float frequency = 440.0f * std::pow(2.0f, (octave_difference * 12.0f + half_step_difference) / 12.0f);

            return frequency;
		}

		Note FrequencyToClosestNote(float freq) {
            freq = freq / 440.0f;
            freq = std::log(freq) / std::log(2.0f);
            freq = freq * 12.0f;
            int8_t octave_difference = (int8_t)(freq) / 12;

            int half_step_difference;

            if(octave_difference != 0) {
				half_step_difference = (int)(freq) / octave_difference;
            } else {
				half_step_difference = (int)(freq) - Note::A;
            }

            Note n;
            n.octave = 4 + octave_difference;

            if(half_step_difference > -5 && half_step_difference < 3) {
				int note_offset = (half_step_difference / 2);
				n.note = (Note::BASE_NOTE)(Note::A + (note_offset * 2));

				if(half_step_difference < 0) {
					n.acc = (Note::ACCIDENTAL)((-half_step_difference) % 2);
					n.note = (Note::BASE_NOTE)(n.note - 2);
				} else if(half_step_difference > 0){
					n.acc = (Note::ACCIDENTAL)(-(half_step_difference % 2));
					n.note = (Note::BASE_NOTE)(n.note + 2);
				} else {
					n.acc = (Note::ACCIDENTAL)(0);
				}

            } else {
				int note_offset = ((half_step_difference - 1) / 2) - 1;
				n.note = (Note::BASE_NOTE)(Note::A + (note_offset * 2) + 1);
				n.acc = (Note::ACCIDENTAL)(-(half_step_difference + 1) % 2);
            }

            return n;
		}

	}
}
