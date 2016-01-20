#ifndef MIDI_CHUNK_HPP
#define MIDI_CHUNK_HPP

#include "MIDI_VLQ.hpp"
#include "SDL2/SDL.h"
#include <memory>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>

namespace geiger {
	namespace midi {

		typedef uint8_t byte;

		namespace detail {

			enum class MIDI_FORMAT : uint16_t {
				SINGLE_TRACK = 0,
                MULTI_TRACK_SIMULT = 1,
                MULTI_TRACK_SEQNTL = 2
			};

			struct MIDI_Header {
				MIDI_FORMAT format;
				uint16_t track_count;
				uint16_t divisions;
			};

			struct MIDI_MidiEvent {
                byte MSB;
                byte LSB;

                inline uint32_t Length() const {
                	return sizeof(byte) * 2;
                }
			};

			struct MIDI_SysexEvent {
				byte type;
                MIDI_VLQ length;
                byte* data;

                MIDI_SysexEvent(byte t, MIDI_VLQ vlq, const byte* raw_data);
                MIDI_SysexEvent(const MIDI_SysexEvent& evt);
                MIDI_SysexEvent(MIDI_SysexEvent&& evt);
                ~MIDI_SysexEvent();

                MIDI_SysexEvent& operator=(const MIDI_SysexEvent& evt);
                MIDI_SysexEvent& operator=(MIDI_SysexEvent&& evt);

                inline uint32_t Length() const {
                	return (uint32_t)(length) + length.Length();
                }
			};

			struct MIDI_MetaEvent {
				byte type;
                MIDI_VLQ length;
                byte* data;

                MIDI_MetaEvent(byte t, MIDI_VLQ vlq, const byte* data);
                MIDI_MetaEvent(const MIDI_MetaEvent& evt);
                MIDI_MetaEvent(MIDI_MetaEvent&& evt);
                ~MIDI_MetaEvent();

                MIDI_MetaEvent& operator=(const MIDI_MetaEvent& evt);
                MIDI_MetaEvent& operator=(MIDI_MetaEvent&& evt);

                inline uint32_t Length() const {
                	return sizeof(byte) + (uint32_t)(length) + length.Length();
                }
			};

			struct MIDI_Event {
				byte type;

				MIDI_Event();
				MIDI_Event(const MIDI_Event& e);
				MIDI_Event(MIDI_Event&& e);
				MIDI_Event(const byte* data, bool running_status = false);
				~MIDI_Event();

				MIDI_Event& operator=(const MIDI_Event& e);
				MIDI_Event& operator=(MIDI_Event&& e);

				union Event {

					Event();
					~Event();

					MIDI_MidiEvent midi_event;
					MIDI_SysexEvent sysex_event;
					MIDI_MetaEvent meta_event;
				} data;

				bool IsMidiEvent() const;
				bool IsSysexEvent() const;
				bool IsMetaEvent() const;

				inline uint32_t Length() const {
					switch(type)
					{
						case 0xFF:
							{
								return sizeof(byte) + data.meta_event.Length();
							}
							break;
						case 0xF0:
						case 0xF7:
							{
								return sizeof(byte) + data.sysex_event.Length();
							}
							break;
						default:
							{
								if(type & 0x80 || type & 0x90 ||
								   type & 0xA0 || type & 0xB0 ||
								   type & 0xC0 || type & 0xD0 ||
								   type & 0xE0)
								{
									return sizeof(byte) + data.midi_event.Length();
								} else if(type == 0) {
									return data.midi_event.Length();
								}
							}
					}

					return 0;
				}

				friend std::ostream& operator<<(std::ostream& os, const MIDI_Event& evt);
			};

			struct MIDI_Message {
				MIDI_VLQ delta_ticks;
				MIDI_Event event;

				MIDI_Message();
				MIDI_Message(const MIDI_VLQ& dt, const MIDI_Event& evt);
				MIDI_Message(const MIDI_Message& m);
				MIDI_Message(MIDI_Message&& m);

				~MIDI_Message();

				MIDI_Message& operator=(const MIDI_Message& m);
				MIDI_Message& operator=(MIDI_Message&& m);

                inline uint32_t Length() const {
                	return delta_ticks.Length() + event.Length();
                }
			};

			struct MIDI_Track {
				std::vector<MIDI_Message> data;

				MIDI_Track();
				MIDI_Track(const MIDI_Track& mtrk);
				MIDI_Track(MIDI_Track&& mtrk);
				MIDI_Track& operator=(const MIDI_Track& mtrk);
				MIDI_Track& operator=(MIDI_Track&& mtrk);

				inline uint32_t Length() const {
					uint32_t sum = 0;
					for(auto& msg : data) {
						sum += msg.Length();
					}

					return sum;
				}
			};
		}

		class MIDI_Chunk
		{
			public:

				MIDI_Chunk();
				MIDI_Chunk(byte* raw_data);
				MIDI_Chunk(const char* type, uint32_t data_len, byte* data);
				MIDI_Chunk(detail::MIDI_Header header);
				MIDI_Chunk(uint32_t length, detail::MIDI_Track track);

				MIDI_Chunk(const MIDI_Chunk& chunk);
				MIDI_Chunk(MIDI_Chunk&& chunk);

				~MIDI_Chunk();

				MIDI_Chunk& operator=(const MIDI_Chunk& chunk);
				MIDI_Chunk& operator=(MIDI_Chunk&& chunk);

				uint32_t Length() const;

				bool IsHeader() const;
				bool IsTrack() const;

				const detail::MIDI_Header& GetHeader() const;
				const detail::MIDI_Track& GetTrack() const;

				detail::MIDI_Header& GetHeader();
				detail::MIDI_Track& GetTrack();

				friend std::istream& operator>>(std::istream& is, MIDI_Chunk& chnk);
				friend std::ostream& operator<<(std::ostream& os, const MIDI_Chunk& chnk);

			private:
				char type[4];
				uint32_t length;

				void DecodeData(const byte* data);

				union {
					detail::MIDI_Header header;
					detail::MIDI_Track track;
				};
		};

	}
}

#endif // MIDI_CHUNK_HPP
