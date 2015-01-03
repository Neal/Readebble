#pragma once

#include <pebble.h>

void rss_init(void);
void rss_deinit(void);
void rss_request_subscriptions();
void rss_add_to_pocket();
void rss_reload_data_and_mark_dirty();
char* rss_get_error();
