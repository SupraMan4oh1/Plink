#pragma once

#include <climits>
#include "AL/alure.h"

namespace Kyanite
{
	class AudioBufferGroup;
	class AudioSource;

	/** @brief Manages the audio system and all its components. */
	class AudioManager
	{
	public:

		/** @brief Create the audio manager on the default device, with default attributes for the 'ALContext'. */
		AudioManager();

		/** @brief Create the audio manager on the specified device, and the specified attributes for the `ALContext`.

		@note `device_name` has no default value, but setting it to `NULL` will cause OpenAL to create the audio context on the default device.
		@note The default value for all context attributes is INT_MAX, as this value is used as a flag by the constructor, to use the default value
		assigned by OpenAL for that attribute.

		@param [in] device_name The name of the device on which the audio context will be created.
		@param [in] mono_sources_hint A hint indicating how many sources should be capable of supporting mono data.
		@param [in] stereo_sources_hint A hint indicating how many sources should be capable of supporting stereo data.
		@param [in] frequency Frequency for the mixing output buffer, in units of Hz.
		@param [in] refresh Refresh interval rate, in units of Hz.
		@param [in] sync Flag, indicating a synchronous context. */
		AudioManager(ALCchar *device_name, ALCint mono_sources_hint = INT_MAX, ALCint stereo_sources_hint = INT_MAX,
		             ALCint frequency = INT_MAX, ALCint refresh = INT_MAX, ALCint sync = INT_MAX);
		~AudioManager();

		/** @brief Stop playing and reset the buffer in any AudioSource using the named buffer in the specified AudioBufferGroup.
		@param [in] buffer_group_name Name of the buffer group which contains the named buffer. 
		@param [in] buffer_name Name of the buffer to be purged. 
		@see purgeBufferGroupFromSources */
		void purgeBufferFromSources(std::string const &buffer_group_name, std::string const &buffer_name);

		/** @brief Stop playing and reset the buffer in any AudioSource using any buffer in the specified AudioBufferGroup.

		Sometimes it's desirable to ensure that any audio sources playing a set of buffers are immediately stopped and dereference the buffer.
		One potential scenario is when changing levels and each level uses a seperate AudioBufferGroup. Once the new group is loaded, it may 
		be desirable to force all the sources in the group for the previous level to stop playing immediately if they haven't already.

		@param [in] buffer_group_name Name of the buffer group which needs its buffers purged from audio sources.
		*/
		void purgeBufferGroupFromSources(std::string const &buffer_group_name);

	protected:

		ALCdevice *m_Device;	//!< The device managed by this manager.
		ALCcontext *m_Context;	//!< The audio context for this manager.

		ALCint m_MaxSourceCount; //!< The max number of concurrent audio sources supported.

		/** @brief Calculates the maximum number of concurrent audio sources that are supported.
		
		@returns The maximum number of concurrent audio sources allowed, as declared as supported by the audio library, 
		or by `MAX_AUDIO_SOURCES` declared in `KyaniteConstants.h`, whichever is smaller. The reason for this is that 
		some audio libs report an erroneously high number of sources, but will fail to function properly before the 
		reported limit is actually reached. */
		ALCint calculateMaxSourceCount(void);

	private:

		/** @brief Causes the audio manager to enter a failure state where it will continue to function as if everything was normal, but 
		the audio system is not actually active.
		
		The failure state functionality is useful for when encountering audio errors that cannot be overcome, but the rest 
		of the program should be able to continue without audio. */
		void enterFailureState(void);
	};

}