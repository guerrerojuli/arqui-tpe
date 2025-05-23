#include <stdint.h>
#include <videoDriver.h>
#include <font.h>
#include <lib.h>
#include <interrupts.h>

//=============================================================================
// VBE MODE INFORMATION STRUCTURE
//=============================================================================

struct vbe_mode_info_structure {
	uint16_t attributes;		// deprecated, only bit 7 should be of interest to you, and it indicates the mode supports a linear frame buffer.
	uint8_t window_a;			// deprecated
	uint8_t window_b;			// deprecated
	uint16_t granularity;		// deprecated; used while calculating bank numbers
	uint16_t window_size;
	uint16_t segment_a;
	uint16_t segment_b;
	uint32_t win_func_ptr;		// deprecated; used to switch banks from protected mode without returning to real mode
	uint16_t pitch;			// number of bytes per horizontal line
	uint16_t width;			// width in pixels
	uint16_t height;			// height in pixels
	uint8_t w_char;			// unused...
	uint8_t y_char;			// ...
	uint8_t planes;
	uint8_t bpp;			// bits per pixel in this mode
	uint8_t banks;			// deprecated; total number of banks in this mode
	uint8_t memory_model;
	uint8_t bank_size;		// deprecated; size of a bank, almost always 64 KB but may be 16 KB...
	uint8_t image_pages;
	uint8_t reserved0;
 
	uint8_t red_mask;
	uint8_t red_position;
	uint8_t green_mask;
	uint8_t green_position;
	uint8_t blue_mask;
	uint8_t blue_position;
	uint8_t reserved_mask;
	uint8_t reserved_position;
	uint8_t direct_color_attributes;
 
	uint32_t framebuffer;		// physical address of the linear frame buffer; write here to draw to the screen
	uint32_t off_screen_mem_off;
	uint16_t off_screen_mem_size;	// size of memory in the framebuffer but not being displayed on the screen
	uint8_t reserved1[206];
} __attribute__ ((packed));

typedef struct vbe_mode_info_structure * VBEInfoPtr;

VBEInfoPtr VBE_mode_info = (VBEInfoPtr) 0x0000000000005C00;

//=============================================================================
// TEXT BUFFER FOR RE-RENDERING
//=============================================================================

typedef struct {
    char c;
    uint32_t color;
} TextChar;

static TextChar text_buffer[SCREEN_TEXT_BUFFER_HEIGHT][SCREEN_TEXT_BUFFER_WIDTH];
static uint32_t cursor_x = 0;
static uint32_t cursor_y = 0;
static uint32_t font_size = 1;

//=============================================================================
// BASIC DRAWING FUNCTIONS
//=============================================================================

/**
 * Puts a single pixel at the specified coordinates
 */
void put_pixel(uint32_t hexColor, uint64_t x, uint64_t y) {
    if (x >= VBE_mode_info->width || y >= VBE_mode_info->height) return;

    uint64_t offset = (x * (VBE_mode_info->bpp / 8)) + (y * VBE_mode_info->pitch);
    uint8_t* framebuffer = (uint8_t*)VBE_mode_info->framebuffer;
    
    framebuffer[offset]     = (hexColor) & 0xFF;         // Blue
    framebuffer[offset+1]   = (hexColor >> 8) & 0xFF;    // Green
    framebuffer[offset+2]   = (hexColor >> 16) & 0xFF;   // Red
}

/**
 * Draws a rectangle with specified color and dimensions
 */
void draw_rect(uint32_t hexColor, uint32_t posX, uint32_t posY, uint32_t width, uint32_t height) {
    for (uint32_t y = posY; y < posY + height && y < VBE_mode_info->height; y++) {
        for (uint32_t x = posX; x < posX + width && x < VBE_mode_info->width; x++) {
            put_pixel(hexColor, x, y);
        }
    }
}

/**
 * Draws a square using the draw_rect function
 */
void draw_square(uint32_t hexColor, uint32_t posX, uint32_t posY, uint32_t size) {
    draw_rect(hexColor, posX, posY, size, size);
}

/**
 * Clears the entire screen with the specified color
 */
void clear_screen(uint32_t clearColor) {
    draw_rect(clearColor, 0, 0, VBE_mode_info->width, VBE_mode_info->height);
}

//=============================================================================
// TEXT RENDERING FUNCTIONS
//=============================================================================

/**
 * Gets current font width in pixels
 */
uint32_t get_font_width() {
    return font_size * CHAR_BIT_WIDTH;
}

/**
 * Gets current font height in pixels
 */
uint32_t get_font_height() {
    return font_size * CHAR_BIT_HEIGHT;
}

/**
 * Gets how many characters fit per line
 */
uint32_t get_chars_per_line() {
    return VBE_mode_info->width / get_font_width();
}

/**
 * Draws a single character at the specified position (FIXED - LSB to MSB reading)
 */
void draw_char(char c, uint32_t hexColor, uint32_t posX, uint32_t posY) {
    for (uint32_t y = 0; y < CHAR_BIT_HEIGHT; y++) {
        for (uint32_t x = 0; x < CHAR_BIT_WIDTH; x++) {
            // FIXED: Read bit from LSB to MSB (bit 0 is leftmost pixel)
            uint8_t bit = FONT[(unsigned char)c][y] & (1 << x);
            if (bit) {
                draw_square(hexColor, posX + x * font_size, posY + y * font_size, font_size);
            }
        }
    }
}

/**
 * Draws a string of characters at the specified position
 */
void draw_string(const char* str, uint32_t len, uint32_t hexColor, uint32_t posX, uint32_t posY) {
    uint32_t fontWidth = get_font_width();
    for (uint32_t i = 0; i < len; i++) {
        draw_char(str[i], hexColor, posX + i * fontWidth, posY);
    }
}

//=============================================================================
// TEXT BUFFER MANAGEMENT
//=============================================================================

/**
 * Re-renders all text from the buffer to the screen
 */
static void render_text_buffer() {
    clear_screen(0x000000);
    
    uint32_t font_width = get_font_width();
    uint32_t font_height = get_font_height();
    uint32_t chars_per_line = get_chars_per_line();
    uint32_t lines_per_screen = VBE_mode_info->height / font_height;
    
    // Determine which lines to show (scroll if necessary)
    uint32_t start_line = 0;
    if (cursor_y >= lines_per_screen) {
        start_line = cursor_y - lines_per_screen + 1;
    }
    
    // Render visible text
    for (uint32_t y = start_line; y < start_line + lines_per_screen && y < SCREEN_TEXT_BUFFER_HEIGHT; y++) {
        for (uint32_t x = 0; x < chars_per_line && x < SCREEN_TEXT_BUFFER_WIDTH; x++) {
            if (text_buffer[y][x].c != ' ') {
                draw_char(text_buffer[y][x].c, text_buffer[y][x].color, 
                         x * font_width, (y - start_line) * font_height);
            }
        }
    }
}

/**
 * Scrolls the text buffer up by one line
 */
static void scroll_text_buffer() {
    // Move all lines up
    for (uint32_t y = 0; y < SCREEN_TEXT_BUFFER_HEIGHT - 1; y++) {
        for (uint32_t x = 0; x < SCREEN_TEXT_BUFFER_WIDTH; x++) {
            text_buffer[y][x] = text_buffer[y + 1][x];
        }
    }
    
    // Clear last line
    for (uint32_t x = 0; x < SCREEN_TEXT_BUFFER_WIDTH; x++) {
        text_buffer[SCREEN_TEXT_BUFFER_HEIGHT - 1][x].c = ' ';
        text_buffer[SCREEN_TEXT_BUFFER_HEIGHT - 1][x].color = 0xFFFFFF;
    }
    
    cursor_y--;
}

/**
 * Writes text to screen at current cursor position
 */
void write_to_video_text_buffer(const char* data, uint32_t data_len, uint32_t hexColor) {
    uint32_t chars_per_line = get_chars_per_line();
    if (chars_per_line > SCREEN_TEXT_BUFFER_WIDTH) {
        chars_per_line = SCREEN_TEXT_BUFFER_WIDTH;
    }
    
    for (uint32_t i = 0; i < data_len; i++) {
        switch (data[i]) {
            case '\n':  // New line
                cursor_x = 0;
                cursor_y++;
                break;
                
            case '\r':  // Carriage return
                cursor_x = 0;
                break;
                
            case '\t':  // Tab (4 spaces)
                cursor_x = (cursor_x + 4) & ~3;  // Align to next multiple of 4
                if (cursor_x >= chars_per_line) {
                    cursor_x = 0;
                    cursor_y++;
                }
                break;
                
            case '\b':  // Backspace
                if (cursor_x > 0) {
                    cursor_x--;
                    text_buffer[cursor_y][cursor_x].c = ' ';
                    text_buffer[cursor_y][cursor_x].color = hexColor;
                } else if (cursor_y > 0) {
                    cursor_y--;
                    cursor_x = chars_per_line - 1;
                    text_buffer[cursor_y][cursor_x].c = ' ';
                    text_buffer[cursor_y][cursor_x].color = hexColor;
                }
                break;
                
            default:    // Regular character
                if (cursor_x >= chars_per_line) {
                    cursor_x = 0;
                    cursor_y++;
                }
                
                // Check if we need to scroll
                if (cursor_y >= SCREEN_TEXT_BUFFER_HEIGHT) {
                    scroll_text_buffer();
                }
                
                // Add character to buffer
                text_buffer[cursor_y][cursor_x].c = data[i];
                text_buffer[cursor_y][cursor_x].color = hexColor;
                cursor_x++;
                break;
        }
    }
    
    render_text_buffer();
}

/**
 * Clears the text buffer and resets cursor
 */
void clear_video_text_buffer() {
    cursor_x = 0;
    cursor_y = 0;
    
    // Clear text buffer
    for (uint32_t y = 0; y < SCREEN_TEXT_BUFFER_HEIGHT; y++) {
        for (uint32_t x = 0; x < SCREEN_TEXT_BUFFER_WIDTH; x++) {
            text_buffer[y][x].c = ' ';
            text_buffer[y][x].color = 0xFFFFFF;
        }
    }
    
    clear_screen(0x000000);
}

/**
 * Sets the font size (1-5) and re-renders all text
 */
void set_font_size(uint32_t fontSize) {
    if (fontSize > 0 && fontSize <= 5) {
        font_size = fontSize;
        render_text_buffer();  // Re-render with new font size
    }
}