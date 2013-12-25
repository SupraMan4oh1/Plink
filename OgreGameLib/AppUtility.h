#pragma once

#ifdef _WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#endif

#include <OgreLogManager.h>
#include <boost/program_options.hpp>

namespace Kyanite
{

	/** @brief Various application related utility methods that are helpful around the program. */
	class AppUtility
	{

	public:

#ifdef _WINDOWS
		static void showWin32Console(void); //!< @brief Shows a console under Windows. @note Only available under Win32.

		/** @brief Parses out the command-line arguments sent to the application.
		@note Only available under Win32.
		@param [in] description The description that defines the different command-line options.
		@param [in] lp_cmd_line The command-line string passed to the application; this is the string that will be parsed.
		@returns A map of the command-line options passed to the program and the values for those options. */
		static boost::program_options::variables_map parseCommandLine(boost::program_options::options_description const &description,
			LPSTR lp_cmd_line);
#endif

		/** @brief Print a message to the log.
		@param [in] msg The message to print.
		@param [in] level The message level of this message.
		@param [in] mask_debug Is this a regular or a debug message. */
		static void logMessage(std::string const &msg, Ogre::LogMessageLevel level = Ogre::LogMessageLevel::LML_NORMAL, bool mask_debug = false);

		/** @brief Print a formatted message to the log.
		@param [in] format_string The format string of the message to print.
		@param [in] level The message level of this message.
		@param [in] mask_debug Is this a regular or a debug message.
		@param [in] ... Arguments for the format string. */
		static void fLogMessage(std::string const &format_string, Ogre::LogMessageLevel level, bool mask_debug, ...);

		/** @overload fLogMessage(std::string const &format_string, Ogre::LogMessageLevel level, bool mask_debug, ...) */
		static void fLogMessage(std::string const &format_string, ...);
	};

}