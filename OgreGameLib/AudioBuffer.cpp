#include "AudioBuffer.h"

#include "AppUtility.h"
#include "AudioSource.h"

using namespace Menura;

AudioBuffer::AudioBuffer(std::string buffer_name, bool retain_in_memory) 
: m_BufferName(std::move(buffer_name)), m_BufferID(AL_NONE), m_IsRetainedInMemory(retain_in_memory), m_IsLoaded(false)
{

}

AudioBuffer::AudioBuffer(std::string buffer_name, std::string file_path, bool retain_in_memory) 
: m_BufferName(std::move(buffer_name)), m_FilePath(std::move(file_path)), m_IsRetainedInMemory(retain_in_memory), m_IsLoaded(false)
{
	loadBuffer();
}

AudioBuffer::~AudioBuffer()
{
	unloadBuffer(true);
}

bool AudioBuffer::loadBuffer(void)
{
	if (m_IsLoaded)
	{
		return true;
	}

	if (!AlureExtension::checkIfFileExists(m_FilePath, true))
	{
		m_FilePath = "";
		return false;
	}

	if (!m_IsRetainedInMemory)
	{
		m_BufferID = alureCreateBufferFromFile(m_FilePath.c_str());
		m_IsLoaded = bufferCreatedSuccessfully(m_BufferID) ? true : false;
	}
	else
	{
		bool loadSuccess = true;
		m_BufferData = m_BufferData == 0 ? AlureExtension::loadAudioDataFromFile(m_FilePath, loadSuccess) : m_BufferData;

		if (loadSuccess)
		{
			m_BufferID = alureCreateBufferFromMemory(&m_BufferData.data[0], m_BufferData.data.size());
			m_IsLoaded = bufferCreatedSuccessfully(m_BufferID) ? true : false;
		}
		else
		{
			m_BufferID = AL_NONE;
			return false;
		}
	}

	return true;
}

bool AudioBuffer::unloadBuffer(bool force_purge)
{
	if (!m_IsLoaded)
	{
		return true;
	}

	if (force_purge)
	{
		for (auto iter = m_ReferencingSources.begin(); iter != m_ReferencingSources.end(); ++iter)
		{
			AudioSource::AudioBufferInterface::unsetBuffer(iter->second, false);
		}

		m_ReferencingSources.clear();
	}

	alDeleteBuffers(1, &m_BufferID);

	if (!alIsBuffer(m_BufferID))
	{
		m_BufferID = AL_NONE;
		m_IsLoaded = false;
	}
	else
	{
		Kyanite::AppUtility::fLogMessage("Encountered error '%s' when attempting to unload the buffer '%s'; \
			buffer is still in active use by sources and cannot be unloaded.",
			alureGetErrorString(), m_BufferName.c_str());

		return false;
	}

	return true;
}

std::string const &AudioBuffer::name(void) const
{
	return m_BufferName;
}

ALuint AudioBuffer::id(void) const
{
	return m_BufferID;
}

ALint AudioBuffer::frequency(void) const
{
	ALint frequency;
	alGetBufferi(m_BufferID, AL_FREQUENCY, &frequency);

	return frequency;
}

ALint AudioBuffer::byteSize(void) const
{
	ALint byteSize;
	alGetBufferi(m_BufferID, AL_SIZE, &byteSize);

	return byteSize;
}

ALint AudioBuffer::bitsPerSample(void) const
{
	ALint bitsPerSample;
	alGetBufferi(m_BufferID, AL_BITS, &bitsPerSample);

	return bitsPerSample;
}

ALint AudioBuffer::channelCount(void) const
{
	ALint channelCount;
	alGetBufferi(m_BufferID, AL_CHANNELS, &channelCount);

	return channelCount;
}

ALint AudioBuffer::sampleCount(void) const
{
	return (byteSize() * 8) / (channelCount() * bitsPerSample());
}

float AudioBuffer::duration(void) const
{
	return (float)sampleCount() / (float)frequency();
}

AudioData &AudioBuffer::audioData(void)
{
	if (!m_IsRetainedInMemory)
	{
		m_IsRetainedInMemory = true;

		bool load_success;
		m_BufferData = AlureExtension::loadAudioDataFromFile(m_FilePath, load_success);

		if (load_success && m_IsLoaded)
		{
			load_success = alureBufferDataFromMemory(&m_BufferData.data[0], m_BufferData.data.size(), m_BufferID);

			if (!load_success)
			{
				Kyanite::AppUtility::fLogMessage("Encountered error '%s' when attempting to update audio buffer from memory.",
					Ogre::LML_CRITICAL, false, alureGetErrorString());
			}
		}
	}
	else if (m_BufferData == 0)
	{
		bool load_success;
		m_BufferData = AlureExtension::loadAudioDataFromFile(m_FilePath, load_success);
	}
	
	return m_BufferData;
}

AudioData const &AudioBuffer::c_AudioData(void) const
{
	if (!m_IsRetainedInMemory)
	{
		bool load_success;
		return AlureExtension::loadAudioDataFromFile(m_FilePath, load_success);
	}
	else
	{
		return m_BufferData;
	}
}

bool AudioBuffer::operator==(AudioBuffer const &other)
{
	return m_BufferID == other.m_BufferID;
}

bool AudioBuffer::bufferCreatedSuccessfully(ALuint buffer_id)
{
	if (buffer_id == AL_NONE)
	{
		Kyanite::AppUtility::fLogMessage("Encountered error '%s' when attempting to load an audio buffer from file '%s'.",
			Ogre::LML_CRITICAL, false, alureGetErrorString(), m_FilePath.c_str());

		return false;
	}

	return true;
}