#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "ledDisplay.h"

#include "../Utilities/utilities.h"

#define I2C_REG_BUFFER_SIZE             2

#define OUTPUT                          "out"
#define INPUT                           "in"

#define ON                              1
#define OFF                             0

#define LED_GPIO_EXPORT_PATH            "/sys/class/gpio/export"

#define CONFIG_P9_17_PIN_I2C            "config-pin p9_17 i2c"
#define CONFIG_P9_18_PIN_I2C            "config-pin p9_18 i2c"

#define SEG_PIN_NUM_61                  61
#define SEG_PIN_NUM_44                  44

#define GPIO_61_BASE                    "/sys/class/gpio/gpio61"
#define GPIO_44_BASE                    "/sys/class/gpio/gpio44"

#define GPIO_61_DIRECTION_PATH          GPIO_61_BASE "/direction"
#define GPIO_44_DIRECTION_PATH          GPIO_44_BASE "/direction"

#define LEFT_LED_GPIO_61_VALUE_PATH     GPIO_61_BASE "/value"
#define RIGHT_LED_GPIO_44_VALUE_PATH    GPIO_44_BASE "/value"

#define I2CDRV_LINUX_BUS0               "/dev/i2c-0"
#define I2CDRV_LINUX_BUS1               "/dev/i2c-1"
#define I2CDRV_LINUX_BUS2               "/dev/i2c-2"

#define I2C_DEVICE_ADDRESS              0x20

#define REG_DIRA                        0x00
#define REG_DIRB                        0x01
#define REG_OUTA                        0x14
#define REG_OUTB                        0x15

#define HEX_DISPLAY_OFF                 0x00
#define BASE_TEN                        10
#define LED_DISPLAY_MAX_NUM             99

#define SLEEP_MS                        10

typedef struct {
    int left;
    int right;
} LEDDigits;

// Each element represents a number's respective half from 0 to 9
static const unsigned char UPPER_EIGHT_BITS[BASE_TEN] = {
    0x87, 0x13, 0x0F, 0x07, 0x8B, 0x8D, 0x8D, 0x14, 0x8F, 0x8F
};
static const unsigned char LOWER_EIGHT_BITS[BASE_TEN] = {
    0xA1, 0x80, 0x31, 0xB0, 0x90, 0xB0, 0xB1, 0x04, 0xB1, 0x90
};

static bool s_stopLEDDisplayThead = false;
static int s_i2cFileDesc;

static pthread_t s_ledDisplayThread;

static LEDDigits s_ledDigits = {.left = 0, .right = 0};

static pthread_mutex_t s_digitsMutex = PTHREAD_MUTEX_INITIALIZER;

// ------------------------- PRIVATE ------------------------- //

static void turnLEDDisplayOnOrOff(int onOrOff, const char* path) 
{
    Utilities_writeIntValueToFile(onOrOff, path);
}

static int initI2cBus(const char* bus, int address) 
{
    int i2cFileDescription = open(bus, O_RDWR);
    if (i2cFileDescription < 0) {
        printf("I2C DRV: Unable to open bus for read/write (%s)\n", bus);
        perror("Error is:");
        exit(-1);
    }

    int result = ioctl(i2cFileDescription, I2C_SLAVE, address);
    if (result < 0) {
        perror("Unable to set I2C device to slave address.");
        exit(-1);
    }
    return i2cFileDescription;
}

static void exportLEDDisplayPins() 
{
    Utilities_exportGpioPin(GPIO_44_BASE, SEG_PIN_NUM_44);
    Utilities_exportGpioPin(GPIO_61_BASE, SEG_PIN_NUM_61);
}

static void setLEDDisplayDirections() 
{
    Utilities_writeStringValueToFile(OUTPUT, GPIO_44_DIRECTION_PATH);
    Utilities_writeStringValueToFile(OUTPUT, GPIO_61_DIRECTION_PATH);
}

static void configureLEDDisplaysToI2C() 
{
    Utilities_runCommand(CONFIG_P9_17_PIN_I2C);
    Utilities_runCommand(CONFIG_P9_18_PIN_I2C);
}

static void writeI2cReg(int i2cFileDescription, unsigned char regAddr, unsigned char value) 
{
    unsigned char buff[I2C_REG_BUFFER_SIZE];
    buff[0] = regAddr;
    buff[1] = value;

    int res = write(i2cFileDescription, buff, I2C_REG_BUFFER_SIZE);
    if (res != I2C_REG_BUFFER_SIZE) {
        perror("Unable to write i2c register");
        exit(-1);
    }
}

static bool receivedExitSignal() 
{
    bool signalReceived = false;
    pthread_mutex_lock(&s_digitsMutex);
    {
        signalReceived = s_stopLEDDisplayThead;
    }
    pthread_mutex_unlock(&s_digitsMutex);
    return signalReceived;
}

static void sendExitSignal() 
{
    pthread_mutex_lock(&s_digitsMutex);
    {
        s_stopLEDDisplayThead = true;
    }
    pthread_mutex_unlock(&s_digitsMutex);
}

// switches quickly between the left and right digit so it appears to display both at the same time
static void *displayLEDDigits(void *args) 
{
    while (!receivedExitSignal()) {

        int rightVal = 0;
        int leftVal = 0;
        pthread_mutex_lock(&s_digitsMutex);
        {
            rightVal = s_ledDigits.right;
            leftVal = s_ledDigits.left;
        }
        pthread_mutex_unlock(&s_digitsMutex);

        turnLEDDisplayOnOrOff(OFF, LEFT_LED_GPIO_61_VALUE_PATH);
        turnLEDDisplayOnOrOff(OFF, RIGHT_LED_GPIO_44_VALUE_PATH);

        writeI2cReg(s_i2cFileDesc, REG_OUTB, UPPER_EIGHT_BITS[rightVal]);
        writeI2cReg(s_i2cFileDesc, REG_OUTA, LOWER_EIGHT_BITS[rightVal]);

        // Display right digit
        turnLEDDisplayOnOrOff(ON, RIGHT_LED_GPIO_44_VALUE_PATH);

        Utilities_sleepForMs(SLEEP_MS);

        // Display left digit;
        if (leftVal != 0) {
            turnLEDDisplayOnOrOff(OFF, LEFT_LED_GPIO_61_VALUE_PATH);
            turnLEDDisplayOnOrOff(OFF, RIGHT_LED_GPIO_44_VALUE_PATH);

            writeI2cReg(s_i2cFileDesc, REG_OUTB, UPPER_EIGHT_BITS[leftVal]);
            writeI2cReg(s_i2cFileDesc, REG_OUTA, LOWER_EIGHT_BITS[leftVal]);

            turnLEDDisplayOnOrOff(ON, LEFT_LED_GPIO_61_VALUE_PATH);

            Utilities_sleepForMs(SLEEP_MS);
        }
    }
    return NULL;
}

void initiateLEDDisplayThread(void) 
{
    pthread_attr_t ledDisplayAttr;
    pthread_attr_init(&ledDisplayAttr);
    pthread_create(&s_ledDisplayThread, &ledDisplayAttr, displayLEDDigits, NULL);
}

// ------------------------- PUBLIC ------------------------- //
void LedDisplay_start() 
{
    configureLEDDisplaysToI2C();
    exportLEDDisplayPins();
    setLEDDisplayDirections();

    s_i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);

    writeI2cReg(s_i2cFileDesc, REG_DIRB, HEX_DISPLAY_OFF);
    writeI2cReg(s_i2cFileDesc, REG_DIRA, HEX_DISPLAY_OFF);

    // start the thread
    initiateLEDDisplayThread();
}

void LedDisplay_stop() 
{
    sendExitSignal();

    pthread_join(s_ledDisplayThread, NULL);

    turnLEDDisplayOnOrOff(OFF, RIGHT_LED_GPIO_44_VALUE_PATH);
    turnLEDDisplayOnOrOff(OFF, LEFT_LED_GPIO_61_VALUE_PATH);
    close(s_i2cFileDesc);
}

void LedDisplay_setDisplayNumber(int number) 
{
    if (number > LED_DISPLAY_MAX_NUM) {
        number = LED_DISPLAY_MAX_NUM;
    } else if (number < 0) {
        number = 0;
    }

    pthread_mutex_lock(&s_digitsMutex);
    {
        s_ledDigits.right = number % BASE_TEN;
        number /= BASE_TEN;
        s_ledDigits.left = number % BASE_TEN;
    }
    pthread_mutex_unlock(&s_digitsMutex);
}
