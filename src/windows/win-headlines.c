#include <pebble.h>
#include "win-headlines.h"
#include "libs/pebble-assist.h"
#include "settings.h"
#include "headlines.h"
#include "subscriptions.h"
#include "rss.h"
#include "win-story.h"
#include "layers/progress_bar.h"
#include "layers/title_bar.h"

#define TEXT_BORDER_INSET (16)

#define GRECT_EDGE_TRIM(RECT, AMOUNT) RECT.origin.x += AMOUNT; RECT.size.w -= (AMOUNT * 2);

static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context);
static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context);
static void menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void menu_select_long_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void window_load(Window *window);
static void window_unload(Window *window);
static void window_set_colors(Window * window);
static void window_appear(Window *window);

static Window *window = NULL;
static MenuLayer *menu_layer = NULL;
static ProgressBarLayer *s_progress_bar_layer;
static int16_t s_title_bar_height;
static TitleBarLayer *s_status_bar;

void win_headlines_init(void) {
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
		.appear = window_appear,
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
	progress_bar_layer_set_progress(s_progress_bar_layer, headlines_progress());
	menu_layer_reload_data_and_mark_dirty(menu_layer);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	return headlines_count() ? headlines_count() : 1;
}

static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	return 0;//MENU_CELL_BASIC_HEADER_HEIGHT;
}

static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	GRect bounds = layer_get_bounds(menu_layer_get_layer(menu_layer));
	GRECT_EDGE_TRIM (bounds, TEXT_BORDER_INSET)
	if (headlines_get_error()) {
		return graphics_text_layout_get_content_size(headlines_get_error(), fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), bounds, GTextOverflowModeFill, GTextAlignmentLeft).h + 12;
	}
	return graphics_text_layout_get_content_size(headlines_get(cell_index->row)->title, settings_get_headlines_font_size(), bounds, GTextOverflowModeFill, GTextAlignmentLeft).h + (settings()->headlines_font_size ? 10 : 8);
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context) {
	//menu_cell_basic_header_draw(ctx, cell_layer, subscriptions_get_current()->title);
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
	GRect bounds = layer_get_bounds(menu_layer_get_layer(menu_layer));
	//bounds.origin.y -= s_title_bar_height;
	GRECT_EDGE_TRIM (bounds, TEXT_BORDER_INSET)
	graphics_context_set_text_color(ctx, (menu_cell_layer_is_highlighted(cell_layer) ? settings_get_colors()->inverted_text : settings_get_colors()->main_text));
	if (headlines_get_error()) {
		graphics_draw_text(ctx, headlines_get_error(), fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), bounds, GTextOverflowModeFill, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft), NULL);
	} else {
		graphics_draw_text(ctx, headlines_get(cell_index->row)->title, settings_get_headlines_font_size(), bounds, GTextOverflowModeFill, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft), NULL);
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

static void window_appear(Window *window) {
	window_set_colors(window);
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

	// Set up the status bar if it's needed
	s_status_bar = title_bar_layer_create_fullscreen(window);
	title_bar_layer_set_text(s_status_bar, subscriptions_get_current()->title);

	title_bar_layer_reduce_height(s_status_bar);

	s_title_bar_height = title_bar_layer_get_height(s_status_bar);

#ifdef PBL_RECT
	GRect menu_bounds = layer_get_frame(menu_layer_get_layer(menu_layer));
	menu_bounds.origin.y += s_title_bar_height;
	menu_bounds.size.h -= s_title_bar_height;
	scroll_layer_set_frame(menu_layer_get_scroll_layer(menu_layer), menu_bounds);
#endif

	// Set up the progress layer
	s_progress_bar_layer = progress_bar_layer_create_fullscreen(window);
	progress_bar_layer_set_pos(s_progress_bar_layer, s_title_bar_height);
}

static void window_unload(Window *window) {
	menu_layer_destroy_safe(menu_layer);
	// Destroy the status bar if there is one
	title_bar_layer_destroy(s_status_bar);
	// Destroy the progress bar
	progress_bar_layer_destroy(s_progress_bar_layer);
}

static void window_set_colors(Window * window) {
	title_bar_layer_set_colors(s_status_bar, settings_get_colors()->title_bar_text, settings_get_colors()->title_bar_background);
	menu_layer_set_normal_colors(menu_layer, settings_get_colors()->main_background, settings_get_colors()->main_text);
	menu_layer_set_highlight_colors(menu_layer, settings_get_colors()->selection_highlight, settings_get_colors()->inverted_text);
	progress_bar_layer_set_colors(s_progress_bar_layer, settings_get_colors()->progress_bar_complete, settings_get_colors()->progress_bar_remaining);
}

