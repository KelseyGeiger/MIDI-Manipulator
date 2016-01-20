#ifndef MIDI_VLQ_HPP
#define MIDI_VLQ_HPP

#include <cstdint>

namespace geiger {
	namespace midi {

		typedef uint8_t byte;

		struct vlq_buf {
			byte vlq[4];

			inline byte& operator[](int i) { return vlq[i]; }
			inline const byte& operator[](int i) const { return vlq[i]; }
		};

		class MIDI_VLQ
		{
			public:

				MIDI_VLQ();
				MIDI_VLQ(const MIDI_VLQ& vlq) = default;
				MIDI_VLQ(MIDI_VLQ&& vlq) = default;
				MIDI_VLQ(const byte* stream);
				MIDI_VLQ(int value);
				MIDI_VLQ(uint32_t value);
                MIDI_VLQ(vlq_buf vlq);

                MIDI_VLQ& operator=(const MIDI_VLQ& vlq) = default;
                MIDI_VLQ& operator=(MIDI_VLQ&& vlq) = default;

				operator uint32_t() const;
				vlq_buf ToVLQBytes() const;

				unsigned int Length() const;

				inline bool operator>(MIDI_VLQ other) const { return complete > other.complete; }
				inline bool operator<(MIDI_VLQ other) const { return complete < other.complete; }
				inline bool operator==(MIDI_VLQ other) const { return complete == other.complete; }
                inline bool operator!=(MIDI_VLQ other) const { return complete != other.complete; }
                inline bool operator>=(MIDI_VLQ other) const { return complete >= other.complete; }
                inline bool operator<=(MIDI_VLQ other) const { return complete <= other.complete; }

			private:
				uint32_t complete;
				unsigned int bytes_read;
		};

	}
}

#endif // MIDI_VLQ_HPP
