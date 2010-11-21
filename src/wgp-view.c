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
        WebKitDOMNode *main_node;
};

G_DEFINE_TYPE (WgpView, wgp_view, G_TYPE_OBJECT);

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
}

void
wgp_view_set_main_node (WgpView *view, WebKitDOMNode *main_node)
{
        g_return_if_fail (main_node != NULL);

        view->priv->main_node = main_node;
}

static void
source_added_cb (GrlPluginRegistry *registry, GrlMediaPlugin *source, WgpView *view)
{
        WebKitDOMElement *p;

        const gchar *source_name;
        source_name = grl_metadata_source_get_name (GRL_METADATA_SOURCE (source));
        g_debug ("Detected new source available: '%s'", source_name);

        p = webkit_dom_document_create_element (view->priv->document, "P", NULL);
        webkit_dom_node_set_text_content (WEBKIT_DOM_NODE (p),
                                          g_strdup_printf ("Plugin: %s", source_name),
                                          NULL);
        webkit_dom_node_append_child (view->priv->sources_node,
                                      WEBKIT_DOM_NODE (p),
                                      NULL);
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
