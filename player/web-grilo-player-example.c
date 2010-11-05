/*
 * web-grilo-player-example.c: Example
 *
 * Copyright (C) 2010 Manuel Rego Casasnovas <mrego@igalia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <gtk/gtk.h>
#include <webkit/webkit.h>

gint
main (gint argc, gchar **argv)
{
        GtkWidget *main_window;
        GtkWidget *scrolled_window;
        GtkWidget *web_view;

        gchar *path_html;
        gchar *uri_html;

	gtk_init (&argc, &argv);

        path_html = PLAYER_DIR "test.html";
        uri_html = g_filename_to_uri(path_html, NULL, NULL);

        main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
        scrolled_window = gtk_scrolled_window_new (NULL, NULL);
        web_view = webkit_web_view_new ();

        gtk_container_add (GTK_CONTAINER (scrolled_window), web_view);
        gtk_container_add (GTK_CONTAINER (main_window), scrolled_window);

        webkit_web_view_load_uri (WEBKIT_WEB_VIEW (web_view), uri_html);

        gtk_window_set_default_size (GTK_WINDOW (main_window), 800, 600);
        gtk_widget_show_all (main_window);

        g_signal_connect (main_window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

        gtk_main ();

        return 0;
}
