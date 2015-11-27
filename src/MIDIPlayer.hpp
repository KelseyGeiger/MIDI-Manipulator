#ifndef MIDIPLAYER_HPP
#define MIDIPLAYER_HPP

#include <string>

namespace geiger {
	namespace midi {

		typedef unsigned char byte;

		class MIDISong;
		class MIDITrack;

		class MIDIPlayer
		{
			public:
				MIDIPlayer();
				virtual ~MIDIPlayer();

				void PlayFile(std::string filename);
				void PlaySong(const MIDISong& song);
				void PlayTrack(const MIDITrack& track);
				void Pause();
				void Play();

				void SetVolume(float volume);

			private:

		};

	}
}

#endif // MIDIPLAYER_HPP
