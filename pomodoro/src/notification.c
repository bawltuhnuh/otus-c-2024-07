#include <gst/gst.h>
#include <gtk/gtk.h>
#include <libnotify/notify.h>

#include "notification.h"

void show_notification(const char *title, const char *message) {
    notify_init("Pomodoro");
    NotifyNotification *n = notify_notification_new(title, message, "dialog-information");
    notify_notification_show(n, NULL);
    g_object_unref(G_OBJECT(n));
}

void play_sound_if_enabled(Settings* settings) {
    if (!settings->sound_enabled) {
        return;
    }

    gchar *path = settings->sound_path;
    if (!g_file_test(path, G_FILE_TEST_EXISTS)) {
        g_warning("Sound file not found: %s", path);
        return;
    }

    GstElement *pipeline = gst_element_factory_make("playbin", "playbin");
    if (!pipeline) {
        g_warning("Failed to create GStreamer pipeline.");
        return;
    }

    gchar *uri = g_strdup_printf("file://%s", path);
    g_object_set(pipeline, "uri", uri, NULL);
    g_free(uri);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    g_timeout_add(2000, (GSourceFunc)gst_object_unref, pipeline);
}
