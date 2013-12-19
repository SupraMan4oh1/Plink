#include "AudioBufferGroup.h"

#include "AppUtility.h"
#include "AudioManager.h"

#include <boost/filesystem.hpp>

using namespace Kyanite;

AudioBufferGroup::AudioBufferGroup(AudioManager * const audio_manager, std::string group_name, std::string path_prefix, 
	std::vector<std::string> const &file_paths, bool load_files) : m_ParentAudioManager(audio_manager), m_IsParentAudioManagerValid(false), 
	m_GroupName(std::move(group_name)), m_PathPrefix(std::move(path_prefix))
{
	if (m_ParentAudioManager && m_ParentAudioManager->currentlyAddingBufferGroup())
	{
		m_IsParentAudioManagerValid = true;
	}

	addBuffers(file_paths);

	if (load_files)
	{
		loadBuffers(false);
	}
}

AudioBufferGroup::AudioBufferGroup(AudioManager * const audio_manager, std::string group_name, std::string path_prefix)
: m_ParentAudioManager(audio_manager), m_IsParentAudioManagerValid(false), m_GroupName(std::move(group_name)), m_PathPrefix(std::move(path_prefix))
{
	if (m_ParentAudioManager && m_ParentAudioManager->currentlyAddingBufferGroup())
	{
		m_IsParentAudioManagerValid = true;
	}
}

AudioBufferGroup::~AudioBufferGroup()
{
	if (m_IsParentAudioManagerValid)
	{
		removeAllBuffers();
	}
}

AudioBufferGroup::AudioBufferGroup(AudioBufferGroup &&source) : m_GroupName(std::move(source.m_GroupName)), 
	m_PathPrefix(std::move(source.m_PathPrefix)), m_Buffers(std::move(source.m_Buffers))
{
	m_ParentAudioManager = source.m_ParentAudioManager;
	m_IsParentAudioManagerValid = source.m_IsParentAudioManagerValid;
	m_IsBufferGroupLoaded = source.m_IsBufferGroupLoaded;

	source.m_ParentAudioManager = NULL;
	source.m_IsParentAudioManagerValid = false;
	source.m_IsBufferGroupLoaded = false;
}

AudioBufferGroup& AudioBufferGroup::operator=(AudioBufferGroup &&source)
{
	if (this != &source)
	{
		if (m_IsParentAudioManagerValid)
		{
			removeAllBuffers();
		}

		m_ParentAudioManager = source.m_ParentAudioManager;
		m_IsParentAudioManagerValid = source.m_IsParentAudioManagerValid;
		m_IsBufferGroupLoaded = source.m_IsBufferGroupLoaded;

		m_GroupName = std::move(source.m_GroupName);
		m_PathPrefix = std::move(source.m_PathPrefix);
		m_Buffers = std::move(source.m_Buffers);

		source.m_ParentAudioManager = NULL;
		source.m_IsParentAudioManagerValid = false;
		source.m_IsBufferGroupLoaded = false;
	}

	return *this;
}

std::string const &AudioBufferGroup::getName(void) const
{
	return m_GroupName;
}

void AudioBufferGroup::setPathPrefix(std::string const &path_prefix, bool remove_all_buffers)
{
	// Same prefix or the AudioManager that spawned this instance is no longer valid, so don't do anything.
	if (m_PathPrefix == path_prefix || !m_IsParentAudioManagerValid)
	{
		return;
	}

	m_ParentAudioManager->purgeBufferGroupFromSources(*this);
	m_PathPrefix = path_prefix;

	if (remove_all_buffers)
	{
		removeAllBuffers();
	}
	else
	{
		std::vector<std::string> buffers_to_remove(m_Buffers.size());

		for (auto iter = m_Buffers.begin(); iter != m_Buffers.end(); ++iter)
		{
			std::string full_file_path(m_PathPrefix + iter->first);

			if (!boost::filesystem::exists(full_file_path) || !boost::filesystem::is_regular_file(full_file_path))
			{
				buffers_to_remove.push_back(iter->first);
			}
		}

		removeBuffers(buffers_to_remove);
	}
}

std::string const &AudioBufferGroup::getPathPrefix(void)
{
	return m_PathPrefix;
}

bool AudioBufferGroup::isBufferGroupLoaded(void)
{
	return m_IsBufferGroupLoaded;
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

boost::unordered_map<std::string, ALuint>::iterator AudioBufferGroup::getIteratorToBuffer(std::string const &file_path)
{
	return m_Buffers.find(file_path);
}

boost::unordered_map<std::string, ALuint>::const_iterator AudioBufferGroup::getIteratorToBuffer(std::string const &file_path) const
{
	return m_Buffers.find(file_path);
}


bool AudioBufferGroup::addBuffer(std::string const &file_path)
{
	// The AudioManager that spawned this instance is no longer valid.
	if (!m_IsParentAudioManagerValid)
	{
		return false;
	}

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
	if (m_IsBufferGroupLoaded)
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

void AudioBufferGroup::removeBuffer(std::string const &file_path)
{
	if (m_IsParentAudioManagerValid)
	{
		auto buffer_iter = getIteratorToBuffer(file_path);

		if (buffer_iter != m_Buffers.end())
		{
			m_ParentAudioManager->purgeBufferFromSources(*this, buffer_iter->second);
			unloadBuffer(buffer_iter);
			m_Buffers.erase(buffer_iter);
		}
	}
}

void AudioBufferGroup::removeBuffers(std::vector<std::string> const &file_paths)
{
	for (size_t i = 0; i < file_paths.size(); ++i)
	{
		removeBuffer(file_paths[i]);
	}
}

void AudioBufferGroup::removeAllBuffers(void)
{
	if (m_IsParentAudioManagerValid)
	{
		m_ParentAudioManager->purgeBufferGroupFromSources(*this);
		unloadBuffers();
		m_Buffers.clear();
	}
}

bool AudioBufferGroup::loadBuffer(boost::unordered_map<std::string, ALuint>::iterator &buffer_to_load, bool verify_files_exist)
{
	// The AudioManager that spawned this instance is no longer valid.
	if (!m_IsParentAudioManagerValid)
	{
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
	m_IsBufferGroupLoaded = true;
	int successful_load_count = 0;

	for (auto iter = m_Buffers.begin(); iter != m_Buffers.end(); ++iter)
	{
		loadBuffer(iter, verify_files_exist) ? ++successful_load_count : 0;
	}

	return successful_load_count;
}

bool AudioBufferGroup::unloadBuffer(boost::unordered_map<std::string, ALuint>::iterator &buffer_to_load)
{
	// The AudioManager that spawned this instance is no longer valid.
	if (!m_IsParentAudioManagerValid)
	{
		return false;
	}

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
	m_IsBufferGroupLoaded = false;
	int failed_unload_count = 0;

	for (auto iter = m_Buffers.begin(); iter != m_Buffers.end(); ++iter)
	{
		unloadBuffer(iter) ? 0 : ++failed_unload_count;
	}

	return failed_unload_count;
}
