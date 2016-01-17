#pragma once

#define TITLE_BAR_DEFAULT_HEIGHT PBL_IF_ROUND_ELSE ((STATUS_BAR_LAYER_HEIGHT + 12), (STATUS_BAR_LAYER_HEIGHT + 24))

#define TITLE_BAR_DEFAULT_WIDTH PBL_IF_RECT_ELSE(144, 180)

typedef Layer TitleBarLayer;
typedef void(* TitleBarLayerUpdateProc)(TitleBarLayer *layer, GContext *ctx);

#define title_bar_layer_get_layer(layer) (Layer *)layer
#define title_bar_layer_destroy_safe(layer) if (layer != NULL) { title_bar_layer_destroy(layer); layer = NULL; }

TitleBarLayer * title_bar_layer_create(GRect bounds);
void title_bar_layer_destroy(TitleBarLayer * title_bar_layer);
void title_bar_layer_set_height(TitleBarLayer * title_bar_layer, int16_t height);
int16_t title_bar_layer_get_height(TitleBarLayer * title_bar_layer);
void title_bar_layer_reduce_height(TitleBarLayer * title_bar_layer);
void title_bar_layer_set_colors(TitleBarLayer * title_bar_layer, GColor foreground, GColor background);
GColor title_bar_layer_get_foreground_color(TitleBarLayer * title_bar_layer);
GColor title_bar_layer_get_background_color(TitleBarLayer * title_bar_layer);
void title_bar_layer_set_update_proc(TitleBarLayer * title_bar_layer, TitleBarLayerUpdateProc update_proc);
void title_bar_layer_stretch(TitleBarLayer * title_bar_layer);
TitleBarLayer * title_bar_layer_create_fullscreen(Window * window);
void title_bar_layer_set_text(TitleBarLayer * title_bar_layer, const char * text);

