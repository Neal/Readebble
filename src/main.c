#include <pebble.h>
#include "appmessage.h"
#include "settings.h"
#include "windows/subscriptions.h"

static void init(void) {
	appmessage_init();
	settings_load();
	subscriptions_init();
}

static void deinit(void) {
	settings_save();
	subscriptions_destroy();
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}
