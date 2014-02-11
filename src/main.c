#include <pebble.h>
#include "appmessage.h"
#include "windows/subscriptions.h"

static void init(void) {
	appmessage_init();
	subscriptions_init();
}

static void deinit(void) {
	subscriptions_destroy();
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}
