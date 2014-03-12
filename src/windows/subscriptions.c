#include <pebble.h>
#include "subscriptions.h"
#include "../libs/pebble-assist.h"
#include "../common.h"
#include "headlines.h"
#include "settings.h"

#define MAX_SUBSCRIPTIONS 30

static subscription_t subscriptions[MAX_SUBSCRIPTIONS];

static int num_subscriptions = 0;
static bool no_subscriptions = false;
static bool out_failed = false;

static void refresh_list();
static void request_data();
static uint16_t menu_get_num_sections_callback(struct MenuLayer *menu_layer, void *callback_context);
static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context);
static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context);
static void menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void menu_select_long_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);

static Window *window;
static MenuLayer *menu_layer;

void subscriptions_init(void) {
	window = window_create();

	menu_layer = menu_layer_create_fullscreen(window);
	menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks) {
		.get_num_sections = menu_get_num_sections_callback,
		.get_num_rows = menu_get_num_rows_callback,
		.get_header_height = menu_get_header_height_callback,
		.get_cell_height = menu_get_cell_height_callback,
		.draw_header = menu_draw_header_callback,
		.draw_row = menu_draw_row_callback,
		.select_click = menu_select_callback,
		.select_long_click = menu_select_long_callback,
	});
	menu_layer_set_click_config_onto_window(menu_layer, window);
	menu_layer_add_to_window(menu_layer, window);

	window_stack_push(window, true);
}

void subscriptions_destroy(void) {
	headlines_destroy();
	settings_destroy();
	menu_layer_destroy_safe(menu_layer);
	window_destroy_safe(window);
}

void subscriptions_in_received_handler(DictionaryIterator *iter) {
	Tuple *index_tuple = dict_find(iter, KEY_INDEX);
	Tuple *title_tuple = dict_find(iter, KEY_TITLE);

	if (index_tuple && title_tuple) {
		if (index_tuple->value->int16 == 0) {
			num_subscriptions = 0;
			no_subscriptions = false;
		}
		subscription_t subscription;
		subscription.index = index_tuple->value->int16;
		strncpy(subscription.title, title_tuple->value->cstring, sizeof(subscription.title) - 1);
		subscriptions[subscription.index] = subscription;
		num_subscriptions++;
		menu_layer_reload_data_and_mark_dirty(menu_layer);
	}
	else if (index_tuple) {
		no_subscriptions = true;
		menu_layer_reload_data_and_mark_dirty(menu_layer);
	}
}

void subscriptions_out_sent_handler(DictionaryIterator *sent) {
}

void subscriptions_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason) {
	out_failed = true;
	menu_layer_reload_data_and_mark_dirty(menu_layer);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static void refresh_list() {
	memset(subscriptions, 0x0, sizeof(subscriptions));
	num_subscriptions = 0;
	no_subscriptions = false;
	out_failed = false;
	menu_layer_set_selected_index(menu_layer, (MenuIndex) { .row = 0, .section = 0 }, MenuRowAlignBottom, false);
	menu_layer_reload_data_and_mark_dirty(menu_layer);
	request_data();
}

static void request_data() {
	Tuplet subscription_tuple = TupletInteger(KEY_SUBSCRIPTION, 1);
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	if (iter == NULL)
		return;
	dict_write_tuplet(iter, &subscription_tuple);
	dict_write_end(iter);
	app_message_outbox_send();
}

static uint16_t menu_get_num_sections_callback(struct MenuLayer *menu_layer, void *callback_context) {
	return 2;
}

static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	if (section_index == 1) return 1;
	return (num_subscriptions) ? num_subscriptions : 1;
}

static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	return MENU_CELL_BASIC_CELL_HEIGHT;
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context) {
	switch (section_index) {
		case 0:
			menu_cell_basic_header_draw(ctx, cell_layer, "Readebble");
			break;
		case 1:
			menu_cell_basic_header_draw(ctx, cell_layer, "Other");
			break;
	}
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
	if (cell_index->section == 1) {
		menu_cell_basic_draw(ctx, cell_layer, "Settings", NULL, NULL);
	} else if (out_failed) {
		graphics_context_set_text_color(ctx, GColorBlack);
		graphics_draw_text(ctx, "Phone unreachable!", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), (GRect) { .origin = { 2, 8 }, .size = { PEBBLE_WIDTH - 4, 128 } }, GTextOverflowModeFill, GTextAlignmentLeft, NULL);
	} else if (no_subscriptions) {
		graphics_context_set_text_color(ctx, GColorBlack);
		graphics_draw_text(ctx, "No subscriptions added.", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), (GRect) { .origin = { 2, 0 }, .size = { PEBBLE_WIDTH - 4, 128 } }, GTextOverflowModeFill, GTextAlignmentLeft, NULL);
	} else if (num_subscriptions == 0) {
		graphics_context_set_text_color(ctx, GColorBlack);
		graphics_draw_text(ctx, "Loading subscriptions...", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), (GRect) { .origin = { 2, 0 }, .size = { PEBBLE_WIDTH - 4, 128 } }, GTextOverflowModeFill, GTextAlignmentLeft, NULL);
	} else {
		menu_cell_basic_draw(ctx, cell_layer, subscriptions[cell_index->row].title, NULL, NULL);
	}
}

static void menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	if (cell_index->section == 1) {
		settings_init();
	}
	else if (num_subscriptions > 0) {
		headlines_destroy();
		headlines_init(subscriptions[cell_index->row]);
	}
}

static void menu_select_long_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	refresh_list();
}
