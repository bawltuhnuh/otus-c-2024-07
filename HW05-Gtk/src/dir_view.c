#include "dir_view.h"

#include <dirent.h>
#include <sys/stat.h>

static char* sep = "/";

enum
{
  COL_NAME = 0,
  NUM_COLS
};

static _Bool is_directory(char* path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0) {
        printf("%s\n", path);
        perror("stat");
        return 0; // Ошибка при вызове stat
    }
    return S_ISDIR(statbuf.st_mode);
}

static _Bool is_dot_directory(char* path) {
    return !strcmp(path, "..") || !strcmp(path, ".");
}

static void add_element_to_model(GtkTreeStore *store, GtkTreeIter* parent, GtkTreeIter* element, char* name) {
    gtk_tree_store_append (store, element, parent);
    gtk_tree_store_set(store, element,
                       COL_NAME,name,
                       -1);
}

static void get_full_path(char* dst, char* dir, char* name){
    strcpy(dst, dir);
    strcat(dst, sep);
    strcat(dst, name);
}

static void add_directory_to_model(GtkTreeStore *store, GtkTreeIter* parent, char* path) {
    DIR *d = opendir(path);
    struct dirent *dir;

    if (d) {
        GtkTreeIter iter;
        while ((dir = readdir(d)) != NULL) {
            add_element_to_model(store, parent, &iter, dir->d_name);
            int len = strlen(path) + strlen(dir->d_name) + 2;
            char dst[len];
            get_full_path(dst, path, dir->d_name);
            if (is_directory(dst) && !is_dot_directory(dir->d_name)) {
                add_directory_to_model(store, &iter, dst);
            }
        }
        closedir(d);
    } else {
        perror("opendir");
    }
}

static GtkTreeModel* create_and_fill_model (void)
{
  GtkTreeStore* store = gtk_tree_store_new (NUM_COLS,
                                            G_TYPE_STRING);

  add_directory_to_model(store, NULL, ".");

  return GTK_TREE_MODEL (store);
}

GtkWidget* create_view_and_model (void)
{
  GtkWidget* view = gtk_tree_view_new();

  GtkCellRenderer* renderer;

  renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                               -1,
                                               "Name",
                                               renderer,
                                               "text", COL_NAME,
                                               NULL);

  GtkTreeModel* model = create_and_fill_model ();

  gtk_tree_view_set_model (GTK_TREE_VIEW (view), model);

  g_object_unref (model);

  return view;
}
