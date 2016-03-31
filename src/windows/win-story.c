#include <pebble.h>
#include "win-story.h"
#include "libs/pebble-assist.h"
#include "settings.h"
#include "story.h"
#include "headlines.h"
#include "subscriptions.h"
#include "rss.h"
#include "layers/progress_bar.h"

#define TITLE_LAYER_PAD (8)
#define STORY_LAYER_PAD (8)

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context);
static void click_config_provider(void *context);
static void window_load(Window *window);
static void window_unload(Window *window);

static Window *window = NULL;
static ScrollLayer *scroll_layer = NULL;
static TextLayer *title_text_layer = NULL;
static TextLayer *story_text_layer = NULL;
static ProgressBarLayer *progress_bar_layer = NULL;
static void window_set_colors(Window * window);
static void window_appear(Window *window);

void win_story_init(void) {
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
		.appear = window_appear,
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

	GRect window_bounds = layer_get_bounds(window_get_root_layer(window));
	GRect title_layer_bounds = layer_get_bounds(text_layer_get_layer(title_text_layer));

	text_layer_set_text(story_text_layer, story_get());

	GSize story_layer_size = window_bounds.size;
	story_layer_size.h *= 10;
	text_layer_set_size(story_text_layer, story_layer_size);

	story_layer_size = text_layer_get_content_size(story_text_layer);
	story_layer_size.w = window_bounds.size.w;
	story_layer_size.h += STORY_LAYER_PAD;
	text_layer_set_size(story_text_layer, story_layer_size);

	scroll_layer_set_content_size(scroll_layer, GSize(window_bounds.size.w, story_layer_size.h + title_layer_bounds.size.h));
	scroll_layer_set_paging(scroll_layer, true);

	progress_bar_layer_set_progress(progress_bar_layer, 100);

	scroll_layer_mark_dirty(scroll_layer);
}

void win_story_update_progress(void) {
	if (!window_is_loaded(window)) return;
	progress_bar_layer_set_progress(progress_bar_layer, story_progress());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
	rss_add_to_pocket();
}

static void click_config_provider(void *context) {
	window_long_click_subscribe(BUTTON_ID_SELECT, 500, select_long_click_handler, NULL);
}

static void window_appear(Window *window) {
	window_set_colors(window);
}

static void window_load(Window *window) {
	GRect window_bounds = layer_get_bounds(window_get_root_layer(window));

	story_text_layer = text_layer_create(window_bounds);
	text_layer_set_font(story_text_layer, settings_get_story_body_font_size());
	text_layer_set_text_alignment(story_text_layer, GTextAlignmentCenter);
	text_layer_set_text(story_text_layer, "Loading...");


	title_text_layer = text_layer_create(window_bounds);
	text_layer_set_font(title_text_layer, settings_get_story_title_font_size());
	text_layer_set_text_alignment(title_text_layer, GTextAlignmentCenter);
	text_layer_set_text(title_text_layer, headlines_get_current()->title);


	scroll_layer = scroll_layer_create_fullscreeen(window);
	scroll_layer_set_click_config_onto_window(scroll_layer, window);
	scroll_layer_set_callbacks(scroll_layer, (ScrollLayerCallbacks) {
		.click_config_provider = click_config_provider,
	});
	scroll_layer_add_to_window(scroll_layer, window);

	scroll_layer_add_child(scroll_layer, text_layer_get_layer(story_text_layer));
	scroll_layer_add_child(scroll_layer, text_layer_get_layer(title_text_layer));

	text_layer_enable_screen_text_flow_and_paging(story_text_layer, 2);
	text_layer_enable_screen_text_flow_and_paging(title_text_layer, 2);

	GSize title_layer_size = window_bounds.size;
	title_layer_size.h *= 10;
	text_layer_set_size(title_text_layer, title_layer_size);
	title_layer_size = text_layer_get_content_size(title_text_layer);
	title_layer_size.w = window_bounds.size.w;
	title_layer_size.h += TITLE_LAYER_PAD;
	text_layer_set_size(title_text_layer, title_layer_size);


	GSize story_layer_size = window_bounds.size;
	story_layer_size.h *= 10;
	text_layer_set_size(story_text_layer, story_layer_size);
	story_layer_size = text_layer_get_content_size(story_text_layer);
	story_layer_size.w = window_bounds.size.w;
	story_layer_size.h += STORY_LAYER_PAD;
	text_layer_set_size(story_text_layer, story_layer_size);

	GRect story_layer_frame = layer_get_frame(text_layer_get_layer(story_text_layer));
	story_layer_frame.origin.y = title_layer_size.h;
	layer_set_frame((Layer*)story_text_layer, story_layer_frame);
	text_layer_enable_screen_text_flow_and_paging(story_text_layer, 2);

	GSize scroll_layer_size = window_bounds.size;
	scroll_layer_size.h = title_layer_size.h + story_layer_size.h;
	scroll_layer_set_content_size(scroll_layer, scroll_layer_size);

	scroll_layer_set_paging(scroll_layer, true);

	// Set up the progress layer
	progress_bar_layer = progress_bar_layer_create_fullscreen(window);
	progress_bar_layer_set_pos(progress_bar_layer, title_layer_size.h);
	progress_bar_layer_set_progress(progress_bar_layer, 0);
	scroll_layer_add_child(scroll_layer, progress_bar_layer_get_layer(progress_bar_layer));
}

static void window_unload(Window *window) {
	text_layer_destroy_safe(title_text_layer);
	text_layer_destroy_safe(story_text_layer);
	scroll_layer_destroy_safe(scroll_layer);

	// Destroy the progress bar
	progress_bar_layer_destroy(progress_bar_layer);
}

static void window_set_colors(Window * window) {
	window_set_background_color(window, settings_get_colors()->main_background);
	text_layer_set_colors(story_text_layer, settings_get_colors()->main_text, settings_get_colors()->main_background);
	text_layer_set_colors(title_text_layer, settings_get_colors()->title_bar_text, settings_get_colors()->title_bar_background);
	progress_bar_layer_set_colors(progress_bar_layer, settings_get_colors()->progress_bar_complete, settings_get_colors()->progress_bar_remaining);
}

