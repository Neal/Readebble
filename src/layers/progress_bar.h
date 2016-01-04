#pragma once

#define PROGRESS_BAR_WINDOW_SIZE (GSize(144, 1))

typedef Layer ProgressBarLayer;
typedef void(* ProgressBarLayerUpdateProc)(ProgressBarLayer *layer, GContext *ctx);

#define progress_bar_layer_get_layer(layer) (Layer *)layer
#define progress_bar_layer_destroy_safe(layer) if (layer != NULL) { progress_bar_layer_destroy(layer); layer = NULL; }

ProgressBarLayer * progress_bar_layer_create(GPoint start, int16_t width);
void progress_bar_layer_destroy(ProgressBarLayer * progress_bar_layer);
void progress_bar_layer_set_pos(ProgressBarLayer * progress_bar_layer, int16_t y);
void progress_bar_layer_set_progress(ProgressBarLayer * progress_bar_layer, uint8_t percent);
//Layer * progress_bar_layer_get_layer(ProgressBarLayer * progress_bar_layer);
void progress_bar_layer_set_colors(ProgressBarLayer * progress_bar_layer, GColor complete, GColor remaining);
GColor progress_bar_layer_get_complete_color(ProgressBarLayer * progress_bar_layer);
GColor progress_bar_layer_get_remaining_color(ProgressBarLayer * progress_bar_layer);
void progress_bar_layer_set_update_proc(ProgressBarLayer * progress_bar_layer, ProgressBarLayerUpdateProc update_proc);

void progress_bar_layer_stretch(ProgressBarLayer * progress_bar_layer);
ProgressBarLayer * progress_bar_layer_create_fullscreen(Window * window);


