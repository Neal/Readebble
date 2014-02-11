#pragma once

void subscriptions_init(void);
void subscriptions_destroy(void);
void subscriptions_in_received_handler(DictionaryIterator *iter);
void subscriptions_out_sent_handler(DictionaryIterator *sent);
void subscriptions_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason);
