#ifndef SETTINGS_H
#define SETTINGS_H

#include <gtk/gtk.h>

typedef struct {
    int work_duration;
    int break_duration;
    gchar *language;
    gboolean sound_enabled;
    gchar *sound_path;
    gboolean autostart_enabled;
} Settings;

Settings* init_settings();

void free_settings(Settings* settings);

void load_settings(Settings* settings);

void save_settings(Settings* settings);

#endif // SETTINGS_H
