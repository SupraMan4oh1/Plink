#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include <cstdlib>
#include <boost/program_options.hpp>

#include "AppUtility.h"
#include "Application.h"

#ifdef _WINDOWS
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#else
int main(int argc, char *argv[])
#endif
{
#ifdef _WINDOWS
	bool show_system_console = false;

	// Declare the supported options.
	boost::program_options::options_description description("Allowed options");
	description.add_options()
		("console", "Display system console with log output.");

	boost::program_options::variables_map variables_map = Kyanite::AppUtility::parseCommandLine(description, lpCmdLine);

	if (variables_map.count("console"))
	{
		show_system_console = true;
		Kyanite::AppUtility::showWin32Console();
	}
#endif

	// Create our application object.
	Application app;

	try
	{
		app.run();
	}
	catch (Ogre::Exception& e)
	{
#ifdef _WINDOWS
		MessageBox(NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
		std::cerr << "An exception has occured: " << e.getFullDescription().c_str() << std::endl;
#endif
	}

#ifdef _WINDOWS
	if (show_system_console == true)
	{
		FreeConsole();
	}
#endif

	return EXIT_SUCCESS;
}