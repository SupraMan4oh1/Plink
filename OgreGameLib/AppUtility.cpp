#include "AppUtility.h"
#include "Constants.h"

#ifdef _WINDOWS

#include <cstdarg>
#include <cstdio>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <string>

using namespace Kyanite;

void AppUtility::showWin32Console(void)
{
	int console_handle;
	long std_io_handle;
	CONSOLE_SCREEN_BUFFER_INFO console_info;
	FILE *file_pointer;

	// Allocate a console.
	AllocConsole();

	// Set the screen buffer to be big enough to let us scroll text.
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &console_info);
	console_info.dwSize.Y = CONSOLE_MAX_LINE_COUNT;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), console_info.dwSize);

	// Redirect unbuffered STDOUT to the console.
	std_io_handle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
	console_handle = _open_osfhandle(std_io_handle, _O_TEXT);
	file_pointer = _fdopen(console_handle, "w");
	*stdout = *file_pointer;
	setvbuf(stdout, NULL, _IONBF, 0);

	// Redirect unbuffered STDIN to the console.
	std_io_handle = (long)GetStdHandle(STD_INPUT_HANDLE);
	console_handle = _open_osfhandle(std_io_handle, _O_TEXT);
	file_pointer = _fdopen(console_handle, "r");
	*stdin = *file_pointer;
	setvbuf(stdin, NULL, _IONBF, 0);

	// Redirect unbuffered STDERR to the console.
	std_io_handle = (long)GetStdHandle(STD_ERROR_HANDLE);
	console_handle = _open_osfhandle(std_io_handle, _O_TEXT);
	file_pointer = _fdopen(console_handle, "w");
	*stderr = *file_pointer;
	setvbuf(stderr, NULL, _IONBF, 0);

	// Make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog point to the console as well.
	std::ios::sync_with_stdio();
}

boost::program_options::variables_map AppUtility::parseCommandLine(boost::program_options::options_description const &description,
	LPSTR lp_cmd_line)
{
	boost::program_options::variables_map variables_map;

	std::vector<std::string> command_line_args = boost::program_options::split_winmain(lp_cmd_line);
	boost::program_options::store(boost::program_options::command_line_parser(command_line_args).options(description).run(), variables_map);
	boost::program_options::notify(variables_map);

	return variables_map;
}

#endif

void AppUtility::logMessage(std::string const &msg, Ogre::LogMessageLevel level, bool mask_debug)
{
	Ogre::LogManager::getSingleton().logMessage(msg, level, mask_debug);
}

void AppUtility::fLogMessage(std::string const &format_string, Ogre::LogMessageLevel level, bool mask_debug, ...)
{
#ifdef NDEBUG
	// We check the debug flag ourselves instead of letting Ogre handle it, because we can skip handling the variable arguments.
	if (mask_debug)
	{
		return;
	}
#endif

	va_list arguments;
	char string_buffer[STRING_BUFFER_LENGTH];

	va_start(arguments, mask_debug);
	vsnprintf_s(string_buffer, STRING_BUFFER_LENGTH, format_string.c_str(), arguments);
	va_end(arguments);

	Ogre::LogManager::getSingleton().logMessage(string_buffer, level, mask_debug);
}