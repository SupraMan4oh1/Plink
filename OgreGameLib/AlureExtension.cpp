#include "AlureExtension.h"

#include "AppUtility.h"
#include <AL/main.h>

#include <boost/filesystem.hpp>

using namespace Menura;

AudioData::AudioData(size_t data_length) : data(std::vector<ALubyte>(data_length))
{

}

AudioData::AudioData(AudioData &&source) : data(std::move(source.data))
{
	format = source.format;
	frequency = source.frequency;
	blockSize = source.blockSize;
}

AudioData &AudioData::operator=(AudioData &&source)
{
	data = std::move(source.data);

	format = source.format;
	frequency = source.frequency;
	blockSize = source.blockSize;
}

bool AudioData::operator==(size_t data_length)
{
	return data.size() == data_length;
}

AudioData AlureExtension::loadAudioDataFromFile(std::string const &file_path, bool &successful)
{
	successful = false;
	alureStream *file_stream = create_stream(file_path.c_str());

	std::unique_ptr<std::istream> fstream(file_stream->fstream);
	std::unique_ptr<alureStream> stream(file_stream);

	ALenum format;
	ALuint frequency, block_size;

	if (!stream->GetFormat(&format, &frequency, &block_size))
	{
		Kyanite::AppUtility::fLogMessage("Could not get an audio sample format from file '%s'.", file_path.c_str());
		return AudioData();
	}

	if (format == AL_NONE)
	{
		Kyanite::AppUtility::fLogMessage("No valid audio format in file '%s'.", file_path.c_str());
		return AudioData();
	}

	if (block_size == 0)
	{
		Kyanite::AppUtility::fLogMessage("Invalid block size in file '%s'.", file_path.c_str());
		return AudioData();
	}

	if (frequency == 0)
	{
		Kyanite::AppUtility::fLogMessage("Invalid sample rate in file '%s'.", file_path.c_str());
		return AudioData();
	}

	ALuint write_position = 0, bytes_written;
	AudioData audio_data(frequency * block_size);

	while ((bytes_written = stream->GetData(&audio_data.data[write_position], audio_data.data.size() - write_position)) > 0)
	{
		write_position += bytes_written;
		audio_data.data.resize(write_position + frequency * block_size);
	}

	audio_data.data.resize(write_position - (write_position % block_size));
	stream.reset(NULL);

	audio_data.format = format;
	audio_data.frequency = frequency;
	audio_data.blockSize = block_size;

	successful = true;
	return audio_data;
}

bool AlureExtension::checkIfFileExists(std::string const &file_path, bool log_enabled)
{
	if (!boost::filesystem::exists(file_path) || !boost::filesystem::is_regular_file(file_path))
	{
		if (log_enabled)
		{
			Kyanite::AppUtility::fLogMessage("The audio file at '%s' does not exist or isn't a file.", file_path.c_str());
		}

		return false;
	}

	return true;
}
