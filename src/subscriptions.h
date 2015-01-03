#pragma once

#include <pebble.h>

typedef struct {
	uint8_t index;
	char title[30];
} Subscription;

void subscriptions_init(void);
void subscriptions_deinit(void);
void subscriptions_in_received_handler(DictionaryIterator *iter);
void subscriptions_reload_data_and_mark_dirty();
void subscriptions_reset();
uint8_t subscriptions_count();
char* subscriptions_get_error();
Subscription* subscriptions_get(uint8_t index);
Subscription* subscriptions_get_current();
uint8_t subscriptions_get_current_index();
void subscriptions_set_current(uint8_t index);
