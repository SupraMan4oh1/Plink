#include "AudioManager.h"

#include <cstdio>
#include <cstring>

#include "KyaniteConstants.h"
#include "AppUtility.h"

using namespace Kyanite;

AudioManager::AudioManager()
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

AudioManager::AudioManager(ALCchar *device_name, ALCint mono_sources_hint, ALCint stereo_sources_hint, ALCint frequency, ALCint refresh, ALCint sync)
{
	ALCint attributes[11] = { ALC_MONO_SOURCES, mono_sources_hint, ALC_STEREO_SOURCES, stereo_sources_hint, 
		ALC_FREQUENCY, frequency, ALC_REFRESH, refresh, ALC_SYNC, sync, 0};

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