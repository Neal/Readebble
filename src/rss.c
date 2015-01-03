#include <pebble.h>
#include "rss.h"
#include "libs/pebble-assist.h"
#include "generated/appinfo.h"
#include "generated/keys.h"
#include "settings.h"
#include "subscriptions.h"
#include "headlines.h"
#include "story.h"

static void in_received_handler(DictionaryIterator *iter, void *context);
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context);
static void timer_callback(void *data);
static AppTimer *timer = NULL;

static char *error = NULL;

void rss_init(void) {
	app_message_register_inbox_received(in_received_handler);
	app_message_register_outbox_failed(out_failed_handler);
	app_message_open_max();

	timer = app_timer_register(1000, timer_callback, NULL);

	settings_init();
	subscriptions_init();
	headlines_init();
	story_init();
}

void rss_deinit(void) {
	free_safe(error);
	story_deinit();
	headlines_deinit();
	subscriptions_deinit();
	settings_deinit();
}

void rss_request_subscriptions() {
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	dict_write_uint8(iter, APP_KEY_METHOD, KEY_METHOD_REQUESTSUBSCRIPTIONS);
	dict_write_end(iter);
	app_message_outbox_send();
	rss_reload_data_and_mark_dirty();
}

void rss_add_to_pocket() {
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	dict_write_uint8(iter, APP_KEY_METHOD, KEY_METHOD_ADDTOPOCKET);
	dict_write_uint8(iter, APP_KEY_INDEX, headlines_get_current_index());
	dict_write_end(iter);
	app_message_outbox_send();
	rss_reload_data_and_mark_dirty();
	vibes_enqueue_custom_pattern((VibePattern) { .durations = (uint32_t []) { 100 }, .num_segments = 1 });
}

void rss_reload_data_and_mark_dirty() {
	subscriptions_reload_data_and_mark_dirty();
	headlines_reload_data_and_mark_dirty();
}

char* rss_get_error() {
	return error;
}

static void in_received_handler(DictionaryIterator *iter, void *context) {
	Tuple *tuple = dict_find(iter, APP_KEY_TYPE);
	if (!tuple) return;
	free_safe(error);
	switch (tuple->value->uint8) {
		case KEY_TYPE_ERROR: {
			tuple = dict_find(iter, APP_KEY_TITLE);
			if (!tuple) break;
			error = malloc(tuple->length);
			strncpy(error, tuple->value->cstring, tuple->length);
			rss_reload_data_and_mark_dirty();
			break;
		}
		case KEY_TYPE_SUBSCRIPTION:
			subscriptions_in_received_handler(iter);
			break;
		case KEY_TYPE_HEADLINE:
			headlines_in_received_handler(iter);
			break;
		case KEY_TYPE_STORY:
			story_in_received_handler(iter);
			break;
	}
}

static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
	free_safe(error);
	error = malloc(sizeof(char) * 56);
	strncpy(error, "Phone unreachable! Make sure the Pebble app is running.", 55);
	rss_reload_data_and_mark_dirty();
}

static void timer_callback(void *data) {
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	dict_write_uint8(iter, APP_KEY_METHOD, KEY_METHOD_BUFFERSIZE);
	dict_write_uint32(iter, APP_KEY_INDEX, app_message_inbox_size_maximum());
	dict_write_end(iter);
	app_message_outbox_send();
}
