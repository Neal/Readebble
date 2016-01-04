#include <pebble.h>
#include "win-headlines.h"
#include "libs/pebble-assist.h"
#include "settings.h"
#include "headlines.h"
#include "subscriptions.h"
#include "rss.h"
#include "win-story.h"
#include "layers/progress_bar.h"

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

static Window *window = NULL;
static MenuLayer *menu_layer = NULL;
static ProgressBarLayer *s_progress_bar_layer;
#ifdef PBL_SDK_3
static TextLayer *s_status_bar;
#endif

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
	return graphics_text_layout_get_content_size(headlines_get(cell_index->row)->title, fonts_get_system_font(settings()->headlines_font_size ? FONT_KEY_GOTHIC_24 : FONT_KEY_GOTHIC_18), bounds, GTextOverflowModeFill, GTextAlignmentLeft).h + (settings()->headlines_font_size ? 10 : 8);
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context) {
	//menu_cell_basic_header_draw(ctx, cell_layer, subscriptions_get_current()->title);
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
	GRect bounds = layer_get_bounds(menu_layer_get_layer(menu_layer));
	GRECT_EDGE_TRIM (bounds, TEXT_BORDER_INSET)
	graphics_context_set_text_color(ctx, GColorBlack);
	if (headlines_get_error()) {
		graphics_draw_text(ctx, headlines_get_error(), fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), bounds, GTextOverflowModeFill, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft), NULL);
	} else {
		graphics_draw_text(ctx, headlines_get(cell_index->row)->title, fonts_get_system_font(settings()->headlines_font_size ? FONT_KEY_GOTHIC_24 : FONT_KEY_GOTHIC_18), bounds, GTextOverflowModeFill, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft), NULL);
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
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

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

#define STATUS_BAR_SIZE_MAX (STATUS_BAR_LAYER_HEIGHT + 12)

#ifdef PBL_SDK_3
	// Set up the status bar if it's needed
	GRect bar_bounds = bounds;
	bar_bounds.size.h = STATUS_BAR_LAYER_HEIGHT + 12;
	s_status_bar = text_layer_create(bar_bounds);
	text_layer_set_text_alignment(s_status_bar, GTextAlignmentCenter);
	text_layer_set_colors(s_status_bar, GColorBlack, GColorPastelYellow);
	text_layer_set_text(s_status_bar, subscriptions_get_current()->title);
	layer_add_child(window_layer, text_layer_get_layer(s_status_bar));
	text_layer_enable_screen_text_flow_and_paging(s_status_bar, 2);
	GSize size = text_layer_get_content_size(s_status_bar);
	size.h += 4;
	size.w = bounds.size.w;
	if (size.h > STATUS_BAR_SIZE_MAX) {
		size.h = STATUS_BAR_SIZE_MAX;
	}
	text_layer_set_size(s_status_bar, size);
#endif

#ifdef PBL_COLOR
	menu_layer_set_normal_colors(menu_layer, GColorWhite, GColorBlack);
	menu_layer_set_highlight_colors(menu_layer, GColorOrange, GColorWhite);
#endif

	// Set up the progress layer
	s_progress_bar_layer = progress_bar_layer_create_fullscreen(window);

	progress_bar_layer_set_pos(s_progress_bar_layer, STATUS_BAR_LAYER_HEIGHT);
}

static void window_unload(Window *window) {
	menu_layer_destroy_safe(menu_layer);

#ifdef PBL_SDK_3
	// Destroy the status bar if there is one
	text_layer_destroy(s_status_bar);
#endif

	// Destroy the progress bar
	progress_bar_layer_destroy(s_progress_bar_layer);
}

