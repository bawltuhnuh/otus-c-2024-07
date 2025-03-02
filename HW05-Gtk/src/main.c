#include "dir_view.h"

#include <gtk/gtk.h>

static void setup_window(GtkWidget* window) {

    gtk_window_set_title(GTK_WINDOW(window), "Directory view");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
    gtk_window_present(GTK_WINDOW(window));
}

int main (int argc, char **argv)
{
  gtk_init(&argc, &argv);

  GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  setup_window(window);
  g_signal_connect(window, "destroy", gtk_main_quit, NULL);

  GtkWidget *scrolled_window = gtk_scrolled_window_new (NULL, NULL);

  GtkWidget *view = create_view_and_model();

  gtk_container_add(GTK_CONTAINER(window), scrolled_window);

  gtk_container_add(GTK_CONTAINER(scrolled_window), view);

  gtk_widget_show_all(window);

  gtk_main();

  return 0;
}
