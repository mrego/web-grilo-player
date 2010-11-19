/*
 * wgp-view.c: Manage application view
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

/**
 * SECTION:wgp-view
 * @short_description: A class to manage application view
 *
 */

#include <wgp-view.h>

#define WGP_VIEW_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE (        \
                                           (obj), WGP_TYPE_VIEW, WgpViewPrivate))

struct _WgpViewPrivate {
        WebKitDOMNode *sources_node;
        WebKitDOMNode *main_node;
};

G_DEFINE_TYPE (WgpView, wgp_view, G_TYPE_OBJECT);

static void
wgp_view_dispose (GObject *gobject)
{
        WgpViewPrivate *priv = WGP_VIEW_GET_PRIVATE (WGP_VIEW (gobject));

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
wgp_view_new (WebKitDOMNode *sources_node, WebKitDOMNode *main_node)
{
        g_return_val_if_fail (sources_node != NULL, NULL);
        g_return_val_if_fail (main_node != NULL, NULL);

        return g_object_new (WGP_TYPE_VIEW,
                             "sources_node", sources_node,
                             "main_node", main_node,
                             NULL);
}
