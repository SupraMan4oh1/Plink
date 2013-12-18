#pragma once

#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreLogManager.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreConfigFile.h>

#include <OISEvents.h>
#include <OISInputManager.h>
#include <OISKeyboard.h>
#include <OISMouse.h>

#include <SdkCameraMan.h>

/** @brief Abstract application class.

BaseApplication is meant to be subclassed by the class that will serve as the core application class, which is responsible for controlling 
the basic flow of the application. */
class BaseApplication : public Ogre::FrameListener, public Ogre::WindowEventListener, public OIS::KeyListener, public OIS::MouseListener
{

public:

	BaseApplication(void);
	virtual ~BaseApplication(void);

	virtual void run(void);						//!< @brief Starts the main-loop of the application.

protected:

	/* ----- Instance Variables ----- */

	bool m_SetupComplete;						//!< Has setup been completed?
	bool m_SetupRun;							//!< Has setup been run?

	Ogre::Root *m_Root;							//!< Ogre root object.
	Ogre::Camera *m_Camera;						//!< Default camera.
	Ogre::SceneManager *m_SceneMgr;				//!< Default scene manager.
	Ogre::RenderWindow *m_Window;				//!< Default render window.
	Ogre::String m_ResourcesCfg;				//!< Path to the resources file.
	Ogre::String m_PluginsCfg;					//!< Path to the plugins file.

	OgreBites::SdkCameraMan *m_CameraMan;		//!< Basic camera controller.
	bool m_CursorWasVisible;					//!< Was the cursor visible before the dialog appeared?
	bool m_Shutdown;							//!< Should the program now shutdown?

	// OIS Input Devices
	OIS::InputManager *m_InputManager;			//!< Default input manager.
	OIS::Mouse *m_Mouse;						//!< Default mouse.
	OIS::Keyboard *m_Keyboard;					//!< Default keyboard.

	/// @brief Setup the application.
	/// @returns `true` if setup completed successfully, `false` if it failed.
	virtual bool setup(void);

	/** @brief Shows the configuration dialog and initializes the application.

	If the configuration settings in the config file (by default `ogre.cfg`) are known to be valid, 
	this can be skipped and `m_Root.restoreConfig()` called instead.

	@param [in] window_title Title of the default application window.
	@returns `true` if configured and initialized successfully, `false` if it failed or was canceled. */
	virtual bool configure(char const *window_title);

	/** @brief Choose the type of the default scene manager.
	@param [in] scene_type The type of scene manager to use. */
	virtual void chooseSceneManager(Ogre::SceneType const scene_type = Ogre::ST_GENERIC);

	/** @brief Create a camera with a given name.
	@param [in] camera_name Name of the new camera. */
	virtual void createCamera(char const *camera_name);

	/** @brief Register this class to handle window and IO events. */
	virtual void createFrameListener(void);

	/** @brief Pure virtual function meant to be implemented by the subclass. This is where the scene should be setup. */
	virtual void createScene(void) = 0;

	/** @brief Destroy the scene. */
	virtual void destroyScene(void);

	/** @brief Create the viewport that will be rendered into.
	@param [in] bg_color The background color of this viewport. */
	virtual void createViewports(Ogre::ColourValue const &bg_color);

	/** @brief Load and setup all the resource paths defined in the resource config. */
	virtual void setupResources(void);

	/** @brief Intended to be overridden by a subclass, this is where you would create any resource listeners. */
	virtual void createResourceListener(void);

	/** @brief Initializes all resource groups. */
	virtual void loadResources(void);

	/* ----- Ogre::FrameListener ----- */

	/** @brief Called after all render targets have had their rendering commands issued, but before render windows have been
	asked to flip their buffers over.

	The usefulness of this event comes from the fact that rendering commands are queued for the GPU to process. 
	These can take a little while to finish, and so while that is happening the CPU can be doing useful things. Once the 
	request to 'flip buffers' happens, the thread requesting it will block until the GPU is ready, which can waste CPU 
	cycles. Therefore, it is often a good idea to use this callback to perform per-frame processing. Of course because the 
	frame's rendering commands have already been issued, any changes you make will only take effect from the next frame, 
	but in most cases that's not noticeable.

	@param [in] evt The frame event passed to this method by Ogre.
	@returns `true` to continue rendering, `false` to drop out of the rendering loop. */
	virtual bool frameRenderingQueued(Ogre::FrameEvent const &evt);

	/* ----- OIS::KeyListener ----- */

	/** @brief Called when a key is pressed.
	@param [in] arg The key event passed to this method by OIS.
	@returns `true` tells OIS to keep feeding the buffered input. `false` discards the currently buffered input. */
	virtual bool keyPressed(OIS::KeyEvent const &arg);

	/** @brief Called when a key is released.
	@param [in] arg The key event passed to this method by OIS.
	@returns `true` tells OIS to keep feeding the buffered input. `false` discards the currently buffered input. */
	virtual bool keyReleased(OIS::KeyEvent const &arg);

	/* ----- OIS::MouseListener ----- */

	/** @brief Called when the mouse is moved.
	@param [in] arg The mouse event passed to this method by OIS.
	@returns `true` tells OIS to keep feeding the buffered input. `false` discards the currently buffered input. */
	virtual bool mouseMoved(OIS::MouseEvent const &arg);

	/** @brief Called when a mouse button is pressed.
	@param [in] arg The mouse event passed to this method by OIS.
	@param [in] button_id The ID of the mouse button that was pressed.
	@returns `true` tells OIS to keep feeding the buffered input. `false` discards the currently buffered input. */
	virtual bool mousePressed(OIS::MouseEvent const &arg, OIS::MouseButtonID button_id);

	/** @brief Called when a mouse button is released.
	@param [in] arg The mouse event passed to this method by OIS.
	@param [in] button_id The ID of the mouse button that was released.
	@returns `true` tells OIS to keep feeding the buffered input. `false` discards the currently buffered input. */
	virtual bool mouseReleased(OIS::MouseEvent const &arg, OIS::MouseButtonID button_id);

	/* ----- Ogre::WindowEventListener ----- */

	/** @brief Called when the render window loses or gains focus.
	@param [in] render_window The render window that the focus changed on. */
	virtual void windowFocusChange(Ogre::RenderWindow *render_window);

	/** @brief Called when the render window is resized.
	@param [in] render_window The render window that was resized. */
	virtual void windowResized(Ogre::RenderWindow *render_window);

	/** @brief Called right before the render window is closed.
	@param [in] render_window The render window to be closed. */
	virtual void windowClosed(Ogre::RenderWindow *render_window);

private:

	BaseApplication(BaseApplication const &source);
	const BaseApplication& operator=(BaseApplication const &source);

	BaseApplication(BaseApplication &&source);
	BaseApplication& operator=(BaseApplication &&source);
};