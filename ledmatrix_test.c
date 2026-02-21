#include <stdio.h>
#include "pico/stdlib.h"

// RGB Pins
#define R1_PIN  2
#define G1_PIN  3
#define B1_PIN  4
#define R2_PIN  5
#define G2_PIN  8
#define B2_PIN  9
// Address PINS
#define ADDR_A_PIN  10
#define ADDR_B_PIN  16
#define ADDR_C_PIN  18
#define ADDR_D_PIN  20
#define ADDR_E_PIN  22
// Control Pins
#define CLK_PIN 11
#define LAT_PIN 12
#define OE_PIN  13

// Basic Colors
#define COLOR_BLACK     0
#define COLOR_RED       1
#define COLOR_GREEN     2
#define COLOR_BLUE      4
#define COLOR_YELLOW    3
#define COLOR_MAGENTA   5
#define COLOR_CYAN      6
#define COLOR_WHITE     7

#define NUM_ROW_PIX 64
#define NUM_COL     32

// Framebuffer to store pixel data
uint8_t framebuffer[NUM_COL][NUM_ROW_PIX];

// Functions
void clear_display(void);
void config_pins(uint32_t pin_mask);
void set_pix(uint row, uint col, uint color);
void set_row_addr(uint row_addr);
void set_rgb1(uint pix_value);
void set_rgb2(uint pix_value);
void display_refresh(void);
void led_test_routine(void);

int main()
{
    stdio_init_all();
    uint32_t gpio_mask = 0;
    gpio_mask = (1 << R1_PIN) | (1 << G1_PIN) | (1 << B1_PIN);
    gpio_mask |= (1 << ADDR_A_PIN) | (1 << ADDR_B_PIN) | (1 << ADDR_C_PIN) | (1 << ADDR_D_PIN) | (1 << ADDR_E_PIN);
    gpio_mask |= (1 << CLK_PIN) | (1 << LAT_PIN) | (1 << OE_PIN);

    printf("PIN Mask: %X\n", gpio_mask);
    config_pins(gpio_mask);

    // Initialize framebuffer to all black
    clear_display();

    // Run the LED test routine
    led_test_routine();

    // After test completes, keep display refreshing
    while (true) {
        display_refresh();
    }
}

void config_pins(uint32_t pin_mask){
    // Setup GPIO Pins
    gpio_init_mask(pin_mask);
    gpio_set_dir_out_masked(pin_mask);

    // Initialize all control pins to safe states
    gpio_put(CLK_PIN, 0);   // Clock low
    gpio_put(LAT_PIN, 0);   // Latch low
    gpio_put(OE_PIN, 1);    // Output disabled (active low, so 1 = off)

    // Initialize RGB1 pins to off
    gpio_put(R1_PIN, 0);
    gpio_put(G1_PIN, 0);
    gpio_put(B1_PIN, 0);

    // Initialize address pins to 0
    gpio_put(ADDR_A_PIN, 0);
    gpio_put(ADDR_B_PIN, 0);
    gpio_put(ADDR_C_PIN, 0);
    gpio_put(ADDR_D_PIN, 0);
    gpio_put(ADDR_E_PIN, 0);
}

void clear_display(void){
    for (int row = 0; row < NUM_COL; row++){
        for (int col = 0; col < NUM_ROW_PIX; col++){
            framebuffer[row][col] = COLOR_BLACK;
        }
    }
}

void set_pix(uint row, uint col, uint color){
    if (row < NUM_COL && col < NUM_ROW_PIX){
        framebuffer[row][col] = color;
    }
}

void set_row_addr(uint row_addr){
    gpio_put(ADDR_A_PIN, row_addr & 1);
    gpio_put(ADDR_B_PIN, (row_addr >> 1) & 1);
    gpio_put(ADDR_C_PIN, (row_addr >> 2) & 1);
    gpio_put(ADDR_D_PIN, (row_addr >> 3) & 1);
    gpio_put(ADDR_E_PIN, (row_addr >> 4) & 1);
}

void set_rgb1(uint pix_value){
    gpio_put(R1_PIN, pix_value & 1);
    gpio_put(G1_PIN, (pix_value >> 1) & 1);
    gpio_put(B1_PIN, (pix_value >> 2) & 1);
}

void set_rgb2(uint pix_value){
    gpio_put(R2_PIN, pix_value & 1);
    gpio_put(G2_PIN, (pix_value >> 1) & 1);
    gpio_put(B2_PIN, (pix_value >> 2) & 1);
}

void display_refresh(void){
    // Cycle through all 32 rows
    // ADDR_E acts as half-select: 0 for rows 0-15, 1 for rows 16-31
    // ADDR_A-D select row within half (0-15)
    for (uint row_addr = 0; row_addr < NUM_COL; row_addr++){
        // Disable output while updating
        gpio_put(OE_PIN, 1);

        // Set the row address with ADDR_E as half-select
        set_row_addr(row_addr);

        // Shift out 64 pixels for this row using RGB1 only
        for (uint col = 0; col < NUM_ROW_PIX; col++){
            // Make sure clock is low before setting data
            gpio_put(CLK_PIN, 0);

            // Set RGB1 for current row
            set_rgb1(framebuffer[row_addr][col]);

            // Data setup time
            sleep_us(1);

            // Clock high to latch the data into shift register
            gpio_put(CLK_PIN, 1);
            sleep_us(1);  // Clock high time

            // Clock low to prepare for next bit
            gpio_put(CLK_PIN, 0);
            sleep_us(1);  // Clock low time
        }

        // Latch the data into the output registers
        gpio_put(LAT_PIN, 1);
        sleep_us(1);  // Latch pulse width 
        gpio_put(LAT_PIN, 0);
  

        // Enable output
        gpio_put(OE_PIN, 0);

        // Row display time (affects brightness) - reduced for proper refresh rate
        sleep_us(100);
    }
}

void led_test_routine(void){
    uint colors[] = {COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_WHITE};
    const char* color_names[] = {"RED", "GREEN", "BLUE", "WHITE"};

    printf("Starting LED test routine (looping continuously)\n");
    printf("================================\n");

    // Loop through entire test sequence continuously
    while(1){
        // PART 1: Full-screen color test
        printf("Part 1: Full-screen color test\n");
        for (uint color_idx = 0; color_idx < 4; color_idx++){
            printf("  Displaying full-screen %s...\n", color_names[color_idx]);

            // Fill entire display with this color
            for (uint row = 0; row < NUM_COL; row++){
                for (uint col = 0; col < NUM_ROW_PIX; col++){
                    framebuffer[row][col] = colors[color_idx];
                }
            }

            // Display for 2 seconds
            uint32_t start_time = to_ms_since_boot(get_absolute_time());
            while (to_ms_since_boot(get_absolute_time()) - start_time < 2000){
                display_refresh();
            }
        }

        printf("\n");
        clear_display();
        sleep_ms(500);

        // PART 2: Row-by-row test with RGB cycling
        printf("Part 2: Row-by-row test (RGB cycling)\n");
        for (uint row = 0; row < NUM_COL; row++){
            // Cycle through RGB colors for this row
            for (uint color_idx = 0; color_idx < 3; color_idx++){
                // Clear display
                clear_display();

                // Light up entire row with current color
                for (uint col = 0; col < NUM_ROW_PIX; col++){
                    set_pix(row, col, colors[color_idx]);
                }

                printf("  Row: %2d, Color: %s\n", row, color_names[color_idx]);

                // Display for 500ms with multiple refreshes
                uint32_t start_time = to_ms_since_boot(get_absolute_time());
                while (to_ms_since_boot(get_absolute_time()) - start_time < 500){
                    display_refresh();
                }
            }
        }

        printf("\n================================\n");
        printf("Test sequence complete, restarting...\n\n");
        clear_display();
        sleep_ms(1000);
    }
}