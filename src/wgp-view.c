/*
 * wgp-view.c: Manage application view
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

/**
 * SECTION:wgp-view
 * @short_description: A class to manage application view
 *
 */

#include <wgp-view.h>
#include <grilo.h>

#define WGP_VIEW_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE (        \
                                           (obj), WGP_TYPE_VIEW, WgpViewPrivate))

struct _WgpViewPrivate {
        WebKitDOMDocument *document;
        WebKitDOMNode *sources_node;
        WebKitDOMElement *sources_node_ul;
        WebKitDOMNode *main_node;
        WebKitDOMElement *main_node_p;
};

G_DEFINE_TYPE (WgpView, wgp_view, G_TYPE_OBJECT);


typedef struct WgpViewAndSourceStrut {
        WgpView *view;
        GrlMetadataSource *source;
} WgpViewAndSourceStrut;

typedef struct WgpViewAndMediaStrut {
        WgpView *view;
        GrlMedia *media;
} WgpViewAndMediaStrut;


/* Private methods headers */
void
wgp_view_load_plugins (WgpView *view);


static void
wgp_view_dispose (GObject *gobject)
{
        WgpViewPrivate *priv = WGP_VIEW_GET_PRIVATE (WGP_VIEW (gobject));

        g_object_unref (priv->document);
        g_object_unref (priv->sources_node);
        g_object_unref (priv->main_node);

        G_OBJECT_CLASS (wgp_view_parent_class)->dispose (gobject);
}

static void
wgp_view_finalize (GObject *gobject)
{
        G_OBJECT_CLASS (wgp_view_parent_class)->finalize (gobject);
}

static void
wgp_view_class_init (WgpViewClass *klass)
{
        GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private (klass, sizeof (WgpViewPrivate));

        gobject_class->finalize = wgp_view_finalize;
        gobject_class->dispose = wgp_view_dispose;

}

static void
wgp_view_init (WgpView *view)
{
        view->priv = WGP_VIEW_GET_PRIVATE (view);
        view->priv->sources_node = NULL;
        view->priv->main_node = NULL;
}

WgpView *
wgp_view_new ()
{
        return g_object_new (WGP_TYPE_VIEW, NULL);
}

void
wgp_view_set_document (WgpView *view, WebKitDOMDocument *document)
{
        g_return_if_fail (document != NULL);

        view->priv->document = document;
}

void
wgp_view_set_sources_node (WgpView *view, WebKitDOMNode *sources_node)
{
        g_return_if_fail (sources_node != NULL);

        view->priv->sources_node = sources_node;

        view->priv->sources_node_ul =
                webkit_dom_document_create_element (view->priv->document,
                                                    "ul",
                                                    NULL);
        webkit_dom_node_append_child (view->priv->sources_node,
                                      WEBKIT_DOM_NODE (view->priv->sources_node_ul),
                                      NULL);
}

void
wgp_view_set_main_node (WgpView *view, WebKitDOMNode *main_node)
{
        g_return_if_fail (main_node != NULL);

        view->priv->main_node = main_node;

        view->priv->main_node_p = webkit_dom_document_create_element (view->priv->document, "p", NULL);
        webkit_dom_node_append_child (view->priv->main_node,
                                      WEBKIT_DOM_NODE (view->priv->main_node_p),
                                      NULL);

}

static void
media_clicked_cb (WebKitDOMEventTarget* target,
                  WebKitDOMEvent* event,
                  WgpViewAndMediaStrut* view_and_media)
{
        WgpView *view;
        GrlMedia *media;
        WebKitDOMElement *video;
        const gchar *title;
        const gchar *url;

        view = view_and_media->view;
        media = view_and_media->media;

        title = grl_media_get_title (media);
        g_debug ("Media clicked: '%s'", title);

        url = grl_media_get_url (media);

        webkit_dom_node_set_text_content (WEBKIT_DOM_NODE (view->priv->main_node_p),
                                          g_strdup_printf ("Media selected: %s - URL: %s", title, url),
                                          NULL);

        video = webkit_dom_document_create_element (view->priv->document, "video", NULL);
        webkit_dom_element_set_attribute (video, "src", url, NULL);
        webkit_dom_node_append_child (WEBKIT_DOM_NODE (view->priv->main_node_p),
                                      WEBKIT_DOM_NODE (video),
                                      NULL);

}


static void
browse_source_cb (GrlMediaSource *source,
                  guint browse_id,
                  GrlMedia *media,
                  guint remaining,
                  gpointer user_data,
                  const GError *error)
{
        WgpView *view;
        WebKitDOMElement *li;
        WebKitDOMNode *ul;
        WebKitDOMElement *new_li;
        WgpViewAndMediaStrut *view_and_media;
        const gchar *title;
        gboolean create_ul = TRUE;

        if (error) {
                g_error ("Browse operation failed. Reason: %s", error->message);
        }

        view = WGP_VIEW (user_data);

        if (media) {
                title = grl_media_get_title (media);

                li = webkit_dom_document_get_element_by_id (view->priv->document,
                                                            grl_metadata_source_get_name (GRL_METADATA_SOURCE (source)));
                if (webkit_dom_node_has_child_nodes (WEBKIT_DOM_NODE (li))) {
                        ul = webkit_dom_node_get_last_child (WEBKIT_DOM_NODE (li));

                        if (g_strcmp0 ("UL", webkit_dom_node_get_node_name (ul)) == 0) {
                                g_debug ("<ul> already created");
                                create_ul = FALSE;
                        }
                }

                if (create_ul) {
                        g_debug ("Creating <ul>");
                        ul = WEBKIT_DOM_NODE (webkit_dom_document_create_element (view->priv->document, "ul", NULL));
                        webkit_dom_node_append_child (WEBKIT_DOM_NODE (li),
                                                      ul,
                                                      NULL);
                }

                new_li = webkit_dom_document_create_element (view->priv->document, "li", NULL);
                webkit_dom_node_set_text_content (WEBKIT_DOM_NODE (new_li),
                                                  g_strdup_printf ("Media: %s", title),
                                                  NULL);
                webkit_dom_element_set_attribute (new_li, "id", title, NULL);
                webkit_dom_node_append_child (ul,
                                              WEBKIT_DOM_NODE (new_li),
                                              NULL);

                view_and_media = g_new0 (WgpViewAndMediaStrut, 1);
                view_and_media->view = view;
                view_and_media->media = media;
                g_signal_connect(li,
                                 "click-event",
                                 G_CALLBACK(media_clicked_cb),
                                 view_and_media);

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
                   WgpViewAndSourceStrut* view_and_source)
{
        WgpView *view;
        GrlMetadataSource *source;
        const gchar *source_name;
        GList * keys;

        view = view_and_source->view;
        source = view_and_source->source;

        source_name = grl_metadata_source_get_name (source);
        g_debug ("Source clicked: '%s'", source_name);

        webkit_dom_node_set_text_content (WEBKIT_DOM_NODE (view->priv->main_node_p),
                                          g_strdup_printf ("Source selected: %s", source_name),
                                          NULL);

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
                                         0, 5,
                                         GRL_RESOLVE_IDLE_RELAY,
                                         browse_source_cb,
                                         view);
        }
}

static void
source_added_cb (GrlPluginRegistry *registry, GrlMediaPlugin *source, WgpView *view)
{
        WebKitDOMElement *li, *span;
        WgpViewAndSourceStrut *view_and_source;
        const gchar *source_name;

        source_name = grl_metadata_source_get_name (GRL_METADATA_SOURCE (source));
        g_debug ("Detected new source available: '%s'", source_name);

        li = webkit_dom_document_create_element (view->priv->document, "li", NULL);
        webkit_dom_element_set_attribute (li, "id", source_name, NULL);
        webkit_dom_node_append_child (WEBKIT_DOM_NODE (view->priv->sources_node_ul),
                                      WEBKIT_DOM_NODE (li),
                                      NULL);

        span = webkit_dom_document_create_element (view->priv->document, "span", NULL);
        webkit_dom_node_set_text_content (WEBKIT_DOM_NODE (span),
                                          g_strdup_printf ("Plugin: %s", source_name),
                                          NULL);
        webkit_dom_node_append_child (WEBKIT_DOM_NODE (li),
                                      WEBKIT_DOM_NODE (span),
                                      NULL);

        view_and_source = g_new0 (WgpViewAndSourceStrut, 1);
        view_and_source->view = view;
        view_and_source->source = GRL_METADATA_SOURCE (source);
        g_signal_connect(span,
                         "click-event",
                         G_CALLBACK(source_clicked_cb),
                         view_and_source);
}

void
wgp_view_run (WgpView *view)
{
        g_assert (view != NULL);

        wgp_view_load_plugins (view);
}

void
wgp_view_load_plugins (WgpView *view)
{
        GrlPluginRegistry *registry;

        /* Load grilo plugins */
        registry = grl_plugin_registry_get_default ();

        g_signal_connect (registry,
                          "source-added",
                          G_CALLBACK (source_added_cb),
                          view);

        if (!grl_plugin_registry_load_all (registry)) {
                g_error ("Failed to load plugins.");
        }
}
