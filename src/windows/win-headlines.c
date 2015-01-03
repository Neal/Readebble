#include <pebble.h>
#include "win-headlines.h"
#include "libs/pebble-assist.h"
#include "settings.h"
#include "headlines.h"
#include "subscriptions.h"
#include "rss.h"
#include "win-story.h"

static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context);
static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context);
static void menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void menu_select_long_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void window_load(Window *window);
static void window_unload(Window *window);

static Window *window = NULL;
static MenuLayer *menu_layer = NULL;

void win_headlines_init(void) {
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
}

void win_headlines_push(void) {
	window_stack_push(window, true);
	headlines_request();
}

void win_headlines_deinit(void) {
	window_destroy_safe(window);
}

void win_headlines_reload_data_and_mark_dirty(void) {
	if (!window_is_loaded(window)) return;
	menu_layer_reload_data_and_mark_dirty(menu_layer);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	return headlines_count() ? headlines_count() : 1;
}

static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	if (headlines_get_error()) {
		return graphics_text_layout_get_content_size(headlines_get_error(), fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(4, 2, 136, 128), GTextOverflowModeFill, GTextAlignmentLeft).h + 12;
	}
	return graphics_text_layout_get_content_size(headlines_get(cell_index->row)->title, fonts_get_system_font(settings()->headlines_font_size ? FONT_KEY_GOTHIC_24 : FONT_KEY_GOTHIC_18), GRect(2, 0, 140, 128), GTextOverflowModeFill, GTextAlignmentLeft).h + (settings()->headlines_font_size ? 10 : 8);
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context) {
	menu_cell_basic_header_draw(ctx, cell_layer, subscriptions_get_current()->title);
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
	graphics_context_set_text_color(ctx, GColorBlack);
	if (headlines_get_error()) {
		graphics_draw_text(ctx, headlines_get_error(), fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(4, 2, 136, 128), GTextOverflowModeFill, GTextAlignmentLeft, NULL);
	} else {
		graphics_draw_text(ctx, headlines_get(cell_index->row)->title, fonts_get_system_font(settings()->headlines_font_size ? FONT_KEY_GOTHIC_24 : FONT_KEY_GOTHIC_18), GRect(2, 0, 140, 128), GTextOverflowModeFill, GTextAlignmentLeft, NULL);
	}
}

static void menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	if (!headlines_count()) return;
	headlines_set_current(cell_index->row);
	win_story_push();
}

static void menu_select_long_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	if (!headlines_count()) return;
	headlines_set_current(cell_index->row);
	rss_add_to_pocket();
}

static void window_load(Window *window) {
	menu_layer = menu_layer_create_fullscreen(window);
	menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks) {
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
}

static void window_unload(Window *window) {
	menu_layer_destroy_safe(menu_layer);
}
