#include <pebble.h>
#include "settings.h"
#include "persist.h"
#include "windows/win-settings.h"

const char * font_size_string[] = {
	"Small",
	"Medium",
	"Large"
};

const char * font_color_string[] = {
	"Dark",
	"Light"
};

const char * headlines_font_size_value[] = {
	FONT_KEY_GOTHIC_14,
	FONT_KEY_GOTHIC_18,
	FONT_KEY_GOTHIC_24
};

const char * story_body_font_size_value[] = {
	FONT_KEY_GOTHIC_14,
	FONT_KEY_GOTHIC_18,
	FONT_KEY_GOTHIC_24
};

const char * story_title_font_size_value[] = {
	FONT_KEY_GOTHIC_14_BOLD,
	FONT_KEY_GOTHIC_18_BOLD,
	FONT_KEY_GOTHIC_24_BOLD
};

Settings _settings = {
	.headlines_font_size = FONT_SIZE_SMALL,
	.story_font_size = FONT_SIZE_SMALL,
	.story_font_color = FONT_COLOR_LIGHT,
};

#ifdef PBL_COLOR
static const InterfaceColors s_interface_colors[] = {
	// Dark
	{
		.main_background = {GColorBlackARGB8},
		.main_text = {GColorWhiteARGB8},
		.inverted_text = {GColorBlackARGB8},
		.selection_highlight = {GColorPastelYellowARGB8},
		.title_bar_background = {GColorOrangeARGB8},
		.title_bar_text = {GColorWhiteARGB8},
		.progress_bar_complete = {GColorPastelYellowARGB8},
		.progress_bar_remaining = {GColorBlackARGB8},
		.settings_text = {GColorCelesteARGB8},
		.settings_inverted_text = {GColorBlueARGB8}
	},
	// Light
	{
		.main_background = {GColorWhiteARGB8},
		.main_text = {GColorBlackARGB8},
		.inverted_text = {GColorWhiteARGB8},
		.selection_highlight = {GColorOrangeARGB8},
		.title_bar_background = {GColorPastelYellowARGB8},
		.title_bar_text = {GColorBlackARGB8},
		.progress_bar_complete = {GColorOrangeARGB8},
		.progress_bar_remaining = {GColorWhiteARGB8},
		.settings_text = {GColorBlueARGB8},
		.settings_inverted_text = {GColorCelesteARGB8}
	}
};
#else
static const InterfaceColors s_interface_colors[] = {
	// Dark
	{
		.main_background = {GColorBlackARGB8},
		.main_text = {GColorWhiteARGB8},
		.inverted_text = {GColorBlackARGB8},
		.selection_highlight = {GColorWhiteARGB8},
		.title_bar_background = {GColorBlackARGB8},
		.title_bar_text = {GColorWhiteARGB8},
		.progress_bar_complete = {GColorWhiteARGB8},
		.progress_bar_remaining = {GColorBlackARGB8},
		.settings_text = {GColorWhiteARGB8},
		.settings_inverted_text = {GColorBlackARGB8}
	},
	// Light
	{
		.main_background = {GColorWhiteARGB8},
		.main_text = {GColorBlackARGB8},
		.inverted_text = {GColorWhiteARGB8},
		.selection_highlight = {GColorBlackARGB8},
		.title_bar_background = {GColorWhiteARGB8},
		.title_bar_text = {GColorBlackARGB8},
		.progress_bar_complete = {GColorBlackARGB8},
		.progress_bar_remaining = {GColorWhiteARGB8},
		.settings_text = {GColorBlackARGB8},
		.settings_inverted_text = {GColorWhiteARGB8}
	}
};
#endif

void settings_init(void) {
	persist_read_data(KEY_PERSIST_SETTINGS, &_settings, sizeof(_settings));
	win_settings_init();
}

void settings_deinit(void) {
	win_settings_deinit();
	persist_write_data(KEY_PERSIST_SETTINGS, &_settings, sizeof(_settings));
}

Settings* settings() {
	return &_settings;
}

GFont settings_get_story_title_font_size(void) {
	return fonts_get_system_font(story_title_font_size_value[_settings.story_font_size]);
}

GFont settings_get_story_body_font_size(void) {
	return fonts_get_system_font(story_body_font_size_value[_settings.story_font_size]);
}

GFont settings_get_headlines_font_size(void) {
	return fonts_get_system_font(headlines_font_size_value[_settings.headlines_font_size]);
}

const char * settings_get_story_font_size_string(void) {
	return font_size_string[_settings.story_font_size];
}

const char * settings_get_headlines_font_size_string(void) {
	return font_size_string[_settings.headlines_font_size];
}

const char * settings_get_story_font_color_string(void) {
	return font_color_string[_settings.story_font_color];
}

const InterfaceColors * settings_get_colors(void) {
	return & s_interface_colors[_settings.story_font_color];
}

void settings_bump_headlines_font_size(void) {
	_settings.headlines_font_size++;
	if (_settings.headlines_font_size >= FONT_SIZE_NUM) {
		_settings.headlines_font_size = 0;
	}
}

void settings_bump_story_font_size(void) {
	_settings.story_font_size++;
	if (_settings.story_font_size >= FONT_SIZE_NUM) {
		_settings.story_font_size = 0;
	}
}

void settings_bump_story_font_color(void) {
	_settings.story_font_color++;
	if (_settings.story_font_color >= FONT_COLOR_NUM) {
		_settings.story_font_color = 0;
	}
}

