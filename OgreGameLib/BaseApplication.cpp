#include "BaseApplication.h"

#include "Constants.h"
#include "AppUtility.h"

#include <boost/filesystem.hpp>

BaseApplication::BaseApplication(void) : m_SetupComplete(false), m_SetupRun(false), m_Root(0), m_Camera(0), m_SceneMgr(0), m_Window(0), 
                                         m_ResourcesCfg(Ogre::StringUtil::BLANK), m_PluginsCfg(Ogre::StringUtil::BLANK), m_CameraMan(0), 
										 m_CursorWasVisible(false), m_Shutdown(false), m_InputManager(0), m_Mouse(0), m_Keyboard(0)
{
#ifdef _DEBUG
	m_ResourcesCfg = RESOURCE_DEBUG_FILE;
	m_PluginsCfg = PLUGIN_DEBUG_FILE;
#else
	m_ResourcesCfg = RESOURCE_FILE;
	m_PluginsCfg = PLUGIN_FILE;
#endif
}

BaseApplication::~BaseApplication(void)
{
	if (m_CameraMan)
	{
		delete m_CameraMan;
	}

	// Remove ourselves as a window listener.
	Ogre::WindowEventUtilities::removeWindowEventListener(m_Window, this);
	windowClosed(m_Window);
	delete m_Root;
}

bool BaseApplication::configure(char const *window_title)
{
	// Show the configuration dialog and initialize the system. You can skip this and use m_Root.restoreConfig() 
	// to load configuration settings if you were sure there are valid ones saved in ogre.cfg.
	if (m_Root->showConfigDialog())
	{
		// If returned true, the user clicked OK, so we can initialize. Here we choose to let the system create 
		// a default rendering window by passing "true".
		m_Window = m_Root->initialise(true, window_title);
		return true;
	}
	else
	{
		return false;
	}
}

void BaseApplication::chooseSceneManager(Ogre::SceneType const scene_type)
{
	// Get the SceneManager.
	m_SceneMgr = m_Root->createSceneManager(scene_type);
}

void BaseApplication::createCamera(char const *camera_name)
{
	// Create the camera.
	m_Camera = m_SceneMgr->createCamera(camera_name);

	// Set the default position of the camera.
	m_Camera->setPosition(Ogre::Vector3(0, 0, 0));

	// Look back along -Z.
	m_Camera->lookAt(Ogre::Vector3(0, 0, -300));
	m_Camera->setNearClipDistance(1);

	m_CameraMan = new OgreBites::SdkCameraMan(m_Camera);	// Create a default camera controller.
}

void BaseApplication::createFrameListener(void)
{
	Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");
	OIS::ParamList paramList;
	size_t windowHnd = 0;
	std::ostringstream windowHndStr;

	m_Window->getCustomAttribute("WINDOW", &windowHnd);
	windowHndStr << windowHnd;
	paramList.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));

	m_InputManager = OIS::InputManager::createInputSystem(paramList);

	m_Keyboard = static_cast<OIS::Keyboard *>(m_InputManager->createInputObject(OIS::OISKeyboard, true));
	m_Mouse = static_cast<OIS::Mouse *>(m_InputManager->createInputObject(OIS::OISMouse, true));

	m_Keyboard->setEventCallback(this);
	m_Mouse->setEventCallback(this);

	// Set the initial mouse clipping size.
	windowResized(m_Window);

	// Register as a window listener.
	Ogre::WindowEventUtilities::addWindowEventListener(m_Window, this);

	m_Root->addFrameListener(this);
}

void BaseApplication::destroyScene(void)
{

}

void BaseApplication::createViewports(Ogre::ColourValue const &bg_color)
{
	// Create one viewport that covers the entire window.
	Ogre::Viewport *viewport = m_Window->addViewport(m_Camera);
	viewport->setBackgroundColour(bg_color);

	// Alter the camera aspect ratio to match the viewport.
	m_Camera->setAspectRatio(Ogre::Real(viewport->getActualWidth()) / Ogre::Real(viewport->getActualHeight()));
}

void BaseApplication::setupResources(void)
{
	if (!boost::filesystem::exists(m_ResourcesCfg) || !boost::filesystem::is_regular_file(m_ResourcesCfg))
	{
		Kyanite::AppUtility::fLogMessage("No resources config file exists at \'%s\'. The program cannot load any resources and must exit.", 
			Ogre::LML_CRITICAL, false, m_ResourcesCfg.c_str());

		exit(EXIT_FAILURE);
	}

	// Load resource paths from a config file.
	Ogre::ConfigFile configFile;
	configFile.load(m_ResourcesCfg);

	// Go through all sections & settings in the file.
	Ogre::ConfigFile::SectionIterator iterator = configFile.getSectionIterator();

	Ogre::String secName, typeName, archName;

	while (iterator.hasMoreElements())
	{
		secName = iterator.peekNextKey();
		Ogre::ConfigFile::SettingsMultiMap *settings = iterator.getNext();
		Ogre::ConfigFile::SettingsMultiMap::iterator i;

		for (i = settings->begin(); i != settings->end(); ++i)
		{
			typeName = i->first;
			archName = i->second;
			Ogre::ResourceGroupManager::getSingleton().addResourceLocation(archName, typeName, secName);
		}
	}
}

void BaseApplication::createResourceListener(void)
{

}

void BaseApplication::loadResources(void)
{
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}

void BaseApplication::run(void)
{
	if (!m_SetupRun)
	{
		m_SetupComplete = setup();
	}

	if (!m_SetupComplete)
	{
		return;
	}

	while (!m_Shutdown)
	{
		// Pump window messages so the program behaves itself.
		Ogre::WindowEventUtilities::messagePump();

		if (m_Window->isClosed())
		{
			m_Shutdown = true;
			break;
		}

		if (m_Window->isActive())
		{
			if (!m_Root->renderOneFrame())
			{
				m_Shutdown = true;
				break;
			}
		}
		else
		{
			m_Root->clearEventTimes();
		}
	}

	// Cleanup
	destroyScene();
}

bool BaseApplication::setup(void)
{
	if (!boost::filesystem::exists(m_PluginsCfg) || !boost::filesystem::is_regular_file(m_PluginsCfg))
	{
		Kyanite::AppUtility::fLogMessage("No plugin config file exists at \'%s\'. The program cannot load any render-systems and will exit.",
			Ogre::LML_CRITICAL, false, m_PluginsCfg.c_str());

		exit(EXIT_FAILURE);
	}

	m_SetupRun = true;
	m_Root = new Ogre::Root(m_PluginsCfg);

	setupResources();

	m_SetupComplete = configure(PROJECT_NAME.c_str());

	if (!m_SetupComplete)
	{
		return false;
	}

	chooseSceneManager();
	createCamera("MainCamera");
	createViewports(Ogre::ColourValue(0, 0, 0));

	// Set the default mipmap level (note that some APIs ignore this).
	Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);

	// Create any resource listeners (for loading screens).
	createResourceListener();

	// Load resources.
	loadResources();

	createFrameListener();

	return true;
}

bool BaseApplication::frameRenderingQueued(Ogre::FrameEvent const &evt)
{
	if (m_Window->isClosed())
	{
		return false;
	}

	if (m_Shutdown)
	{
		return false;
	}

	// Need to capture/update each device.
	m_Keyboard->capture();
	m_Mouse->capture();

	// Update the camera.
	m_CameraMan->frameRenderingQueued(evt);

	return true;
}

bool BaseApplication::keyPressed(OIS::KeyEvent const &arg)
{
	// Refresh all textures.
	if (arg.key == OIS::KC_F5)
	{
		Ogre::TextureManager::getSingleton().reloadAll();
	}

	// Take a screenshot.
	if (arg.key == OIS::KC_SYSRQ)
	{
		m_Window->writeContentsToTimestampedFile("screenshot", ".jpg");
	}

	// Quit
	if (arg.key == OIS::KC_ESCAPE)
	{
		m_Shutdown = true;
	}

	return true;
}

bool BaseApplication::keyReleased(OIS::KeyEvent const &arg)
{
	return true;
}

bool BaseApplication::mouseMoved(OIS::MouseEvent const &arg)
{
	return true;
}

bool BaseApplication::mousePressed(OIS::MouseEvent const &arg, OIS::MouseButtonID button_id)
{
	return true;
}

bool BaseApplication::mouseReleased(OIS::MouseEvent const &arg, OIS::MouseButtonID button_id)
{
	return true;
}

void BaseApplication::windowFocusChange(Ogre::RenderWindow *render_window)
{
	return;
}

void BaseApplication::windowResized(Ogre::RenderWindow *render_window)
{
	unsigned int width, height, depth;
	int left, top;

	render_window->getMetrics(width, height, depth, left, top);

	// Adjust the mouse clipping area.
	const OIS::MouseState &mouseState = m_Mouse->getMouseState();
	mouseState.width = width;
	mouseState.height = height;
}

void BaseApplication::windowClosed(Ogre::RenderWindow *render_window)
{
	// We need to unattach OIS before closing the window (very important under Linux).
	// We also need to ensure that we only close for the window that created OIS.
	if (render_window == m_Window)
	{
		if (m_InputManager)
		{
			m_InputManager->destroyInputObject(m_Mouse);
			m_InputManager->destroyInputObject(m_Keyboard);

			OIS::InputManager::destroyInputSystem(m_InputManager);
			m_InputManager = 0;
		}
	}
}