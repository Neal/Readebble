#pragma once

typedef enum {
	FONT_SIZE_INVALID = -1,

	FONT_SIZE_SMALL,
	FONT_SIZE_MEDIUM,
	FONT_SIZE_LARGE,

	FONT_SIZE_NUM
} FONT_SIZE;

typedef enum {
	FONT_COLOR_INVALID = -1,

	FONT_COLOR_DARK,
	FONT_COLOR_LIGHT,

	FONT_COLOR_NUM
} FONT_COLOR;

typedef struct {
	FONT_SIZE headlines_font_size;
	FONT_SIZE story_font_size;
	FONT_COLOR story_font_color;
} Settings;

typedef struct {
	GColor main_background;
	GColor main_text;
	GColor inverted_text;
	GColor selection_highlight;
	GColor title_bar_background;
	GColor title_bar_text;
	GColor progress_bar_complete;
	GColor progress_bar_remaining;
	GColor settings_text;
	GColor settings_inverted_text;
} InterfaceColors;

void settings_init(void);
void settings_deinit(void);
Settings* settings();

GFont settings_get_story_title_font_size(void);
GFont settings_get_story_body_font_size(void);
GFont settings_get_headlines_font_size(void);

const char * settings_get_headlines_font_size_string(void);
const char * settings_get_story_font_size_string(void);
const char * settings_get_story_font_color_string(void);

void settings_bump_headlines_font_size(void);
void settings_bump_story_font_size(void);
void settings_bump_story_font_color(void);

const InterfaceColors * settings_get_colors(void);


