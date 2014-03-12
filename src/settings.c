#include <pebble.h>
#include "settings.h"
#include "common.h"

enum {
	FONT_SIZE_SMALL,
	FONT_SIZE_LARGE,
};

enum {
	FONT_COLOR_BLACK,
	FONT_COLOR_WHITE,
};

Settings _settings = {
	.headlines_font_size = FONT_SIZE_SMALL,
	.summary_font_size = FONT_COLOR_BLACK,
	.summary_font_color = FONT_COLOR_BLACK,
};

void settings_load(void) {
	if (persist_exists(KEY_SETTINGS)) {
		int res = persist_read_data(KEY_SETTINGS, &_settings, sizeof(_settings));
		APP_LOG(APP_LOG_LEVEL_DEBUG, "settings_load: %d", res);
	}
	else {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "settings_load: No settings.");
	}
}

Settings* settings() {
	return &_settings;
}

void settings_save(void) {
	int res = persist_write_data(KEY_SETTINGS, &_settings, sizeof(_settings));
	APP_LOG(APP_LOG_LEVEL_DEBUG, "settings_save: %d", res);
}
