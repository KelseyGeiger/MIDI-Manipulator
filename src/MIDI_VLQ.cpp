#include "MIDI_VLQ.hpp"

namespace geiger {
	namespace midi {

		MIDI_VLQ::MIDI_VLQ() {
			complete = 0;
			bytes_read = 0;
		}

		MIDI_VLQ::MIDI_VLQ(const byte* stream)
		{
            complete = 0;
            bytes_read = 0;

            int counter = 3;
            int idx = 0;

            while(counter >= 0 && (stream[idx] & 0x80) != 0) {

				complete = (complete << 7) | (stream[idx] & 0x7F);
				counter--;
				idx++;
				bytes_read++;
            }

            if((stream[idx] & 0x80) == 0) {

				complete = (complete << 7) | (stream[idx] & 0x7F);
				bytes_read++;

            }
		}

		MIDI_VLQ::MIDI_VLQ(int value)
		{
			complete = (uint32_t)(value);
			bytes_read = 0;

			while(bytes_read < 4 && value > 0) {
				bytes_read++;
				value = value >> 7;
			}
		}

		MIDI_VLQ::MIDI_VLQ(uint32_t value)
		{
			complete = value;
			bytes_read = 0;

			while(bytes_read < 4 && value > 0) {
				bytes_read++;
				value = value >> 7;
			}
		}

        MIDI_VLQ::MIDI_VLQ(vlq_buf vlq)
        {
			complete = 0;
			bytes_read = 0;

			int counter = 3;
			int idx = 0;

			while(counter >= 0 && (vlq[idx] & 0x80) != 0) {

				complete = (complete << 7) | (vlq[idx] & 0x7F);
				counter--;
				idx++;
				bytes_read++;
			}

			if((vlq[idx] & 0x80) == 0) {

				complete = (complete << 7) | (vlq[idx] & 0x7F);

			}
        }

		MIDI_VLQ::operator uint32_t() const
		{

			return complete;

		}

		vlq_buf MIDI_VLQ::ToVLQBytes() const
		{

			vlq_buf vlq = { { 0, 0, 0, 0 } };

			int counter = 0;
			int idx = 0;
            uint32_t value_copy = complete;

			//find the count of 7-bit bytes (septets) in this value
			while(counter < 4 && (value_copy >> 7) != 0) {

				counter++;
				value_copy = value_copy >> 7;

			}

			while(counter > 0) {

				vlq[idx] = (complete >> (counter * 7)) & 0x7F;
				vlq[idx] |= 0x80;

				idx++;
				counter--;

			}

			vlq[idx] = complete & 0x7F;

			return vlq;
		}

		unsigned int MIDI_VLQ::Length() const
		{
			return bytes_read;
		}

	}
}
