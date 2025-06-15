#include <gtk/gtk.h>
#include <gst/gst.h>
#include <locale.h>
#include <libintl.h>

#include "settings.h"
#include "main_window.h"

int main(int argc, char *argv[]) {
    Settings* settings = init_settings();
    load_settings(settings);

    setenv("LANGUAGE", settings->language, 1);

    setlocale(LC_ALL, "");
    bindtextdomain("pomodoro", "/usr/share/locale");
    textdomain("pomodoro");

    gst_init(&argc, &argv);
    gtk_init(&argc, &argv);

    AppContext* context = init_context(settings);

    gtk_widget_show_all(context->main_window);
    gtk_main();
    return 0;
}
