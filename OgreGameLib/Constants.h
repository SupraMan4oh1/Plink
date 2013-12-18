#pragma once

/**
@file Constants.h
@brief Constants used throughout the project.

@note Constants should only be values that are either needed at compile-time, or constant values (obviously), 
that are needed before any scripting capabilities are loaded, preventing the easy loading of them from a file.
They may also be values that for various reasons, should not be alterable by end-users.

@note All relative paths are relative to the executable.
*/

#include <cstddef>
#include <string>

static const std::string PROJECT_NAME = "Fly By Night"; //!< @brief The name of the project.

static const std::string RESOURCE_FILE = "resources.cfg";			//!< @brief Relative path to the resources config file.
static const std::string RESOURCE_DEBUG_FILE = "resources_d.cfg";	//!< @brief Relative path to the debug resources config file.
static const std::string PLUGIN_FILE = "plugins.cfg";				//!< @brief Relative path to the plugins file.
static const std::string PLUGIN_DEBUG_FILE = "plugins_d.cfg";		//!< @brief Relative path to the debug plugins file.

static const std::string DEFAULT_LOG_FILE = "fly_by_night.log";		//!< @brief Relative path to the default log file.

static const size_t MAX_FILE_PATH_LENGTH = 1024; /**< @brief The maximum supported file-path length. This value is used to create 
temp file-path buffers on the stack in performance critical code. */

static const size_t STRING_BUFFER_LENGTH = 1024; //!< Buffer length for generic strings created on the stack in performance critical code.
static const size_t CONSOLE_MAX_LINE_COUNT = 1024;  //!< The number of lines the custom console will be able to display at once.

static const std::string PREFERENCE_FILE = "preferences.lua";                      //!< @brief Relative path to the default preferences file.
static const std::string DEFAULT_PREFERENCE_FILE = "default_preferences.lua";      /**< @brief Relative path to the backup default preferences file. 
When `PREFERENCE_FILE` doesn't exist, this is the file that is copied and used to recreate it. */
static const std::string PREFERENCE_SET_NAME = "Preferences";                      //!< @brief Name of default preferences set.