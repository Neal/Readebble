#pragma once
#include "../common.h"

void news_init(headline_t h);
void news_destroy(void);
void news_in_received_handler(DictionaryIterator *iter);
void news_out_sent_handler(DictionaryIterator *sent);
void news_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason);
