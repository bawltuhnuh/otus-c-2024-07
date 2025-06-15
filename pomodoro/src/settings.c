#include "settings.h"

#define DEFAULT_SOUND_PATH "/usr/share/sounds/freedesktop/stereo/bell.oga"
#define DEFAULT_WORK_DURATION 25
#define DEFAULT_BREAK_DURATION 5
#define DEFAULT_AUTOSTART FALSE
#define DEFAULT_SOUND_ENABLED TRUE
#define DEFAULT_LANGUAGE "en_US.UTF-8"

#define GROUP_NAME "Settings"
#define SOUND_PATH "sound_path"
#define WORK_DURATION "work_duration"
#define BREAK_DURATION "break_duration"
#define AUTOSTART "autostart"
#define SOUND_ENABLED "sound_enabled"
#define LANGUAGE "language"

#define DIR_NAME "pomodoro"
#define CONFIG_NAME "config.ini"

Settings* init_settings() {
    Settings* settings = g_new0(Settings, 1);
    return settings;
}

void free_settings(Settings* settings) {
    g_free(settings->language);
    g_free(settings->sound_path);
    g_free(settings);
}

int load_int_or_default(GKeyFile* key_file, const gchar* group_name, const gchar* key, int def) {
    if (key_file == NULL) {
        return def;
    }
    GError* error = NULL;
    int value = g_key_file_get_integer(key_file, group_name, key, &error);
    if (error) {
        g_warning("Не удалось загрузить %s: %s", key, error->message);
        g_clear_error(&error);
        return def;
    }
    return value;
}

gchar* load_string_or_default(GKeyFile* key_file, const gchar* group_name, const gchar* key, gchar* def) {
    if (key_file == NULL) {
        return g_strdup(def);
    }
    GError* error = NULL;
    gchar* value = g_key_file_get_string(key_file, group_name, key, &error);
    if (error) {
        g_warning("Не удалось загрузить %s: %s", key, error->message);
        g_clear_error(&error);
        return g_strdup(def);
    }
    return value;
}

gboolean load_bool_or_default(GKeyFile* key_file, const gchar* group_name, const gchar* key, gboolean def) {
    if (key_file == NULL) {
        return def;
    }
    GError* error = NULL;
    gboolean value = g_key_file_get_boolean(key_file, group_name, key, &error);
    if (error) {
        g_warning("Не удалось загрузить %s: %s", key, error->message);
        g_clear_error(&error);
        return def;
    }
    return value;
}

void load_settings(Settings* settings) {
    GKeyFile* key_file = g_key_file_new();
    gchar* config_path = g_build_filename(g_get_user_config_dir(), DIR_NAME, CONFIG_NAME, NULL);

    g_key_file_load_from_file(key_file, config_path, G_KEY_FILE_NONE, NULL);

    settings->work_duration = load_int_or_default(key_file, GROUP_NAME, WORK_DURATION, DEFAULT_WORK_DURATION);
    settings->break_duration = load_int_or_default(key_file, GROUP_NAME, BREAK_DURATION, DEFAULT_BREAK_DURATION);
    settings->sound_enabled = load_bool_or_default(key_file, GROUP_NAME, SOUND_ENABLED, DEFAULT_SOUND_ENABLED);
    settings->language = load_string_or_default(key_file, GROUP_NAME, LANGUAGE, DEFAULT_LANGUAGE);
    settings->sound_path = load_string_or_default(key_file, GROUP_NAME, SOUND_PATH, DEFAULT_SOUND_PATH);
    settings->autostart_enabled = load_bool_or_default(key_file, GROUP_NAME, AUTOSTART, DEFAULT_AUTOSTART);

    g_key_file_free(key_file);
    g_free(config_path);
}

void save_settings(Settings* settings) {
    GKeyFile *key_file = g_key_file_new();
    gchar *config_dir = g_build_filename(g_get_user_config_dir(), DIR_NAME, NULL);
    g_mkdir_with_parents(config_dir, 0755);

    g_key_file_set_integer(key_file, GROUP_NAME, WORK_DURATION, settings->work_duration);
    g_key_file_set_integer(key_file, GROUP_NAME, BREAK_DURATION, settings->break_duration);
    g_key_file_set_boolean(key_file, GROUP_NAME, SOUND_ENABLED, settings->sound_enabled);
    g_key_file_set_string(key_file, GROUP_NAME, LANGUAGE, settings->language);
    g_key_file_set_string(key_file, GROUP_NAME, SOUND_PATH, settings->sound_path);
    g_key_file_set_boolean(key_file, GROUP_NAME, AUTOSTART, settings->autostart_enabled);

    gchar *data = g_key_file_to_data(key_file, NULL, NULL);
    gchar *config_path = g_build_filename(config_dir, CONFIG_NAME, NULL);
    g_file_set_contents(config_path, data, -1, NULL);

    g_free(data);
    g_free(config_dir);
    g_free(config_path);
    g_key_file_free(key_file);
}
