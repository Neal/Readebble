#include <pebble.h>
#include "story.h"
#include "libs/pebble-assist.h"
#include "generated/appinfo.h"
#include "generated/keys.h"
#include "headlines.h"
#include "windows/win-story.h"

#define STORY_MAX_LEN 2000

static char story[STORY_MAX_LEN];

void story_init(void) {
	win_story_init();
}

void story_deinit(void) {
	win_story_deinit();
}

void story_in_received_handler(DictionaryIterator *iter) {
	Tuple *tuple = dict_find(iter, APP_KEY_METHOD);
	if (!tuple) return;
	switch (tuple->value->uint8) {
		case KEY_METHOD_DATA:
			tuple = dict_find(iter, APP_KEY_TITLE);
			if (!tuple) break;
			if (strlen(story) + tuple->length < STORY_MAX_LEN)
				strcat(story, tuple->value->cstring);
			break;
		case KEY_METHOD_READY:
			win_story_reload_data_and_mark_dirty();
			break;
	}
}

char* story_get() {
	return (story == NULL) ? "Loading..." : story;
}

void story_request() {
	story[0] = '\0';
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	dict_write_uint8(iter, APP_KEY_METHOD, KEY_METHOD_REQUESTSTORY);
	dict_write_uint8(iter, APP_KEY_INDEX, headlines_get_current_index());
	dict_write_end(iter);
	app_message_outbox_send();
}
