#pragma once

typedef struct {
	int index;
	char title[30];
} subscription_t;

typedef struct {
	int index;
	char title[160];
} headline_t;

enum {
	KEY_SUBSCRIPTION = 0x0,
	KEY_HEADLINE = 0x1,
	KEY_INDEX = 0x2,
	KEY_TITLE = 0x3,
	KEY_SUMMARY = 0x4,
};
