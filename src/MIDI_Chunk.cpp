#include "MIDI_Chunk.hpp"

namespace geiger
{
	namespace midi
	{

		namespace detail
		{

			MIDI_SysexEvent::MIDI_SysexEvent(byte t, MIDI_VLQ vlq, const byte* raw_data)
			{
				type = t;
                length = vlq;
                if((uint32_t)(length) > 0) {
					data = new byte[(uint32_t)length]{0};
					for(uint32_t i = 0; i < (uint32_t)length; i++) {
						data[i] = raw_data[i];
					}
				} else {
					data = nullptr;
				}
			}

            MIDI_SysexEvent::MIDI_SysexEvent(const MIDI_SysexEvent& evt)
            {
				type = evt.type;
				length = evt.length;
				if((uint32_t)(length) > 0) {
					data = new byte[(uint32_t)length]{0};
					for(uint32_t i = 0; i < (uint32_t)length; i++) {
						data[i] = evt.data[i];
					}
				} else {
					data = nullptr;
				}
            }

            MIDI_SysexEvent::MIDI_SysexEvent(MIDI_SysexEvent&& evt)
            {
				type = evt.type;
				evt.type = 0;

				length = evt.length;
				evt.length = MIDI_VLQ(0);

				data = evt.data;
				evt.data = nullptr;
            }

            MIDI_SysexEvent::~MIDI_SysexEvent()
            {
            	if(data) {
					delete[] data;
            	}
            }

            MIDI_SysexEvent& MIDI_SysexEvent::operator=(const MIDI_SysexEvent& evt)
            {
				type = evt.type;
				length = evt.length;
				if((uint32_t)(length) > 0) {
					data = new byte[(uint32_t)length]{0};
					for(uint32_t i = 0; i < (uint32_t)length; i++) {
						data[i] = evt.data[i];
					}
				} else {
					data = nullptr;
				}

				return *this;
            }

            MIDI_SysexEvent& MIDI_SysexEvent::operator=(MIDI_SysexEvent&& evt)
            {
				type = evt.type;
				evt.type = 0;

				length = evt.length;
				evt.length = MIDI_VLQ(0);

				data = evt.data;
				evt.data = nullptr;

				return *this;
            }


			MIDI_MetaEvent::MIDI_MetaEvent(byte t, MIDI_VLQ vlq, const byte* raw_data)
			{
				type = t;
				length = vlq;
				if((uint32_t)(length) > 0) {
					data = new byte[(uint32_t)length]{0};
					for(uint32_t i = 0; i < (uint32_t)length; i++) {
						data[i] = raw_data[i];
					}
				} else {
					data = nullptr;
				}
			}

            MIDI_MetaEvent::MIDI_MetaEvent(const MIDI_MetaEvent& evt)
            {
				type = evt.type;
				length = evt.length;
				if((uint32_t)(length) > 0) {
					data = new byte[(uint32_t)length]{0};
					for(uint32_t i = 0; i < (uint32_t)length; i++) {
						data[i] = evt.data[i];
					}
				} else {
					data = nullptr;
				}
            }

            MIDI_MetaEvent::MIDI_MetaEvent(MIDI_MetaEvent&& evt)
            {
				type = evt.type;
				evt.type = 0;

				length = evt.length;
				evt.length = MIDI_VLQ(0);

				data = evt.data;
				evt.data = nullptr;
            }

            MIDI_MetaEvent::~MIDI_MetaEvent()
            {
            	if(data) {
					delete[] data;
            	}
            }

            MIDI_MetaEvent& MIDI_MetaEvent::operator=(const MIDI_MetaEvent& evt)
            {
				type = evt.type;
				length = evt.length;
				if((uint32_t)(length) > 0) {
					data = new byte[(uint32_t)length]{0};
					for(uint32_t i = 0; i < (uint32_t)length; i++) {
						data[i] = evt.data[i];
					}
				} else {
					data = nullptr;
				}

				return *this;
            }

            MIDI_MetaEvent& MIDI_MetaEvent::operator=(MIDI_MetaEvent&& evt)
            {
				type = evt.type;
				evt.type = 0;

				length = evt.length;
				evt.length = MIDI_VLQ(0);

				data = evt.data;
				evt.data = nullptr;

				return *this;
            }

			MIDI_Event::MIDI_Event()
			{
				type = 0x70;
			}

			MIDI_Event::MIDI_Event(const byte* raw_data, bool running_status)
			{
				const byte* position = raw_data;
				type = *position;
				position++;

				switch(type) {
					case 0xFF:
						{
							byte t = *position;
							position++;
							MIDI_VLQ vlq = MIDI_VLQ(position);
							position += vlq.Length();

							::new(&data.meta_event) MIDI_MetaEvent(t, vlq, position);
						}
						break;
					case 0xF0:
					case 0xF7:
						{
							MIDI_VLQ vlq = MIDI_VLQ(position);
							position += vlq.Length();

                            ::new(&data.sysex_event) MIDI_SysexEvent(type, vlq, position);
						}
						break;
					default:
						{
							if((type >= 0x80 && type <= 0xEF))
							{
								byte MSB = *position;
								position++;
								byte LSB = *position;
								position++;

								data.midi_event.MSB = MSB;
								data.midi_event.LSB = LSB;

							} else if(running_status) {
								data.midi_event.MSB = type;
								data.midi_event.LSB = *position;
								type = 0;
							} else {
								std::cerr << "[MIDI_Chunk] Error reading data into track message event\n\t";
								std::cerr << "Reason: Invalid event type.\n\n";
							}
						}
				}
			}

			MIDI_Event::MIDI_Event(const MIDI_Event& e)
			{
				type = e.type;

				switch(type) {
					case 0xFF:
						{
							::new(&data.meta_event) MIDI_MetaEvent(e.data.meta_event);
						}
						break;
					case 0xF0:
					case 0xF7:
						{
							::new(&data.sysex_event) MIDI_SysexEvent(e.data.sysex_event);
						}
						break;
					default:
						{
							if((type >= 0x80 && type <= 0xEF) || type == 0)
							{
								data.midi_event = e.data.midi_event;
							} else {
								std::cerr << "[MIDI_Chunk] Error copying MIDI_Event\n\t";
								std::cerr << "Reason: Invalid source type.\n\n";
							}
						}
				}
			}

			MIDI_Event::MIDI_Event(MIDI_Event&& e)
			{
				type = e.type;

				switch(type) {
					case 0xFF:
						{
							::new(&data.meta_event) MIDI_MetaEvent(std::move(e.data.meta_event));
						}
						break;
					case 0xF0:
					case 0xF7:
						{
							::new(&data.sysex_event) MIDI_SysexEvent(std::move(e.data.sysex_event));
						}
						break;
					default:
						{
							if((type >= 0x80 && type <= 0xEF) || type == 0)
							{
								data.midi_event = e.data.midi_event;
							} else {
								std::cerr << "[MIDI_Chunk] Error moving MIDI_Event\n\t";
								std::cerr << "Reason: Invalid source type.\n\n";
							}
						}
				}
			}

			MIDI_Event::~MIDI_Event()
			{
				switch(type) {
					case 0xFF:
						{
							data.meta_event.~MIDI_MetaEvent();
						}
						break;
					case 0xF0:
					case 0xF7:
						{
							data.sysex_event.~MIDI_SysexEvent();
						}
						break;
					default:
						{
							if((type >= 0x80 && type <= 0xEF) || type == 0)
							{
								data.midi_event.~MIDI_MidiEvent();
							}
						}
				}
			}

			MIDI_Event& MIDI_Event::operator=(const MIDI_Event& e)
			{
				type = e.type;

				switch(type) {
					case 0xFF:
						{
							::new(&data.meta_event) MIDI_MetaEvent(e.data.meta_event);
						}
						break;
					case 0xF0:
					case 0xF7:
						{
							::new(&data.sysex_event) MIDI_SysexEvent(e.data.sysex_event);
						}
						break;
					default:
						{
							if((type >= 0x80 && type <= 0xEF) || type == 0)
							{
								data.midi_event = e.data.midi_event;
							} else {
								std::cerr << "[MIDI_Chunk] Error copying MIDI_Event\n\t";
								std::cerr << "Reason: Invalid source type.\n\n";
							}
						}
				}

				return *this;
			}

			MIDI_Event& MIDI_Event::operator=(MIDI_Event&& e)
			{
				type = e.type;

				switch(type) {
					case 0xFF:
						{
							::new(&data.meta_event) MIDI_MetaEvent(std::move(e.data.meta_event));
						}
						break;
					case 0xF0:
					case 0xF7:
						{
							::new(&data.sysex_event) MIDI_SysexEvent(std::move(e.data.sysex_event));
						}
						break;
					default:
						{
							if((type >= 0x80 && type <= 0xEF) || type == 0)
							{
								data.midi_event = e.data.midi_event;
							} else {
								std::cerr << "[MIDI_Chunk] Error moving MIDI_Event\n\t";
								std::cerr << "Reason: Invalid source type.\n\n";
							}
						}
				}

				return *this;
			}

			bool MIDI_Event::IsMidiEvent() const
			{
				return (((type != 0xF0) &&
						(type != 0xF7) &&
						(type != 0xFF) &&
						((type >= 0x80) && (type <= 0xEF))) ||
						(type == 0));
			}

			bool MIDI_Event::IsSysexEvent() const
			{
				return (type == 0xF0 || type == 0xF7);
			}

			bool MIDI_Event::IsMetaEvent() const
			{
				return (type == 0xFF);
			}

			std::ostream& operator<<(std::ostream& os, const MIDI_Event& evt)
			{
				switch(evt.type) {
					case 0xFF:
						{
							os.put((char)evt.type);
							os.put((char)evt.data.meta_event.type);

							auto buf = evt.data.meta_event.length.ToVLQBytes();
							os.write((char*)&buf[0], evt.data.meta_event.length.Length());

							if((uint32_t)evt.data.meta_event.length > 0) {
								os.write((char*)evt.data.meta_event.data, (uint32_t)(evt.data.meta_event.length));
							}

						}
						break;
					case 0xF0:
					case 0xF7:
						{
							os.put((char)evt.type);
                            auto buf = evt.data.sysex_event.length.ToVLQBytes();

							os.write((char*)&buf[0], evt.data.sysex_event.length.Length());
							os.write((char*)evt.data.sysex_event.data, (uint32_t)evt.data.sysex_event.length);
						}
						break;
					default:
						{
							if(evt.type >= 0x80 && evt.type <= 0xEF)
							{
								os.put((char)evt.type);
								os.put((char)evt.data.midi_event.MSB);
								os.put((char)evt.data.midi_event.LSB);
							} else if(evt.type == 0) {
								os.put((char)evt.data.midi_event.MSB);
								os.put((char)evt.data.midi_event.LSB);
							} else {
								std::cerr << "[MIDI_Chunk] Error outputting MIDI_Event\n\t";
								std::cerr << "Reason: Invalid event type.\n\n";
							}
						}
				}

				return os;
			}

			MIDI_Event::Event::Event()
			{
				//midi_event.type = 0;
				midi_event.MSB = 0;
				midi_event.LSB = 0;
			}

			MIDI_Event::Event::~Event() {}

			MIDI_Message::MIDI_Message() : delta_ticks(0), event() {}

			MIDI_Message::MIDI_Message(const MIDI_VLQ& dt, const MIDI_Event& evt) : delta_ticks(dt), event(evt) {}

			MIDI_Message::MIDI_Message(const MIDI_Message& m)
			{
				delta_ticks = m.delta_ticks;
				event = m.event;
			}

			MIDI_Message::MIDI_Message(MIDI_Message&& m)
			{
				delta_ticks = m.delta_ticks;
				event = std::move(m.event);
			}

            MIDI_Message::~MIDI_Message() {}

            MIDI_Message& MIDI_Message::operator=(const MIDI_Message& m)
            {
            	delta_ticks = m.delta_ticks;
            	event = m.event;

            	return *this;
            }

            MIDI_Message& MIDI_Message::operator=(MIDI_Message&& m) {
				delta_ticks = m.delta_ticks;
				event = std::move(m.event);

				return *this;
            }

            MIDI_Track::MIDI_Track() : data() {}

            MIDI_Track::MIDI_Track(const MIDI_Track& mtrk)
            {
            	for(auto msg : mtrk.data) {
					data.push_back(msg);
				}
            }

			MIDI_Track::MIDI_Track(MIDI_Track&& mtrk)
			{
				data = std::move(mtrk.data);
			}

			MIDI_Track& MIDI_Track::operator=(const MIDI_Track& mtrk)
			{
				for(auto msg : mtrk.data) {
					data.push_back(msg);
				}

				return *this;
			}

			MIDI_Track& MIDI_Track::operator=(MIDI_Track&& mtrk)
			{
				data = std::move(mtrk.data);

				return *this;
			}
		}

		MIDI_Chunk::MIDI_Chunk()
		{
			for(int i = 0; i < 4; i++) {
				type[i] = '\0';
			}
			length = 0;
		}

		MIDI_Chunk::MIDI_Chunk(byte* raw_data)
		{
			const char acceptedTypes[2][4] =
			{
				{'M', 'T', 'h', 'd'},
				{'M', 'T', 'r', 'k'}
			};

			length = 0;
			byte* position = raw_data;

			for(uint32_t i = 0; i < 4; i++) {
				type[i] = *position;
				position++;
			}

			//make sure the chunk type is valid
			int tp = 0;
			for(int i = 2; i < 4; i++) {
				if(type[i] != acceptedTypes[tp][i]) {
					if(type[i] == acceptedTypes[1][i]) {
						tp = 1;
					} else {
						std::cerr << "[MIDI_Chunk] Error decoding byte buffer starting at " << (void*)(raw_data) << "\n\t";
						std::cerr << "Reason: Invalid 'type' string.\n\n";
						return;
					}
				}
			}

			//MIDI values are MSB-first by default
			for(int i = 0; i < 4; i++) {
				length = (length << 8) | (byte)(*position);
				position++;
			}

			if(length == 0) {
				std::cerr << "[MIDI_Chunk] Error decoding byte buffer starting at " << (void*)(raw_data) << "\n\t";
				std::cerr << "Reason: The value of the length field is zero.\n\n";
				return;
			}

			DecodeData(position);
		}

		MIDI_Chunk::MIDI_Chunk(const char* type_, uint32_t data_len, byte* data)
		{
			const char acceptedTypes[2][4] =
			{
				{'M', 'T', 'h', 'd'},
				{'M', 'T', 'r', 'k'}
			};

			int tp = 0;
			for(int i = 0; i < 4; i++) {
				type[i] = type_[i];

				if(type[i] != acceptedTypes[tp][i]) {
					if(type[i] == acceptedTypes[1][i]) {
						tp = 1;
					} else {
						std::cerr << "[MIDI_Chunk] Error decoding byte buffer starting at " << (void*)(data) << " given type string " << std::string(type_) << "\n\t";
						std::cerr << "Reason: Invalid 'type' string.\n\n";
						return;
					}
				}
			}

			length = data_len;

			if(length == 0) {
				std::cerr << "[MIDI_Chunk] Error decoding byte buffer starting at " << (void*)(data) << " with given length.\n\t";
				std::cerr << "Reason: The value of the length field is zero.\n\n";
				return;
			}

			DecodeData(data);
		}

		MIDI_Chunk::MIDI_Chunk(detail::MIDI_Header hd)
		{
			char head[] = {'M', 'T', 'h', 'd'};
			for(int i = 0; i < 4; i++) {
				type[i] = head[i];
			}

			length = sizeof(detail::MIDI_Header);
			header = hd;
		}

		MIDI_Chunk::MIDI_Chunk(uint32_t len, detail::MIDI_Track trk)
		{
			char trck[] = {'M', 'T', 'r', 'k'};
			for(int i = 0; i < 4; i++) {
				type[i] = trck[i];
			}

			length = len;

			::new(&track) detail::MIDI_Track(trk);
		}

		MIDI_Chunk::MIDI_Chunk(const MIDI_Chunk& chunk)
		{
			if(chunk.IsHeader()) {
				header = chunk.header;
				char head[] = {'M', 'T', 'h', 'd'};
				for(int i = 0; i < 4; i++) {
					type[i] = head[i];
				}

				length = chunk.length;
			} else if(chunk.IsTrack()) {
				char trck[] = {'M', 'T', 'r', 'k'};
				for(int i = 0; i < 4; i++) {
					type[i] = trck[i];
				}

				length = chunk.length;

				::new(&track) detail::MIDI_Track(chunk.track);
			} else {
				std::cerr << "[MIDI_Chunk] Error copying chunk data\n\t";
				std::cerr << "Reason: Attempted to copy invalid chunk.\n\n";
				return;
			}
		}

		MIDI_Chunk::MIDI_Chunk(MIDI_Chunk&& chunk)
		{
			if(chunk.IsHeader()) {
				header = chunk.header;
				char head[] = {'M', 'T', 'h', 'd'};
				for(int i = 0; i < 4; i++) {
					type[i] = head[i];
				}

				length = chunk.length;
			} else if(chunk.IsTrack()) {
				track = std::move(chunk.track);
				char trck[] = {'M', 'T', 'r', 'k'};
				for(int i = 0; i < 4; i++) {
					type[i] = trck[i];
				}

				length = chunk.length;

				::new(&track) detail::MIDI_Track(chunk.track);
			} else {
				std::cerr << "[MIDI_Chunk] Error moving chunk data\n\t";
				std::cerr << "Reason: Attempted to move invalid chunk.\n\n";
				return;
			}
		}

		MIDI_Chunk::~MIDI_Chunk()
		{
			if(IsHeader()) {
				header.~MIDI_Header();
			} else {
				track.~MIDI_Track();
			}
		}

		MIDI_Chunk& MIDI_Chunk::operator=(const MIDI_Chunk& chunk)
		{
			if(chunk.IsHeader()) {
				header = chunk.header;
				char head[] = {'M', 'T', 'h', 'd'};
				for(int i = 0; i < 4; i++) {
					type[i] = head[i];
				}

				length = chunk.length;
			} else if(chunk.IsTrack()) {
				track = chunk.track;
				char trck[] = {'M', 'T', 'r', 'k'};
				for(int i = 0; i < 4; i++) {
					type[i] = trck[i];
				}

				length = chunk.length;

				::new(&track) detail::MIDI_Track(chunk.track);
			} else {
				std::cerr << "[MIDI_Chunk] Error copying chunk data\n\t";
				std::cerr << "Reason: Attempted to copy invalid chunk.\n\n";
			}

			return *this;
		}

		MIDI_Chunk& MIDI_Chunk::operator=(MIDI_Chunk&& chunk)
		{
			if(chunk.IsHeader()) {
				header = chunk.header;
				char head[] = {'M', 'T', 'h', 'd'};
				for(int i = 0; i < 4; i++) {
					type[i] = head[i];
				}

				length = chunk.length;
			} else if(chunk.IsTrack()) {
				track = std::move(chunk.track);
				char trck[] = {'M', 'T', 'r', 'k'};
				for(int i = 0; i < 4; i++) {
					type[i] = trck[i];
				}

				length = chunk.length;

				::new(&track) detail::MIDI_Track(chunk.track);
			} else {
				std::cerr << "[MIDI_Chunk] Error moving chunk data\n\t";
				std::cerr << "Reason: Attempted to move invalid chunk.\n\n";
			}

			return *this;
		}

		uint32_t MIDI_Chunk::Length() const
		{
			return length;
		}

		bool MIDI_Chunk::IsHeader() const
		{
			bool head = true;
			char head_str[4] = {'M', 'T', 'h', 'd'};

			for(int i = 0; i < 4 && head; i++) {
				head = head && type[i] == head_str[i];
			}

			return head;
		}

		bool MIDI_Chunk::IsTrack() const
		{
			bool trck = true;
			char track_str[4] = {'M', 'T', 'r', 'k'};
			for(int i = 0; i < 4 && trck; i++) {
				trck = trck && type[i] == track_str[i];
			}

			return trck;
		}

		const detail::MIDI_Header& MIDI_Chunk::GetHeader() const
		{
            return header;
		}

		const detail::MIDI_Track& MIDI_Chunk::GetTrack() const
		{
			return track;
		}

		detail::MIDI_Header& MIDI_Chunk::GetHeader()
		{
			return header;
		}

		detail::MIDI_Track& MIDI_Chunk::GetTrack()
		{
			return track;
		}

		void MIDI_Chunk::DecodeData(const byte* data)
		{
			const byte* position = data;

			if(IsHeader()) {
				uint16_t tmp = 0;
				tmp = *position;
				position++;
				tmp = (tmp << 8) | *position;
				position++;

				header.format = (detail::MIDI_FORMAT)(tmp);

				tmp = 0;
                tmp = *position;
                position++;
                tmp = (tmp << 8) | *position;
                position++;

                header.track_count = tmp;

                tmp = 0;
				tmp = *position;
				position++;
				tmp = (tmp << 8) | *position;
				position++;

				header.divisions = tmp;

			} else if(IsTrack()) {

				::new(&track) detail::MIDI_Track();
				bool running_status = false;

				while(position < (data + length)) {
					MIDI_VLQ dt = MIDI_VLQ(position);
					position += dt.Length();

					detail::MIDI_Event evt(position, running_status);

					if(evt.IsMidiEvent()) {
                        running_status = true;
					} else {
						running_status = false;
					}

					position += evt.Length();

                    detail::MIDI_Message msg{dt, evt};

                    track.data.push_back(msg);

                    if(evt.IsMetaEvent()) {
						if(evt.data.meta_event.type == 0x2F) {
							break;
						}
                    }
				}

			} else {
				std::cerr << "[MIDI_Chunk] Error decoding binary data\n\t";
				std::cerr << "Reason: Invalid chunk type.\n\n";
			}
		}


		std::istream& operator>>(std::istream& is, geiger::midi::MIDI_Chunk& chnk)
		{
			is.read(chnk.type, 4);
			byte val = 0;
			for(int i = 0; i < 4; i++) {
				val = is.get();
				chnk.length = (chnk.length << 8) | val;
			}

			if(!chnk.IsHeader() && !chnk.IsTrack()) {
				std::cerr << "[MIDI_Chunk] Error inputting MIDI Chunk from input stream\n\t";
				std::cerr << "Reason: Invalid chunk type.\n\n";
				return is;
			}

			char* tmp_buf = new char[chnk.length]{0};
			char* write_pos = tmp_buf;
			is.read(write_pos, chnk.length);
			auto actual_read = is.gcount();

			auto bytes_left = chnk.length - actual_read;
			while(bytes_left > 0 && is.good() && !is.eof()) {
				write_pos += actual_read;
				is.read(write_pos, bytes_left);
				actual_read = is.gcount();
				bytes_left = bytes_left - actual_read;
			}

			if(is.eof() && bytes_left > 0) {
				std::cerr << "[MIDI_Chunk] Error reading MIDI Chunk\n\t";
				std::cerr << "Reason: Reached end of file before length bytes were read.\n\n";
			} else if(is.bad() && bytes_left > 0) {
				std::cerr << "[MIDI_Chunk] Error reading MIDI Chunk\n\t";
				std::cerr << "Reason: Unrecoverable I/O error.\n\n";
			} else if(is.rdstate() & std::ios_base::failbit && bytes_left > 0) {
				std::cerr << "[MIDI_Chunk] Error reading MIDI Chunk\n\t";
				std::cerr << "Reason: Stream extraction error.\n\n";
			}

			chnk.DecodeData((byte*)tmp_buf);

			delete[] tmp_buf;

			return is;
		}

		std::ostream& operator<<(std::ostream& os, const geiger::midi::MIDI_Chunk& chnk)
		{
			if(!chnk.IsHeader() && !chnk.IsTrack()) {
				std::cerr << "[MIDI_Chunk] Error inputting MIDI Chunk from input stream\n\t";
				std::cerr << "Reason: Invalid chunk type.\n\n";
				return os;
			}

			os.write((char*)chnk.type, 4);

			for(int i = 3; i >= 0; i--) {
				char b = (chnk.length >> 8 * i) & 0xFF;
				os.put(b);
			}

			if(chnk.IsHeader()) {
				os.put((char)((uint16_t)(chnk.header.format) >> 8));
				os.put((char)((uint16_t)(chnk.header.format) & 0xFF));

				os.put((char)(chnk.header.track_count >> 8));
				os.put((char)(chnk.header.track_count & 0xFF));

				os.put((char)(chnk.header.divisions >> 8));
				os.put((char)(chnk.header.divisions & 0xFF));
			} else if(chnk.IsTrack()) {
				for(auto& msg : chnk.track.data) {
					auto buf = msg.delta_ticks.ToVLQBytes();

                    for(uint32_t i = 0; i < msg.delta_ticks.Length(); i++) {
						os.put((char)(buf[i]));
                    }

					os << msg.event;
				}
			} else {
				std::cerr << "[MIDI_Chunk] Error outputting a MIDI_Chunk\n\t";
				std::cerr << "Reason: Invalid header type.\n\n";
			}

			return os;
		}


	}
}
