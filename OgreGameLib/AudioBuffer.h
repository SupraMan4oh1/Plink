#pragma once

#include <string>
#include <vector>
#include <boost/unordered_map.hpp>
#include <AL/al.h>

#include "AlureExtension.h"

namespace Menura
{
	class AudioSource;

	/** @brief Audio buffer that stores audio data. */
	class AudioBuffer
	{
	public:

		/** @brief Defines a subset of the AudioBuffer interface that only AudioSource should have access to. */
		class AudioSourceInterface
		{
			friend AudioSource;

			/** @see AudioBuffer::addReferencingSource(AudioSource const &audio_source) */
			static void addReferencingSource(AudioBuffer &audio_buffer, AudioSource const &audio_source);
			/** @see AudioBuffer::removeReferencingSource(AudioSource const &audio_source) */
			static void removeReferencingSource(AudioBuffer &audio_buffer, AudioSource const &audio_source);
		};

		friend AudioSourceInterface;

		/** @brief Create a new AudioBuffer without loading any audio data.
		@param [in] buffer_name Human readable name of the buffer. 
		@param [in] retain_in_memory `true` causes the AudioBuffer to retain an editable copy of the audio data that can be used to update 
		the internal buffer held by the audio system. `false` causes the AudioBuffer to discard the copy, so only half the memory is needed, 
		but the internal buffer can no longer be updated.
		@returns A new AudioBuffer. */
		AudioBuffer(std::string buffer_name = "", bool retain_in_memory = false);

		/** @brief Create a new AudioBuffer loaded with the audio data in the file at `file_path`. 
		@param [in] buffer_name Human readable name of the buffer.
		@param [in] file_path Path to the file to load audio data from. 
		@param [in] retain_in_memory `true` causes the AudioBuffer to retain an editable copy of the audio data that can be used to update 
		the internal buffer held by the audio system. `false` causes the AudioBuffer to discard the copy, so only half the memory is needed, 
		but the internal buffer can no longer be updated. 
		@returns A new AudioBuffer. */
		AudioBuffer(std::string buffer_name, std::string file_path, bool retain_in_memory = false);
		~AudioBuffer();

		/** @brief Load the buffer into the audio system. @returns `true` if successful, `false` if failed. */
		bool loadBuffer(void);

		/** @brief Unload the buffer from the audio system. 
		@note Under OpenAL, if `force_purge` is `false` and any audio sources are using this buffer, it will fail to unload.
		@param [in] force_purge Forces all audio sources using this buffer to unset the buffer if `true`.
		@returns `true` if successful, `false` if failed. */
		bool unloadBuffer(bool force_purge = false);

		/** @brief Get the name of the buffer. @returns Name of the buffer. */
		std::string const &name(void) const;

		/** @brief Get the ID of the buffer. @returns ID of the buffer. */
		ALuint id(void) const;

		/** @brief Get the frequency of the buffer in samples per second [Hz]. @returns The frequency of the buffer. */
		ALint frequency(void) const;

		/** @brief Get the byte-length of the buffer data. @returns The length in bytes of the buffer data. */
		ALint byteSize(void) const;

		/** @brief Get the number of bits per sample. @returns The number of bits per sample. */
		ALint bitsPerSample(void) const;

		/** @brief Get the number of channels for the buffer data. @returns The number of channels for the buffer data. */
		ALint channelCount(void) const;

		/** @brief Get the number of samples stored in the buffer data. @returns The number of samples in the buffer data. */
		ALint sampleCount(void) const;

		/** @brief Get the duration of the buffer data in seconds. @returns The duration of the buffer data in seconds. */
		float duration(void) const;

		/** @brief Get the audio data stored in the buffer. @returns The data stored in the buffer. @see c_AudioData(void) */
		AudioData &audioData(void);

		/** @brief `const` version of audioData(void). @see audioData(void) */
		AudioData const &c_AudioData(void) const;

		bool operator==(AudioBuffer const &other);

	protected:

		std::string m_BufferName;	//!< @brief The user defined name of the buffer.
		ALuint m_BufferID;			//!< @brief The audio system defined ID of the buffer.

		std::string m_FilePath;		//!< @brief The file-path to the audio file this buffer was loaded from. Only used if loaded from a file.
		AudioData m_BufferData;		//!< @brief A copy of the audio data stored in the buffer and its attributes.

		bool m_IsRetainedInMemory;	//!< @brief Is an editable copy of the audio data stored in memory?
		bool m_IsLoaded;			//!< @brief Is this buffer also keeping a loaded buffer with the audio system?

		boost::unordered_map<int, AudioSource &> m_ReferencingSources;	//!< @brief All the audio sources that are currently using this buffer.

		/** @brief Adds an AudioSource to the list of sources that are currently referencing this buffer.
		@param [in] audio_source The AudioSource to add. */
		void addReferencingSource(AudioSource const &audio_source);

		/** @brief Remove an AudioSource from the list of sources that are currently referencing this buffer.
		@param [in] audio_source The AudioSource to remove. */
		void removeReferencingSource(AudioSource const &audio_source);

		/** @brief Checks if the internal audio buffer was created successfully, and prints an error message if it wasn't.
		@note This is meant to be called immediately after trying to create the internal audio buffer. 
		@param [in] buffer_id The new id returned when trying to create the buffer. 
		@returns `true` if created successfully, `false` if it failed. */
		bool bufferCreatedSuccessfully(ALuint buffer_id);
	};

}