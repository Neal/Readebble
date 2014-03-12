#include <pebble.h>
#include "headlines.h"
#include "../libs/pebble-assist.h"
#include "../common.h"
#include "../settings.h"
#include "news.h"

#define MAX_HEADLINES 30

static headline_t headlines[MAX_HEADLINES];
static subscription_t subscription;
static int num_headlines;
static bool out_failed;
static bool can_receive;

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

void headlines_init(subscription_t s) {
	subscription = s;

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

	refresh_list();
}

void headlines_destroy(void) {
	can_receive = false;
	news_destroy();
	menu_layer_destroy_safe(menu_layer);
	window_destroy_safe(window);
}

void headlines_in_received_handler(DictionaryIterator *iter) {
	Tuple *index_tuple = dict_find(iter, KEY_INDEX);
	Tuple *title_tuple = dict_find(iter, KEY_TITLE);

	if (index_tuple && title_tuple && can_receive) {
		if (index_tuple->value->int16 == 0) num_headlines = 0;
		out_failed = false;
		headline_t headline;
		headline.index = index_tuple->value->int16;
		strncpy(headline.title, title_tuple->value->cstring, sizeof(headline.title) - 1);
		headlines[headline.index] = headline;
		num_headlines++;
		menu_layer_reload_data_and_mark_dirty(menu_layer);
	}
	else if (index_tuple) {
		can_receive = true;
	}
	else if (!can_receive) {
		app_message_outbox_send();
	}
}

void headlines_out_sent_handler(DictionaryIterator *sent) {
}

void headlines_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason) {
	out_failed = true;
	menu_layer_reload_data_and_mark_dirty(menu_layer);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static void refresh_list() {
	memset(headlines, 0x0, sizeof(headlines));
	num_headlines = 0;
	out_failed = false;
	can_receive = false;
	menu_layer_set_selected_index(menu_layer, (MenuIndex) { .row = 0, .section = 0 }, MenuRowAlignBottom, false);
	menu_layer_reload_data_and_mark_dirty(menu_layer);
	request_data();
}

static void request_data() {
	Tuplet headline_tuple = TupletInteger(KEY_HEADLINE, 1);
	Tuplet index_tuple = TupletInteger(KEY_INDEX, subscription.index);
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	if (iter == NULL)
		return;
	dict_write_tuplet(iter, &headline_tuple);
	dict_write_tuplet(iter, &index_tuple);
	dict_write_end(iter);
	app_message_outbox_send();
}

static uint16_t menu_get_num_sections_callback(struct MenuLayer *menu_layer, void *callback_context) {
	return 1;
}

static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	return (num_headlines) ? num_headlines : 1;
}

static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	if (num_headlines != 0) {
		return graphics_text_layout_get_content_size(headlines[cell_index->row].title, fonts_get_system_font(settings()->headlines_font_size ? FONT_KEY_GOTHIC_24 : FONT_KEY_GOTHIC_18), (GRect) { .origin = { 2, 0 }, .size = { PEBBLE_WIDTH - 4, 128 } }, GTextOverflowModeFill, GTextAlignmentLeft).h + (settings()->headlines_font_size ? 10 : 6);
	}
	return MENU_CELL_BASIC_CELL_HEIGHT;
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context) {
	menu_cell_basic_header_draw(ctx, cell_layer, subscription.title);
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
	if (out_failed) {
		graphics_context_set_text_color(ctx, GColorBlack);
		graphics_draw_text(ctx, "Phone unreachable!", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), (GRect) { .origin = { 2, 8 }, .size = { PEBBLE_WIDTH - 4, 128 } }, GTextOverflowModeFill, GTextAlignmentLeft, NULL);
	} else if (num_headlines == 0) {
		graphics_context_set_text_color(ctx, GColorBlack);
		graphics_draw_text(ctx, "Loading headlines...", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), (GRect) { .origin = { 2, 8 }, .size = { PEBBLE_WIDTH - 4, 128 } }, GTextOverflowModeFill, GTextAlignmentLeft, NULL);
	} else {
		graphics_context_set_text_color(ctx, GColorBlack);
		graphics_draw_text(ctx, headlines[cell_index->row].title, fonts_get_system_font(settings()->headlines_font_size ? FONT_KEY_GOTHIC_24 : FONT_KEY_GOTHIC_18), (GRect) { .origin = { 2, 0 }, .size = { PEBBLE_WIDTH - 4, 128 } }, GTextOverflowModeFill, GTextAlignmentLeft, NULL);
	}
}

static void menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	if (num_headlines > 0) {
		can_receive = false;
		news_init(headlines[cell_index->row]);
	}
}

static void menu_select_long_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	refresh_list();
}
