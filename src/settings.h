#pragma once

typedef enum {
	FONT_SIZE_SMALL,
	FONT_SIZE_LARGE,
} FONT_SIZE;

typedef enum {
	FONT_COLOR_BLACK,
	FONT_COLOR_WHITE,
} FONT_COLOR;

typedef struct {
	FONT_SIZE headlines_font_size;
	FONT_SIZE story_font_size;
	FONT_COLOR story_font_color;
} Settings;

void settings_init(void);
void settings_deinit(void);
Settings* settings();
