#pragma once

#include <pebble.h>

void story_init(void);
void story_deinit(void);
void story_in_received_handler(DictionaryIterator *iter);
char* story_get();
void story_free();
void story_request();
uint8_t story_progress();

