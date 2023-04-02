/**
 * The remote login client will run whenever someone logs into the bbg.
 * After they are authenticated by the host OS, this program will run and allow
 * the user to login to the security system. Depending on the system settings,
 * the user may be asked for a remote login password AND a multi-factor
 * authentication code displayed on the LEDs. If the login process fails then there will
 * be a timeout before the user is able to enter login credentials again.
*/

#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <signal.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <termios.h>

#include "clientNet.h"

#include "../Networking/networking.h"
#include "../Utilities/utilities.h"
#include "../EventLogger/logger.h"
#include "../Share/host.h"

#define PASSWORD_LENGTH                         16
#define MFA_BUFFER_SIZE                         5

#define WHITESPACE                              " \n\r\t"

// guest commands
#define COMMAND_GET_DANGER_LEVEL                "gdanger"
#define COMMAND_GET_NUM_TRIGGERS                "gtrigger"
#define COMMAND_LOGIN                           "login"
#define COMMAND_END_SESSION                     "exit"
#define COMMAND_HELP                            "help"
#define COMMAND_CLEAR_SCREEN                    "clear"

// admin commands
#define COMMAND_TOGGLE                          "toggle"
#define COMMAND_DUMP_LOGS                       "dlog"
#define COMMAND_SET_DANGER_THRESHOLD            "sdanger"
#define COMMAND_RESET_DANGER_THRESHOLD          "rdthresh"
#define COMMAND_RESET_DANGER_LEVEL              "rdlevel"
#define COMMAND_CONFIGURE                       "conf"
#define COMMAND_STOP                            "stop"
#define COMMAND_LOGOUT                          "logout"
#define COMMAND_HOST_ACCESS                     "haccess"
#define COMMAND_CLEAR_LOGS                      "clearlogs"
#define COMMAND_CHANGE_PASSWORD                 "chpassword"
#define COMMAND_CHANGE_JOYSTICK_PATTERN         "chjpattern"

#define EXIT_CODE_HACCESS                       EXIT_SUCCESS
#define EXIT_CODE_STANDARD                      1
#define ASCII_CYAN                              "\033[96;1m"
#define ASCII_RESET                             "\033[0m"


typedef enum {
    SIGNAL_NONE,
    SIGNAL_STOP,
} Signal;


static bool isLoggedIn = false;
static int exitCode = EXIT_CODE_STANDARD;

static void getPassword(char *buffer, size_t size);
static void getInput(char *buffer, size_t size);
static void sigintHandler(int sig_num);
static void askServer(char *command);
static void login(void);
static void logout(void); 
static void help(void);
static void clearScreen(void);
static void dumpLogs(void);
static void clearLogs(void);
static void accessHost(void);
static void stop(void);
static void resetDangerThreshold(void);
static void resetDangerLevel(void);
static void changePassword(void);
static void changeJoystickPattern(void);

static Signal execute(char *command);
static void prompt(void);
static void displayWarning(void);
static void recvFromServ(char *message);
static void sendPing(void);

static bool patternIsValid(char *buffer, size_t size);
static bool mfaPatternIsValid(char *buffer, size_t size);

// receive message from the server. exit on timeout after 1s
static void recvFromServ(char *message)
{
    if (!ClientNet_receive(message)) {
        ClientNet_cleanup();
        printf("Connection reset on bbg. Request timed out\n");
        exit(isLoggedIn ? EXIT_CODE_HACCESS : EXIT_CODE_STANDARD);
    }
}

// gets the input from stdin and removes the newline character
static void getInput(char *buffer, size_t size)
{
    char ignore[256];
    fgets(buffer, size, stdin);
    int idx = strcspn(buffer, "\n");
    char ch = buffer[idx];
    while (ch != '\n') {
        fgets(ignore, sizeof(ignore), stdin);
        ch = ignore[strcspn(ignore, "\n")];
    }
    buffer[idx] = 0;
}

static void getPassword(char *buffer, size_t size)
{
    // prmpt user for password
    // https://stackoverflow.com/questions/59922972/how-to-stop-echo-in-terminal-using-c
    struct termios term;
    tcgetattr(fileno(stdin), &term);

    term.c_lflag &= ~ECHO;
    tcsetattr(fileno(stdin), 0, &term);

    getInput(buffer, size);

    term.c_lflag |= ECHO;
    tcsetattr(fileno(stdin), 0, &term);
    
    printf("\n");
}

// returns true if the joystick pattern is valid
static bool patternIsValid(char *buffer, size_t size)
{
    for (int i = 0; i < size; i++) {
        char ch = buffer[i];
        if (ch < '1' || ch > '4') {
            return false;
        }
    }
    return true;
}
static bool mfaPatternIsValid(char *buffer, size_t size)
{
    for (int i = 0; i < size; i++) {
        char ch = buffer[i];
        if (ch != '1' && ch != '0') {
            return false;
        }
    }
    return true;
}

// source: https://www.geeksforgeeks.org/write-a-c-program-that-doesnt-terminate-when-ctrlc-is-pressed/
static void sigintHandler(int sig_num)
{
    /* Reset handler to catch SIGINT next time.
    Refer http://en.cppreference.com/w/c/program/signal */
    signal(SIGINT, sigintHandler);
}

// asks the server for the answer and then prints the response
static void askServer(char *command)
{
    ClientNet_send(command, strlen(command));
    
    char response[RESPONSE_PACKET_SIZE];
    recvFromServ(response);

    printf("%s\n", response);
}

static void login()
{
    if (isLoggedIn) {
        printf("Already logged in.\n");
        return;
    }
    Logger_logInfo("attempted login by guest");

    printf("Enter password for user admin: ");

    char password[PASSWORD_LENGTH];
    getPassword(password, sizeof(password));

    // send to the server
    char response[RESPONSE_PACKET_SIZE];
    char command[RELAY_PACKET_SIZE];
    snprintf(command, sizeof(command), CLIENT_REQ_LOGIN " %s", password);
    ClientNet_send(command, strlen(command));
    recvFromServ(response);

    if (strncmp(response, STATUS_CODE_OK, sizeof(STATUS_CODE_OK)) == 0) {
        isLoggedIn = true;
        Logger_logInfo("guest logged in as admin");
        printf("Logged in as admin.\n");
        return;
    } else if (strncmp(response, STATUS_CODE_BAD, sizeof(STATUS_CODE_BAD)) == 0) {
        Logger_logInfo("gus loggedin failed");
        printf("Incorrect password.\n");
        return;
    }
    
    // extra MFA steps
    printf("%s", response);

    // prmpt user for mfa code
    char mfaPassword[MFA_BUFFER_SIZE];
    getInput(mfaPassword, sizeof(mfaPassword));
    if (!mfaPatternIsValid(mfaPassword, sizeof(mfaPassword))) {
        Logger_logInfo("gus loggedin failed");
        printf("Incorrect format of MFA code. Only 0s and 1s allowed.\n");
        return;
    }

    // send to the server
    snprintf(command, sizeof(command), CLIENT_REQ_MFA " %s", mfaPassword);
    ClientNet_send(command, strlen(command));
    recvFromServ(response);

    if (strncmp(response, STATUS_CODE_OK, sizeof(STATUS_CODE_OK)) == 0) {
        isLoggedIn = true;
        Logger_logInfo("gus logged in as admin");
        printf("Logged in as admin.\n");
    } else if (strncmp(response, STATUS_CODE_BAD, sizeof(STATUS_CODE_BAD)) == 0) {
        Logger_logInfo("gus loggedin failed");
        printf("Incorrect MFA code.\n");
    } else {
        printf("%s", response);
        assert(false);
    }
}
static void logout() 
{
    if (!isLoggedIn) {
        return;
    }
    isLoggedIn = false;
    printf("Logout ok.\n");
}
static void help()
{
    if (isLoggedIn) {
        printf(
            "Available commands:\n"
            COMMAND_GET_DANGER_LEVEL ": gets the danger level\n"
            COMMAND_GET_NUM_TRIGGERS ": gets the number of triggers\n"
            COMMAND_END_SESSION ": ends the ssh session\n"
            COMMAND_CLEAR_SCREEN ": clears the screen\n"
            COMMAND_TOGGLE ": toggles the device\n"
            COMMAND_DUMP_LOGS ": prints the activity log\n"
            COMMAND_SET_DANGER_THRESHOLD ": set the danger threshold\n"
            COMMAND_RESET_DANGER_THRESHOLD ": reset the danger threshold\n"
            COMMAND_RESET_DANGER_LEVEL ": reset the danger level\n"
            COMMAND_CONFIGURE ": configures system settings\n"
            COMMAND_STOP ": stops the system\n"
            COMMAND_LOGOUT ": logout of admin\n"
            COMMAND_HOST_ACCESS ": access host OS for full system control\n"
            COMMAND_CLEAR_LOGS ": clears all the log files\n"
        );
        return;
    }
    printf(
        "Available commands:\n"
        COMMAND_GET_DANGER_LEVEL ": gets the danger level\n"
        COMMAND_GET_NUM_TRIGGERS ": gets the number of triggers\n"
        COMMAND_LOGIN ": login as admin\n"
        COMMAND_END_SESSION ": ends the ssh session\n"
        COMMAND_CLEAR_SCREEN ": clears the screen\n"
    );
}
static void clearScreen()
{
    system("clear");
}

// precondition: these functions require isLoggedIn = true
static void dumpLogs()
{
    Logger_dump();
}
static void clearLogs()
{
    Logger_clearLogs();
}
static void accessHost()
{
    Logger_logInfo("admin request host access");
    exitCode = EXIT_CODE_HACCESS;
}
static void stop()
{
    ClientNet_send(COMMAND_STOP, sizeof(COMMAND_STOP));
    
    char response[RESPONSE_PACKET_SIZE];
    recvFromServ(response);

    Logger_logWarning("admin has stopped beaglecam");
    printf("%s\n", response);

    exitCode = EXIT_CODE_HACCESS;
}
static void resetDangerThreshold()
{
    ClientNet_send(COMMAND_RESET_DANGER_THRESHOLD, sizeof(COMMAND_RESET_DANGER_THRESHOLD));
    
    char response[RESPONSE_PACKET_SIZE];
    recvFromServ(response);

    Logger_logInfo("admin reset danger threhsold");
    printf("%s\n", response);
}
static void resetDangerLevel()
{
    ClientNet_send(COMMAND_RESET_DANGER_LEVEL, sizeof(COMMAND_RESET_DANGER_LEVEL));
    
    char response[RESPONSE_PACKET_SIZE];
    recvFromServ(response);

    Logger_logInfo("admin reset danger level");
    printf("%s\n", response);
}
static void changePassword()
{
    Logger_logWarning("user attempted command change password");

    // verify identity
    printf("Enter current password: ");

    char password[PASSWORD_LENGTH];
    getPassword(password, sizeof(password));

    char command[RELAY_PACKET_SIZE];
    snprintf(command, sizeof(command), CLIENT_REQ_AUTH " %s", password);
    ClientNet_send(command, strlen(command));

    char response[RESPONSE_PACKET_SIZE];
    recvFromServ(response);

    if (strncmp(response, STATUS_CODE_BAD, sizeof(STATUS_CODE_BAD)) == 0) {
        printf("Incorrect password.\n");
        return;
    }

    // set new password
    char confirmedPassword[PASSWORD_LENGTH];
    printf("Enter new password: ");
    getPassword(password, sizeof(password));

    printf("Confirm new password: ");
    getPassword(confirmedPassword, sizeof(confirmedPassword));

    if (strncmp(password, confirmedPassword, PASSWORD_LENGTH) != 0) {
        printf("Password does not match.\n");
        return;
    }

    // finalize
    snprintf(command, sizeof(command), CLIENT_REQ_SETPASS " %s", password);
    ClientNet_send(command, strlen(command));
    recvFromServ(response);

    if (strncmp(response, STATUS_CODE_BAD, sizeof(STATUS_CODE_BAD)) == 0) {
        Logger_logInfo("admin password change rejected");
        printf("Password change rejected.\n");
    } else if (strncmp(response, STATUS_CODE_OK, sizeof(STATUS_CODE_OK)) == 0) {
        Logger_logInfo("admin changed login password");
        printf("Password change ok.\n");
    }
}
static void changeJoystickPattern()
{
    // verify identity
    printf("Enter password: ");

    char password[PASSWORD_LENGTH];
    getPassword(password, sizeof(password));

    char command[RELAY_PACKET_SIZE];
    snprintf(command, sizeof(command), CLIENT_REQ_AUTH " %s", password);
    ClientNet_send(command, strlen(command));

    char response[RESPONSE_PACKET_SIZE];
    recvFromServ(response);

    if (strncmp(response, STATUS_CODE_BAD, sizeof(STATUS_CODE_BAD)) == 0) {
        printf("Incorrect password.\n");
        return;
    }

    // mfa is mandatory for changing menu system password!
    printf("Enter MFA code displayed on LEDs: ");
    char mfaPassword[MFA_BUFFER_SIZE];
    getInput(mfaPassword, sizeof(mfaPassword));
    if (!mfaPatternIsValid(mfaPassword, sizeof(mfaPassword))) {
        printf("Incorrect format of MFA code. Only 0s and 1s allowed.\n");
        return;
    }

    snprintf(command, sizeof(command), CLIENT_REQ_MFA " %s", mfaPassword);
    ClientNet_send(command, strlen(command));
    recvFromServ(response);

    if (strncmp(response, STATUS_CODE_BAD, sizeof(STATUS_CODE_BAD)) == 0) {
        printf("Incorrect MFA code.\n");
        return;
    }

    // finally change the pattern
    printf(
        "\nWarning: This will affect the joystick pattern required" 
        "to login as admin on the beagle cam's menu!\n\n"
        "Instructions:\n"
        "**Pattern must only contain numbers 1 to 4 (no spaces)**\n"
        "UP    = 1\n"
        "RIGHT = 2\n"
        "DOWN  = 3\n"
        "LEFT  = 4\n"
    );

    // set new pattern
    char confirmedPassword[PASSWORD_LENGTH];
    printf("Enter new pattern: ");
    getPassword(password, sizeof(password));
    if (!patternIsValid(password, sizeof(password))) {
        printf("Incorrect format of pattern.\n");
        return;
    }

    printf("Confirm new pattern: ");
    getPassword(confirmedPassword, sizeof(confirmedPassword));
    if (strncmp(password, confirmedPassword, PASSWORD_LENGTH) != 0) {
        printf("Pattern does not match.\n");
        return;
    }

    // finalize
    snprintf(command, sizeof(command), CLIENT_REQ_JSETPASS " %s", password);
    ClientNet_send(command, strlen(command));
    recvFromServ(response);

    if (strncmp(response, STATUS_CODE_BAD, sizeof(STATUS_CODE_BAD)) == 0) {
        Logger_logInfo("menu system password change rejected");
        printf("Pattern change rejected.\n");
    } else if (strncmp(response, STATUS_CODE_OK, sizeof(STATUS_CODE_OK)) == 0) {
        Logger_logInfo("admin changed menu system password");
        printf("Pattern change ok.\n");
    }
}

// precondition: command cannot have a newline
static Signal execute(char *command)
{
    // copy the command into another buffer
    char buffer[RELAY_PACKET_SIZE];
    snprintf(buffer, sizeof(buffer), "%s", command);

    // getting the first part of the command
    char *cmd = strtok(command, WHITESPACE);

    // check if the commands are recognized by the client
    if (cmd == NULL) {
        return SIGNAL_NONE;
    }
    if (strncmp(cmd, COMMAND_END_SESSION, sizeof(COMMAND_END_SESSION)) == 0) {
        Logger_logInfo("user has ended the session");
        exitCode = EXIT_CODE_STANDARD;
        return SIGNAL_STOP;

    } else if (strncmp(cmd, COMMAND_HELP, sizeof(COMMAND_HELP)) == 0) {
        help();
        return SIGNAL_NONE;

    } else if (strncmp(cmd, COMMAND_CLEAR_SCREEN, sizeof(COMMAND_CLEAR_SCREEN)) == 0) {
        clearScreen();
        return SIGNAL_NONE;

    } else if (strncmp(cmd, COMMAND_LOGIN, sizeof(COMMAND_LOGIN)) == 0) {
        login();
        return SIGNAL_NONE;

    } else if (strncmp(cmd, COMMAND_LOGOUT, sizeof(COMMAND_LOGOUT)) == 0) {
        logout();
        return SIGNAL_NONE;

    } else if (strncmp(cmd, COMMAND_GET_DANGER_LEVEL, sizeof(COMMAND_GET_DANGER_LEVEL)) == 0) {
        askServer(buffer);
        return SIGNAL_NONE;

    } else if (strncmp(cmd, COMMAND_GET_NUM_TRIGGERS, sizeof(COMMAND_GET_NUM_TRIGGERS)) == 0) {
        askServer(buffer);
        return SIGNAL_NONE;
    } 
    
    if (!isLoggedIn) {
        printf("Unrecognized command '%s'\n", cmd);
        return SIGNAL_NONE;
    }

    // the commands below require user to be logged in as admin
    if (strncmp(cmd, COMMAND_STOP, sizeof(COMMAND_STOP)) == 0) {
        stop();
        return SIGNAL_STOP;

    } else if (strncmp(cmd, COMMAND_CLEAR_LOGS, sizeof(COMMAND_CLEAR_LOGS)) == 0) {
        clearLogs();

    } else if (strncmp(cmd, COMMAND_HOST_ACCESS, sizeof(COMMAND_HOST_ACCESS)) == 0) {
        accessHost();

    } else if (strncmp(cmd, COMMAND_DUMP_LOGS, sizeof(COMMAND_DUMP_LOGS)) == 0) {
        dumpLogs();

    } else if (strncmp(cmd, COMMAND_RESET_DANGER_THRESHOLD, sizeof(COMMAND_RESET_DANGER_THRESHOLD)) == 0) {
        resetDangerThreshold();

    } else if (strncmp(cmd, COMMAND_RESET_DANGER_LEVEL, sizeof(COMMAND_RESET_DANGER_LEVEL)) == 0) {
        resetDangerLevel();

    } else if (strncmp(cmd, COMMAND_CHANGE_PASSWORD, sizeof(COMMAND_CHANGE_PASSWORD)) == 0) {
        changePassword();

    } else if (strncmp(cmd, COMMAND_CHANGE_JOYSTICK_PATTERN, sizeof(COMMAND_CHANGE_JOYSTICK_PATTERN)) == 0) {
        changeJoystickPattern();

    } else if (strncmp(cmd, COMMAND_TOGGLE, sizeof(COMMAND_TOGGLE)) == 0) {
        askServer(buffer);

    } else if (strncmp(cmd, COMMAND_SET_DANGER_THRESHOLD, sizeof(COMMAND_SET_DANGER_THRESHOLD)) == 0) {
        askServer(buffer);

    } else if (strncmp(cmd, COMMAND_CONFIGURE, sizeof(COMMAND_CONFIGURE)) == 0) {
        askServer(buffer);

    } else {
        printf("Unrecognized command '%s'\n", cmd);
    }
    return SIGNAL_NONE;
}

static void prompt()
{
    if (isLoggedIn) {
        printf(ASCII_CYAN "admin@" HOST_NAME ASCII_RESET ": " ASCII_CYAN "$ " ASCII_RESET);
        return;
    }
    printf(ASCII_CYAN "guest@" HOST_NAME ASCII_RESET ": " ASCII_CYAN "$ " ASCII_RESET);
}
static void displayWarning()
{
    printf(
        "You are now in a remote ssh session with the security system!\n"
        "WARNING: Please remember to exit after you are done with the session.\n"
    );
}

static void sendPing(void)
{
    ClientNet_send(CLIENT_REQ_PING, sizeof(CLIENT_REQ_PING));
    char response[RESPONSE_PACKET_SIZE];
    recvFromServ(response);
    if (strncmp(response, STATUS_CODE_BAD, sizeof(STATUS_CODE_BAD)) == 0) {
        ClientNet_cleanup();
        Logger_logInfo("ssh connection refused by server");
        printf("Connection refused by the server.\n");
        exit(EXIT_CODE_STANDARD);
    }
}

int main(int argc, char **argv)
{
    signal(SIGINT, sigintHandler);

    Logger_logInfo("guest connected via remote ssh client");

    ClientNet_init();

    // check if the server wants to ping
    sendPing();

    displayWarning();

    char command[RELAY_PACKET_SIZE];

    Signal signal = SIGNAL_NONE;
    while(signal != SIGNAL_STOP) {
        prompt();
        getInput(command, sizeof(command));
        signal = execute(command);
        command[0] = 0;
    }

    ClientNet_cleanup();
    return exitCode;
}

