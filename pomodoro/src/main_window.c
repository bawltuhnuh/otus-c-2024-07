#include <gtk/gtk.h>
#include <libintl.h>
#include <glib/gstdio.h>

#include "main_window.h"
#include "notification.h"

#define _(STRING) gettext(STRING)

void init_timer_state(AppContext* context) {
    context->timer_state = g_new0(TimerState, 1);
    context->timer_state->is_running = FALSE;
    context->timer_state->is_work_time = TRUE;
    context->timer_state->remaining_seconds = 0;
}

gboolean update_timer(gpointer user_data) {
    AppContext* context = (AppContext*)user_data;
    if (context->timer_state->remaining_seconds <= 0) {
        context->timer_state->is_work_time = !context->timer_state->is_work_time;
        int new_time = context->timer_state->is_work_time ? context->settings->work_duration
                                                          : context->settings->break_duration;
        context->timer_state->remaining_seconds = new_time * 60;

        const char *msg = context->timer_state->is_work_time ? _("Work time!") : _("Break time!");
        show_notification("Pomodoro", msg);
        play_sound_if_enabled(context->settings);
        gtk_label_set_text(GTK_LABEL(context->label), msg);
        return TRUE;
    }

    int minutes = context->timer_state->remaining_seconds / 60;
    int seconds = context->timer_state->remaining_seconds % 60;
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%s %02d:%02d", context->timer_state->is_work_time ? _("Work") : _("Break"), minutes, seconds);
    gtk_label_set_text(GTK_LABEL(context->label), buffer);

    context->timer_state->remaining_seconds--;
    return TRUE;
}

void toggle_settings(GtkButton *button, gpointer user_data) {
    AppContext* context = (AppContext*)user_data;
    gboolean visible = !gtk_revealer_get_reveal_child(context->revealer);
    gtk_revealer_set_reveal_child(context->revealer, visible);
    if (!visible) {
        gtk_window_resize(GTK_WINDOW(context->main_window), 1, 1);
    }
}

void toggle_timer(GtkButton *button, gpointer user_data) {
    AppContext* context = (AppContext*)user_data;
    if (context->timer_state->is_running) {
        g_source_remove(context->timer_state->timer_id);
        gtk_button_set_label(button, _("Start"));
    } else {
        int minutes = context->settings->work_duration;
        context->timer_state->remaining_seconds = minutes * 60;
        context->timer_state->is_work_time = TRUE;
        context->timer_state->timer_id = g_timeout_add(1000, update_timer, context);
        gtk_button_set_label(button, _("Stop"));
    }
    context->timer_state->is_running = !context->timer_state->is_running;
}

gboolean on_window_state_event(GtkWidget *widget, GdkEventWindowState *event, gpointer user_data) {
    if (event->new_window_state & GDK_WINDOW_STATE_ICONIFIED) {
        gtk_widget_hide(widget);
        gtk_window_set_skip_taskbar_hint(GTK_WINDOW(widget), TRUE);
        return TRUE;
    }
    return FALSE;
}

void on_language_changed(GtkComboBox *combo, gpointer user_data) {
    Settings* settings = (Settings*)user_data;
    const gchar *new_lang = gtk_combo_box_get_active_id(combo);

    if (!new_lang || (settings->language && g_strcmp0(new_lang, settings->language) == 0)) {
        return;
    }

    GtkWidget *dialog = gtk_message_dialog_new(NULL,
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_YES_NO,
        _("The language changes will take effect after the restart. Restart now?"));

    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    g_free(settings->language);
    settings->language = g_strdup(new_lang);
    save_settings(settings);

    if (response == GTK_RESPONSE_YES) {
        gchar **argv;
        g_shell_parse_argv(g_get_prgname(), NULL, &argv, NULL);
        execvp(argv[0], argv);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
}

GtkWidget* init_settings_button(GtkWidget* main_area) {
    GtkWidget* settings_button = gtk_button_new_with_label(_("Settings"));
    gtk_box_pack_start(GTK_BOX(main_area), settings_button, FALSE, FALSE, 5);
    return settings_button;
}

GtkWidget* init_start_button(GtkWidget* main_area) {
    GtkWidget* start_button = gtk_button_new_with_label(_("Start"));
    gtk_box_pack_start(GTK_BOX(main_area), start_button, FALSE, FALSE, 5);
    return start_button;
}

GtkWidget* init_label(GtkWidget* main_area) {
    GtkWidget* label = gtk_label_new(_("Ready"));
    gtk_box_pack_start(GTK_BOX(main_area), label, FALSE, FALSE, 5);
    return label;
}

void init_main_area(AppContext* context, GtkWidget* main_hbox) {
    GtkWidget *main_area = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_size_request(main_area, 280, -1);
    gtk_box_pack_start(GTK_BOX(main_hbox), main_area, TRUE, TRUE, 0);

    //gtk_box_pack_start(GTK_BOX(main_area),gtk_label_new(NULL), TRUE, TRUE, 0);

    context->settings_button = init_settings_button(main_area);
    context->start_button = init_start_button(main_area);
    context->label = init_label(main_area);

    //gtk_box_pack_start(GTK_BOX(main_area),gtk_label_new(NULL), TRUE, TRUE, 0);
}

GtkWidget* init_work_spin(GtkWidget* settings_box, Settings* settings) {
    GtkWidget *work_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(work_hbox), gtk_label_new(_("Work duration:")), FALSE, FALSE, 0);
    GtkWidget* work_spin = gtk_spin_button_new_with_range(1, 120, 1);

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(work_spin), settings->work_duration);
    gtk_box_pack_start(GTK_BOX(work_hbox), work_spin, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(work_hbox), gtk_label_new("min"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(settings_box), work_hbox, FALSE, FALSE, 0);
    return work_spin;
}

GtkWidget* init_break_spin(GtkWidget* settings_box, Settings* settings) {
    GtkWidget *break_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(break_hbox), gtk_label_new(_("Break duration:")), FALSE, FALSE, 0);
    GtkWidget* break_spin = gtk_spin_button_new_with_range(1, 60, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(break_spin), settings->break_duration);
    gtk_box_pack_start(GTK_BOX(break_hbox), break_spin, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(break_hbox), gtk_label_new("min"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(settings_box), break_hbox, FALSE, FALSE, 0);
    return break_spin;
}

GtkWidget* init_sound_checkbox(GtkWidget* settings_box, Settings* settings) {
    GtkWidget* sound_check = gtk_check_button_new_with_label(_("Notification sound"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(sound_check), settings->sound_enabled);
    gtk_box_pack_start(GTK_BOX(settings_box), sound_check, FALSE, FALSE, 0);
    return sound_check;
}

GtkWidget* init_language_combo(GtkWidget* settings_box, Settings* settings) {
    GtkWidget *lang_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(lang_hbox), gtk_label_new(_("Language:")), FALSE, FALSE, 0);

    GtkWidget* language_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(language_combo), "en_US.UTF-8", "English");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(language_combo), "ru_RU.UTF-8", "Русский");
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(language_combo), settings->language);
    gtk_box_pack_start(GTK_BOX(lang_hbox), language_combo, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(settings_box), lang_hbox, FALSE, FALSE, 0);
    return language_combo;
}

GtkWidget* init_autostart_checkbox(GtkWidget* settings_box, Settings* settings) {
    GtkWidget* autostart_check = gtk_check_button_new_with_label(_("Add to autostart"));
    gtk_box_pack_start(GTK_BOX(settings_box), autostart_check, FALSE, FALSE, 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(autostart_check), settings->autostart_enabled);
    return autostart_check;
}

GtkWidget* init_sound(GtkWidget* settings_box, Settings* settings) {
    GtkWidget *file_label = gtk_label_new(_("Sound file:"));
    GtkWidget* sound_file_button = gtk_file_chooser_button_new(_("Select sound"), GTK_FILE_CHOOSER_ACTION_OPEN);
    gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(sound_file_button), settings->sound_path);
    gtk_box_pack_start(GTK_BOX(settings_box), file_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(settings_box), sound_file_button, FALSE, FALSE, 0);
    return sound_file_button;
}

void handle_autostart(gboolean autostart) {
    gchar *autostart_path = g_build_filename(g_get_user_config_dir(), "autostart", "pomodoro.desktop", NULL);
    const gchar *system_desktop = "/usr/share/applications/pomodoro.desktop";

    if (autostart) {
        g_mkdir_with_parents(g_path_get_dirname(autostart_path), 0755);

        if (!g_file_test(system_desktop, G_FILE_TEST_EXISTS)) {
            g_warning("System .desktop file not found: %s", system_desktop);
        } else {
            // Копируем system .desktop -> autostart
            gchar *contents = NULL;
            gsize length;
            if (g_file_get_contents(system_desktop, &contents, &length, NULL)) {
                // Обязательно добавим/перезапишем строку автозапуска
                gchar *final_contents = g_strconcat(contents,
                    "\nX-GNOME-Autostart-enabled=true\n", NULL);
                g_file_set_contents(autostart_path, final_contents, -1, NULL);
                g_free(contents);
                g_free(final_contents);
            }
        }
    } else {
        g_remove(autostart_path); // Удаляем из автозапуска
    }
    g_free(autostart_path);
}

void on_autostart_changed(GtkToggleButton *toggle_button, gpointer user_data) {
    Settings* settings = (Settings*)user_data;
    settings->autostart_enabled = gtk_toggle_button_get_active(toggle_button);
    handle_autostart(settings->autostart_enabled);
}

void on_sound_changed(GtkToggleButton *toggle_button, gpointer user_data) {
    Settings* settings = (Settings*)user_data;
    settings->sound_enabled = gtk_toggle_button_get_active(toggle_button);
}

void on_work_duration_changed(GtkSpinButton *spin_button, gpointer user_data) {
    Settings* settings = (Settings*)user_data;
    settings->work_duration = gtk_spin_button_get_value_as_int(spin_button);
}

void on_break_duration_changed(GtkSpinButton *spin_button, gpointer user_data) {
    Settings* settings = (Settings*)user_data;
    settings->break_duration = gtk_spin_button_get_value_as_int(spin_button);
}

void on_file_set(GtkFileChooser *file_chooser, gpointer user_data){
    Settings* settings = (Settings*)user_data;
    g_free(settings->sound_path);
    settings->sound_path = gtk_file_chooser_get_filename(file_chooser);
}

void finalize_connections(AppContext* context) {
    g_signal_connect(context->settings_button, "clicked", G_CALLBACK(toggle_settings), context);
    g_signal_connect(context->start_button, "clicked", G_CALLBACK(toggle_timer), context);
    g_signal_connect(context->language_combo, "changed", G_CALLBACK(on_language_changed), context->settings);
    g_signal_connect(context->autostart, "toggled", G_CALLBACK(on_autostart_changed), context->settings);
    g_signal_connect(context->sound_check, "toggled", G_CALLBACK(on_sound_changed), context->settings);
    g_signal_connect(context->work_spin, "value-changed", G_CALLBACK(on_work_duration_changed), context->settings);
    g_signal_connect(context->break_spin, "value-changed", G_CALLBACK(on_break_duration_changed), context->settings);
    g_signal_connect(context->sound_file_button, "file-set", G_CALLBACK(on_file_set), context->settings);
}

GtkWidget* get_page(GtkWidget* notebook, const gchar* label) {
    GtkWidget *page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(page), 10);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page, gtk_label_new(label));
    return page;
}

void init_settings_box(AppContext* context) {
    GtkWidget* settings_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(context->revealer), settings_box);

    GtkWidget *notebook = gtk_notebook_new();

    GtkWidget *timer_page = get_page(notebook, _("Timer"));
    GtkWidget *notify_page = get_page(notebook, _("Notification"));
    GtkWidget *misc_page = get_page(notebook, _("Misc"));

    gtk_box_pack_start(GTK_BOX(settings_box), notebook, TRUE, TRUE, 5);

    context->work_spin = init_work_spin(timer_page, context->settings);
    context->break_spin = init_break_spin(timer_page, context->settings);
    context->sound_check = init_sound_checkbox(notify_page, context->settings);
    context->language_combo = init_language_combo(misc_page, context->settings);
    context->autostart = init_autostart_checkbox(misc_page, context->settings);
    context->sound_file_button = init_sound(notify_page, context->settings);
}

void init_settings_revealer(AppContext* context, GtkWidget* main_hbox) {
    context->revealer = GTK_REVEALER(gtk_revealer_new());
    gtk_revealer_set_transition_type(context->revealer, GTK_REVEALER_TRANSITION_TYPE_SLIDE_RIGHT);
    gtk_revealer_set_transition_duration(context->revealer, 0);
    gtk_revealer_set_reveal_child(context->revealer, FALSE);
    gtk_box_pack_start(GTK_BOX(main_hbox), GTK_WIDGET(context->revealer), FALSE, FALSE, 0);

    init_settings_box(context);
}

GtkWidget* init_main_hbox(AppContext* context) {
    GtkWidget *main_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(main_hbox), 10);
    gtk_container_add(GTK_CONTAINER(context->main_window), main_hbox);

    init_main_area(context, main_hbox);
    init_settings_revealer(context, main_hbox);
}

void on_window_close(GtkWidget *widget, gpointer user_data) {
    Settings* settings = (Settings*)user_data;
    save_settings(settings);
}

void init_main_window(AppContext* context) {
    context->main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(context->main_window), "Pomodoro");
    gtk_window_set_default_size(GTK_WINDOW(context->main_window), 280, 120);
    gtk_window_set_resizable(GTK_WINDOW(context->main_window), TRUE);
    g_signal_connect(context->main_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(context->main_window, "window-state-event", G_CALLBACK(on_window_state_event), NULL);
    g_signal_connect(context->main_window, "destroy", G_CALLBACK(on_window_close), context->settings);
}

static void tray_icon_on_click(GtkStatusIcon *status_icon, gpointer user_data) {
    GtkWindow *window = GTK_WINDOW(user_data);
    gtk_window_deiconify(window);
    gtk_widget_show_all(GTK_WIDGET(window));
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(window), FALSE);
    gtk_window_present(window);
}

static void tray_icon_on_menu(GtkStatusIcon *status_icon, guint button, guint activate_time, gpointer user_data) {
    GtkWidget *menu = gtk_menu_new();
    GtkWidget *quit_item = gtk_menu_item_new_with_label(_("Quit"));
    g_signal_connect(quit_item, "activate", G_CALLBACK(gtk_main_quit), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), quit_item);
    gtk_widget_show_all(menu);
    gtk_menu_popup_at_pointer(GTK_MENU(menu), NULL);
}

void init_tray(AppContext* context) {
    context->tray_icon = gtk_status_icon_new_from_icon_name("alarm");
    gtk_status_icon_set_tooltip_text(context->tray_icon, "Pomodoro");
    gtk_status_icon_set_visible(context->tray_icon, TRUE);
    g_signal_connect(context->tray_icon, "activate", G_CALLBACK(tray_icon_on_click), context->main_window);
    g_signal_connect(context->tray_icon, "popup-menu", G_CALLBACK(tray_icon_on_menu), NULL);
}

AppContext* init_context(Settings* settings) {
    AppContext* context = g_new0(AppContext, 1);
    context->settings = settings;

    init_timer_state(context);
    init_main_window(context);
    init_main_hbox(context);
    init_tray(context);
    finalize_connections(context);
    return context;
}

void free_context(AppContext* context) {
    gtk_widget_destroy(context->main_window);
    g_free(context->timer_state);
    free_settings(context->settings);
}
