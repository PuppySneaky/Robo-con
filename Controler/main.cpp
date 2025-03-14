/*============================================================================================================================================================================
Information
==============================================================================================================================================================================
SDK: 		v2.1.1
Toolchain:	14.2Rel1
Ninja:		v1.12.1
CMake:		v4.0.0-rc4
============================================================================================================================================================================*/



/*============================================================================================================================================================================
Includes    Libaries
============================================================================================================================================================================*/
#include    <stdio.h>
#include    "pico/stdlib.h"
#include    "hardware/pio.h"
#include    "pico/cyw43_arch.h"
#include    "hardware/uart.h"



/*============================================================================================================================================================================
Define      Variable            Value   Description
============================================================================================================================================================================*/

// Pin definitions
#define     MOTOR_A1            6     // Motor A forward
#define     MOTOR_A2            7     // Motor A reverse
#define     MOTOR_B1            8     // Motor B forward
#define     MOTOR_B2            9     // Motor B reverse
#define     MOTOR_C1            10    // Motor C forward
#define     MOTOR_C2            11    // Motor C reverse
#define     MOTOR_D1            12    // Motor D forward
#define     MOTOR_D2            13    // Motor D reverse
#define     LED_PIN             16     // Status LED on Pico (not CYW43)

// Constants
#define     MAX_SPEED           255
#define     BRAKE_INTENSITY     200
#define     DEFAULT_WAVELENGTH  1000 // Default wavelength in ms

/*============================================================================================================================================================================
Sub-Functions
============================================================================================================================================================================*/

// Function prototypes
void setup();
uint32_t wavelengthFunction(int8_t value);
void singleMotor(uint8_t motor, int speed);
void allMotor(int speed);
void cycler(int speed);
void pause();
void free();
void partCycler(int speed, uint8_t side);
void boot();
void emergencyPause();

// Global variables
bool isRunning = false;

int main() {
    // Initialize the Pico
    setup();
    
    printf("Robocon IoT Control System initialized\n");
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    
    // Demo sequence - replace with your actual control logic
    boot();
    sleep_ms(2000);
    
    allMotor(200);
    sleep_ms(wavelengthFunction(5)); // Use wavelength function with value 5
    
    cycler(150);
    sleep_ms(wavelengthFunction(3)); // Use wavelength function with value 3
    
    partCycler(180, 0); // Left side pivot
    sleep_ms(wavelengthFunction(2)); // Use wavelength function with value 2
    
    emergencyPause();
    sleep_ms(1000);
    
    free();
    
    // Main control loop
    while (true) {
        // Here you would handle incoming commands from UART or other sources
        // For this example, we'll just keep the program running
        sleep_ms(100);
        gpio_put(LED_PIN, !gpio_get(LED_PIN)); // Blink LED to show program is running
    }
    
    return 0;
}

// Initialize the hardware
void setup() {
    stdio_init_all();
    
    // Initialize CYW43 for built-in LED
    if (cyw43_arch_init()) {
        printf("Wi-Fi initialization failed\n");
        return;
    }
    
    // Configure motor control pins
    gpio_init(MOTOR_A1);
    gpio_init(MOTOR_A2);
    gpio_init(MOTOR_B1);
    gpio_init(MOTOR_B2);
    gpio_init(MOTOR_C1);
    gpio_init(MOTOR_C2);
    gpio_init(MOTOR_D1);
    gpio_init(MOTOR_D2);
    gpio_init(LED_PIN);
    
    gpio_set_dir(MOTOR_A1, GPIO_OUT);
    gpio_set_dir(MOTOR_A2, GPIO_OUT);
    gpio_set_dir(MOTOR_B1, GPIO_OUT);
    gpio_set_dir(MOTOR_B2, GPIO_OUT);
    gpio_set_dir(MOTOR_C1, GPIO_OUT);
    gpio_set_dir(MOTOR_C2, GPIO_OUT);
    gpio_set_dir(MOTOR_D1, GPIO_OUT);
    gpio_set_dir(MOTOR_D2, GPIO_OUT);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    
    // Initialize all motor pins to LOW
    gpio_put(MOTOR_A1, 0);
    gpio_put(MOTOR_A2, 0);
    gpio_put(MOTOR_B1, 0);
    gpio_put(MOTOR_B2, 0);
    gpio_put(MOTOR_C1, 0);
    gpio_put(MOTOR_C2, 0);
    gpio_put(MOTOR_D1, 0);
    gpio_put(MOTOR_D2, 0);
    gpio_put(LED_PIN, 0);
    
    sleep_ms(100); // Short delay to ensure initialization
}

// Convert int8 value to wavelength time in milliseconds
uint32_t wavelengthFunction(int8_t value) {
    // Map int8 value (-128 to 127) to a percentage (0-100%)
    // Then calculate time based on percentage of 1 second
    float percentage;
    
    if (value < 0) {
        // For negative values, map -128 -> 0 to 0% -> 50%
        percentage = 50.0f * (1.0f + (float)value / 128.0f);
    } else {
        // For positive values, map 0 -> 127 to 50% -> 100%
        percentage = 50.0f + (50.0f * (float)value / 127.0f);
    }
    
    // Calculate wavelength in milliseconds (percentage of 1000ms)
    uint32_t wavelength = (uint32_t)(percentage * 10.0f);
    
    printf("Wavelength value %d converted to %lu ms (%.1f%%)\n", value, wavelength, percentage);
    return wavelength;
}

// Control a single motor
void singleMotor(uint8_t motor, int speed) {
    uint8_t forward_pin = 0;
    uint8_t reverse_pin = 0;
    
    // Select the correct pins based on motor number
    switch (motor) {
        case 0: // Motor A
            forward_pin = MOTOR_A1;
            reverse_pin = MOTOR_A2;
            break;
        case 1: // Motor B
            forward_pin = MOTOR_B1;
            reverse_pin = MOTOR_B2;
            break;
        case 2: // Motor C
            forward_pin = MOTOR_C1;
            reverse_pin = MOTOR_C2;
            break;
        case 3: // Motor D
            forward_pin = MOTOR_D1;
            reverse_pin = MOTOR_D2;
            break;
        default:
            return; // Invalid motor number
    }
    
    // Set the motor direction based on speed
    if (speed > 0) {
        // Forward
        gpio_put(forward_pin, 1);
        gpio_put(reverse_pin, 0);
    } else if (speed < 0) {
        // Reverse
        gpio_put(forward_pin, 0);
        gpio_put(reverse_pin, 1);
    } else {
        // Stop
        gpio_put(forward_pin, 0);
        gpio_put(reverse_pin, 0);
    }
}

// Rotate all 4 wheels in the same direction
void allMotor(int speed) {
    printf("All motors at speed: %d\n", speed);
    
    // Set all motors to the same speed
    singleMotor(0, speed); // Motor A
    singleMotor(1, speed); // Motor B
    singleMotor(2, speed); // Motor C
    singleMotor(3, speed); // Motor D
}

// Rotate wheels in opposite directions (for cycling/spinning motion)
void cycler(int speed) {
    printf("Cycling at speed: %d\n", speed);
    
    // Left side forward, right side backward
    singleMotor(0, speed);    // Motor A forward
    singleMotor(1, speed);    // Motor B forward
    singleMotor(2, -speed);   // Motor C backward
    singleMotor(3, -speed);   // Motor D backward
}

// Lock all wheels (apply handbrakes)
void pause() {
    printf("Pausing - handbrakes applied\n");
    
    // Apply equal force in both directions to lock wheels
    gpio_put(MOTOR_A1, 1);
    gpio_put(MOTOR_A2, 1);
    gpio_put(MOTOR_B1, 1);
    gpio_put(MOTOR_B2, 1);
    gpio_put(MOTOR_C1, 1);
    gpio_put(MOTOR_C2, 1);
    gpio_put(MOTOR_D1, 1);
    gpio_put(MOTOR_D2, 1);
}

// Unlock wheels (release handbrakes)
void free() {
    printf("Freeing wheels - handbrakes released\n");
    
    // Release all motors
    gpio_put(MOTOR_A1, 0);
    gpio_put(MOTOR_A2, 0);
    gpio_put(MOTOR_B1, 0);
    gpio_put(MOTOR_B2, 0);
    gpio_put(MOTOR_C1, 0);
    gpio_put(MOTOR_C2, 0);
    gpio_put(MOTOR_D1, 0);
    gpio_put(MOTOR_D2, 0);
    
    isRunning = false;
}

// Rotate on one side (partial cycler)
void partCycler(int speed, uint8_t side) {
    if (side == 0) {
        printf("Part cycler - pivoting on left side\n");
        // Left side stationary, right side moving
        singleMotor(0, 0);      // Motor A stopped
        singleMotor(1, 0);      // Motor B stopped
        singleMotor(2, speed);  // Motor C forward
        singleMotor(3, speed);  // Motor D forward
    } else {
        printf("Part cycler - pivoting on right side\n");
        // Right side stationary, left side moving
        singleMotor(0, speed);  // Motor A forward
        singleMotor(1, speed);  // Motor B forward
        singleMotor(2, 0);      // Motor C stopped
        singleMotor(3, 0);      // Motor D stopped
    }
}

// Start engines at full power
void boot() {
    printf("Booting - starting engines at 100%%\n");
    
    // Flash the LED to indicate boot sequence
    for (int i = 0; i < 3; i++) {
        gpio_put(LED_PIN, 1);
        sleep_ms(100);
        gpio_put(LED_PIN, 0);
        sleep_ms(100);
    }
    
    // Gradual startup of all motors
    for (int i = 0; i <= MAX_SPEED; i += 10) {
        allMotor(i);
        sleep_ms(20);
    }
    
    // Run at full speed briefly
    allMotor(MAX_SPEED);
    sleep_ms(500);
    
    // Reduce to idle speed
    allMotor(50);
    sleep_ms(200);
    
    isRunning = true;
}

// Emergency stop by briefly reversing motors
void emergencyPause() {
    printf("Emergency stop - applying reverse thrust\n");
    
    // Record current state
    bool was_running = isRunning;
    
    if (was_running) {
        // Apply reverse thrust to quickly stop
        allMotor(-BRAKE_INTENSITY);
        sleep_ms(200);
    }
    
    // Stop all motors
    allMotor(0);
    
    // Apply handbrakes
    pause();
    
    isRunning = false;
}

/*============================================================================================================================================================================
Main Function
============================================================================================================================================================================*/
int main()
{
    while (true)
    {
        /* code */
    }
    
}

