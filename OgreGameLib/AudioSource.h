#pragma once

#include <memory>

namespace Menura
{
	class AudioBuffer;

	/** @brief Audio source.

	Encapsulates all the functionality of an audio source, as well as handling behind the scene details such as concurrent source 
	limits and audio component IDs. */
	class AudioSource
	{
	public:

		/** @brief Defines a subset of the AudioSource interface that only AudioBuffer should have access to. */
		class AudioBufferInterface
		{
			friend AudioBuffer;
			static void unsetBuffer(AudioSource &audio_source, bool notify_buffer = true);	//!< @see AudioSource::unsetBuffer(bool notify_buffer)
		};

		friend AudioBufferInterface;

		AudioSource();
		~AudioSource();

		/** @brief Unsets the audio buffer used by this audio source, and alerts the buffer to the change. */
		void unsetBuffer(void);

	protected:

		AudioBuffer *m_Buffer;		//!< @brief The audio buffer that this source plays audio data from.

		/** @brief Unsets the audio buffer used by this audio source.
		@param [in] notify_buffer The audio buffer held by this source will be notified if `true`, 
		otherwise no notification will be sent if `false`. */
		void unsetBuffer(bool notify_buffer);
	};
}
