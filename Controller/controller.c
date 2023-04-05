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
#include "../Ethernet/Client/ethernetClient.h"
#include "../Buzzer/buzzer.h"

void Controller_start(void)
{
    // init modules
    Http_init();
    Settings_init();
    PasswordManager_init();

    Joystick_init();
    MotionSensor_init();
    PasswordInput_init();
    Buzzer_init();
    
    // start threads
    LedDisplay_start();
    DangerAnalyzer_start();

    Menu_start();
    Mfa_start();

    Stream_Controller_start();
    Camera_init();
}
void Controller_stop(void)
{
    Camera_cleanup();
    Stream_Controller_stop();
    
    Mfa_stop();
    Menu_stop();

    DangerAnalyzer_stop();
    LedDisplay_stop();

    Buzzer_cleanup();
    PasswordInput_cleanup();
    MotionSensor_cleanup();
    Joystick_cleanup();

    PasswordManager_cleanup();
    Settings_cleanup();
    Http_cleanup();
}
