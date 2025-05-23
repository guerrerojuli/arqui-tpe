#include <videoDriver.h>
#include <font.h>
#include <lib.h>

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

typedef struct {
	uint32_t hex_color;
	char c;
} ScreenChar;

struct ScreenInfo {
	uint32_t index_x;			// Current cursor X position in text buffer
	uint32_t index_y;			// Current cursor Y position in text buffer
	uint32_t font_size;			// Current font size in pixels
	ScreenChar buffer[SCREEN_TEXT_BUFFER_HEIGHT * SCREEN_TEXT_BUFFER_WIDTH];
} screen_info = {0, 0, DEFAULT_FONT_SIZE};

// Dirty flag for optimization - tracks if video buffer needs updating
static int buffer_dirty = 1;

// Video buffer for double buffering
static uint8_t video_buffer[MAX_VIDEO_BUFFER_WIDTH * MAX_VIDEO_BUFFER_HEIGHT * MAX_VIDEO_BUFFER_BYTES_PER_PIXEL];

//=============================================================================
// VALIDATION AND BOUNDS CHECKING
//=============================================================================

/**
 * Validates if pixel coordinates are within screen bounds
 */
static inline int is_valid_position(uint32_t x, uint32_t y) {
    return x < VBE_mode_info->width && y < VBE_mode_info->height;
}

/**
 * Validates if text coordinates are within text buffer bounds
 */
static inline int is_valid_text_position(uint32_t x, uint32_t y) {
    return x < get_chars_per_buff_line() && y < SCREEN_TEXT_BUFFER_HEIGHT;
}

/**
 * Checks if a rectangle fits within screen bounds
 */
static inline int is_within_bounds(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    return x + width <= get_video_buffer_width() && y + height <= get_video_buffer_height();
}

//=============================================================================
// DIMENSION HELPER FUNCTIONS
//=============================================================================

/**
 * Gets the effective video buffer width (limited by hardware)
 */
static uint32_t get_video_buffer_width() {
    return (MAX_VIDEO_BUFFER_WIDTH > VBE_mode_info->width) ? VBE_mode_info->width : MAX_VIDEO_BUFFER_WIDTH;
}

/**
 * Gets the effective video buffer height (limited by hardware)
 */
static uint32_t get_video_buffer_height() {
    return (MAX_VIDEO_BUFFER_HEIGHT > VBE_mode_info->height) ? VBE_mode_info->height : MAX_VIDEO_BUFFER_HEIGHT;
}

/**
 * Gets the bytes per pixel for the video buffer
 */
static uint32_t get_video_buffer_bytes_per_pixel() {
    uint32_t bytesPerPixel = VBE_mode_info->bpp / 8;
    return (MAX_VIDEO_BUFFER_BYTES_PER_PIXEL > bytesPerPixel) ? bytesPerPixel : MAX_VIDEO_BUFFER_BYTES_PER_PIXEL;
}

/**
 * Calculates font width in pixels based on current font size
 */
static uint32_t get_font_width() {
    return screen_info.font_size * CHAR_BIT_WIDTH;
}

/**
 * Calculates font height in pixels based on current font size
 */
static uint32_t get_font_height() {
    return screen_info.font_size * CHAR_BIT_HEIGHT;
}

/**
 * Calculates how many characters fit in one screen line
 */
static uint32_t get_chars_per_line() {
    return get_video_buffer_width() / get_font_width();
}

/**
 * Gets the effective characters per line limited by buffer width
 */
static uint32_t get_chars_per_buff_line() {
    uint32_t charsPerLine = get_chars_per_line();
    return (charsPerLine <= SCREEN_TEXT_BUFFER_WIDTH) ? charsPerLine : SCREEN_TEXT_BUFFER_WIDTH;
}

//=============================================================================
// COLOR HELPER FUNCTIONS
//=============================================================================

uint32_t rgb_to_hex(uint8_t r, uint8_t g, uint8_t b) {
    return (r) | (g << 8) | (b << 16);
}

RGB hex_to_rgb(uint32_t hex_color) {
    return (RGB){
        .r = hex_color & 0xFF,
        .g = (hex_color >> 8) & 0xFF,
        .b = (hex_color >> 16) & 0xFF
    };
}

//=============================================================================
// FRAME BUFFER MANAGEMENT
//=============================================================================

void update_frame_buffer() {
    // Only update if buffer has been modified (dirty flag optimization)
    if (buffer_dirty) {
        memcpy((void*)VBE_mode_info->framebuffer, video_buffer, 
               get_video_buffer_width() * get_video_buffer_height() * get_video_buffer_bytes_per_pixel());
        buffer_dirty = 0;
    }
}

//=============================================================================
// BASIC DRAWING FUNCTIONS
//=============================================================================

void put_pixel(uint32_t hex_color, uint32_t x, uint32_t y) {
    if (!is_valid_position(x, y)) return;
    
    uint64_t offset = (x * get_video_buffer_bytes_per_pixel()) + (y * VBE_mode_info->pitch);
    video_buffer[offset]     = (hex_color) & 0xFF;
    video_buffer[offset+1]   = (hex_color >> 8) & 0xFF; 
    video_buffer[offset+2]   = (hex_color >> 16) & 0xFF;
    
    buffer_dirty = 1;  // Mark buffer as needing update
}

void draw_square(uint32_t hex_color, uint32_t x, uint32_t y, uint32_t size) {
    if (!is_valid_position(x, y)) return;
    
    RGB color = hex_to_rgb(hex_color);
    uint32_t bytes_per_pixel = get_video_buffer_bytes_per_pixel();
    
    // Clip the square to screen boundaries
    uint32_t max_width = (x + size <= get_video_buffer_width()) ? size : get_video_buffer_width() - x;
    uint32_t max_height = (y + size <= get_video_buffer_height()) ? size : get_video_buffer_height() - y;
    
    // Draw the square row by row for better cache performance
    for (uint32_t row = 0; row < max_height; row++) {
        uint64_t row_offset = (y + row) * VBE_mode_info->pitch;
        for (uint32_t col = 0; col < max_width; col++) {
            uint64_t offset = (x + col) * bytes_per_pixel + row_offset;
            video_buffer[offset]     = color.r;
            video_buffer[offset+1]   = color.g; 
            video_buffer[offset+2]   = color.b;
        }
    }
    
    buffer_dirty = 1;  // Mark buffer as needing update
}

//=============================================================================
// TEXT RENDERING FUNCTIONS
//=============================================================================

void draw_char(char c, uint32_t x, uint32_t y, uint32_t color) {
    if (!is_valid_position(x, y)) return;
    
    // Store the character in the text buffer for font size changes
    uint32_t buffer_x = x / screen_info.font_size;
    uint32_t buffer_y = y / screen_info.font_size;
    
    if (buffer_x < SCREEN_TEXT_BUFFER_WIDTH && buffer_y < SCREEN_TEXT_BUFFER_HEIGHT) {
        uint32_t index = buffer_y * SCREEN_TEXT_BUFFER_WIDTH + buffer_x;
        screen_info.buffer[index].c = c;
        screen_info.buffer[index].hex_color = color;
    }
    
    // Render the character pixel by pixel using font bitmap
    for (uint32_t row = 0; row < CHAR_BIT_HEIGHT; row++) {
        uint8_t font_row = FONT[(unsigned char)c][row];
        for (uint32_t col = 0; col < CHAR_BIT_WIDTH; col++) {
            if (font_row & (0x01 << col)) {
                draw_square(color, x + (col * screen_info.font_size), 
                           y + (row * screen_info.font_size), screen_info.font_size);
            }
        }
    }
}

void draw_string(const char* str, uint32_t x, uint32_t y, uint32_t color) {
    if (!is_valid_position(x, y)) return;
    
    uint32_t current_x = x;
    uint32_t current_y = y;
    
    // Process each character in the string
    for (int i = 0; str[i] != '\0'; i++) {
        switch (str[i]) {
            case '\n':  // New line
                current_y += get_font_height();
                current_x = x;
                break;
            case '\t':  // Tab (4 spaces)
                current_x += get_font_width() * 4;
                break;
            case '\r':  // Carriage return
                current_x = x;
                break;
            case '\b':  // Backspace
                if (current_x > x) {
                    current_x -= get_font_width();
                }
                break;
            default:    // Regular character
                draw_char(str[i], current_x, current_y, color);
                current_x += get_font_width();
                break;
        }
    }
}

//=============================================================================
// TEXT BUFFER MANAGEMENT
//=============================================================================

void scroll_up() {
    // Move all lines up by one position
    for (uint32_t i = 0; i < (SCREEN_TEXT_BUFFER_HEIGHT - 1) * SCREEN_TEXT_BUFFER_WIDTH; i++) {
        screen_info.buffer[i] = screen_info.buffer[i + SCREEN_TEXT_BUFFER_WIDTH];
    }
    
    // Clear the bottom line
    uint32_t last_line_start = (SCREEN_TEXT_BUFFER_HEIGHT - 1) * SCREEN_TEXT_BUFFER_WIDTH;
    for (uint32_t i = 0; i < SCREEN_TEXT_BUFFER_WIDTH; i++) {
        screen_info.buffer[last_line_start + i].c = ' ';
        screen_info.buffer[last_line_start + i].hex_color = 0x000000;
    }
    
    buffer_dirty = 1;
}

void write_to_video_text_buffer(const char* data, uint32_t data_len, uint32_t hex_color) {
    for (uint32_t i = 0; i < data_len; i++) {
        switch (data[i]) {
            case '\n':  // New line
                screen_info.index_y += 1;
                if (screen_info.index_y >= SCREEN_TEXT_BUFFER_HEIGHT) {
                    scroll_up();
                    screen_info.index_y = SCREEN_TEXT_BUFFER_HEIGHT - 1;
                }
                screen_info.index_x = 0;
                break;
                
            case '\t':  // Tab (4 spaces)
                write_to_video_text_buffer("    ", 4, hex_color);
                break;
                
            case '\r':  // Carriage return
                screen_info.index_x = 0;
                break;
                
            case '\b':  // Backspace
                if (screen_info.index_x > 0) {
                    screen_info.index_x -= 1;
                    // Clear the character at current position
                    uint32_t index = screen_info.index_y * SCREEN_TEXT_BUFFER_WIDTH + screen_info.index_x;
                    screen_info.buffer[index].c = ' ';
                    screen_info.buffer[index].hex_color = hex_color;
                } else if (screen_info.index_y > 0) {
                    // Move to end of previous line
                    screen_info.index_x = get_chars_per_buff_line() - 1;
                    screen_info.index_y -= 1;
                    uint32_t index = screen_info.index_y * SCREEN_TEXT_BUFFER_WIDTH + screen_info.index_x;
                    screen_info.buffer[index].c = ' ';
                    screen_info.buffer[index].hex_color = hex_color;
                }
                break;
                
            default:    // Regular character
                // Add character to buffer at current cursor position
                uint32_t index = screen_info.index_y * SCREEN_TEXT_BUFFER_WIDTH + screen_info.index_x;
                screen_info.buffer[index].c = data[i];
                screen_info.buffer[index].hex_color = hex_color;
                
                // Advance cursor position
                screen_info.index_x += 1;
                if (screen_info.index_x >= get_chars_per_buff_line()) {
                    screen_info.index_x = 0;
                    screen_info.index_y += 1;
                    if (screen_info.index_y >= SCREEN_TEXT_BUFFER_HEIGHT) {
                        scroll_up();
                        screen_info.index_y = SCREEN_TEXT_BUFFER_HEIGHT - 1;
                    }
                }
                break;
        }
    }
    
    buffer_dirty = 1;
}

void clear_video_text_buffer(uint32_t background_color) {
    // Clear all characters in the text buffer
    for (uint32_t i = 0; i < SCREEN_TEXT_BUFFER_HEIGHT * SCREEN_TEXT_BUFFER_WIDTH; i++) {
        screen_info.buffer[i].c = ' ';
        screen_info.buffer[i].hex_color = background_color;
    }
    
    // Reset cursor to top-left
    screen_info.index_x = 0;
    screen_info.index_y = 0;
    
    buffer_dirty = 1;
}

//=============================================================================
// SCREEN MANAGEMENT FUNCTIONS
//=============================================================================

void set_font_size(uint32_t size) {
    if (size < 1) return;  // Validate minimum font size
    
    screen_info.font_size = size;
    
    // Clear the screen and redraw all text with new size
    clear_screen(0x000000);
    
    // Redraw all characters from the text buffer with new font size
    uint32_t x = 0;
    uint32_t y = 0;
    
    for (uint32_t i = 0; i < SCREEN_TEXT_BUFFER_HEIGHT * SCREEN_TEXT_BUFFER_WIDTH; i++) {
        ScreenChar sc = screen_info.buffer[i];
        if (sc.c == '\n') {
            x = 0;
            y++;
            continue;
        }
        if (sc.c != '\0' && sc.c != ' ') {  // Skip empty and space characters
            draw_char(sc.c, x * screen_info.font_size, y * screen_info.font_size, sc.hex_color);
        }
        x++;
        if (x >= get_chars_per_buff_line()) {
            x = 0;
            y++;
        }
    }
    
    update_frame_buffer();
}

void clear_screen(uint32_t color) {
    RGB rgb_color = hex_to_rgb(color);
    
    // Optimization: use memset for solid colors (when R=G=B)
    if (rgb_color.r == rgb_color.g && rgb_color.g == rgb_color.b) {
        memset(video_buffer, rgb_color.r, 
               get_video_buffer_width() * get_video_buffer_height() * get_video_buffer_bytes_per_pixel());
    } else {
        // Use pixel-by-pixel approach for non-uniform colors
        uint32_t total_size = get_video_buffer_width() * get_video_buffer_height() * get_video_buffer_bytes_per_pixel();
        for (uint32_t i = 0; i < total_size; i += 3) {
            video_buffer[i]     = rgb_color.r;
            video_buffer[i+1]   = rgb_color.g;
            video_buffer[i+2]   = rgb_color.b;
        }
    }
    
    // Clear the text buffer
    clear_video_text_buffer(color);
    
    buffer_dirty = 1;
    update_frame_buffer();
}