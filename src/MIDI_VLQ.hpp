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
				MIDI_VLQ(byte* stream);
				MIDI_VLQ(uint32_t value);
                MIDI_VLQ(vlq_buf vlq);

				operator uint32_t() const;

				vlq_buf ToVLQBytes() const;

			private:
				uint32_t complete;

		};

	}
}

#endif // MIDI_VLQ_HPP
