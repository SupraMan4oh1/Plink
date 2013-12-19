#include "Application.h"

#include "Constants.h"
#include "Globals.h"

#include "AudioManager.h"
#include "AudioBufferGroup.h"

Application::Application(void)
{
	Globals::app = this;

	// We want to have our own custom log manager, so we create this (it will be made the singleton because it is the 
	// first log manager created) before we setup all of Ogre's facilities.
	Ogre::LogManager *default_log_manager = new Ogre::LogManager;
	default_log_manager->createLog(DEFAULT_LOG_FILE, true, true, false);

	// Setup all of Ogre's facilities.
	if (!setup())
	{
		return;
	}

	// Create the scene.
	createScene();

	Kyanite::AudioManager *newAudioManager = new Kyanite::AudioManager;
	newAudioManager->createBufferGroup("TestBufferGroup");
}

Application::~Application(void)
{
	Globals::app = NULL;
}

Ogre::Root &Application::root(void)
{
	return *m_Root;
}

Ogre::SceneManager &Application::sceneManager(void)
{
	return *m_SceneMgr;
}

bool Application::frameRenderingQueued(Ogre::FrameEvent const &evt)
{
	bool ret = BaseApplication::frameRenderingQueued(evt);

	return ret;
}

void Application::createScene(void)
{
	m_SceneMgr->setAmbientLight(Ogre::ColourValue(0.5f, 0.5f, 0.5f));
}

bool Application::keyPressed(OIS::KeyEvent const &arg)
{
	BaseApplication::keyPressed(arg);
	m_CameraMan->injectKeyDown(arg);

	return true;
}

bool Application::keyReleased(OIS::KeyEvent const &arg)
{
	BaseApplication::keyReleased(arg);
	m_CameraMan->injectKeyUp(arg);

	return true;
}

bool Application::mouseMoved(OIS::MouseEvent const &arg)
{
	BaseApplication::mouseMoved(arg);
	m_CameraMan->injectMouseMove(arg);

	return true;
}

bool Application::mousePressed(OIS::MouseEvent const &arg, OIS::MouseButtonID buttonID)
{
	BaseApplication::mousePressed(arg, buttonID);
	m_CameraMan->injectMouseDown(arg, buttonID);

	return true;
}

bool Application::mouseReleased(OIS::MouseEvent const &arg, OIS::MouseButtonID buttonID)
{
	BaseApplication::mouseReleased(arg, buttonID);
	m_CameraMan->injectMouseUp(arg, buttonID);

	return true;
}

void Application::windowFocusChange(Ogre::RenderWindow *renderWindow)
{

}

void Application::windowResized(Ogre::RenderWindow *renderWindow)
{
	BaseApplication::windowResized(renderWindow);
}