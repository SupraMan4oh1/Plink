#pragma once

#include <string>
#include <vector>

#include <AL/alure.h>

namespace Menura
{
	/** @brief Contains decoded audio data ready to be loaded by the audio system, as well as all descriptive information about the data 
	that may also be needed to use it. */
	struct AudioData
	{
		std::vector<ALubyte> data;	//!< @brief The audio data.

		ALenum format;				//!< @brief Format of the audio data.
		ALuint frequency;			//!< @brief Frequency of the audio data.
		ALuint blockSize;			/**< @brief Block size of the audio data. 
									@details Block size is the size in bytes of a single sample of the data. e.x. 4 bytes for STEREO16. */

		AudioData(size_t data_length = 0);

		AudioData(AudioData const &source) = default;
		AudioData &operator=(AudioData const &source) = default;

		AudioData(AudioData &&source);
		AudioData &operator=(AudioData &&source);

		bool operator==(size_t data_length);
	};

	/** @brief Extends the functionality of the Alure library by providing additional audio and file utilities. 
	@note This class relies on an internal interface used within the Alure library, that wasn't meant to part of the public 
	interface. However the extended functionality added by this class wouldn't be possible without using this internal 
	interface, at least not without rewriting enough functionality that Alure would no longer be needed. As such, there 
	is the possibility that this class will be broken in future versions of Alure, and will need changes to function properly 
	if this internal interface ever changes. */
	class AlureExtension
	{
	public:

		/** @brief Loads audio data from a file and into an AudioData instance, where it can be accessed and/or edited.
		@param [in] file_path Path of the file to load.
		@param [out] successful Set to `true` if the file was successfully loaded, `false` if an error occured. 
		@returns AudioData, which contains the loaded audio data and attributes that describe the data. */
		static AudioData loadAudioDataFromFile(std::string const &file_path, bool &successful);

		/** @brief Checks if the file exists, with optional error logging if it doesn't. 
		@param [in] file_path Path of the file to check. 
		@param [in] log_enabled Print an error message to the log if `true` and the file doesn't exist.
		@returns `true` if the file was found, `false` if not found. */
		static bool checkIfFileExists(std::string const &file_path, bool log_enabled = false);
	};

}
