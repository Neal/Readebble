#include <pebble.h>
#include "rss.h"

int main(void) {
	rss_init();
	app_event_loop();
	rss_deinit();
}
