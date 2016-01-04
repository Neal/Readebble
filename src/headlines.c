#include <pebble.h>
#include "headlines.h"
#include "libs/pebble-assist.h"
#include "generated/appinfo.h"
#include "generated/keys.h"
#include "subscriptions.h"
#include "windows/win-headlines.h"

static Headline *headlines = NULL;

static uint8_t num_headlines = 0;
static uint8_t current_headline = 0;
static uint8_t progress = 0;

static char *error = NULL;

void headlines_init(void) {
	progress = 0;
	win_headlines_init();
}

void headlines_deinit(void) {
	free_safe(error);
	free_safe(headlines);
	win_headlines_deinit();
}

void headlines_in_received_handler(DictionaryIterator *iter) {
	Tuple *tuple = dict_find(iter, APP_KEY_METHOD);
	if (!tuple) return;
	free_safe(error);
	switch (tuple->value->uint8) {
		case KEY_METHOD_ERROR: {
			tuple = dict_find(iter, APP_KEY_TITLE);
			if (!tuple) break;
			error = malloc(tuple->length);
			strncpy(error, tuple->value->cstring, tuple->length);
			progress = 100;
			headlines_reload_data_and_mark_dirty();
			break;
		}
		case KEY_METHOD_SIZE:
			free_safe(headlines);
			tuple = dict_find(iter, APP_KEY_INDEX);
			if (!tuple) break;
			num_headlines = tuple->value->uint8;
			progress = (num_headlines == 0 ? 100 : 0);
			headlines = malloc(sizeof(Headline) * num_headlines);
			if (headlines == NULL) num_headlines = 0;
			break;
		case KEY_METHOD_DATA: {
			if (!headlines_count()) break;
			tuple = dict_find(iter, APP_KEY_INDEX);
			if (!tuple) break;
			uint8_t index = tuple->value->uint8;
			Headline *headline = headlines_get(index);
			headline->index = index;
			tuple = dict_find(iter, APP_KEY_TITLE);
			if (tuple) {
				strncpy(headline->title, tuple->value->cstring, sizeof(headline->title) - 1);
			}
			progress = ((int)(index + 1) * 100) / num_headlines;
			LOG("headline: %d '%s'", headline->index, headline->title);
			headlines_reload_data_and_mark_dirty();
			break;
		}
	}
}

void headlines_reload_data_and_mark_dirty() {
	win_headlines_reload_data_and_mark_dirty();
}

uint8_t headlines_progress() {
	return progress;
}

uint8_t headlines_count() {
	return num_headlines;
}

char* headlines_get_error() {
	return (error == NULL && !headlines_count()) ? "Loading..." : error;
}

Headline* headlines_get(uint8_t index) {
	return (index < headlines_count()) ? &headlines[index] : NULL;
}

Headline* headlines_get_current() {
	return &headlines[current_headline];
}

uint8_t headlines_get_current_index() {
	return current_headline;
}

void headlines_set_current(uint8_t index) {
	current_headline = index;
}

void headlines_request() {
	for (int i = 0; i < num_headlines; i++) {
		memset(headlines_get(i)->title, 0x0, sizeof(headlines_get(i)->title));
	}
	num_headlines = 0;
	free_safe(headlines);
	free_safe(error);
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	dict_write_uint8(iter, APP_KEY_METHOD, KEY_METHOD_REQUESTHEADLINES);
	dict_write_uint8(iter, APP_KEY_INDEX, subscriptions_get_current_index());
	dict_write_end(iter);
	app_message_outbox_send();
	headlines_reload_data_and_mark_dirty();
}

