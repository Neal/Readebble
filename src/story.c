#include <pebble.h>
#include "story.h"
#include "libs/pebble-assist.h"
#include "generated/appinfo.h"
#include "generated/keys.h"
#include "headlines.h"
#include "windows/win-story.h"

#define STORY_MAX_LEN 2000

static char story[STORY_MAX_LEN];
static uint32_t story_size;
static uint32_t story_loaded;
static uint8_t progress = 0;

void story_update_progress();

void story_init(void) {
	story_size = 0u;
	story_loaded = 0u;
	win_story_init();
}

void story_deinit(void) {
	win_story_deinit();
}

void story_in_received_handler(DictionaryIterator *iter) {
	Tuple *tuple = dict_find(iter, APP_KEY_METHOD);
	if (!tuple) return;
	switch (tuple->value->uint8) {
		case KEY_METHOD_SIZE:
			tuple = dict_find(iter, APP_KEY_INDEX);
			if (!tuple) break;
			story_size = tuple->value->uint32;
			progress = (story_size == 0 ? 100 : 0);
			break;
		case KEY_METHOD_DATA:
			tuple = dict_find(iter, APP_KEY_TITLE);
			if (!tuple) break;
			if (strlen(story) + tuple->length < STORY_MAX_LEN) {
				strcat(story, tuple->value->cstring);
				story_loaded += tuple->length;
				progress = (story_loaded * 100) / story_size;
				story_update_progress();
				LOG("loaded: %u", (unsigned int)story_loaded);
			}
			break;
		case KEY_METHOD_READY:
			progress = 100;
			win_story_reload_data_and_mark_dirty();
			break;
	}
}

char* story_get() {
	return (story == NULL) ? "Loading..." : story;
}

void story_request() {
	story[0] = '\0';
	story_size = 0u;
	story_loaded = 0u;
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	dict_write_uint8(iter, APP_KEY_METHOD, KEY_METHOD_REQUESTSTORY);
	dict_write_uint8(iter, APP_KEY_INDEX, headlines_get_current_index());
	dict_write_end(iter);
	app_message_outbox_send();
}

void story_update_progress() {
	win_story_update_progress();
}

uint8_t story_progress() {
	return progress;
}

