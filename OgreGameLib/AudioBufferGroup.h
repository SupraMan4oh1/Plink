#pragma once

#include <memory>
#include <string>
#include <boost/unordered_map.hpp>

#include <AL/alure.h>

namespace Kyanite
{
	/** @brief Group of audio buffers.

	Manages a group of audio buffers, including controlling access and buffer lifetime. Grouping buffers is useful, because if you need 
	to load a specific sound into a buffer, you usually need to load a set of others along with it. The same goes for unloading and other 
	buffer management. 
	
	@note The AudioBufferGroup class lives within the scope of the AudioManager. It must be spawned from the AudioManager and it only 
	remains a valid instance of a buffer group so long as the AudioManager that spawned it is still valid. This tight coupling is 
	because of the tight coupling between components of the underlying audio system. The AudioManager maintains the components of the 
	audio system in which buffers are relevant, so buffers and thus the buffer groups only remain relevant as long as those systems 
	are still active. */
	class AudioBufferGroup
	{
	public:

		AudioBufferGroup(std::string group_name, std::vector<std::string> const &file_paths, bool load_files = false);
		~AudioBufferGroup();

		/** @brief Get the ID of the buffer with the corresponding file-path. 
		@param [in] file_path The file-path associated with the buffer to retrieve the ID of. 
		@returns The ID of the matching buffer, or 0 if not found. */
		ALuint getBuffer(std::string const &file_path);

		/** @brief Add the audio file at the given path to this group.

		Audio files added to an audio buffer group are managed by that group.

		@note This function only validates that the given file actually exists; it cannot validate whether it's in a supported audio
		format, or even an audio file at all, until the audio buffers are actually loaded.

		@param [in] file_path The file-path the audio file is located at.
		@returns 'true' if file-path was valid, 'false' if it wasn't. If this group is already meant to be loaded, the return value 
		changes to 'true' if the file-path was valid and the corresponding buffer successfully loaded; 'false' if either fails. */
		bool addFile(std::string const &file_path);

		/** @brief Add the audio files at the given paths to this group.

		Audio files added to an audio buffer group are managed by that group.

		@note This function only validates that the given files actually exist; it cannot validate whether they're in a supported audio
		format, or even an audio file at all, until the audio buffers are actually loaded.

		@param [in] file_paths std::vector of file-paths to add to the group.
		@returns The number of valid file-paths added to the group (and buffers loaded if this group is already meant to be loaded). */
		int addFiles(std::vector<std::string> const &file_paths);

		/** @brief Create and load all the buffers that need loading, from the managed audio files.

		Only loads buffers that are not yet loaded.

		@param [in] verify_files_exist Verify that the audio files pointed to still exist if 'true'; skip and only verify that the buffer
		successfully loaded if 'false'.
		@returns The number of buffers that were successfully loaded. */
		int loadBuffers(bool verify_files_exist = false);

		/** @brief Unloads all buffers. 
		@returns The number of buffers that failed to unload. */
		int unloadBuffers(void);

	protected:

		std::string m_GroupName;								//!< @brief Name of the buffer group.
		boost::unordered_map<std::string, ALuint> m_Buffers;	//!< @brief Map of the file names to the buffer IDs, for all the buffers in this group.
		bool m_BufferGroupLoaded;								//!< @brief Is this buffer group currently meant to be loaded or unloaded?

		/** @brief Create and load the buffer pointed to by the given buffer-map iterator. 
		
		Only loads the buffer if it is not yet loaded.
		@param [in] map-buffer iterator pointing to the buffer to load.
		@param [in] verify_files_exist Verify that the audio files pointed to still exist if 'true'; skip and only verify that the buffer 
		successfully loaded if 'false'.
		@returns 'true' if the buffer was loaded successfully, 'false' if it failed to load. */
		bool loadBuffer(boost::unordered_map<std::string, ALuint>::iterator &buffer_to_load, bool verify_files_exist = false);

		/** @brief Unload the buffer pointed to by the given buffer-map iterator.
		@param [in] map-buffer iterator pointing to the buffer to unload. 
		@returns 'true' if the buffer unloaded successfully, 'false' if it failed and is still loaded. */
		bool unloadBuffer(boost::unordered_map<std::string, ALuint>::iterator &buffer_to_load);
	};

	/** @brief Shared pointer to an AudioBufferGroup. */
	typedef std::shared_ptr<AudioBufferGroup> AudioBufferGroupSharedPtr;
} 