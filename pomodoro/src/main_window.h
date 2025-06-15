#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "settings.h"

typedef struct {
    gboolean is_running;
    gboolean is_work_time;
    guint timer_id;
    int remaining_seconds;
} TimerState;

typedef struct {
    GtkWidget* main_window;
    GtkWidget* label;
    GtkWidget* start_button;
    GtkWidget* settings_button;

    GtkWidget* work_spin;
    GtkWidget* break_spin;
    GtkWidget* sound_check;
    GtkWidget* sound_file_button;
    GtkWidget* language_combo;
    GtkWidget* autostart;
    GtkRevealer* revealer;
    GtkStatusIcon* tray_icon;

    Settings* settings;
    TimerState* timer_state;
} AppContext;

AppContext* init_context(Settings* settings);

#endif // MAIN_WINDOW_H
