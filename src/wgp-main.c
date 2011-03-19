/*
 * wgp-main.c: Main file
 *
 * Copyright (C) 2010 Manuel Rego Casasnovas <mrego@igalia.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <webkit/webkit.h>
#include <grilo.h>
#include "config.h"
#include "wgp-util.h"

static WebKitDOMDocument *document = NULL;
static WebKitDOMNode *sources_node = NULL;
static WebKitDOMNode *main_node = NULL;

static GrlPluginRegistry *registry = NULL;

static GrlMediaSource *current_source = NULL;
static GList *breadcrumbs_list = NULL;


static void
browse_source_cb (GrlMediaSource *source,
                  guint browse_id,
                  GrlMedia *media,
                  guint remaining,
                  gpointer user_data,
                  const GError *error);

static void
media_clicked_cb (WebKitDOMEventTarget* target,
                  WebKitDOMEvent* event,
                  GrlMedia *media);

static void
source_clicked_cb (WebKitDOMEventTarget* target,
                   WebKitDOMEvent* event,
                   GrlMetadataSource *source);

static void
source_added_cb (GrlPluginRegistry *registry,
                 GrlMediaPlugin *source,
                 gpointer user_data);

static void
breadcrumbs_set_last (gpointer source_or_media);


static void
plugins_clicked_cb (WebKitDOMEventTarget* target,
                    WebKitDOMEvent* event,
                    gpointer user_data)
{
        GList *sources;
        GList *l;

        wgp_util_remove_all_children (main_node);
        wgp_util_remove_all_children (sources_node);

        webkit_dom_node_set_text_content (
                main_node,
                "Grilo plugins",
                NULL);

        breadcrumbs_set_last (NULL);

        sources = grl_plugin_registry_get_sources (registry, FALSE);

        for (l = sources; l; l = l->next) {
                source_added_cb (registry, GRL_MEDIA_PLUGIN (l->data), NULL);
        }
}


static void
draw_link (gpointer source_or_media, gpointer node)
{
        GrlMetadataSource *source;
        GrlMedia *media;
        WebKitDOMNode *breadcrumbs_node;
        WebKitDOMElement *input;
        WebKitDOMElement *label;
        const gchar *title;

        breadcrumbs_node = WEBKIT_DOM_NODE (node);

        input = webkit_dom_document_create_element (document, "input", NULL);
        webkit_dom_element_set_attribute (input, "type", "radio", NULL);
        label = webkit_dom_document_create_element (document, "label", NULL);

        if (GRL_IS_MEDIA (source_or_media)) {
                media = GRL_MEDIA (source_or_media);
                title = grl_media_get_title (media);

                webkit_dom_element_set_attribute (input, "id", title, NULL);
                webkit_dom_element_set_attribute (label, "for", title, NULL);

                webkit_dom_node_set_text_content (
                        WEBKIT_DOM_NODE (label),
                        g_strdup_printf ("%s", title),
                        NULL);

                g_signal_connect (label,
                                  "click-event",
                                  G_CALLBACK (media_clicked_cb),
                                  media);
        } else if (GRL_IS_METADATA_SOURCE (source_or_media)) {
                source = GRL_METADATA_SOURCE (source_or_media);
                title = grl_metadata_source_get_name (source);

                webkit_dom_element_set_attribute (input, "id", title, NULL);
                webkit_dom_element_set_attribute (label, "for", title, NULL);

                webkit_dom_node_set_text_content (
                        WEBKIT_DOM_NODE (label),
                        g_strdup_printf ("%s", title),
                        NULL);

                g_signal_connect (label,
                                  "click-event",
                                  G_CALLBACK (source_clicked_cb),
                                  source);
        } else {
                g_error ("Wrong type for source_or_media param");
        }

        webkit_dom_node_append_child (breadcrumbs_node,
                                      WEBKIT_DOM_NODE (input),
                                      NULL);
        webkit_dom_node_append_child (breadcrumbs_node,
                                      WEBKIT_DOM_NODE (label),
                                      NULL);
}

static void
repaint_up_link ()
{
        WebKitDOMNode *breadcrumbs_node;
        WebKitDOMElement *input;
        WebKitDOMElement *label;
        WebKitDOMElement *script;

        breadcrumbs_node = WEBKIT_DOM_NODE (
                webkit_dom_document_get_element_by_id (document, "breadcrumbs"));
        wgp_util_remove_all_children (breadcrumbs_node);

        input = webkit_dom_document_create_element (document, "input", NULL);
        webkit_dom_element_set_attribute (input, "type", "radio", NULL);
        webkit_dom_element_set_attribute (input, "id", "plugins", NULL);

        label = webkit_dom_document_create_element (document, "label", NULL);
        webkit_dom_element_set_attribute (label, "for", "plugins", NULL);
        webkit_dom_node_set_text_content (
                WEBKIT_DOM_NODE (label),
                "Plugins",
                NULL);
        g_signal_connect (label,
                          "click-event",
                          G_CALLBACK (plugins_clicked_cb),
                          NULL);

        webkit_dom_node_append_child (breadcrumbs_node,
                                      WEBKIT_DOM_NODE (input),
                                      NULL);
        webkit_dom_node_append_child (breadcrumbs_node,
                                      WEBKIT_DOM_NODE (label),
                                      NULL);

        g_list_foreach (breadcrumbs_list, draw_link, breadcrumbs_node);

        script = webkit_dom_document_create_element (document, "script", NULL);
        webkit_dom_node_set_text_content (
                WEBKIT_DOM_NODE (script),
                "$('#breadcrumbs').buttonset();",
                NULL);
        webkit_dom_node_append_child (breadcrumbs_node,
                                      WEBKIT_DOM_NODE (script),
                                      NULL);
}


static void
breadcrumbs_set_last (gpointer source_or_media)
{
        GList *element;

        if (source_or_media == NULL) {
                breadcrumbs_list = NULL;
        } else {
                element = g_list_find (breadcrumbs_list, source_or_media);

                if (element != NULL) {
                        while (element != g_list_last (breadcrumbs_list)) {
                                breadcrumbs_list = g_list_remove (
                                        breadcrumbs_list,
                                        g_list_last (breadcrumbs_list)->data);
                        }
                } else {
                        breadcrumbs_list = g_list_append (
                                breadcrumbs_list,
                                source_or_media);
                }
        }

        repaint_up_link ();
}


static void
media_clicked_cb (WebKitDOMEventTarget* target,
                  WebKitDOMEvent* event,
                  GrlMedia *media)
{
        WebKitDOMElement *element = NULL;
        GList * keys;
        const gchar *title;
        const gchar *url;

        title = grl_media_get_title (media);
        g_debug ("Media clicked: '%s'", title);

        wgp_util_remove_all_children (main_node);
        webkit_dom_node_set_text_content (
                main_node,
                g_strdup_printf ("Media selected: %s", title),
                NULL);

        if (GRL_IS_MEDIA_BOX (media)) {
                wgp_util_remove_all_children (sources_node);

                g_debug ("Browsing media: %s", title);
                keys = grl_metadata_key_list_new (GRL_METADATA_KEY_TITLE,
                                                  GRL_METADATA_KEY_DURATION,
                                                  GRL_METADATA_KEY_URL,
                                                  GRL_METADATA_KEY_CHILDCOUNT,
                                                  NULL);
                grl_media_source_browse (current_source,
                                         media,
                                         keys,
                                         0, 100,
                                         GRL_RESOLVE_IDLE_RELAY,
                                         browse_source_cb,
                                         NULL);
                breadcrumbs_set_last (media);
        } else {
                g_debug ("Play media: %s", title);
                url = grl_media_get_url (media);

                if (GRL_IS_MEDIA_IMAGE (media)) {
                        element = webkit_dom_document_create_element (document, "img", NULL);
                }
                if (GRL_IS_MEDIA_AUDIO (media)) {
                        element = webkit_dom_document_create_element (document, "audio", NULL);
                        webkit_dom_element_set_attribute (element, "controls", "controls", NULL);
                        webkit_dom_element_set_attribute (element, "autoplay", "true", NULL);
                }
                if (GRL_IS_MEDIA_VIDEO (media)) {
                        element = webkit_dom_document_create_element (document, "video", NULL);
                        webkit_dom_element_set_attribute (element, "controls", "controls", NULL);
                        webkit_dom_element_set_attribute (element, "autoplay", "true", NULL);
                        webkit_dom_element_set_attribute (element, "width", "400", NULL);
                }

                if (element != NULL) {
                        webkit_dom_node_append_child (
                                main_node,
                                WEBKIT_DOM_NODE (webkit_dom_document_create_element (document, "br", NULL)),
                                NULL);

                        webkit_dom_element_set_attribute (element, "src", url, NULL);
                        webkit_dom_node_append_child (main_node,
                                                      WEBKIT_DOM_NODE (element),
                                                      NULL);
                } else {
                        g_error ("Unknown media type");
                }
        }

}


static void
browse_source_cb (GrlMediaSource *source,
                  guint browse_id,
                  GrlMedia *media,
                  guint remaining,
                  gpointer user_data,
                  const GError *error)
{
        WebKitDOMElement *paragraph;
        const gchar *title;
        gchar *text;

        if (error) {
                g_error ("Browse operation failed. Reason: %s", error->message);
        }

        if (media) {
                title = grl_media_get_title (media);
                paragraph = webkit_dom_document_create_element (document,
                                                                "p",
                                                                NULL);
                webkit_dom_element_set_attribute (paragraph, "id", title, NULL);
                webkit_dom_element_set_attribute (paragraph,
                                                  "class",
                                                  "ui-widget-content",
                                                  NULL);

                if (GRL_IS_MEDIA_BOX (media)) {
                        text = g_strdup_printf ("+ %s", title);
                } else {
                        text = g_strdup_printf ("- %s", title);
                }
                webkit_dom_node_set_text_content (WEBKIT_DOM_NODE (paragraph),
                                                  text,
                                                  NULL);
                webkit_dom_node_append_child (sources_node,
                                              WEBKIT_DOM_NODE (paragraph),
                                              NULL);

                current_source = source;
                g_signal_connect (paragraph,
                                  "click-event",
                                  G_CALLBACK (media_clicked_cb),
                                  media);
        }

        if (remaining == 0) {
                g_debug ("Browse operation finished!");
        } else {
                g_debug ("%d results remaining!", remaining);
        }
}


static void
source_clicked_cb (WebKitDOMEventTarget* target,
                   WebKitDOMEvent* event,
                   GrlMetadataSource *source)
{
        const gchar *source_name;
        GList * keys;

        source_name = grl_metadata_source_get_name (source);
        g_debug ("Source clicked: '%s'", source_name);

        webkit_dom_node_set_text_content (
                WEBKIT_DOM_NODE (main_node),
                g_strdup_printf ("Source selected: %s", source_name),
                NULL);
        wgp_util_remove_all_children (sources_node);

        if (grl_metadata_source_supported_operations (source) & GRL_OP_BROWSE) {
                g_debug ("Browsing source: %s", source_name);
                keys = grl_metadata_key_list_new (GRL_METADATA_KEY_TITLE,
                                                  GRL_METADATA_KEY_DURATION,
                                                  GRL_METADATA_KEY_URL,
                                                  GRL_METADATA_KEY_CHILDCOUNT,
                                                  NULL);
                grl_media_source_browse (GRL_MEDIA_SOURCE (source),
                                         NULL,
                                         keys,
                                         0, 100,
                                         GRL_RESOLVE_IDLE_RELAY,
                                         browse_source_cb,
                                         NULL);
                breadcrumbs_set_last (source);
        }
}


static void
source_added_cb (GrlPluginRegistry *registry,
                 GrlMediaPlugin *source,
                 gpointer user_data)
{
        WebKitDOMElement *paragraph;
        const gchar *source_name;

        source_name = grl_metadata_source_get_name (
                GRL_METADATA_SOURCE (source));
        g_debug ("Detected new source available: '%s'", source_name);

        paragraph = webkit_dom_document_create_element (document,
                                                        "p",
                                                        NULL);
        webkit_dom_element_set_attribute (paragraph, "id", source_name, NULL);
        webkit_dom_element_set_attribute (paragraph,
                                          "class",
                                          "ui-widget-content",
                                          NULL);

        webkit_dom_node_set_text_content (WEBKIT_DOM_NODE (paragraph),
                                          g_strdup_printf ("+ %s", source_name),
                                          NULL);
        webkit_dom_node_append_child (sources_node,
                                      WEBKIT_DOM_NODE (paragraph),
                                      NULL);
        g_signal_connect(paragraph,
                         "click-event",
                         G_CALLBACK(source_clicked_cb),
                         GRL_METADATA_SOURCE (source));
}


static void
load_grilo_plugins ()
{
        webkit_dom_node_set_text_content (
                main_node,
                "Grilo plugins",
                NULL);

        /* Load grilo plugins */
        registry = grl_plugin_registry_get_default ();

        g_signal_connect (registry,
                          "source-added",
                          G_CALLBACK (source_added_cb),
                          NULL);

        if (!grl_plugin_registry_load_all (registry)) {
                g_error ("Failed to load plugins.");
        }
}

static void
fill_about (WebKitDOMNode *about_node)
{
        WebKitDOMNode *about_dialog_node = NULL;
        WebKitDOMElement *icon = NULL;
        WebKitDOMElement *element = NULL;
        gchar *text = NULL;

        icon = webkit_dom_document_create_element (document, "img", NULL);
        webkit_dom_element_set_attribute (icon, "src", "/usr/share/icons/Tango/32x32/apps/help-browser.png", NULL);
        webkit_dom_element_set_attribute (icon, "title", "About", NULL);
        webkit_dom_element_set_attribute (icon, "onClick", "$('#about_dialog').dialog('open');", NULL);
        webkit_dom_node_append_child (about_node,
                                      WEBKIT_DOM_NODE (icon),
                                      NULL);

        element = webkit_dom_document_create_element (document, "p", NULL);
        text = g_strdup_printf ("%s - %s",
                                PACKAGE_STRING,
                                "Desktop application developed in HTML using " \
                                "WebKitGTK+ to play multimedia content provided by Grilo.");
        webkit_dom_node_set_text_content (WEBKIT_DOM_NODE (element),
                                          text,
                                          NULL);

        about_dialog_node = WEBKIT_DOM_NODE (
                webkit_dom_document_get_element_by_id (document, "about_dialog"));
        webkit_dom_node_append_child (about_dialog_node,
                                      WEBKIT_DOM_NODE (element),
                                      NULL);
}

static void
web_view_loaded_cb (WebKitWebView *view,
                    WebKitWebFrame *frame,
                    gpointer user_data)
{
        WebKitDOMNode *about_node = NULL;

        document = webkit_web_view_get_dom_document (view);

        sources_node = WEBKIT_DOM_NODE (
                webkit_dom_document_get_element_by_id (document, "sources"));
        main_node = WEBKIT_DOM_NODE (
                webkit_dom_document_get_element_by_id (document, "main"));
        about_node = WEBKIT_DOM_NODE (
                webkit_dom_document_get_element_by_id (document, "about"));

        /* Initi DOM */
        breadcrumbs_set_last (NULL);
        fill_about (about_node);

        load_grilo_plugins ();
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
        gtk_window_set_title (GTK_WINDOW (main_window), "Web Grilo Player");
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
