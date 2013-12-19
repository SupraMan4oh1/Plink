#include "AudioManager.h"

#include <cstdio>
#include <cstring>

#include "KyaniteConstants.h"
#include "AppUtility.h"

#include "AudioBufferGroup.h"
#include "AudioSource.h"

using namespace Kyanite;

AudioManager::AudioManager(std::string default_buffer_group_path_prefix) : m_BufferGroupPathPrefix(std::move(default_buffer_group_path_prefix))
{
	ALboolean error = alureInitDevice(NULL, NULL);

	if (error == AL_FALSE)
	{
		AppUtility::fLogMessage("Cannot initialize the audio device or create a context. Encountered error: `%s`", 
			Ogre::LogMessageLevel::LML_CRITICAL, false, alureGetErrorString());

		enterFailureState();
		return;
	}

	m_Context = alcGetCurrentContext();
	m_Device = alcGetContextsDevice(m_Context);

	calculateMaxSourceCount();
	createDefaultBufferGroup();

	AppUtility::fLogMessage("AudioManager: Number of concurrent audio sources supported: %d", Ogre::LML_NORMAL, true, m_MaxSourceCount);




	ALuint buffer = alureCreateBufferFromFile("bird.ogg");
	ALuint source;
	alGenSources(1, &source);

	if ((error = alGetError()) != AL_NO_ERROR)
	{
		AppUtility::fLogMessage("alGenSources: %s", Ogre::LogMessageLevel::LML_CRITICAL, false, alGetString(error));
	}

	alSourcei(source, AL_BUFFER, buffer);

	if ((error = alGetError()) != AL_NO_ERROR)
	{
		AppUtility::fLogMessage("alSourcei: %s", Ogre::LogMessageLevel::LML_CRITICAL, false, alGetString(error));
	}

	ALfloat position[3] = { 10, 10, 0 };
	ALfloat velocity[3] = { 0, 0, 0 };
	ALfloat others[3] = { 0, 0, 0 };
	alSourcefv(source, AL_POSITION, position);
	alSourcefv(source, AL_VELOCITY, velocity);
	alSourcefv(source, AL_DIRECTION, others);

	alListenerfv(AL_POSITION, others);

	alSourcePlay(source);
}

AudioManager::AudioManager(std::string default_buffer_group_path_prefix, ALCchar const *device_name, 
	ALCint mono_sources_hint, ALCint stereo_sources_hint, ALCint frequency, ALCint refresh, ALCint sync) 
	: m_BufferGroupPathPrefix(std::move(default_buffer_group_path_prefix))
{
	ALCint attributes[11];

	int attr_count = 0;

	if (mono_sources_hint != INT_MAX) {
		attributes[attr_count] = ALC_MONO_SOURCES;
		attributes[attr_count + 1] = mono_sources_hint;
		attr_count += 2;
	}

	if (stereo_sources_hint != INT_MAX) {
		attributes[attr_count] = ALC_STEREO_SOURCES;
		attributes[attr_count + 1] = stereo_sources_hint;
		attr_count += 2;
	}

	if (frequency != INT_MAX) {
		attributes[attr_count] = ALC_FREQUENCY;
		attributes[attr_count + 1] = frequency;
		attr_count += 2;
	}

	if (refresh != INT_MAX) {
		attributes[attr_count] = ALC_REFRESH;
		attributes[attr_count + 1] = refresh;
		attr_count += 2;
	}

	if (sync != INT_MAX) {
		attributes[attr_count] = ALC_REFRESH;
		attributes[attr_count + 1] = sync;
		attr_count += 2;
	}

	attributes[attr_count] = 0;

	ALboolean error = alureInitDevice(device_name, attributes);

	if (error == AL_FALSE)
	{
		AppUtility::fLogMessage("Cannot initialize the audio device or create a context. Encountered error: %s",
			Ogre::LogMessageLevel::LML_CRITICAL, false, alureGetErrorString());

		enterFailureState();
		return;
	}

	m_Context = alcGetCurrentContext();
	m_Device = alcGetContextsDevice(m_Context);

	calculateMaxSourceCount();
	createDefaultBufferGroup();

	AppUtility::fLogMessage("AudioManager: Number of concurrent audio sources supported: %d", Ogre::LML_NORMAL, true, m_MaxSourceCount);
}

AudioManager::~AudioManager()
{
	ALboolean error = alureShutdownDevice();

	if (error == AL_FALSE)
	{
		AppUtility::fLogMessage("Encountered error: `%s` when attempting to shutdown the audio device.",
			Ogre::LogMessageLevel::LML_CRITICAL, false, alureGetErrorString());
	}
}

AudioBufferGroup &AudioManager::getBufferGroup(std::string const &buffer_group_name, bool create_new_group, bool &group_was_found)
{
	auto found_buffer = m_BufferGroups.find(buffer_group_name);

	if (found_buffer != m_BufferGroups.end())
	{
		group_was_found = true;
		return found_buffer->second;
	}
	else
	{
		group_was_found = false;

		if (create_new_group)
		{
			auto new_group = m_BufferGroups.emplace(buffer_group_name, this, buffer_group_name, "").first;
			return new_group->second;
		}
		else
		{
			return m_BufferGroups[DEFAULT_AUDIO_GROUP_NAME];
		}
	}
}

AudioBufferGroup &AudioManager::getBufferGroup(std::string const &buffer_group_name, bool create_new_group = true)
{
	bool throwaway_temp = false;
	return getBufferGroup(buffer_group_name, create_new_group, throwaway_temp);
}

AudioBufferGroup &AudioManager::createBufferGroup(std::string const &buffer_group_name)
{
	bool throwaway_temp = false;
	return getBufferGroup(buffer_group_name, true, throwaway_temp);
}

void AudioManager::removeBufferGroup(std::string const &buffer_group_name)
{
	// The default buffer group should never be deleted as long as this audio manager is active. Instead we'll just empty it 
	// of all buffers, which effectively is the same result.
	if (buffer_group_name == DEFAULT_AUDIO_GROUP_NAME)
	{
		AudioBufferGroup default_group = getBufferGroup(DEFAULT_AUDIO_GROUP_NAME);
		default_group.removeAllBuffers();
	}
	else
	{
		auto group_to_remove = m_BufferGroups.find(buffer_group_name);

		if (group_to_remove != m_BufferGroups.end())
		{
			m_BufferGroups.erase(group_to_remove);
		}
	}
}

void AudioManager::removeAllBufferGroups(void)
{
	m_BufferGroups.clear();
	createDefaultBufferGroup();
}

ALCint AudioManager::calculateMaxSourceCount(void)
{
	ALCint attribute_count = 0;

	alcGetIntegerv(m_Device, ALC_ATTRIBUTES_SIZE, 1, &attribute_count);

	std::vector<ALCint> attributes(attribute_count);
	alcGetIntegerv(m_Device, ALC_ALL_ATTRIBUTES, attribute_count, &attributes[0]);

	for (size_t i = 0; i < attributes.size(); ++i)
	{
		if (attributes[i] == ALC_MONO_SOURCES)
		{
			m_MaxSourceCount = attributes[i + 1];
			break;
		}
	}

	m_MaxSourceCount = m_MaxSourceCount > MAX_AUDIO_SOURCES ? MAX_AUDIO_SOURCES : m_MaxSourceCount;
	return m_MaxSourceCount;
}

void AudioManager::createDefaultBufferGroup(void)
{
	auto default_buffer = m_BufferGroups.find(DEFAULT_AUDIO_GROUP_NAME);

	if (default_buffer == m_BufferGroups.end())
	{
		m_BufferGroups.emplace(DEFAULT_AUDIO_GROUP_NAME, this, DEFAULT_AUDIO_GROUP_NAME, m_BufferGroupPathPrefix);
	}
}

void AudioManager::enterFailureState(void)
{
	AppUtility::logMessage("AudioManager entered failure state.", Ogre::LML_CRITICAL);

	ALboolean error = alureShutdownDevice();

	if (error == AL_FALSE)
	{
		AppUtility::fLogMessage("Encountered error: `%s` when attempting to shutdown the audio device.",
			Ogre::LogMessageLevel::LML_CRITICAL, false, alureGetErrorString());
	}

	m_Device = NULL;
	m_Context = NULL;
	m_MaxSourceCount = 0;
}