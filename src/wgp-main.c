/*
 * wgp-main.c: Main file
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

#include <webkit/webkit.h>
#include <grilo.h>

#include <wgp-view.h>

static void
web_view_loaded_cb (WebKitWebView *view,
                    WebKitWebFrame *frame,
                    gpointer user_data)
{
        WebKitDOMDocument *document;
        WebKitDOMHTMLElement *body;
        WebKitDOMNodeList *list;
        WebKitDOMNamedNodeMap *map;
        WebKitDOMNode *node, *id, *sources_node, *main_node;
        WgpView *wgp_view;

        gulong length, i;

        document = webkit_web_view_get_dom_document (view);
        body = webkit_dom_document_get_body (document);
        list = webkit_dom_node_get_child_nodes (WEBKIT_DOM_NODE (body));

        length = webkit_dom_node_list_get_length (list);

        sources_node = NULL;
        main_node = NULL;

        for (i = 0; i < length; i++) {
                node = webkit_dom_node_list_item (list, i);

                if (webkit_dom_node_has_attributes (node)) {
                        map = webkit_dom_node_get_attributes (node);
                        id = webkit_dom_named_node_map_get_named_item (map, "id");

                        if (g_strcmp0 ("sources",
                                    webkit_dom_node_get_node_value (id))) {
                                sources_node = node;
                                continue;
                        }

                        if (g_strcmp0 ("main",
                                    webkit_dom_node_get_node_value (id))) {
                                main_node = node;
                                continue;
                        }
                }
        }

        wgp_view = wgp_view_new ();
        wgp_view_set_document (wgp_view, document);
        wgp_view_set_sources_node (wgp_view, sources_node);
        wgp_view_set_main_node (wgp_view, main_node);

        wgp_view_run (wgp_view);
}


gint
main (gint argc, gchar **argv)
{
        GtkWidget *main_window;
        GtkWidget *scrolled_window;
        GtkWidget *web_view;

        gchar *path_html;
        gchar *uri_html;

	gtk_init (&argc, &argv);
	grl_init (&argc, &argv);

        /* Build URI for index.html file */
        path_html = HTML_DIR "index.html";
        uri_html = g_filename_to_uri(path_html, NULL, NULL);

        main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
        scrolled_window = gtk_scrolled_window_new (NULL, NULL);
        web_view = webkit_web_view_new ();

        gtk_container_add (GTK_CONTAINER (scrolled_window), web_view);
        gtk_container_add (GTK_CONTAINER (main_window), scrolled_window);

        /* Load URI */
        webkit_web_view_load_uri (WEBKIT_WEB_VIEW (web_view), uri_html);
        g_signal_connect (web_view,
                          "document-load-finished",
                          G_CALLBACK (web_view_loaded_cb),
                          NULL);

        gtk_window_set_default_size (GTK_WINDOW (main_window), 800, 600);
        gtk_widget_show_all (main_window);

        g_signal_connect (main_window,
                          "destroy",
                          G_CALLBACK (gtk_main_quit),
                          NULL);

        gtk_main ();

        return 0;
}
