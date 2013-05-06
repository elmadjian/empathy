/*
 * Copyright (C) 2009-2013 Collabora Ltd.
 *
 * Authors: Marco Barisione <marco.barisione@collabora.co.uk>
 *          Guillaume Desmottes <guillaume.desmottes@collabora.co.uk>
 *          Sjoerd Simons <sjoerd.simons@collabora.co.uk>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __TPAW_UTILS_H__
#define __TPAW_UTILS_H__

#include <glib.h>
#include <telepathy-glib/telepathy-glib.h>

G_BEGIN_DECLS

void tpaw_connect_new_account (TpAccount *account,
    TpAccountManager *account_manager);

gchar *tpaw_protocol_icon_name (const gchar *protocol);
const gchar *tpaw_protocol_name_to_display_name (const gchar *proto_name);
const gchar *tpaw_service_name_to_display_name (const gchar *proto_name);

G_END_DECLS

#endif /*  __TPAW_UTILS_H__ */
