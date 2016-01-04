#pragma once

#include <pebble.h>

typedef struct {
	uint8_t index;
	char title[160];
} Headline;

void headlines_init(void);
void headlines_deinit(void);
void headlines_in_received_handler(DictionaryIterator *iter);
void headlines_reload_data_and_mark_dirty();
uint8_t headlines_count();
uint8_t headlines_progress();
char* headlines_get_error();
Headline* headlines_get(uint8_t index);
Headline* headlines_get_current();
uint8_t headlines_get_current_index();
void headlines_set_current(uint8_t index);
void headlines_request();
