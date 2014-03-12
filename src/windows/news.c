#include <pebble.h>
#include "news.h"
#include "../libs/pebble-assist.h"
#include "../common.h"
#include "../settings.h"

static headline_t headline;
static char title[64];
static char summary[2048];

static void request_data();
static void back_single_click_handler(ClickRecognizerRef recognizer, void *context);
static void click_config_provider(void *context);
static void window_refresh();

static Window *window;
static ScrollLayer *scroll_layer;
static TextLayer *title_layer;
static TextLayer *summary_layer;

void news_init(headline_t h) {
	headline = h;

	request_data();

	window = window_create();
	window_set_background_color(window, settings()->summary_font_color ? GColorBlack : GColorWhite);

	scroll_layer = scroll_layer_create(layer_get_bounds(window_get_root_layer(window)));
	scroll_layer_set_click_config_onto_window(scroll_layer, window);
	scroll_layer_set_callbacks(scroll_layer, (ScrollLayerCallbacks) {
		.click_config_provider = click_config_provider,
	});

	window_refresh();

	layer_add_child(window_get_root_layer(window), scroll_layer_get_layer(scroll_layer));

	window_stack_push(window, true);
}

void news_destroy(void) {
	text_layer_destroy_safe(title_layer);
	text_layer_destroy_safe(summary_layer);
	scroll_layer_destroy_safe(scroll_layer);
	window_destroy_safe(window);
}

void news_in_received_handler(DictionaryIterator *iter) {
	Tuple *title_tuple = dict_find(iter, KEY_TITLE);
	Tuple *summary_tuple = dict_find(iter, KEY_SUMMARY);

	if (summary_tuple) {
		if (title_tuple) {
			strncpy(title, title_tuple->value->cstring, sizeof(title) - 1);
			window_refresh();
		} else {
			strcat(summary, summary_tuple->value->cstring);
		}
	}
}

void news_out_sent_handler(DictionaryIterator *sent) {
}

void news_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason) {
	strncpy(title, "Phone unreachable!", sizeof(title) - 1);
	window_refresh();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static void request_data() {
	strncpy(title, "Loading story...", sizeof(title) - 1);
	summary[0] = '\0';
	Tuplet summary_tuple = TupletInteger(KEY_SUMMARY, 1);
	Tuplet index_tuple = TupletInteger(KEY_INDEX, headline.index);
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	if (iter == NULL)
		return;
	dict_write_tuplet(iter, &summary_tuple);
	dict_write_tuplet(iter, &index_tuple);
	dict_write_end(iter);
	app_message_outbox_send();
}

static void back_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	app_message_outbox_send();
	news_destroy();
	window_stack_pop(true);
}

static void click_config_provider(void *context) {
	window_single_click_subscribe(BUTTON_ID_BACK, back_single_click_handler);
}

static void window_refresh() {
	text_layer_destroy_safe(title_layer);
	text_layer_destroy_safe(summary_layer);

	GRect max_text_bounds = GRect(2, 0, PEBBLE_WIDTH - 4, 2000);

	title_layer = text_layer_create(max_text_bounds);
	text_layer_set_font(title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_text(title_layer, title);
	text_layer_set_text_color(title_layer, settings()->summary_font_color ? GColorWhite : GColorBlack);
	text_layer_set_background_color(title_layer, GColorClear);

	GSize title_max_size = text_layer_get_content_size(title_layer);
	text_layer_set_size(title_layer, GSize(title_max_size.w, title_max_size.h + 14));

	summary_layer = text_layer_create(GRect(2, title_max_size.h + 14, max_text_bounds.size.w, max_text_bounds.size.h - (title_max_size.h + 14)));
	text_layer_set_font(summary_layer, fonts_get_system_font(settings()->summary_font_size ? FONT_KEY_GOTHIC_24_BOLD : FONT_KEY_GOTHIC_18_BOLD));
	text_layer_set_text(summary_layer, summary);
	text_layer_set_text_color(summary_layer, settings()->summary_font_color ? GColorWhite : GColorBlack);
	text_layer_set_background_color(summary_layer, GColorClear);

	GSize summary_max_size = text_layer_get_content_size(summary_layer);
	text_layer_set_size(summary_layer, GSize(summary_max_size.w, summary_max_size.h + 14));

	scroll_layer_set_content_size(scroll_layer, GSize(PEBBLE_WIDTH, title_max_size.h + 14 + summary_max_size.h + 8));

	scroll_layer_add_child(scroll_layer, text_layer_get_layer(title_layer));
	scroll_layer_add_child(scroll_layer, text_layer_get_layer(summary_layer));
}
