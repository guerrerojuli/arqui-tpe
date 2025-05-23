#ifndef VIDEO_DRIVER_H
#define VIDEO_DRIVER_H

#include <stdint.h>

// Buffer dimensions
#define MAX_VIDEO_BUFFER_WIDTH 800
#define MAX_VIDEO_BUFFER_HEIGHT 600
#define MAX_VIDEO_BUFFER_BYTES_PER_PIXEL 3

// Font dimensions
#define CHAR_BIT_WIDTH 8
#define CHAR_BIT_HEIGHT 16

// Text buffer dimensions for 4px font size
#define SCREEN_TEXT_BUFFER_WIDTH 200  // 800px width / 4px font size
#define SCREEN_TEXT_BUFFER_HEIGHT 150 // 600px height / 4px font size
#define DEFAULT_FONT_SIZE 4

/**
 * RGB color structure for easier color manipulation
 */
typedef struct {
    uint8_t r, g, b;
} RGB;

/**
 * Draws a single pixel at the specified coordinates.
 * @param hex_color RGB color (0xRRGGBB)
 * @param x X coordinate
 * @param y Y coordinate
 */
void put_pixel(uint32_t hex_color, uint32_t x, uint32_t y);

/**
 * Draws a character at the specified position.
 * @param c Character to draw
 * @param x X coordinate
 * @param y Y coordinate
 * @param color RGB color (0xRRGGBB)
 */
void draw_char(char c, uint32_t x, uint32_t y, uint32_t color);

/**
 * Draws a string at the specified position.
 * @param str String to draw
 * @param x X coordinate
 * @param y Y coordinate
 * @param color RGB color (0xRRGGBB)
 */
void draw_string(const char* str, uint32_t x, uint32_t y, uint32_t color);

/**
 * Draws a filled square.
 * @param hex_color RGB color (0xRRGGBB)
 * @param x X coordinate of top-left corner
 * @param y Y coordinate of top-left corner
 * @param size Width and height in pixels
 */
void draw_square(uint32_t hex_color, uint32_t x, uint32_t y, uint32_t size);

/**
 * Clears the entire screen with the specified color.
 * Uses optimized memset when possible for solid colors.
 * @param color RGB color (0xRRGGBB)
 */
void clear_screen(uint32_t color);

/**
 * Sets the font size and updates the display accordingly.
 * @param size New font size in pixels (minimum 1)
 */
void set_font_size(uint32_t size);

/**
 * Updates the frame buffer with the current video buffer contents.
 * Only performs copy if buffer has been modified (dirty flag optimization).
 */
void update_frame_buffer(void);

/**
 * Writes text data to the video text buffer with cursor management.
 * Handles special characters: \n, \t, \r, \b
 * Automatically scrolls when reaching bottom of screen.
 * @param data Text data to write
 * @param data_len Length of data in bytes
 * @param hex_color RGB color for the text (0xRRGGBB)
 */
void write_to_video_text_buffer(const char* data, uint32_t data_len, uint32_t hex_color);

/**
 * Scrolls the text buffer up by one line.
 * All text moves up and the bottom line is cleared.
 */
void scroll_up(void);

/**
 * Converts RGB components to hexadecimal color value.
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @return Hexadecimal color value (0xRRGGBB)
 */
uint32_t rgb_to_hex(uint8_t r, uint8_t g, uint8_t b);

/**
 * Converts hexadecimal color to RGB structure.
 * @param hex_color Hexadecimal color value (0xRRGGBB)
 * @return RGB structure with separated color components
 */
RGB hex_to_rgb(uint32_t hex_color);

/**
 * Clears the text buffer and resets cursor position.
 * Sets all characters to spaces and cursor to (0,0).
 * @param background_color Background color for cleared text
 */
void clear_video_text_buffer(uint32_t background_color);

#endif
