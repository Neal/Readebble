#include <pebble.h>
#include "win-subscriptions.h"
#include "libs/pebble-assist.h"
#include "subscriptions.h"
#include "rss.h"
#include "win-headlines.h"
#include "win-settings.h"
#include "settings.h"

#define MENU_NUM_SECTIONS (1)
#define TEXT_BORDER_INSET (16)

#define GRECT_EDGE_TRIM(RECT, AMOUNT) RECT.origin.x += AMOUNT; RECT.size.w -= (AMOUNT * 2);

static uint16_t menu_get_num_sections_callback(struct MenuLayer *menu_layer, void *callback_context);
static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context);
static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context);
static void menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void menu_select_long_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void window_load(Window *window);
static void window_unload(Window *window);
int menu_item_count();
int menu_item_settings_pos();
static void window_set_colors(Window * window);
static void window_appear(Window *window);

static Window *window = NULL;
static MenuLayer *menu_layer = NULL;
static StatusBarLayer *s_status_bar;
#ifdef PBL_ROUND
static TextLayer *s_lower_bar;
#endif

int menu_item_count() {
	int items;
	items = subscriptions_count() + 1;
	if (items < 2) {
		items = 2;
	}
	return items;
}

int menu_item_settings_pos() {
	int position;
	position = subscriptions_count();
	if (position < 1) {
		position = 1;
	}
	return position;
}

void win_subscriptions_init(void) {
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
		.appear = window_appear,
	});
	window_stack_push(window, true);
}

void win_subscriptions_deinit(void) {
	window_destroy_safe(window);
}

void win_subscriptions_reload_data_and_mark_dirty(void) {
	if (!window_is_loaded(window)) return;
	menu_layer_reload_data_and_mark_dirty(menu_layer);
}




// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static uint16_t menu_get_num_sections_callback(struct MenuLayer *menu_layer, void *callback_context) {
	return MENU_NUM_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	return menu_item_count();
}

static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	return 0;
}

static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	if (cell_index->row == menu_item_settings_pos()) return MENU_CELL_BASIC_CELL_HEIGHT;
	if (subscriptions_get_error()) {
		GRect bounds = layer_get_bounds(menu_layer_get_layer(menu_layer));
		GRECT_EDGE_TRIM (bounds, TEXT_BORDER_INSET)
		return graphics_text_layout_get_content_size(subscriptions_get_error(), fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), bounds, GTextOverflowModeFill, GTextAlignmentLeft).h + 12;
	}
	return MENU_CELL_BASIC_CELL_HEIGHT;
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context) {
	// Do nothing
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
	if (cell_index->row == menu_item_settings_pos()) {
#ifdef PBL_COLOR
		graphics_context_set_text_color(ctx, (menu_cell_layer_is_highlighted(cell_layer) ? settings_get_colors()->settings_inverted_text : settings_get_colors()->settings_text));
#endif
		menu_cell_basic_draw(ctx, cell_layer, "Settings", NULL, NULL);
	} else if (subscriptions_get_error()) {
		GRect bounds = layer_get_bounds(cell_layer);
		GRECT_EDGE_TRIM (bounds, TEXT_BORDER_INSET)
		
		graphics_context_set_text_color(ctx, (menu_cell_layer_is_highlighted(cell_layer) ? settings_get_colors()->inverted_text : settings_get_colors()->main_text));
		
		graphics_draw_text(ctx, subscriptions_get_error(), fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), bounds, GTextOverflowModeFill, PBL_IF_ROUND_ELSE (GTextAlignmentCenter, GTextAlignmentLeft), NULL);
	} else {
		menu_cell_basic_draw(ctx, cell_layer, subscriptions_get(cell_index->row)->title, NULL, NULL);
	}
}

static void menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	if (cell_index->row == menu_item_settings_pos()) {
		return win_settings_push();
	}
	if (!subscriptions_count()) return;
	subscriptions_set_current(cell_index->row);
	win_headlines_push();
}

static void menu_select_long_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	rss_request_subscriptions();
}

static void window_appear(Window *window) {
	window_set_colors(window);
}

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
#ifdef PBL_ROUND
	GRect bounds = layer_get_bounds(window_layer);
#endif

	menu_layer = menu_layer_create_fullscreen(window);
	menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks) {
		.get_num_sections = menu_get_num_sections_callback,
		.get_num_rows = menu_get_num_rows_callback,
		.get_header_height = menu_get_header_height_callback,
		.draw_header = menu_draw_header_callback,
		.get_cell_height = menu_get_cell_height_callback,
		.draw_row = menu_draw_row_callback,
		.select_click = menu_select_callback,
		.select_long_click = menu_select_long_callback,
	});
	menu_layer_set_click_config_onto_window(menu_layer, window);
	menu_layer_add_to_window(menu_layer, window);

	// Set up the status bar if it's needed
	s_status_bar = status_bar_layer_create();
	layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar));

#ifdef PBL_ROUND
	// Set up the lower message bar
	const GEdgeInsets message_insets = {.top = bounds.size.h - STATUS_BAR_LAYER_HEIGHT};
	s_lower_bar = text_layer_create(grect_inset(bounds, message_insets));
	text_layer_set_text_alignment(s_lower_bar, GTextAlignmentCenter);
	text_layer_set_text(s_lower_bar, "");
	layer_add_child(window_layer, text_layer_get_layer(s_lower_bar));
#else
	GRect menu_bounds = layer_get_frame(menu_layer_get_layer(menu_layer));
	menu_bounds.origin.y += STATUS_BAR_LAYER_HEIGHT;
	menu_bounds.size.h -= STATUS_BAR_LAYER_HEIGHT;
	scroll_layer_set_frame(menu_layer_get_scroll_layer(menu_layer), menu_bounds);
#endif
}

static void window_unload(Window *window) {
	menu_layer_destroy_safe(menu_layer);

	// Destroy the status bar if there is one
	status_bar_layer_destroy(s_status_bar);
#ifdef PBL_ROUND
	text_layer_destroy(s_lower_bar);
#endif
}

static void window_set_colors(Window * window) {
	status_bar_layer_set_colors(s_status_bar, settings_get_colors()->main_background, settings_get_colors()->main_text);
	menu_layer_set_normal_colors(menu_layer, settings_get_colors()->main_background, settings_get_colors()->main_text);
	menu_layer_set_highlight_colors(menu_layer, settings_get_colors()->selection_highlight, settings_get_colors()->inverted_text);
#ifdef PBL_ROUND
	text_layer_set_colors(s_lower_bar, settings_get_colors()->main_text, settings_get_colors()->main_background);
#endif
}

