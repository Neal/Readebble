#pragma once
#include "../common.h"

void headlines_init(subscription_t s);
void headlines_destroy(void);
void headlines_in_received_handler(DictionaryIterator *iter);
void headlines_out_sent_handler(DictionaryIterator *sent);
void headlines_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason);
