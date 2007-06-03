/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2007 Collabora Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * Authors: Xavier Claessens <xclaesse@gmail.com>
 */

#include <config.h>

#include <stdlib.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include <libgnome/gnome-program.h>
#include <libgnomeui/gnome-ui-init.h>

#include <libtelepathy/tp-conn.h>
#include <libtelepathy/tp-chan.h>
#include <libmissioncontrol/mc-account.h>

#include <libempathy/gossip-contact.h>
#include <libempathy/gossip-debug.h>
#include <libempathy/gossip-utils.h>
#include <libempathy/empathy-chandler.h>
#include <libempathy/empathy-tp-chat.h>
#include <libempathy-gtk/gossip-private-chat.h>
#include <libempathy-gtk/gossip-group-chat.h>

#define DEBUG_DOMAIN "ChatMain"

#define BUS_NAME "org.gnome.Empathy.Chat"
#define OBJECT_PATH "/org/freedesktop/Telepathy/ChannelHandler"

/* Time to wait before exit, in seconds */
#define EXIT_TIMEOUT 5


static guint    chat_count = 0;
static guint    exit_timeout = 0;
static gboolean debug_mode = FALSE;

static gboolean
exit_timeout_cb (gpointer user_data)
{
	gossip_debug (DEBUG_DOMAIN, "Timeout, exiting");

	gtk_main_quit ();

	return FALSE;
}

static void
exit_timeout_start (void)
{
	if (exit_timeout || debug_mode) {
		return;
	}

	exit_timeout = g_timeout_add (EXIT_TIMEOUT * 1000,
				      (GSourceFunc) exit_timeout_cb,
				      NULL);
}

static void
exit_timeout_stop (void)
{
	if (exit_timeout) {
		gossip_debug (DEBUG_DOMAIN, "Exit timeout canceled");
		g_source_remove (exit_timeout);
		exit_timeout = 0;
	}
}

static void
chat_finalized_cb (gpointer    user_data,
		   GossipChat *chat)
{
	chat_count--;
	if (chat_count == 0) {
		gossip_debug (DEBUG_DOMAIN, "No more chat, start exit timeout");
		exit_timeout_start ();
	}
}

static void
new_channel_cb (EmpathyChandler *chandler,
		TpConn          *tp_conn,
		TpChan          *tp_chan,
		gpointer         user_data)
{
	MissionControl *mc;
	McAccount      *account;
	GossipChat     *chat;
	gchar          *id;

	mc = gossip_mission_control_new ();
	account = mission_control_get_account_for_connection (mc, tp_conn, NULL);
	id = gossip_get_channel_id (account, tp_chan);
	chat = gossip_chat_window_find_chat (account, id);

	g_free (id);
	g_object_unref (mc);

	if (chat) {
		/* The chat already exists */
		if (!gossip_chat_is_connected (chat)) {
			EmpathyTpChat *tp_chat;

			/* The chat died, give him the new text channel */
			tp_chat = empathy_tp_chat_new (account, tp_chan);
			gossip_chat_set_tp_chat (chat, tp_chat);
			g_object_unref (tp_chat);
		}
		gossip_chat_present (chat);

		g_object_unref (account);
		return;
	}

	if (tp_chan->handle_type == TP_HANDLE_TYPE_CONTACT) {
		/* We have a new private chat channel */
		chat = GOSSIP_CHAT (gossip_private_chat_new (account, tp_chan));
	}
	else if (tp_chan->handle_type == TP_HANDLE_TYPE_ROOM) {
		/* We have a new group chat channel */
		chat = GOSSIP_CHAT (gossip_group_chat_new (account, tp_chan));
	}

	g_object_weak_ref (G_OBJECT (chat),
			   (GWeakNotify) chat_finalized_cb,
			   NULL);

	exit_timeout_stop ();
	chat_count++;

	gossip_chat_present (GOSSIP_CHAT (chat));

	g_object_unref (chat);
	g_object_unref (account);

}

int
main (int argc, char *argv[])
{
	EmpathyChandler *chandler;
	GnomeProgram    *program;

	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	program = gnome_program_init ("empathy-chat",
				      PACKAGE_VERSION,
				      LIBGNOMEUI_MODULE,
				      argc, argv,
				      GNOME_PROGRAM_STANDARD_PROPERTIES,
				      GNOME_PARAM_HUMAN_READABLE_NAME, PACKAGE_NAME,
				      NULL);

	gtk_window_set_default_icon_name ("empathy");
	gtk_icon_theme_append_search_path (gtk_icon_theme_get_default (),
					   DATADIR G_DIR_SEPARATOR_S "empathy");

	if (g_getenv ("EMPATHY_DEBUG")) {
		debug_mode = TRUE;
	}

	//sexit_timeout_start ();
	chandler = empathy_chandler_new (BUS_NAME, OBJECT_PATH);

	g_signal_connect (chandler, "new-channel",
			  G_CALLBACK (new_channel_cb),
			  NULL);

	gtk_main ();

	g_object_unref (program);
	g_object_unref (chandler);

	return EXIT_SUCCESS;
}

