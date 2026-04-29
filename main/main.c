// Lab4_Ex2

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

//function declarations
void setup_gpio();
char readKeyPadBlock(char _keys[4][4], int _scanColPins[4], int _rowPins[4]);

#define ROWS 4
#define COLS 4

//variables
int scanRowPins[ROWS] = {38, 37, 36, 35};
int colPins[COLS] = {39, 45, 48, 47};

//2d array of chars on the keypad
char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

void app_main() {
    setup_gpio(); //run the setup to initialize the pins
    while (1) {
        char key = readKeyPadBlock(keys, colPins, scanRowPins); 

        //check if "scan_keypad" returned a button press
        if (key != '\0') {
          printf("You pressed: %c\n", key);
        }

        printf("Working...\n");
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/* Function Name    - setup_gpio
 * Description      - sets the direction of the row pins to output and the column pins to input & pullup only
 * Return type      - void
 * Parameters       - NA
 */
void setup_gpio() {
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
}


char readKeyPadBlock(char _keys[4][4], int _scanColPins[4], int _rowPins[4])
{
    
    uint8_t scanVal;
    char retVal = '\0';
    int row = 4;
    int col = 4;

    do {
        printf("Scanning...\n");

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
        vTaskDelay(pdMS_TO_TICKS(200));
    } while (retVal == '\0');

    return retVal;
}
