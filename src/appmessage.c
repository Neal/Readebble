#include <pebble.h>
#include "appmessage.h"
#include "common.h"
#include "windows/subscriptions.h"
#include "windows/headlines.h"
#include "windows/news.h"

static void in_received_handler(DictionaryIterator *iter, void *context);
static void in_dropped_handler(AppMessageResult reason, void *context);
static void out_sent_handler(DictionaryIterator *sent, void *context);
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context);

void appmessage_init(void) {
	app_message_register_inbox_received(in_received_handler);
	app_message_register_inbox_dropped(in_dropped_handler);
	app_message_register_outbox_sent(out_sent_handler);
	app_message_register_outbox_failed(out_failed_handler);
	app_message_open(512 /* inbound_size */, 32 /* outbound_size */);
}

static void in_received_handler(DictionaryIterator *iter, void *context) {
	if (dict_find(iter, KEY_SUBSCRIPTION)) {
		subscriptions_in_received_handler(iter);
	}
	else if (dict_find(iter, KEY_HEADLINE)) {
		headlines_in_received_handler(iter);
	}
	else if (dict_find(iter, KEY_SUMMARY)) {
		news_in_received_handler(iter);
	}
}

static void in_dropped_handler(AppMessageResult reason, void *context) {
}

static void out_sent_handler(DictionaryIterator *sent, void *context) {
	if (dict_find(sent, KEY_SUBSCRIPTION)) {
		subscriptions_out_sent_handler(sent);
	}
	else if (dict_find(sent, KEY_HEADLINE)) {
		headlines_out_sent_handler(sent);
	}
	else if (dict_find(sent, KEY_SUMMARY)) {
		news_out_sent_handler(sent);
	}
}

static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
	if (dict_find(failed, KEY_SUBSCRIPTION)) {
		subscriptions_out_failed_handler(failed, reason);
	}
	else if (dict_find(failed, KEY_HEADLINE)) {
		headlines_out_failed_handler(failed, reason);
	}
	else if (dict_find(failed, KEY_SUMMARY)) {
		news_out_failed_handler(failed, reason);
	}
}
