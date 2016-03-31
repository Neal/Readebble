#include <pebble.h>
#include "subscriptions.h"
#include "libs/pebble-assist.h"
#include "generated/appinfo.h"
#include "generated/keys.h"
#include "windows/win-subscriptions.h"

static Subscription *subscriptions = NULL;

static uint8_t num_subscriptions = 0;
static uint8_t current_subscription = 0;

static char *error = NULL;

void subscriptions_init(void) {
	win_subscriptions_init();
}

void subscriptions_deinit(void) {
	free_safe(error);
	free_safe(subscriptions);
	win_subscriptions_deinit();
}

void subscriptions_in_received_handler(DictionaryIterator *iter) {
	Tuple *tuple = dict_find(iter, APP_KEY_METHOD);
	if (!tuple) return;
	free_safe(error);
	switch (tuple->value->uint8) {
		case KEY_METHOD_ERROR: {
			tuple = dict_find(iter, APP_KEY_TITLE);
			if (!tuple) break;
			error = malloc(tuple->length);
			strncpy(error, tuple->value->cstring, tuple->length);
			subscriptions_reload_data_and_mark_dirty();
			break;
		}
		case KEY_METHOD_SIZE:
			free_safe(subscriptions);
			tuple = dict_find(iter, APP_KEY_INDEX);
			if (!tuple) break;
			num_subscriptions = tuple->value->uint8;
			subscriptions = malloc(sizeof(Subscription) * num_subscriptions);
			if (subscriptions == NULL) num_subscriptions = 0;
			break;
		case KEY_METHOD_DATA: {
			if (!subscriptions_count()) break;
			tuple = dict_find(iter, APP_KEY_INDEX);
			if (!tuple) break;
			uint8_t index = tuple->value->uint8;
			Subscription *subscription = subscriptions_get(index);
			subscription->index = index;
			tuple = dict_find(iter, APP_KEY_TITLE);
			if (tuple) {
				strncpy(subscription->title, tuple->value->cstring, sizeof(subscription->title) - 1);
			}
			//LOG("subscription: %d '%s'", subscription->index, subscription->title);
			subscriptions_reload_data_and_mark_dirty();
			break;
		}
	}
}

void subscriptions_reload_data_and_mark_dirty() {
	win_subscriptions_reload_data_and_mark_dirty();
}

void subscriptions_reset() {
	num_subscriptions = 0;
}

uint8_t subscriptions_count() {
	return num_subscriptions;
}

char* subscriptions_get_error() {
	return (error == NULL && !subscriptions_count()) ? "Loading..." : error;
}

Subscription* subscriptions_get(uint8_t index) {
	return (index < subscriptions_count()) ? &subscriptions[index] : NULL;
}

Subscription* subscriptions_get_current() {
	return &subscriptions[current_subscription];
}

uint8_t subscriptions_get_current_index() {
	return current_subscription;
}

void subscriptions_set_current(uint8_t index) {
	current_subscription = index;
}
