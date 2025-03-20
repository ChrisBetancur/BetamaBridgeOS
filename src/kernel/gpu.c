#include <kernel/gpu.h>
#include <kernel/gpu_utils.h>
#include <kernel/memory.h>
#include <framebuffer.h>

static void write_pixel(uint32_t x, uint32_t y, const pixel_t * pix) {
    uint8_t * location = fbinfo.buf + y*fbinfo.pitch + x*BYTES_PER_PIXEL;

    memcpy(location, pix, BYTES_PER_PIXEL);
}


void gpu_putc(char c) {
    uint8_t w,h;
    uint8_t mask;
    const uint8_t * bmp = font(c);
    uint32_t i, num_rows = fbinfo.height/CHAR_HEIGHT;

    // shift everything up one row
    if (fbinfo.chars_y >= num_rows) {
        // Copy a whole character row into the one above it
        for (i = 0; i < num_rows-1; i++)
            memcpy(fbinfo.buf + fbinfo.pitch*i*CHAR_HEIGHT, fbinfo.buf + fbinfo.pitch*(i+1)*CHAR_HEIGHT, fbinfo.pitch * CHAR_HEIGHT);
        // zero out the last row
        //zero_memory(fbinfo.buf + fbinfo.pitch*i*CHAR_HEIGHT,fbinfo.pitch * CHAR_HEIGHT);

            // Fill the last row with the current background color.
        for (uint32_t y = (num_rows - 1) * CHAR_HEIGHT; y < num_rows * CHAR_HEIGHT; y++) {
            for (uint32_t x = 0; x < fbinfo.width; x++) {
                write_pixel(x, y, &fbinfo.current_bg);
            }
        }
        fbinfo.chars_y--;
    }


    if (c == '\n' || c == '\r') {
        fbinfo.chars_x = 0;
        fbinfo.chars_y++;
        return;
    }

    if (c == 0x08 || c == 0x7F) {
        //fbinfo.chars_x--;
        bmp = font(' ');
        fbinfo.chars_x--;
    }

    for(w = 0; w < CHAR_WIDTH; w++) {
        for(h = 0; h < CHAR_HEIGHT; h++) {
            mask = 1 << (w);
            if (bmp[h] & mask)
                write_pixel(fbinfo.chars_x*CHAR_WIDTH + w, fbinfo.chars_y*CHAR_HEIGHT + h, &PIXEL_BLACK);
            else
                write_pixel(fbinfo.chars_x*CHAR_WIDTH + w, fbinfo.chars_y*CHAR_HEIGHT + h, &PIXEL_WHITE);
        }
    }

    if (c != 0x08 && c != 0x7f)
        fbinfo.chars_x++;
    if (fbinfo.chars_x > fbinfo.chars_width) {
        fbinfo.chars_x = 0;
        fbinfo.chars_y++;
    }
}

void gpu_puts(const char* str) {
    for (size_t i = 0; str[i] != '\0'; i++)
        gpu_putc(str[i]);
}

static void color_background(pixel_t pixel) {
    uint8_t *fb = (uint8_t*)fbinfo.buf;

    // Loop through every pixel
    for (uint32_t y = 0; y < fbinfo.height; y++) {
        for (uint32_t x = 0; x < fbinfo.width; x++) {
            // Calculate the byte offset for the current pixel
            uint32_t offset = y * fbinfo.pitch + x * BYTES_PER_PIXEL;

            // Set RGB values for white (0xFF each)
            fb[offset]     = pixel.red;   // Blue component
            fb[offset + 1] = pixel.green;   // Green component
            fb[offset + 2] = pixel.blue;   // Red component
        }
    }
}

void framebuffer_set_background(color_t color) {
    if (!fbinfo.buf) {
        // Framebuffer not initialized
        return;
    }
    switch (color) {
        case COLOR_BLACK:
            fbinfo.current_bg = PIXEL_BLACK;
            color_background(PIXEL_BLACK);
            break;
        case COLOR_WHITE:
            fbinfo.current_bg = PIXEL_WHITE;
            color_background(PIXEL_WHITE);
            break;
        case COLOR_RED:
            fbinfo.current_bg = PIXEL_RED;
            color_background(PIXEL_RED);
            break;
        case COLOR_GREEN:
            fbinfo.current_bg = PIXEL_GREEN;
            color_background(PIXEL_GREEN);
            break;
        case COLOR_BLUE:
            fbinfo.current_bg = PIXEL_BLUE;
            color_background(PIXEL_BLUE);
            break;
        default:
            break;
    }
}
