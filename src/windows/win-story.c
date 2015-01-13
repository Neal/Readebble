#include <pebble.h>
#include "win-story.h"
#include "libs/pebble-assist.h"
#include "settings.h"
#include "story.h"
#include "headlines.h"
#include "subscriptions.h"
#include "rss.h"

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context);
static void click_config_provider(void *context);
static void window_load(Window *window);
static void window_unload(Window *window);

static Window *window = NULL;
static ScrollLayer *scroll_layer = NULL;
static TextLayer *title_text_layer = NULL;
static TextLayer *story_text_layer = NULL;

void win_story_init(void) {
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
}

void win_story_push(void) {
	window_stack_push(window, true);
	story_request();
}

void win_story_deinit(void) {
	window_destroy_safe(window);
}

void win_story_reload_data_and_mark_dirty(void) {
	if (!window_is_loaded(window)) return;

	text_layer_set_text(story_text_layer, story_get());

	GSize title_layer_size = text_layer_get_content_size(title_text_layer);
	GSize story_layer_size = text_layer_get_content_size(story_text_layer);

	title_layer_size.h += 8;
	story_layer_size.h += 8;

	text_layer_set_size(title_text_layer, title_layer_size);
	text_layer_set_size(story_text_layer, story_layer_size);

	scroll_layer_set_content_size(scroll_layer, GSize(144, title_layer_size.h + story_layer_size.h));

	scroll_layer_mark_dirty(scroll_layer);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
	rss_add_to_pocket();
}

static void click_config_provider(void *context) {
	window_long_click_subscribe(BUTTON_ID_SELECT, 500, select_long_click_handler, NULL);
}

static void window_load(Window *window) {
	window_set_background_color(window, settings()->story_font_color ? GColorBlack : GColorWhite);

	scroll_layer = scroll_layer_create_fullscreeen(window);
	scroll_layer_set_click_config_onto_window(scroll_layer, window);
	scroll_layer_set_callbacks(scroll_layer, (ScrollLayerCallbacks) {
		.click_config_provider = click_config_provider,
	});
	scroll_layer_add_to_window(scroll_layer, window);

	title_text_layer = text_layer_create(GRect(3, -2, 140, 96));
	text_layer_set_colors(title_text_layer, settings()->story_font_color ? GColorWhite : GColorBlack, GColorClear);
	text_layer_set_font(title_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_text(title_text_layer, headlines_get_current()->title);

	GSize title_layer_size = text_layer_get_content_size(title_text_layer);
	title_layer_size.h += 8;

	text_layer_set_size(title_text_layer, title_layer_size);

	story_text_layer = text_layer_create(GRect(3, title_layer_size.h, 140, 1024));
	text_layer_set_colors(story_text_layer, settings()->story_font_color ? GColorWhite : GColorBlack, GColorClear);
	text_layer_set_font(story_text_layer, fonts_get_system_font(settings()->story_font_size ? FONT_KEY_GOTHIC_24_BOLD : FONT_KEY_GOTHIC_18_BOLD));
	text_layer_set_text(story_text_layer, "Loading...");

	GSize story_layer_size = text_layer_get_content_size(story_text_layer);
	story_layer_size.h = story_layer_size.h + 8;

	text_layer_set_size(story_text_layer, story_layer_size);

	scroll_layer_set_content_size(scroll_layer, GSize(144, title_layer_size.h + story_layer_size.h));

	scroll_layer_add_child(scroll_layer, text_layer_get_layer(title_text_layer));
	scroll_layer_add_child(scroll_layer, text_layer_get_layer(story_text_layer));
}

static void window_unload(Window *window) {
	text_layer_destroy_safe(title_text_layer);
	text_layer_destroy_safe(story_text_layer);
	scroll_layer_destroy_safe(scroll_layer);
}
