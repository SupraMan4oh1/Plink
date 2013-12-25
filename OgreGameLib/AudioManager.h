#pragma once

#include <climits>
#include <string>
#include <boost/unordered_map.hpp>

#include "AL/alure.h"

namespace Menura
{
	class AudioBufferGroup;
	class AudioSource;

	/** @brief Manages the audio system and all its components. */
	class AudioManager
	{
		friend AudioBufferGroup;

	public:

		/** @brief Get the currently active AudioManager. */
		static AudioManager &getActiveManager(void);

		/** @brief Makes this instance the active AudioManager. 
		@note Audio componenets belong to the AudioManager that was active when they were constructed. */
		void makeActive(void);

		/** @brief Checks if this is currently the active manager. @returns `true` if currently the active manager. */
		bool isActive(void);

		/** @brief Create the audio manager on the default device, with default attributes for the 'ALContext'. 
		@param [in] default_buffer_group_path_prefix Default path-prefix for new buffer groups. */
		AudioManager(std::string default_buffer_group_path_prefix = "");

		/** @brief Create the audio manager on the specified device, and the specified attributes for the `ALContext`.

		@note `device_name` has no default value, but setting it to `NULL` will cause OpenAL to create the audio context on the default device.
		@note The default value for all context attributes is INT_MAX, as this value is used as a flag by the constructor, to use the default value
		assigned by OpenAL for that attribute.

		@param [in] default_buffer_group_path_prefix Default path-prefix for new buffer groups.
		@param [in] device_name The name of the device on which the audio context will be created.
		@param [in] mono_sources_hint A hint indicating how many sources should be capable of supporting mono data.
		@param [in] stereo_sources_hint A hint indicating how many sources should be capable of supporting stereo data.
		@param [in] frequency Frequency for the mixing output buffer, in units of Hz.
		@param [in] refresh Refresh interval rate, in units of Hz.
		@param [in] sync Flag, indicating a synchronous context. */
		AudioManager(std::string default_buffer_group_path_prefix, ALCchar const *device_name, 
			ALCint mono_sources_hint = INT_MAX, ALCint stereo_sources_hint = INT_MAX, 
			ALCint frequency = INT_MAX, ALCint refresh = INT_MAX, ALCint sync = INT_MAX);
		~AudioManager();

		/** @brief Get the buffer group with the given name.
		@details If no group with the queried name exists, a new group with that name is created and returned if 'create_new_group' 
		is 'true'. Otherwise the default buffer group is returned.

		@param [in] buffer_group_name Name of the buffer group.
		@param [in] create_new_group If 'true', a new group is created if none already exist with the queried name. Otherwise 
		the default group is returned if a group with that name doesn't exist.
		@param [out] group_was_found Sets the value to 'true' if the group with the queried name exists, or to 'false' if a new 
		group was created or the default group was returned.
		@returns The requested AudioBufferGroup. */
		AudioBufferGroup &getBufferGroup(std::string const &buffer_group_name, bool create_new_group, bool &group_was_found);

		/** @overload getBufferGroup(std::string const &buffer_group_name, bool create_new_group, bool &group_was_found) */
		AudioBufferGroup &getBufferGroup(std::string const &buffer_group_name, bool create_new_group = true);

		/** @brief Create a new buffer group with the given name, or return the group with that name if it already exists.

		@param [in] buffer_group_name Name of the buffer group.
		@returns The requested AudioBufferGroup. */
		AudioBufferGroup &createBufferGroup(std::string const &buffer_group_name, std::string const &path_prefix);

		/** @overload createBufferGroup(std::string const &buffer_group_name, std::string const &path_prefix) */
		AudioBufferGroup &createBufferGroup(std::string const &buffer_group_name);

		/** @brief Remove the buffer group with the given name. Does nothing if the group with that name doesn't exist.
		@param [in] Name of the buffer group. */
		void removeBufferGroup(std::string const &buffer_group_name);

		/** @brief Remove all buffer groups. */
		void removeAllBufferGroups(void);

		/** @brief Stop playing and reset the buffer in any AudioSource using the named buffer in the specified AudioBufferGroup.

		@param [in] buffer_group_name Name of the buffer group which contains the named buffer. 
		@param [in] buffer_name Name of the buffer to be purged. 
		@see purgeBufferGroupFromSources(std::string const &buffer_group_name) */
		void purgeBufferFromSources(std::string const &buffer_group_name, std::string const &buffer_name);

		/** @overload purgeBufferFromSources(std::string const &buffer_group_name, std::string const &buffer_name) */
		void purgeBufferFromSources(AudioBufferGroup const &buffer_group, std::string const &buffer_name);

		/** @overload purgeBufferFromSources(std::string const &buffer_group_name, std::string const &buffer_name) */
		void purgeBufferFromSources(AudioBufferGroup const &buffer_group, ALuint buffer_id);

		/** @brief Stop playing and reset the buffer in any AudioSource using any buffer in the specified AudioBufferGroup.

		Sometimes it's desirable to ensure that any audio sources playing a set of buffers are immediately stopped and dereference the buffer.
		One potential scenario is when changing levels and each level uses a seperate AudioBufferGroup. Once the new group is loaded, it may 
		be desirable to force all the sources in the group for the previous level to stop playing immediately if they haven't already.

		@param [in] buffer_group_name Name of the buffer group which needs its buffers purged from audio sources. */
		void purgeBufferGroupFromSources(std::string const &buffer_group_name);

		/** @overload purgeBufferGroupFromSources(std::string const &buffer_group_name) */
		void purgeBufferGroupFromSources(AudioBufferGroup const &buffer_group);

	protected:

		std::string m_BufferGroupPathPrefix;	//!< The default path-prefix to use for new buffer groups.
		ALCdevice *m_Device;					//!< The device managed by this manager.
		ALCcontext *m_Context;					//!< The audio context for this manager.

		ALCint m_MaxSourceCount;				//!< The max number of concurrent audio sources supported.

		boost::unordered_map<std::string, AudioBufferGroup> m_BufferGroups;		//!< The audio buffer groups maintained by this manager.

		/** @brief Calculates the maximum number of concurrent audio sources that are supported.
		
		@returns The maximum number of concurrent audio sources allowed, as declared as supported by the audio library, 
		or by `MAX_AUDIO_SOURCES` declared in `KyaniteConstants.h`, whichever is smaller. The reason for this is that 
		some audio libs report an erroneously high number of sources, but will fail to function properly before the 
		reported limit is actually reached. */
		ALCint calculateMaxSourceCount(void);

		/** @brief Creates (or recreates as the case may be) the default buffer group. */
		void createDefaultBufferGroup(void);

	private:

		bool m_IsBufferGroupBeingAdded;			//!< This is a flag used by the validity check for buffer groups.

		/** @brief Used to check the validity of a buffer group.
		@details To ensure that they're a valid instance, `AudioBufferGroup`s check with their `AudioManager` during creation to see if 
		they're actually being added to the 'AudioManager', or if they're being constructed outside of the manager. 
		@returns 'true' if a buffer group is currently being added to the manager, 'false' if not. */
		bool currentlyAddingBufferGroup(void);

		/** @brief Causes the audio manager to enter a failure state where it will continue to function as if everything was normal, but 
		the audio system is not actually active.
		
		The failure state functionality is useful for when encountering audio errors that cannot be overcome, but the rest 
		of the program should be able to continue without audio. */
		void enterFailureState(void);
	};

}