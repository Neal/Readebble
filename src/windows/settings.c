#include <pebble.h>
#include "settings.h"
#include "../libs/pebble-assist.h"
#include "../common.h"
#include "../settings.h"

#define MENU_NUM_SECTIONS 2
#define MENU_SECTION_HEADLINES 0
#define MENU_SECTION_SUMMARY 1

#define MENU_SECTION_ROWS_HEADLINES 1
#define MENU_SECTION_ROWS_SUMMARY 2

#define MENU_ROW_HEADLINES_SIZE 0
#define MENU_ROW_SUMMARY_SIZE 0
#define MENU_ROW_SUMMARY_COLOR 1

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

void settings_init(void) {
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

void settings_destroy(void) {
	menu_layer_destroy_safe(menu_layer);
	window_destroy_safe(window);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static uint16_t menu_get_num_sections_callback(struct MenuLayer *menu_layer, void *callback_context) {
	return MENU_NUM_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	switch (section_index) {
		case MENU_SECTION_HEADLINES:
			return MENU_SECTION_ROWS_HEADLINES;
		case MENU_SECTION_SUMMARY:
			return MENU_SECTION_ROWS_SUMMARY;
		default:
			return 0;
	}
}

static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	return 38;
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context) {
	switch (section_index) {
		case MENU_SECTION_HEADLINES:
			menu_cell_basic_header_draw(ctx, cell_layer, "Headlines");
			break;
		case MENU_SECTION_SUMMARY:
			menu_cell_basic_header_draw(ctx, cell_layer, "Summary");
			break;
	}
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
	char label[12] = "";
	char value[6] = "";
	switch (cell_index->section) {
		case MENU_SECTION_HEADLINES:
			switch (cell_index->row) {
				case MENU_ROW_HEADLINES_SIZE:
					strcpy(label, "Font Size");
					strcpy(value, settings()->headlines_font_size ? "Large": "Small");
					break;
			}
			break;
		case MENU_SECTION_SUMMARY:
			switch (cell_index->row) {
				case MENU_ROW_SUMMARY_SIZE:
					strcpy(label, "Font Size");
					strcpy(value, settings()->summary_font_size ? "Large": "Small");
					break;
				case MENU_ROW_SUMMARY_COLOR:
					strcpy(label, "Font Color");
					strcpy(value, settings()->summary_font_color ? "White": "Black");
					break;
			}
			break;
	}
	graphics_context_set_text_color(ctx, GColorBlack);
	graphics_draw_text(ctx, label, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(4, 2, 136, 28), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
	if (strlen(value) > 0) {
		graphics_draw_text(ctx, value, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(4, 7, 134, 24), GTextOverflowModeTrailingEllipsis, GTextAlignmentRight, NULL);
	}
}

static void menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	switch (cell_index->section) {
		case MENU_SECTION_HEADLINES:
			switch (cell_index->row) {
				case MENU_ROW_HEADLINES_SIZE:
					settings()->headlines_font_size = ! settings()->headlines_font_size;
					break;
			}
			break;
		case MENU_SECTION_SUMMARY:
			switch (cell_index->row) {
				case MENU_ROW_SUMMARY_SIZE:
					settings()->summary_font_size = ! settings()->summary_font_size;
					break;
				case MENU_ROW_SUMMARY_COLOR:
					settings()->summary_font_color = ! settings()->summary_font_color;
					break;
			}
			break;
	}
	menu_layer_reload_data(menu_layer);
}

static void menu_select_long_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
}
