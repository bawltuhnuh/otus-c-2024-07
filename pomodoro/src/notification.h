#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include "settings.h"

void show_notification(const char *title, const char *message);

void play_sound_if_enabled(Settings* settings);

#endif // NOTIFICATION_H
