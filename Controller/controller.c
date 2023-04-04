#include "controller.h"

#include "../Settings/settings.h"
#include "../HttpRequest/http.h"
#include "../MotionSensor/motionSensor.h"
#include "../Joystick/joystick.h"
#include "../LEDDisplay/ledDisplay.h"
#include "../LEDMFA/ledMfa.h"
#include "../Menu/menu.h"
#include "../PasswordManager/passwordManager.h"
#include "../PasswordInput/passwordInput.h"
#include "../DangerAnalyzer/dangerAnalyzer.h"
#include "../WebCam/Stream/StreamController.h"
#include "../WebCam/Stream/Stream.h"

void Controller_start(void)
{
    // init modules
    Http_init();
    Settings_init();
    PasswordManager_init();

    Joystick_init();
    MotionSensor_init();
    PasswordInput_init();
    
    // start threads
    LedDisplay_start();
    DangerAnalyzer_start();

    Menu_start();
    Mfa_start();

    Stream_init();
    Stream_Controller_start();
}
void Controller_stop(void)
{
    Stream_Controller_stop();
    Stream_cleanup();
    
    Mfa_stop();
    Menu_stop();

    DangerAnalyzer_stop();
    LedDisplay_stop();

    PasswordInput_cleanup();
    MotionSensor_cleanup();
    Joystick_cleanup();

    PasswordManager_cleanup();
    Settings_cleanup();
    Http_cleanup();
}
