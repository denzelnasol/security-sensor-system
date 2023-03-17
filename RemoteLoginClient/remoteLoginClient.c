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

#include "clientNet.h"

#include "../Networking/networking.h"
#include "../Utilities/utilities.h"
#include "../EventLogger/logger.h"

#define PASSWORD_LENGTH                         32
#define BUFFER_LIMIT                            512
#define MFA_BUFFER_SIZE                         4

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

#define EXIT_CODE_HACCESS                       EXIT_SUCCESS
#define EXIT_CODE_STANDARD                      1


typedef enum {
    SIGNAL_NONE,
    SIGNAL_STOP,
} Signal;


static bool isLoggedIn = false;
static int exitCode = EXIT_CODE_STANDARD;

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

static Signal execute(char *command);
static void prompt(void);
static void displayWarning(void);


// the user cannot 'ctrl+c' to terminate the program.
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
    ClientNet_receive(response);

    printf("%s\n", response);
}

static void login()
{
    if (isLoggedIn) {
        printf("Already logged in.\n");
        return;
    }
    char response[RELAY_PACKET_SIZE];

    ClientNet_send(COMMAND_LOGIN, sizeof(COMMAND_LOGIN));
    ClientNet_receive(response);

    printf("%s", response);

    // prmpt user for password
    char password[PASSWORD_LENGTH];
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = 0;

    // send to the server
    ClientNet_send(password, strlen(password));
    ClientNet_receive(response);

    if (strncmp(response, STATUS_CODE_OK, sizeof(STATUS_CODE_OK))) {
        isLoggedIn = true;
        printf("Logged in as admin.\n");
        return;
    } else if (strncmp(response, STATUS_CODE_BAD, sizeof(STATUS_CODE_BAD))) {
        printf("Incorrect password.\n");
        return;
    }
    
    printf("%s", response);
    // extra MFA steps
    // prmpt user for mfa code
    char mfaPassword[MFA_BUFFER_SIZE];
    fgets(mfaPassword, sizeof(mfaPassword), stdin);
    mfaPassword[strcspn(mfaPassword, "\n")] = 0;

    // send to the server
    ClientNet_send(mfaPassword, strlen(mfaPassword));
    ClientNet_receive(response);

    if (strncmp(response, STATUS_CODE_OK, sizeof(STATUS_CODE_OK))) {
        isLoggedIn = true;
        printf("Logged in as admin.\n");
    } else if (strncmp(response, STATUS_CODE_BAD, sizeof(STATUS_CODE_BAD))) {
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
    printf("Logout ok.");
}
static void help()
{
    if (isLoggedIn) {
        printf(
            "Available commands:\n"
            COMMAND_GET_DANGER_LEVEL ": gets the danger level\n"
            COMMAND_GET_NUM_TRIGGERS ": gets the number of triggers\n"
            COMMAND_LOGIN ": login as admin\n"
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
    Utilities_runCommand("clear");
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
    exitCode = EXIT_CODE_HACCESS;
}
static void stop()
{
    ClientNet_send(COMMAND_STOP, sizeof(COMMAND_STOP));
    
    char response[RESPONSE_PACKET_SIZE];
    ClientNet_receive(response);

    printf("%s\n", response);

    exitCode = EXIT_CODE_HACCESS;
}
static void resetDangerThreshold()
{
    ClientNet_send(COMMAND_RESET_DANGER_THRESHOLD, sizeof(COMMAND_RESET_DANGER_THRESHOLD));
    
    char response[RESPONSE_PACKET_SIZE];
    ClientNet_receive(response);

    printf("%s\n", response);
}
static void resetDangerLevel()
{
    ClientNet_send(COMMAND_RESET_DANGER_LEVEL, sizeof(COMMAND_RESET_DANGER_LEVEL));
    
    char response[RESPONSE_PACKET_SIZE];
    ClientNet_receive(response);

    printf("%s\n", response);
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
        exitCode = EXIT_CODE_STANDARD;
        return SIGNAL_STOP;

    } else if (strncmp(cmd, COMMAND_HELP, sizeof(COMMAND_HELP)) == 0) {
        help();

    } else if (strncmp(cmd, COMMAND_CLEAR_SCREEN, sizeof(COMMAND_CLEAR_SCREEN)) == 0) {
        clearScreen();

    } else if (strncmp(cmd, COMMAND_LOGIN, sizeof(COMMAND_LOGIN)) == 0) {
        login();

    } else if (strncmp(cmd, COMMAND_LOGOUT, sizeof(COMMAND_LOGOUT)) == 0) {
        logout();

    } else if (strncmp(cmd, COMMAND_GET_DANGER_LEVEL, sizeof(COMMAND_GET_DANGER_LEVEL)) == 0) {
        askServer(buffer);

    } else if (strncmp(cmd, COMMAND_GET_NUM_TRIGGERS, sizeof(COMMAND_GET_NUM_TRIGGERS)) == 0) {
        askServer(buffer);
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
        printf("admin@thebbg> ");
        return;
    }
    printf("guest@thebbg> ");
}
static void displayWarning()
{
    printf(
        "You are now in a remote ssh session with the security system!\n"
        "WARNING: Please remember to exit after you are done with the session.\n"
    );
}

int main(int argc, char **argv)
{
    ClientNet_init();

    // signal(SIGINT, sigintHandler);
    displayWarning();

    char command[RELAY_PACKET_SIZE];

    Signal signal = SIGNAL_NONE;
    while(signal != SIGNAL_STOP) {
        prompt();
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0;
        signal = execute(command);
    }

    ClientNet_cleanup();

    return exitCode;
}

