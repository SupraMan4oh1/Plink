#pragma once

#include <memory>

namespace Kyanite
{
	/** @brief Audio source.

	Encapsulates all the functionality of an audio source, as well as handling behind the scene details such as concurrent source 
	limits and audio component IDs.

	@note The AudioSource class lives within the scope of the AudioManager. It must be spawned from the AudioManager and it only
	remains a valid instance of a source so long as the AudioManager that spawned it is still valid. This tight coupling is
	because of the tight coupling between components of the underlying audio system. The AudioManager maintains the components of the
	audio system in which sources are relevant, and thus only remain relevant as long as those systems are still active. */
	class AudioSource
	{
	public:

		AudioSource();
		~AudioSource();
	};

	typedef std::shared_ptr<AudioSource> AudioSourceSharedPtr;
}