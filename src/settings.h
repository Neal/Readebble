#pragma once

typedef struct {
	int headlines_font_size;
	int summary_font_size;
	int summary_font_color;
} Settings;

Settings* settings();
void settings_load(void);
void settings_save(void);
