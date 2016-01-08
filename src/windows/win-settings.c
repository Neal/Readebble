#include <pebble.h>
#include "win-settings.h"
#include "libs/pebble-assist.h"
#include "settings.h"

#define MENU_NUM_SECTIONS 1

enum {
	MENU_ROW_INVALID = -1,
	
	MENU_ROW_HEADLINES_SIZE,
	MENU_ROW_SUMMARY_SIZE,
	MENU_ROW_SUMMARY_COLOR,
	
	MENU_ROW_NUM
} MENU_ROW;

static uint16_t menu_get_num_sections_callback(struct MenuLayer *menu_layer, void *callback_context);
static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context);
static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context);
static void menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void window_load(Window *window);
static void window_unload(Window *window);

static Window *window = NULL;
static MenuLayer *menu_layer = NULL;
#ifdef PBL_SDK_3
static StatusBarLayer *s_status_bar;
static TextLayer *s_lower_bar;
#endif

static char *settings_labels[MENU_ROW_NUM] = {
	"Headline font size", 
	"Story font fize", 
	"Story text color"
};

void win_settings_init(void) {
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
}

void win_settings_push(void) {
	window_stack_push(window, true);
}

void win_settings_deinit(void) {
	window_destroy_safe(window);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static uint16_t menu_get_num_sections_callback(struct MenuLayer *menu_layer, void *callback_context) {
	return MENU_NUM_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	return MENU_ROW_NUM;
}

static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	return 0;
}

static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
#ifdef PBL_RECT
	return 38;
#else
	if (menu_layer_is_index_selected(menu_layer, cell_index)) {
		return MENU_CELL_ROUND_FOCUSED_SHORT_CELL_HEIGHT;
	}
	else {
		return MENU_CELL_ROUND_UNFOCUSED_TALL_CELL_HEIGHT;
	}
#endif
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context) {
	// Do nothing
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
	char value[6] = "";
	switch (cell_index->row) {
		case MENU_ROW_HEADLINES_SIZE:
			strncpy(value, settings()->headlines_font_size ? "Large": "Small", sizeof(value));
			break;
		case MENU_ROW_SUMMARY_SIZE:
			strncpy(value, settings()->story_font_size ? "Large": "Small", sizeof(value));
			break;
		case MENU_ROW_SUMMARY_COLOR:
			strncpy(value, settings()->story_font_color ? "White": "Black", sizeof(value));
			break;
	}
	menu_cell_basic_draw(ctx, cell_layer, settings_labels[cell_index->row], value, NULL);
}

static void menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	switch (cell_index->row) {
		case MENU_ROW_HEADLINES_SIZE:
			settings()->headlines_font_size = !settings()->headlines_font_size;
			break;
		case MENU_ROW_SUMMARY_SIZE:
			settings()->story_font_size = !settings()->story_font_size;
			break;
		case MENU_ROW_SUMMARY_COLOR:
			settings()->story_font_color = !settings()->story_font_color;
			break;
	}
	menu_layer_reload_data(menu_layer);
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
		.get_cell_height = menu_get_cell_height_callback,
		.draw_header = menu_draw_header_callback,
		.draw_row = menu_draw_row_callback,
		.select_click = menu_select_callback,
	});
	menu_layer_set_click_config_onto_window(menu_layer, window);
	menu_layer_add_to_window(menu_layer, window);

	// Set up the status bar if it's needed
	s_status_bar = status_bar_layer_create();
	layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar));
	status_bar_layer_set_colors(s_status_bar, GColorWhite, GColorBlack);

#ifdef PBL_ROUND
	// Set up the lower message bar
	const GEdgeInsets message_insets = {.top = bounds.size.h - STATUS_BAR_LAYER_HEIGHT};
	s_lower_bar = text_layer_create(grect_inset(bounds, message_insets));
	text_layer_set_text_alignment(s_lower_bar, GTextAlignmentCenter);
	text_layer_set_colors(s_lower_bar, GColorBlack, GColorWhite);
	text_layer_set_text(s_lower_bar, "");
	layer_add_child(window_layer, text_layer_get_layer(s_lower_bar));
#else
	GRect menu_bounds = layer_get_bounds(menu_layer_get_layer(menu_layer));
	menu_bounds.origin.y += STATUS_BAR_LAYER_HEIGHT;
	menu_bounds.size.h -= STATUS_BAR_LAYER_HEIGHT * 2;
	layer_set_bounds(menu_layer_get_layer(menu_layer), menu_bounds);
#endif

#ifdef PBL_COLOR
	menu_layer_set_normal_colors(menu_layer, GColorWhite, GColorBlack);
	menu_layer_set_highlight_colors(menu_layer, GColorOrange, GColorWhite);
#endif
}

static void window_unload(Window *window) {
	menu_layer_destroy_safe(menu_layer);

#ifdef PBL_SDK_3
	// Destroy the status bar if there is one
	status_bar_layer_destroy(s_status_bar);
	text_layer_destroy(s_lower_bar);
#endif
}

