#pragma once

#include "BaseApplication.h"

/** @brief Application class that is central to the entire program.

The Application class serves as the core of the program, and is responsible for controlling the basic flow of the application. 
It is from here that necessary subsystems, such as audio and physics are also launched and managed. */
class Application : public BaseApplication
{
public:

	Application(void);
	~Application(void);

	Ogre::Root &root(void);						//!< @brief Get the scene root. @returns The scene root.
	Ogre::SceneManager &sceneManager(void);		//!< @brief Get the default scene manager. @returns The default scene manager.

protected:

	bool frameRenderingQueued(const Ogre::FrameEvent &evt);			//!< @see BaseApplication::frameRenderingQueued
	void createScene(void);											//!< @brief Create the scene here. @see BaseApplication::createScene

	/* ----- OIS::KeyListener ----- */

	bool keyPressed(const OIS::KeyEvent &arg);	//!< @see BaseApplication::keyPressed
	bool keyReleased(const OIS::KeyEvent &arg);	//!< @see BaseApplication::keyReleased

	/* ----- OIS::MouseListener ----- */

	bool mouseMoved(const OIS::MouseEvent &arg);									//!< @see BaseApplication::mouseMoved
	bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID buttonID);		//!< @see BaseApplication::mousePressed
	bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID buttonID);	//!< @see BaseApplication::mouseReleased

	void windowFocusChange(Ogre::RenderWindow *renderWindow);						//!< @see BaseApplication::windowFocusChange
	void windowResized(Ogre::RenderWindow *renderWindow);							//!< @see BaseApplication::windowResized

private:

	Application(Application const &source);
	const Application& operator=(Application const &source);

	Application(Application &&source);
	Application& operator=(Application &&source);
};