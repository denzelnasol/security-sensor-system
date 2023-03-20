#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
// #include <pthread.h>

#include "menu.h"

#include "../Joystick/joystick.h"
// #include "MockObjects/joystick.h"
#include "../LEDDisplay/ledDisplay.h"
#include "../PasswordInput/passwordInput.h"
#include "../Timer/timer.h"
#include "../Utilities/utilities.h"

#define SLEEP_FREQUENCY_MS                      10

#define MATCH_ALL_SELECTION_INDICATOR_VALUE     -2
#define MATCH_NONE_SELECTION_INDICATOR_VALUE    -1

#define ADMIN_NUM_OPTIONS                       11
#define GUEST_NUM_OPTIONS                       3

#define AUTO_LOGOUT_AFTER_30_SEC_MS             30000
#define AUTO_LOGOUT_AFTER_1_MIN_MS              60000
#define AUTO_LOGOUT_AFTER_2_MIN_MS              1200000


/**
 * 
Menu layout:

Guest
├── 0: Login
├── 1: Check Danger Level
└── 2: Check Number Of Triggers

Admin
├── 0: Logout
├── 1: Check Danger Level
├── 2: Check Number Of Triggers
├── 3: Toggle Motion Sensor
│   ├── 0: Off
│   └── 1: On
├── 4: Toggle Camera
│   ├── 0: Off
│   └── 1: On
├── 5: Toggle Logging
│   ├── 0: Off
│   └── 1: On
├── 6: Set Danger Level Threshold
├── 7: Reset Danger Level Threshold To Factory Default
│   ├── 0: Cancel
│   └── 1: Confirm
├── 8: Reset Danger Level To 0
│   ├── 0: Cancel
│   └── 1: Confirm
├── 9: Configure Auto-Logout Timeout
│   ├── 0: Logout after 30 seconds of inactivity
│   ├── 1: Logout after 1 min of inactivity
│   └── 2: Logout after 2 min of inactivity
└── 10: Configure Security Policy
    ├── 0: Enable mfa authentication on leds (restrict level: medium)
    ├── 1: Disable mfa (restrict level: low)
    └── 2: Disable remote access (restrict level: high)

*/

// ------------------------- PRIVATE ------------------------- //

// all menu options at the first level
typedef enum {
    OPT_LOGOUT_OR_LOGIN,
    OPT_CHECK_DANGER,
    OPT_CHECK_TRIGGERS,
    OPT_TOGGLE_MOTION_SENSOR,
    OPT_TOGGLE_CAMERA,
    OPT_TOGGLE_LOGGER,
    OPT_SET_DANGER_THRESHOLD,
    OPT_RESET_DANGER_THRESHOLD,
    OPT_RESET_DANGER_LEVEL,
    OPT_CONFIGURE_AUTO_LOGOUT_TIMOUT,
    OPT_CONFIGURE_SECURITY_POLICY,
} Option;

// auto logout policies
typedef enum {
    SOPT_AL_AFTER_30_SEC,
    SOPT_AL_AFTER_1_MIN,
    SOPT_AL_AFTER_2_MIN,
    SOPT_AL_SENTINEL_MAX,
} SOptALPolicy;

// remote access policies
typedef enum {
    SOPT_RA_DISABLE_MFA,
    SOPT_RA_ENABLE_MFA,
    SOPT_RA_DISABLE_REMOTE_ACCESS,
    SOPT_RA_SENTINEL_MAX,
} SOptRAPolicy;

typedef enum {
    ACTION_TOGGLE_ON_OR_CONFIRM,
    ACTION_TOGGLE_OFF_OR_CANCEL,
    ACTION_TOGGLE_SENTINEL_MAX,
} Action;

typedef enum {
    SIGNAL_EXIT,
    SIGNAL_CONTINUE,
} Signal;

typedef enum {
    STATE_MENU,
    STATE_SUBMENU,
} StateType;

// stores info about a menu state
typedef struct {
    // these fields must be >= 0
    int currentOpt;
    int numOpts;

    // this is used to tell the display to show an indicator marking 
    int selectedOpt;
} StateInfo;

// stores info about the current state
typedef struct {
    StateInfo *state;
    StateType type;
} State;

static long inactivityTimeoutMs = AUTO_LOGOUT_AFTER_30_SEC_MS;
static Timer inactivityTimer;

static StateInfo menu;
static StateInfo subMenu;

static State currentState;

static bool isLoggedIn = false;


static void next(StateInfo *state);
static void prev(StateInfo *state);
static void back(State *state);
static void setMenuState(State *state);
static void setSubMenuState(State *state);
static void setAdminView(StateInfo *menu);
static void setGuestView(StateInfo *menu);
static void logout(void);
static Signal login(void);
static Signal logoutOrLogin(void);
static Signal processUserInput(void);
static Signal handleUserInput(JoystickInput input);
static Signal handleCenter(void);
static void handleLeft(void);
static void startGui(void);
static void toggleMotionSensor(void);
static void toggleCamera(void);
static void toggleLogger(void);
static void setDangerThreshold(void);
static void resetDangerThreshold(void);
static void resetDangerLevel(void);
static void configureAutoLogout(void);
static void configureSecurityPolicy(void);
static void doFromSubMenu(void);
static Signal doFromMenu(void);
static void setViewCheckDangerLevel(void);
static void setViewCheckNumTriggers(void);
static void setViewToggleMotionSensor(void);
static void setViewToggleCamera(void);
static void setViewToggleLogger(void);
static void setViewSetDangerThreshold(void);
static void setViewResetDangerThreshold(void);
static void setViewResetDangerLevel(void);
static void setViewConfigureAutoLogout(void);
static void setViewConfigureSecurityPolicy(void); 


static void next(StateInfo *state)
{
    if (state->numOpts == 0) {
        return;
    }
    state->currentOpt = (state->currentOpt + 1) % state->numOpts;
}
static void prev(StateInfo *state)
{
    if (state->numOpts == 0) {
        return;
    }
    state->currentOpt = (state->currentOpt + state->numOpts - 1) % state->numOpts;
}

// precondition: the current state is submenu
static void back(State *state)
{
    assert(state->type == STATE_SUBMENU);
    setMenuState(state);
}

static void setMenuState(State *state)
{
    state->state = &menu;
    state->type = STATE_MENU;
}
static void setSubMenuState(State *state) 
{
    state->state = &subMenu;
    state->type = STATE_SUBMENU;
}

static void setAdminView(StateInfo *view)
{
    view->currentOpt = 0;
    view->numOpts = ADMIN_NUM_OPTIONS;
    view->selectedOpt = MATCH_ALL_SELECTION_INDICATOR_VALUE;
}
static void setGuestView(StateInfo *view)
{
    view->currentOpt = 0;
    view->numOpts = GUEST_NUM_OPTIONS;
    view->selectedOpt = MATCH_NONE_SELECTION_INDICATOR_VALUE;
}

static bool isExitSignalReceived()
{
    // todo: set the exit signal
    return false;
}

static void logout()
{
    setGuestView(&menu);
    setMenuState(&currentState);
    isLoggedIn = false;
}
static Signal login()
{
    printf("loggin in\n");
    LedDisplay_showSpecial();
    
    Timer_start(inactivityTimeoutMs, &inactivityTimer);

    PInputState passwordStatus = P_INPUT_CONTINUE;
    while (passwordStatus == P_INPUT_CONTINUE) {
        // go back to menu if timer times out
        if (Timer_isExpired(&inactivityTimer)) {
            back(&currentState);
            return SIGNAL_CONTINUE;
        }
        if (isExitSignalReceived()) {
            return SIGNAL_EXIT;
        }
        JoystickInput input = Joystick_getPressed();
        if (input != JOYSTICK_NONE) {
            // reset the timer
            Timer_start(inactivityTimeoutMs, &inactivityTimer);
        }
        passwordStatus = PasswordInput_sendNext(input);
        Utilities_sleepForMs(SLEEP_FREQUENCY_MS);
    }

    // handle error states
    if (passwordStatus == P_INPUT_NO_MATCH) {
        printf("incorrect password\n");
        back(&currentState);
        return SIGNAL_CONTINUE;
    }
    if (passwordStatus == P_INPUT_TOO_LONG) {
        printf("password too long\n");
        back(&currentState);
        return SIGNAL_CONTINUE;
    }

    assert(passwordStatus == P_INPUT_MATCH);

    setAdminView(&menu);
    setMenuState(&currentState);
    isLoggedIn = true;
    return SIGNAL_CONTINUE;
}

static Signal logoutOrLogin()
{
    if (isLoggedIn) {
        logout();
        return SIGNAL_CONTINUE;
    }
    return login();
}


static void debugDisplay(int opt, int setIndicator)
{
    bool isMenuState = currentState.type == STATE_MENU;
    printf("overall state:\n"
            "\tstate: %s\n", isMenuState ? "MENU": "Submenu");
    printf("\nstate:\n"
            "\tcurrently displaying: %d\n"
            "\tindicator set: %s\n"
            "\tnumber of options: %d\n"
            "\tselected option: %d\n", 
            opt, setIndicator == 1 ? "yes": "no", 
            currentState.state->numOpts, 
            currentState.state->selectedOpt);
}


static void startGui() 
{
    // debugDisplay(currentState.state->currentOpt, 0);

    Signal signal = SIGNAL_CONTINUE;
    while (signal != SIGNAL_EXIT) {
        //---- update MENU model -----//
        signal = processUserInput();

        //---- update SEGMENT DISPLAY view -----//

        // display the right-side decimal point to indicate that this option is currently active
        if (currentState.state->selectedOpt == MATCH_ALL_SELECTION_INDICATOR_VALUE || 
            (currentState.type == STATE_SUBMENU && subMenu.currentOpt == subMenu.selectedOpt)) 
        {
            LedDisplay_setDisplayNumber(currentState.state->currentOpt, S16_SET_INDICATOR);
            // debugDisplay(currentState.state->currentOpt, 1);
        } else {
            LedDisplay_setDisplayNumber(currentState.state->currentOpt, S16_SET_NONE);
            // debugDisplay(currentState.state->currentOpt, 0);
        }
    }
}

// waits for a user to input a valid input through the joystick and then calls the correct handler
// this will also listen for the exit signal and return a SIGNAL_EXIT signal so the gui can terminate
// returns when exit signal is received or is auto logout has timed out or user enters valid input
static Signal processUserInput()
{
    // keep polling the joystick 
    // if timeout and logged in then execute logout 
    Timer_start(inactivityTimeoutMs, &inactivityTimer);

    JoystickInput input = JOYSTICK_NONE;
    while (input == JOYSTICK_NONE) {
        if (isLoggedIn && Timer_isExpired(&inactivityTimer)) {
            logout();
            return SIGNAL_CONTINUE;
        }
        if (isExitSignalReceived()) {
            return SIGNAL_EXIT;
        }
        input = Joystick_getPressed();
        Utilities_sleepForMs(SLEEP_FREQUENCY_MS);
    }

    return handleUserInput(input);
}

// precondition: input is valid (cannot be JOYSTICK_NONE)
// determine the next thing to do based on if the user is logged in or not
static Signal handleUserInput(JoystickInput input)
{
    Signal signal = SIGNAL_CONTINUE;
    switch (input) {
        case JOYSTICK_UP:
            next(currentState.state);
            break;
        case JOYSTICK_DOWN:
            prev(currentState.state);
            break;
        case JOYSTICK_LEFT:
            handleLeft();
            break;
        case JOYSTICK_CENTER:
            signal = handleCenter();
            break;
        default:
            // ignore other inputs
            break;
    }
    return signal;
}

// left means to go back to the menu
static void handleLeft()
{
    if (currentState.type == STATE_SUBMENU) {
        back(&currentState);
    }
}

// center means to 'click' on something
static Signal handleCenter()
{
    if (currentState.type == STATE_SUBMENU) {
        doFromSubMenu();
        return SIGNAL_CONTINUE;
    }
    return doFromMenu();
}

// turns the motion sensor on or off
static void toggleMotionSensor()
{
    // MotionSensorToggle val = MOTION_SENSOR_OFF;
    if (subMenu.currentOpt == (int)ACTION_TOGGLE_ON_OR_CONFIRM) {
        // val = MOTION_SENSOR_ON;
    }
    // motionSensorController.toggle(val);
    subMenu.selectedOpt = subMenu.currentOpt;
}
// turns the camera on or off
static void toggleCamera()
{
    // CamToggle val = CAM_OFF;
    if (subMenu.currentOpt == (int)ACTION_TOGGLE_ON_OR_CONFIRM) {
        // val = CAM_ON;
    }
    // camController.toggle(val);
    subMenu.selectedOpt = subMenu.currentOpt;
}
// turns the logger on or off
static void toggleLogger()
{
    // LoggerToggle val = LOGGER_OFF;
    if (subMenu.currentOpt == (int)ACTION_TOGGLE_ON_OR_CONFIRM) {
        // val = LOGGER_ON;
    }
    // logger.toggle(val);
    subMenu.selectedOpt = subMenu.currentOpt;
}

static void setDangerThreshold()
{
    int newDangerThresh = subMenu.currentOpt;
    // analyzer.setDangerThreshold(newDangerThresh);
    subMenu.selectedOpt = newDangerThresh;
}
// for submenu options with confirm/cancel, when user selects an option, perform the action and then
// go back to the menu
static void resetDangerThreshold()
{
    if (subMenu.currentOpt == (int)ACTION_TOGGLE_ON_OR_CONFIRM) {
        // analyzer.resetDangerThreshold();
    }
    back(&currentState);
}
static void resetDangerLevel()
{
    if (subMenu.currentOpt == (int)ACTION_TOGGLE_ON_OR_CONFIRM) {
        // analyzer.resetDangerLevel();
    }
    back(&currentState);
}

static void configureAutoLogout()
{
    // SettingsAutoLogoutOption opt = SETTINGS_AUTO_LOGOUT_AFTER_30_SEC
    // switch (subMenu.currentOpt) {
    //     case AUTO_LOGOUT_AFTER_30_SEC:
    //         inactivityTimeoutMs = AUTO_LOGOUT_AFTER_30_SEC_MS;
    //         break;
    //     case AUTO_LOGOUT_AFTER_1_MIN:
    //         opt = SETTINGS_AUTO_LOGOUT_AFTER_1_MIN;
    //         inactivityTimeoutMs = AUTO_LOGOUT_AFTER_1_MIN_MS;
    //         break;
    //     case AUTO_LOGOUT_AFTER_2_MIN:
    //         opt = SETTINGS_AUTO_LOGOUT_AFTER_2_MIN;
    //         inactivityTimeoutMs = AUTO_LOGOUT_AFTER_2_MIN_MS;
    //         break;
    //     default:
    //         assert(false);
    // }
    //// write preferences to file
    // settingsModule.save(SETTINGS_AUTO_LOGOUT, opt);
    subMenu.selectedOpt = subMenu.currentOpt;
}

static void configureSecurityPolicy()
{
    // SettingsSecurityPolicy opt = SETTINGS_SECURITY_POLICY_DISABLE_REMOTE_AUTH;
    // switch (subMenu.currentOpt) {
    //     case SECURITY_POLICY_DISABLE_REMOTE_AUTH:
    //         break;
    //     case SECURITY_POLICY_ENABLE_REMOTE_AUTH:
    //         opt = SETTINGS_SECURITY_POLICY_ENABLE_REMOTE_AUTH;
    //         break;
    //     case SECURITY_POLICY_DISABLE_REMOTE_ACCESS:
    //         opt = SETTINGS_SECURITY_POLICY_DISABLE_REMOTE_ACCESS;
    //         break;
    //     default:
    //         assert(false);
    // }
    //// write preferences to file
    // settingsModule.save(SETTINGS_SECURITY_POLICY, opt);
    subMenu.selectedOpt = subMenu.currentOpt;
}


// precondition: currentState.type == submenu
static void doFromSubMenu()
{
    assert(currentState.type == STATE_SUBMENU);
    switch ((Option)menu.currentOpt) {
        case OPT_TOGGLE_MOTION_SENSOR:
            toggleMotionSensor();
            break;
        case OPT_TOGGLE_CAMERA:
            toggleCamera();
            break;
        case OPT_TOGGLE_LOGGER:
            toggleLogger();
            break;
        case OPT_SET_DANGER_THRESHOLD:
            setDangerThreshold();
            break;
        case OPT_CONFIGURE_AUTO_LOGOUT_TIMOUT:
            configureAutoLogout();
            break;
        case OPT_CONFIGURE_SECURITY_POLICY:
            configureSecurityPolicy();
            break;
        case OPT_RESET_DANGER_THRESHOLD:
            resetDangerThreshold();
            break;
        case OPT_RESET_DANGER_LEVEL:
            resetDangerLevel();
            break;

        // there are no submenu actions for these options
        case OPT_LOGOUT_OR_LOGIN:
        case OPT_CHECK_DANGER:
        case OPT_CHECK_TRIGGERS:
            break;

        default:
            assert(false);
    }
}

// if there is a submenu then this function will just setViewigate to the submenu
// otherwise it will perform the action
// precondition: currentState.type == menu
// postcondition: currentState.type == submenu unless it is logout
static Signal doFromMenu()
{
    assert(currentState.type == STATE_MENU);

    // go into a submenu from the menu. so we have to switch states
    setSubMenuState(&currentState);

    switch ((Option)menu.currentOpt) {
        case OPT_LOGOUT_OR_LOGIN:
            return logoutOrLogin();
        case OPT_CHECK_DANGER:
            setViewCheckDangerLevel();
            break;
        case OPT_CHECK_TRIGGERS:
            setViewCheckNumTriggers();
            break;
        case OPT_TOGGLE_MOTION_SENSOR:
            setViewToggleMotionSensor();
            break;
        case OPT_TOGGLE_CAMERA:
            setViewToggleCamera();
            break;
        case OPT_TOGGLE_LOGGER:
            setViewToggleLogger();
            break;
        case OPT_SET_DANGER_THRESHOLD:
            setViewSetDangerThreshold();
            break;
        case OPT_RESET_DANGER_THRESHOLD:
            setViewResetDangerThreshold();
            break;
        case OPT_RESET_DANGER_LEVEL:
            setViewResetDangerLevel();
            break;
        case OPT_CONFIGURE_AUTO_LOGOUT_TIMOUT:
            setViewConfigureAutoLogout();
            break;
        case OPT_CONFIGURE_SECURITY_POLICY:
            setViewConfigureSecurityPolicy();
            break;
        default:
            assert(false);
    }
    return SIGNAL_CONTINUE;
}

static void setViewCheckDangerLevel()
{
    subMenu.selectedOpt = MATCH_NONE_SELECTION_INDICATOR_VALUE;

    // setting the numOpts field to 0 prevents user from increment/dec the value
    subMenu.numOpts = 0;
    // subMenu.currentOpt = analyzer.getDangerLevel();
}
static void setViewCheckNumTriggers()
{
    subMenu.selectedOpt = MATCH_NONE_SELECTION_INDICATOR_VALUE;
    subMenu.numOpts = 0;
    // subMenu.currentOpt = motionSensorController.getTriggerCount();
}

static void setViewToggleMotionSensor()
{
    subMenu.currentOpt = 0;
    subMenu.numOpts = (int)ACTION_TOGGLE_SENTINEL_MAX;
    // subMenu.selectedOpt = (int)motionSensorController.getToggleState();
}
static void setViewToggleCamera()
{
    subMenu.currentOpt = 0;
    subMenu.numOpts = (int)ACTION_TOGGLE_SENTINEL_MAX;
    // subMenu.selectedOpt = (int)camera.getToggleState();
}
static void setViewToggleLogger()
{
    subMenu.currentOpt = 0;
    subMenu.numOpts = (int)ACTION_TOGGLE_SENTINEL_MAX;
    // subMenu.selectedOpt = (int)logger.getToggleState();
}
static void setViewSetDangerThreshold()
{
    subMenu.currentOpt = 0;
    subMenu.numOpts = 100;
    // subMenu.selectedOpt = (int)settings.getDangerThreshold();
}
static void setViewResetDangerThreshold()
{
    subMenu.selectedOpt = MATCH_NONE_SELECTION_INDICATOR_VALUE;
    subMenu.currentOpt = 0;
    subMenu.numOpts = (int)ACTION_TOGGLE_SENTINEL_MAX;
}
static void setViewResetDangerLevel()
{
    subMenu.selectedOpt = MATCH_NONE_SELECTION_INDICATOR_VALUE;
    subMenu.currentOpt = 0;
    subMenu.numOpts = (int)ACTION_TOGGLE_SENTINEL_MAX;
}
static void setViewConfigureAutoLogout()
{
    subMenu.currentOpt = 0;
    subMenu.numOpts = (int)SOPT_AL_SENTINEL_MAX;
    // subMenu.selectedOpt = (int)settings.getAutoLogoutConfigurationOption();
}
static void setViewConfigureSecurityPolicy() 
{
    subMenu.currentOpt = 0;
    subMenu.numOpts = (int)SOPT_RA_SENTINEL_MAX;
    // subMenu.selectedOpt = (int)settings.getAutoLogoutConfigurationOption();
}


// ------------------------- PUBLIC ------------------------- //
void Menu_start(void)
{
    PasswordInput_init();


    isLoggedIn = false;
    setGuestView(&menu);
    setMenuState(&currentState);
    // inactivityTimeoutMs = settings.getAutoLogoutTimeoutInMs();

    // do pthread stuff

    printf("starting gui...\n");

    startGui();
}
void Menu_stop(void)
{
    // do pthread stuff
}


//--------------- FOR TESTING-----------//
int main(int argc, char **argv)
{
    Menu_start();
    return 0;
}
