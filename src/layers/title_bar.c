#include <pebble.h>
#include "layers/title_bar.h"

typedef struct TitleBarData {
	TitleBarLayerUpdateProc update_proc;
	GColor foreground;
	GColor background;
	TextLayer * text_layer;
} TitleBarData;

static void title_bar_layer_update_proc(TitleBarLayer *title_bar_layer, GContext *ctx);
void title_bar_layer_centre_text_vertically(TitleBarLayer * title_bar_layer);

TitleBarLayer * title_bar_layer_create_fullscreen(Window * window) {
	GRect window_bounds = layer_get_bounds(window_get_root_layer(window));

	GRect layer_bounds = (GRect){
		.origin = GPoint(0, 0),
		.size = GSize(window_bounds.size.w, TITLE_BAR_DEFAULT_HEIGHT)
	};
	
	TitleBarLayer *title_bar_layer = title_bar_layer_create(layer_bounds);

	Layer *window_layer = window_get_root_layer(window);
	layer_add_child(window_layer, title_bar_layer_get_layer(title_bar_layer));

	TitleBarData *title_bar_data = (TitleBarData *)layer_get_data((Layer *)title_bar_layer);
	text_layer_enable_screen_text_flow_and_paging(title_bar_data->text_layer, 2);
	title_bar_layer_centre_text_vertically(title_bar_layer);

	return title_bar_layer;
}

TitleBarLayer * title_bar_layer_create(GRect bounds) {
	// Set up the title layer
	TitleBarLayer *title_bar_layer = layer_create_with_data(bounds, sizeof(TitleBarData));
	TitleBarData *title_bar_data = (TitleBarData *)layer_get_data((Layer *)title_bar_layer);
	title_bar_data->foreground = GColorBlack;
	title_bar_data->background = GColorWhite;
	title_bar_data->update_proc = NULL;

	title_bar_data->text_layer = text_layer_create(bounds);
	text_layer_set_text_alignment(title_bar_data->text_layer, GTextAlignmentCenter);
	text_layer_set_text_color(title_bar_data->text_layer, GColorBlack);
	text_layer_set_background_color(title_bar_data->text_layer, GColorClear);

	layer_add_child((Layer*)title_bar_layer, text_layer_get_layer(title_bar_data->text_layer));

	layer_set_update_proc(title_bar_layer, title_bar_layer_update_proc);

	return title_bar_layer;
}

void title_bar_layer_stretch(TitleBarLayer * title_bar_layer) {
	TitleBarData *title_bar_data = (TitleBarData *)layer_get_data((Layer *)title_bar_layer);
	GRect window_bounds = layer_get_bounds(window_get_root_layer(layer_get_window((Layer *)title_bar_layer)));
	GRect layer_bounds = layer_get_bounds((Layer *)title_bar_layer);

	layer_bounds.origin.x = 0;
	layer_bounds.size.w = window_bounds.size.w;
	layer_set_frame((Layer *)title_bar_layer, layer_bounds);

	layer_set_frame(text_layer_get_layer(title_bar_data->text_layer), layer_bounds);
	title_bar_layer_centre_text_vertically(title_bar_layer);
}

void title_bar_layer_add_to_window(TitleBarLayer * title_bar_layer, Window *window) {
	TitleBarData *title_bar_data = (TitleBarData *)layer_get_data((Layer *)title_bar_layer);
	layer_add_child(window_get_root_layer(window), (Layer *)title_bar_layer);
	text_layer_enable_screen_text_flow_and_paging(title_bar_data->text_layer, 2);
	title_bar_layer_centre_text_vertically(title_bar_layer);
}

void title_bar_layer_destroy(TitleBarLayer * title_bar_layer) {
	TitleBarData *title_bar_data = (TitleBarData *)layer_get_data((Layer *)title_bar_layer);

	// Remove text layer
	layer_remove_child_layers(text_layer_get_layer(title_bar_data->text_layer));
	layer_remove_from_parent(text_layer_get_layer(title_bar_data->text_layer));
	text_layer_destroy(title_bar_data->text_layer);

	// Disconnect from other layers
	layer_remove_child_layers((Layer *)title_bar_layer);
	layer_remove_from_parent((Layer *)title_bar_layer);

	// Free all memory
	layer_destroy((Layer *)title_bar_layer);
}

void title_bar_layer_set_height(TitleBarLayer * title_bar_layer, int16_t height) {
	TitleBarData *title_bar_data = (TitleBarData *)layer_get_data((Layer *)title_bar_layer);
	GRect window_bounds = layer_get_bounds(window_get_root_layer(layer_get_window((Layer *)title_bar_layer)));

	GRect layer_bounds = (GRect){
		.origin = GPoint(0, 0),
		.size = GSize(window_bounds.size.w, height)
	};
	layer_set_frame((Layer *)title_bar_layer, layer_bounds);
	layer_set_frame(text_layer_get_layer(title_bar_data->text_layer), layer_bounds);
	title_bar_layer_centre_text_vertically(title_bar_layer);
}

int16_t title_bar_layer_get_height(TitleBarLayer * title_bar_layer) {
	GRect layer_bounds = layer_get_bounds(title_bar_layer_get_layer(title_bar_layer));
	return layer_bounds.size.h;
}

void title_bar_layer_set_colors(TitleBarLayer * title_bar_layer, GColor foreground, GColor background) {
	TitleBarData *title_bar_data = (TitleBarData *)layer_get_data((Layer *)title_bar_layer);

	text_layer_set_text_color(title_bar_data->text_layer, foreground);
	title_bar_data->foreground = foreground;
	title_bar_data->background = background;
	layer_mark_dirty((Layer *)title_bar_layer);
}

GColor title_bar_layer_get_foreground_color(TitleBarLayer * title_bar_layer) {
	TitleBarData *title_bar_data = (TitleBarData *)layer_get_data((Layer *)title_bar_layer);

	return title_bar_data->foreground;
}

GColor title_bar_layer_get_background_color(TitleBarLayer * title_bar_layer) {
	TitleBarData *title_bar_data = (TitleBarData *)layer_get_data((Layer *)title_bar_layer);

	return title_bar_data->background;
}

void title_bar_layer_set_update_proc(TitleBarLayer * title_bar_layer, TitleBarLayerUpdateProc update_proc) {
	TitleBarData *title_bar_data = (TitleBarData *)layer_get_data((Layer *)title_bar_layer);

	title_bar_data->update_proc = update_proc;
}

static void title_bar_layer_update_proc(Layer *layer, GContext *ctx) {
	TitleBarData *title_bar_data = (TitleBarData *)layer_get_data(layer);

	if (title_bar_data->update_proc != NULL) {
		title_bar_data->update_proc((TitleBarLayer *)layer, ctx);
	}
	else {
		GRect bounds = layer_get_bounds(layer);
		graphics_context_set_fill_color(ctx, title_bar_data->background);
		graphics_fill_rect(ctx, bounds, 0, GCornerNone);
	}
}

void title_bar_layer_set_text(TitleBarLayer * title_bar_layer, const char * text) {
	TitleBarData *title_bar_data = (TitleBarData *)layer_get_data((Layer *)title_bar_layer);

	text_layer_set_text(title_bar_data->text_layer, text);
	title_bar_layer_centre_text_vertically(title_bar_layer);
}

void title_bar_layer_centre_text_vertically(TitleBarLayer * title_bar_layer) {
	TitleBarData *title_bar_data = (TitleBarData *)layer_get_data((Layer *)title_bar_layer);

	GRect title_bar_bounds = layer_get_bounds((Layer *)title_bar_layer); 
	GRect text_bounds = layer_get_bounds(text_layer_get_layer(title_bar_data->text_layer));
	GSize text_size = text_layer_get_content_size(title_bar_data->text_layer);

	text_bounds.origin.y = (title_bar_bounds.size.h - text_size.h) / 2;
	
	layer_set_frame(text_layer_get_layer(title_bar_data->text_layer), text_bounds);
}


