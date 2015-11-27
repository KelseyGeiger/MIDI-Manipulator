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
            uint32_t new_rate = (sample_rate < other.sample_rate) ? sample_rate : other.sample_rate;
            uint32_t new_dur = (duration_milliseconds < other.duration_milliseconds) ?
                                duration_milliseconds :
								other.duration_milliseconds;

			SoundSample sum{new_rate, new_dur};

			for(uint32_t i = 0; i < sum.buffer_length; i++) {
				float t = (float)(i) / (float)(sum.sample_rate);
				uint32_t this_index = (uint32_t)(t * sample_rate);
				uint32_t other_index = (uint32_t)(t * other.sample_rate);

				sum.audio_buffer[i] = audio_buffer[this_index] + other.audio_buffer[other_index];
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

	}
}
