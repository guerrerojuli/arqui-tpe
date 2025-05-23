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

//=============================================================================
// BASIC DRAWING FUNCTIONS
//=============================================================================

/**
 * Draws a single pixel at the specified coordinates.
 * @param hexColor RGB color (0xRRGGBB)
 * @param x X coordinate
 * @param y Y coordinate
 */
void put_pixel(uint32_t hexColor, uint64_t x, uint64_t y);

/**
 * Draws a rectangle with specified color and dimensions.
 * @param hexColor RGB color (0xRRGGBB)
 * @param posX X coordinate of top-left corner
 * @param posY Y coordinate of top-left corner
 * @param width Width in pixels
 * @param height Height in pixels
 */
void draw_rect(uint32_t hexColor, uint32_t posX, uint32_t posY, uint32_t width, uint32_t height);

/**
 * Draws a filled square.
 * @param hexColor RGB color (0xRRGGBB)
 * @param posX X coordinate of top-left corner
 * @param posY Y coordinate of top-left corner
 * @param size Width and height in pixels
 */
void draw_square(uint32_t hexColor, uint32_t posX, uint32_t posY, uint32_t size);

//=============================================================================
// TEXT RENDERING FUNCTIONS
//=============================================================================

/**
 * Draws a single character at the specified position.
 * @param c Character to draw
 * @param hexColor RGB color (0xRRGGBB)
 * @param posX X coordinate
 * @param posY Y coordinate
 */
void draw_char(char c, uint32_t hexColor, uint32_t posX, uint32_t posY);

/**
 * Draws a string of characters at the specified position.
 * @param str String to draw
 * @param len Length of the string
 * @param hexColor RGB color (0xRRGGBB)
 * @param posX X coordinate
 * @param posY Y coordinate
 */
void draw_string(const char* str, uint32_t len, uint32_t hexColor, uint32_t posX, uint32_t posY);

//=============================================================================
// DIMENSION HELPER FUNCTIONS
//=============================================================================

/**
 * Gets the font width in pixels based on current font size.
 * @return Font width in pixels
 */
uint32_t get_font_width(void);

/**
 * Gets the font height in pixels based on current font size.
 * @return Font height in pixels
 */
uint32_t get_font_height(void);

/**
 * Gets the number of characters that fit in one screen line.
 * @return Characters per line
 */
uint32_t get_chars_per_line(void);

//=============================================================================
// TEXT BUFFER MANAGEMENT
//=============================================================================

/**
 * Writes text data to the video text buffer with cursor management.
 * Handles special characters: \n, \t, \r, \b
 * @param data Text data to write
 * @param data_len Length of data in bytes
 * @param hexColor RGB color for the text (0xRRGGBB)
 */
void write_to_video_text_buffer(const char* data, uint32_t data_len, uint32_t hexColor);

/**
 * Clears the video text buffer and resets cursor position.
 * Sets all characters to spaces and cursor to (0,0).
 */
void clear_video_text_buffer(void);

//=============================================================================
// SCREEN MANAGEMENT FUNCTIONS
//=============================================================================

/**
 * Sets the font size and redraws the screen.
 * @param fontSize New font size in pixels (1-5)
 */
void set_font_size(uint32_t fontSize);

/**
 * Clears the entire screen with the specified color.
 * @param clearColor RGB color (0xRRGGBB)
 */
void clear_screen(uint32_t clearColor);

#endif
