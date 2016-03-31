#include <pebble.h>
#include "layers/progress_bar.h"

typedef struct _ProgressBarData {
	uint8_t progress;
	ProgressBarLayerUpdateProc update_proc;
	GColor complete;
	GColor remaining;
} ProgressBarData;

static void progress_bar_layer_update_proc(ProgressBarLayer *progress_bar_layer, GContext *ctx);
#ifdef PBL_ROUND
float math_sqrt(const float num);
#endif

ProgressBarLayer * progress_bar_layer_create_fullscreen(Window * window) {
	GRect window_bounds = layer_get_bounds(window_get_root_layer(window));
	GPoint start = GPoint(0, STATUS_BAR_LAYER_HEIGHT - 2);
	int16_t width = window_bounds.size.w;
	
	ProgressBarLayer *progress_bar_layer = progress_bar_layer_create(start, width);

	Layer *window_layer = window_get_root_layer(window);
	layer_add_child(window_layer, progress_bar_layer_get_layer(progress_bar_layer));

	return progress_bar_layer;
}

ProgressBarLayer * progress_bar_layer_create(GPoint start, int16_t width) {
	// Set up the progress layer
	GRect bounds = (GRect){
		.origin = start,
		.size = GSize(width, 1)
	};
	
	ProgressBarLayer *progress_bar_layer = layer_create_with_data(bounds, sizeof(ProgressBarData));
	ProgressBarData *progress_bar_data = (ProgressBarData *)layer_get_data((Layer *)progress_bar_layer);
	progress_bar_data->progress = 0;
	progress_bar_data->complete = GColorBlack;
	progress_bar_data->remaining = GColorWhite;
	progress_bar_data->update_proc = NULL;

	layer_set_update_proc(progress_bar_layer, progress_bar_layer_update_proc);

	return progress_bar_layer;
}

void progress_bar_layer_stretch(ProgressBarLayer * progress_bar_layer) {
	GRect window_bounds = layer_get_bounds(window_get_root_layer(layer_get_window((Layer *)progress_bar_layer)));
	GRect layer_bounds = layer_get_bounds((Layer *)progress_bar_layer);

	layer_bounds.origin.x = 0;
	layer_bounds.size.w = window_bounds.size.w;
	layer_set_frame((Layer *)progress_bar_layer, layer_bounds);
}

void progress_bar_layer_add_to_window(ProgressBarLayer * progress_bar_layer, Window *window) {
	layer_add_child(window_get_root_layer(window), (Layer *)progress_bar_layer);
}

void progress_bar_layer_destroy(ProgressBarLayer * progress_bar_layer) {
	// Disconnect from other layers
	layer_remove_child_layers((Layer *)progress_bar_layer);
	layer_remove_from_parent((Layer *)progress_bar_layer);

	// Free all memory
	layer_destroy((Layer *)progress_bar_layer);
}

void progress_bar_layer_set_pos(ProgressBarLayer * progress_bar_layer, int16_t y) {
	GRect window_bounds = layer_get_bounds(window_get_root_layer(layer_get_window((Layer *)progress_bar_layer)));

#ifdef PBL_ROUND
	float radius = ((float)window_bounds.size.h / 2.0);
	float offset = (radius - (float)y);
	int16_t width = (int16_t)math_sqrt((radius * radius) - (offset * offset));
#else
	int16_t width = window_bounds.size.w / 2;
#endif

	GRect layer_bounds = (GRect){
		.origin = GPoint((window_bounds.size.w / 2) - width, y),
		.size = GSize(width * 2, 1)
	};
	layer_set_frame((Layer *)progress_bar_layer, layer_bounds);
}

void progress_bar_layer_set_progress(ProgressBarLayer * progress_bar_layer, uint8_t percent) {
	ProgressBarData *progress_bar_data = (ProgressBarData *)layer_get_data((Layer *)progress_bar_layer);

	progress_bar_data->progress = percent;
	layer_mark_dirty((Layer *)progress_bar_layer);
}

void progress_bar_layer_set_colors(ProgressBarLayer * progress_bar_layer, GColor complete, GColor remaining) {
	ProgressBarData *progress_bar_data = (ProgressBarData *)layer_get_data((Layer *)progress_bar_layer);

	progress_bar_data->complete = complete;
	progress_bar_data->remaining = remaining;
	layer_mark_dirty((Layer *)progress_bar_layer);
}

GColor progress_bar_layer_get_complete_color(ProgressBarLayer * progress_bar_layer) {
	ProgressBarData *progress_bar_data = (ProgressBarData *)layer_get_data((Layer *)progress_bar_layer);

	return progress_bar_data->complete;
}

GColor progress_bar_layer_get_remaining_color(ProgressBarLayer * progress_bar_layer) {
	ProgressBarData *progress_bar_data = (ProgressBarData *)layer_get_data((Layer *)progress_bar_layer);

	return progress_bar_data->remaining;
}

void progress_bar_layer_set_update_proc(ProgressBarLayer * progress_bar_layer, ProgressBarLayerUpdateProc update_proc) {
	ProgressBarData *progress_bar_data = (ProgressBarData *)layer_get_data((Layer *)progress_bar_layer);

	progress_bar_data->update_proc = update_proc;
}

static void progress_bar_layer_update_proc(Layer *layer, GContext *ctx) {
	ProgressBarData *progress_bar_data = (ProgressBarData *)layer_get_data(layer);

	if (progress_bar_data->update_proc != NULL) {
		progress_bar_data->update_proc((ProgressBarLayer *)layer, ctx);
	}
	else {
		GRect bounds = layer_get_bounds(layer);
	
		int width = (int)(float)(((float)progress_bar_data->progress / 100.0f) * bounds.size.w);
		// Complete
		graphics_context_set_stroke_color(ctx, progress_bar_data->complete);
		graphics_draw_line(ctx, GPointZero, GPoint(width, 0));
		// Remaining
		graphics_context_set_stroke_color(ctx, progress_bar_data->remaining);
		graphics_draw_line(ctx, GPoint(width, 0), GPoint(bounds.size.w, 0));
	}
}

#ifdef PBL_ROUND
// See https://forums.getpebble.com/discussion/comment/79122/#Comment_79122
float math_sqrt(const float num) {
	const uint MAX_STEPS = 40;
	const float MAX_ERROR = 0.001;

	float answer = num;
	float ans_sqr = answer * answer;
	uint step = 0;
	while((ans_sqr - num > MAX_ERROR) && (step++ < MAX_STEPS)) {
		answer = (answer + (num / answer)) / 2;
		ans_sqr = answer * answer;
	}
	return answer;
}
#endif

