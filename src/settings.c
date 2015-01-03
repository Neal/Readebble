#include <pebble.h>
#include "settings.h"
#include "persist.h"
#include "windows/win-settings.h"

Settings _settings = {
	.headlines_font_size = FONT_SIZE_SMALL,
	.story_font_size = FONT_SIZE_SMALL,
	.story_font_color = FONT_COLOR_WHITE,
};

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
