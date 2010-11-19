/*
 * wgp-view.h: Manage application view
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

#ifndef __WGP_VIEW_H__
#define __WGP_VIEW_H__

#include <webkit/webkit.h>


G_BEGIN_DECLS

#define WGP_TYPE_VIEW (wgp_view_get_type ())
#define WGP_VIEW(obj)                                                   \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), WGP_TYPE_VIEW, WgpView))
#define WGP_IS_VIEW(obj)                                        \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), WGP_TYPE_VIEW))
#define WGP_VIEW_CLASS(klass)                                           \
        (G_TYPE_CHECK_CLASS_CAST ((klass), WGP_TYPE_VIEW, WgpViewClass))
#define WGP_IS_VIEW_CLASS(klass)                                \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), WGP_TYPE_VIEW))
#define WGP_VIEW_GET_CLASS(obj)                                         \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), WGP_TYPE_VIEW, WgpViewClass))

typedef struct _WgpView WgpView;
typedef struct _WgpViewClass WgpViewClass;
typedef struct _WgpViewPrivate WgpViewPrivate;

struct _WgpView {
        GObject parent_instance;

        /*< private >*/
        WgpViewPrivate *priv;
};

struct _WgpViewClass {
        GObjectClass parent_class;
};

GType
wgp_view_get_type (void);

WgpView *
wgp_view_new (WebKitDOMNode *sources_node, WebKitDOMNode *main_node);

#endif /* __WGP_VIEW_H__ */
