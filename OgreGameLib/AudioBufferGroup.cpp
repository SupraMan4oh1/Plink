#include "AudioBufferGroup.h"

#include "AppUtility.h"
#include "AudioManager.h"

#include <boost/filesystem.hpp>

using namespace Kyanite;

AudioBufferGroup::AudioBufferGroup(AudioManager const * const audio_manager, std::string group_name, std::string path_prefix, 
	std::vector<std::string> const &file_paths, bool load_files) : m_ParentAudioManager(audio_manager), m_ParentAudioManagerValid(true), 
	m_GroupName(std::move(group_name)), m_PathPrefix(std::move(path_prefix))
{
	addBuffers(file_paths);

	if (load_files)
	{
		loadBuffers(false);
	}
}

AudioBufferGroup::~AudioBufferGroup()
{
}

ALuint AudioBufferGroup::getBuffer(std::string const &file_path) const
{
	auto found_buffer = m_Buffers.find(file_path);

	if (found_buffer != m_Buffers.end())
	{
		return found_buffer->second;
	}

	return 0;
}

bool AudioBufferGroup::addBuffer(std::string const &file_path)
{
	std::string full_file_path(m_PathPrefix + file_path);

	// Don't attempt to load the file if it doesn't exist or isn't a file.
	if (!boost::filesystem::exists(full_file_path) || !boost::filesystem::is_regular_file(full_file_path))
	{
		AppUtility::fLogMessage("AudioBufferGroup: '%s' -- Cannot add the audio file at '%s' to the group; not an actual file.",
			Ogre::LML_CRITICAL, false, m_GroupName.c_str(), full_file_path.c_str());

		return false;
	}

	// OpenAL guarantees us that 0 will always represent the NULL buffer (i.e. not a valid buffer), so we can safely use it 
	// as a flag that represents a buffer that isn't loaded.
	auto emplace_ret = m_Buffers.emplace(file_path, 0);

	// The file at this location is already part of this buffer group.
	if (!emplace_ret.second)
	{
		AppUtility::fLogMessage("AudioBufferGroup: '%s' -- The audio file at '%s' is already part of this group; skipping.",
			Ogre::LML_NORMAL, true, m_GroupName.c_str(), full_file_path.c_str());

		return false;
	}

	// Automatically load this file if this group is already supposed to be loaded.
	if (m_BufferGroupLoaded)
	{
		return loadBuffer(emplace_ret.first);
	}

	return true;
}

int AudioBufferGroup::addBuffers(std::vector<std::string> const &file_paths)
{
	int successful_add_count = 0;

	for (size_t i = 0; i < file_paths.size(); ++i)
	{
		addBuffer(file_paths[i]) ? ++successful_add_count : 0;
	}

	return successful_add_count;
}

bool AudioBufferGroup::loadBuffer(boost::unordered_map<std::string, ALuint>::iterator &buffer_to_load, bool verify_files_exist)
{
	// The AudioManager that spawned this instance is no longer valid and we're forced to fail silently.
	if (!m_ParentAudioManager)
	{
		AppUtility::fLogMessage("AudioBufferGroup: '%s' -- The parent AudioManager of this buffer group is no longer valid. \
								It is no longer possible to load buffers.", Ogre::LML_NORMAL, false, m_GroupName.c_str());
		return false;
	}

	// Skip entirely if the buffer is already loaded.
	if (buffer_to_load->second != 0)
	{
		AppUtility::fLogMessage("AudioBufferGroup: '%s' -- The buffer '%s' is already loaded; skipping.",
			Ogre::LML_NORMAL, true, m_GroupName.c_str(), buffer_to_load->first.c_str());

		return false;
	}

	// Verify that the file still exists if the caller wants the additional error checking.
	if (verify_files_exist)
	{
		std::string full_file_path(m_PathPrefix + buffer_to_load->first);

		if (!boost::filesystem::exists(full_file_path) || !boost::filesystem::is_regular_file(full_file_path))
		{
			AppUtility::fLogMessage("AudioBufferGroup: '%s' -- Cannot load the audio file at '%s'; not an actual file. This file \
									was either deleted or changed since it was added to the buffer group.",
									Ogre::LML_CRITICAL, false, m_GroupName.c_str(), full_file_path.c_str());

			return false;
		}
	}

	ALuint new_buffer = alureCreateBufferFromFile(buffer_to_load->first.c_str());

	// An error occured while loading the file into the buffer.
	if (new_buffer == AL_NONE)
	{
		AppUtility::fLogMessage("AudioBufferGroup: '%s' -- Encountered error: `%s` when attempting to load the audio buffer '%s'.",
			Ogre::LogMessageLevel::LML_CRITICAL, false, m_GroupName.c_str(), alureGetErrorString(), buffer_to_load->first.c_str());

		return false;
	}

	buffer_to_load->second = new_buffer;

	return true;
}

int AudioBufferGroup::loadBuffers(bool verify_files_exist)
{
	m_BufferGroupLoaded = true;
	int successful_load_count = 0;

	for (auto iter = m_Buffers.begin(); iter != m_Buffers.end(); ++iter)
	{
		loadBuffer(iter, verify_files_exist) ? ++successful_load_count : 0;
	}

	return successful_load_count;
}

bool AudioBufferGroup::unloadBuffer(boost::unordered_map<std::string, ALuint>::iterator &buffer_to_load)
{
	if (buffer_to_load->second != 0)
	{
		alDeleteBuffers(1, &buffer_to_load->second);
		
		if (!alIsBuffer(buffer_to_load->second))
		{
			buffer_to_load->second = 0;
		}
		else
		{
			AppUtility::fLogMessage("AudioBufferGroup: '%s' -- Couldn't unload the buffer '%s'; \
									buffer is still in active use by sources and cannot be unloaded.",
									Ogre::LogMessageLevel::LML_CRITICAL, false, m_GroupName.c_str(), 
									alureGetErrorString(), buffer_to_load->first.c_str());

			return false;
		}
	}

	return true;
}

int AudioBufferGroup::unloadBuffers(void)
{
	m_BufferGroupLoaded = false;
	int failed_unload_count = 0;

	for (auto iter = m_Buffers.begin(); iter != m_Buffers.end(); ++iter)
	{
		unloadBuffer(iter) ? 0 : ++failed_unload_count;
	}

	return failed_unload_count;
}
