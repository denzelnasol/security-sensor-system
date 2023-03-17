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

#define REG_LOWER                        0x14
#define REG_UPPER                        0x15

#define SPECIAL_SYMBOL_LOWER            0x1e
#define SPECIAL_SYMBOL_UPPER            0x78


#define BASE_TEN                        10
#define LED_DISPLAY_MAX_NUM             99

#define INDICATOR_BIT                   0x40

#define SLEEP_MS                        10

typedef struct {
    unsigned char upper;
    unsigned char lower; 
} Digit;

typedef struct {
    Digit msd;
    Digit lsd;
    bool isSpecial;
} Number;

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

static Number s_ledDigits;

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
    Digit rightVal, leftVal;
    bool isSpecial = false;
    while (!receivedExitSignal()) {
        pthread_mutex_lock(&s_digitsMutex);
        {
            rightVal = s_ledDigits.right;
            leftVal = s_ledDigits.left;
            isSpecial = s_ledDigits.isSpecial;
        }
        pthread_mutex_unlock(&s_digitsMutex);

        if (isSpecial) {
            turnLEDDisplayOnOrOff(OFF, LEFT_LED_GPIO_61_VALUE_PATH);
            turnLEDDisplayOnOrOff(OFF, RIGHT_LED_GPIO_44_VALUE_PATH);

            writeI2cReg(s_i2cFileDesc, REG_UPPER, SPECIAL_SYMBOL_UPPER);
            writeI2cReg(s_i2cFileDesc, REG_LOWER, SPECIAL_SYMBOL_LOWER);

            turnLEDDisplayOnOrOff(ON, RIGHT_LED_GPIO_44_VALUE_PATH);
            turnLEDDisplayOnOrOff(ON, LEFT_LED_GPIO_61_VALUE_PATH);

            Utilities_sleepForMs(SLEEP_MS);
            continue;
        }

        turnLEDDisplayOnOrOff(OFF, LEFT_LED_GPIO_61_VALUE_PATH);
        turnLEDDisplayOnOrOff(OFF, RIGHT_LED_GPIO_44_VALUE_PATH);

        writeI2cReg(s_i2cFileDesc, REG_UPPER, rightVal.upper);
        writeI2cReg(s_i2cFileDesc, REG_LOWER, rightVal.lower);

        // Display right digit
        turnLEDDisplayOnOrOff(ON, RIGHT_LED_GPIO_44_VALUE_PATH);

        Utilities_sleepForMs(SLEEP_MS);

        // Display left digit;
        if (leftVal != 0) {
            turnLEDDisplayOnOrOff(OFF, LEFT_LED_GPIO_61_VALUE_PATH);
            turnLEDDisplayOnOrOff(OFF, RIGHT_LED_GPIO_44_VALUE_PATH);

            writeI2cReg(s_i2cFileDesc, REG_UPPER, leftVal.upper);
            writeI2cReg(s_i2cFileDesc, REG_LOWER, leftVal.lower);

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

void LedDisplay_setDisplayNumber(int number, unsigned int indicatorOptions) 
{
    if (number > LED_DISPLAY_MAX_NUM) {
        number = LED_DISPLAY_MAX_NUM;
    } else if (number < 0) {
        number = 0;
    }

    Digit lsd, msd;
    int lsdDigit = number % BASE_TEN;
    int msdDigit = (number / BASE_TEN) % BASE_TEN;
    lsd.upper = UPPER_EIGHT_BITS[lsdDigit];
    msd.upper = UPPER_EIGHT_BITS[msdDigit];
    lsd.lower = LOWER_EIGHT_BITS[lsdDigit];
    msd.lower = LOWER_EIGHT_BITS[msdDigit];

    if ((indicatorOptions & S16_SET_INDICATOR) == S16_SET_INDICATOR) {
        msd.lower += INDICATOR_BIT;
    }

    pthread_mutex_lock(&s_digitsMutex);
    {
        s_ledDigits.lsd = lsd;
        s_ledDigits.msd = msd;
        s_ledDigits.isSpecial = false;
    }
    pthread_mutex_unlock(&s_digitsMutex);
}

void LedDisplay_showSpecial(void)
{
    pthread_mutex_lock(&s_digitsMutex);
    {
        s_ledDigits.isSpecial = true;
    }
    pthread_mutex_unlock(&s_digitsMutex);
}

