// Lab4_Ex2

//INCLUDE
#include <stdio.h>
#include <math.h> 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "driver/adc.h"

//FUNCTION DEF
void setup_gpio();
char readKeyPadNoBlock(char _keys[4][4], int _scanColPins[4], int _rowPins[4]);
float readTemperature(adc1_channel_t channel, float nominalRes, float B);

//VARIABLES
#define ROWS 4
#define COLS 4
#define THERMO ADC1_CHANNEL_3   
#define BUZZER 14
#define BLUE_LED 6
#define RED_LED 15
#define GREEN_LED 17
int scanRowPins[ROWS] = {38, 37, 36, 35};
int colPins[COLS] = {39, 45, 48, 47};
char lastKey = '\0';

    //2d array of chars on the keypad
char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

//MAIN
void app_main() {

    //run the setup to initialize the pins
    setup_gpio(); 

    //main loop
    while (1) {
        char key = readKeyPadNoBlock(keys, colPins, scanRowPins); 

        //check if "scan_keypad" returned a button press different to the last one
        if (key != '\0' && key != lastKey) {
          printf("You pressed: %c\n", key);
          gpio_set_level(BUZZER, 1);
          gpio_set_level(BLUE_LED, 1);
          gpio_set_level(GREEN_LED, 1);
          gpio_set_level(RED_LED, 1);
          vTaskDelay(pdMS_TO_TICKS(200));
          gpio_set_level(BUZZER, 0);
        }
        vTaskDelay(pdMS_TO_TICKS(50));
        
        //change last key pressed
        lastKey = key;

        //***************

        float tempC = readTemperature(THERMO, 10000.0, 3950.0);
        printf("Current temperature is: %.2lf°C\n", tempC);
    }
}

//FUNCTIONS

/* Function Name    - setup_gpio
 * Description      - sets up gpio pins and adc channels
 * Return type      - void
 * Parameters       - NA
 */
void setup_gpio() 
{
    //setup keypad pins
    for (int i = 0; i < ROWS; i++) {
        gpio_set_direction(scanRowPins[i], GPIO_MODE_OUTPUT);
        gpio_set_level(scanRowPins[i], 1);
    }
    for (int i = 0; i < COLS; i++) {
        gpio_set_direction(colPins[i], GPIO_MODE_INPUT);
        // all buttons are set as pullup configuration 
        // using the internal pullup resistor in the microcontroller
        gpio_set_pull_mode(colPins[i], GPIO_PULLUP_ONLY); 
    }

    //setup adc1 channel
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(THERMO, ADC_ATTEN_DB_12);

    //setup buzzer
    gpio_set_direction(BUZZER, GPIO_MODE_OUTPUT);

    //setup leds
    gpio_set_direction(BLUE_LED, GPIO_MODE_OUTPUT);
    gpio_set_direction(RED_LED, GPIO_MODE_OUTPUT);
    gpio_set_direction(GREEN_LED, GPIO_MODE_OUTPUT);

}

/* Function Name    - readkeyPadNoBlock
 * Description      - reads the active pin of the keypad
 * Return type      - char
 * Parameters       - the array of possible outputs, the array of column pins to scan, the array of row pins to set
 */
char readKeyPadNoBlock(char _keys[4][4], int _scanColPins[4], int _rowPins[4])
{
    
    uint8_t scanVal;
    char retVal = '\0';
    int row = 4;
    int col = 4;

    for (int i = 0; i < row; i++) {
        scanVal = ~(1 << i); 
        
        for (int j = 0; j < row; j++) {
            // 1. here, 'pins' can be either row pins or column pins. Select the correct one
            gpio_set_level(_rowPins[j], (scanVal >> j) & 1); 
        }
        vTaskDelay(pdMS_TO_TICKS(5));

        for (int k = 0; k < col; k++) {
            // 2. pullup configuration
            if (gpio_get_level(_scanColPins[k]) == 0) { 
                retVal = _keys[i][k];
            }
        }
    }
    vTaskDelay(pdMS_TO_TICKS(10));

    return retVal;
}

/* Function Name    - readTemperature
 * Description      - reads the analog value of the pin channel, then converts it to 10 bit width, then converts the answer into a temperature in degrees celsius.
 * Return type      - float
 * Parameters       - the adc1 channel that is being read "channel", the float that defines the nominal resistance of the thermistor "nominalRes", and the float that determines the thermal index of the thermistor "B"
 */
float readTemperature(adc1_channel_t channel, float nominalRes, float B)
{
    int thermoData = adc1_get_raw(channel);
    thermoData >>= 2;
    float Rt = nominalRes * (thermoData * 3.3 / 1043.0) / (3.3-(thermoData * 3.3 /1043.0));
    float thermoTemp = (1.0 / ((1.0/298.15) + (log(Rt/nominalRes)/B) )) - 273.15;
    thermoTemp = (1.0 / ((1.0/298.15) + (log(Rt/9700.0)/3950.0) )) - 273.15;
    return thermoTemp;
}