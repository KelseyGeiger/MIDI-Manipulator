#ifndef SYNTH_HPP
#define SYNTH_HPP

#include "SDL2/SDL_audio.h"
#include <cmath>
#include <limits>
#include <atomic>
#include <string>
#include <vector>
#include <chrono>
#include <initializer_list>

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

            float& operator[](size_t index);
            const float& operator[](size_t index) const;
		};

		struct Note {
			enum BASE_NOTE {
				C = 1,
				D = 3,
				E = 5,
				F = 6,
				G = 8,
				A = 10,
				B = 12
			};

			enum ACCIDENTAL {
				SHARP = 1,
				NATURAL = 0,
				FLAT = -1
			};

			Note();
			Note(BASE_NOTE n, ACCIDENTAL a, int8_t oct);

			BASE_NOTE note;
			ACCIDENTAL acc;
			int8_t octave;
		};

		struct Chord {
			std::vector<Note> notes;

			Chord();
			Chord(std::initializer_list<Note> notes);
			void AddNote(Note n);
		};

		float NoteToFrequency(Note n);
		Note FrequencyToNote(float freq);

		class Synth
		{
			public:
				virtual ~Synth() {}

				virtual float Value(float t) = 0;

				virtual SoundSample GenerateSample(uint32_t sample_rate, uint32_t duration_milliseconds, int32_t offset_milliseconds) = 0;

				virtual void PlayNote(Note n) = 0;

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
