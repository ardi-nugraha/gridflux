/*
 * This file is part of gridflux.
 *
 * gridflux is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * gridflux is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with gridflux.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2025 Ardi Nugraha
 */

#ifndef MINE_H
#define MINE_H

#include "tree.h"
#include <stdio.h>
#include <stdlib.h>
#define WNCK_I_KNOW_THIS_IS_UNSTABLE
#include <gtk/gtk.h>
#include <libwnck/libwnck.h>
#include <time.h>
#include <xcb/xcb.h>

#define MAX_WINDOWS_PER_WORKSPACE 5
#define POLLING_INTERVAL 300

#ifdef SESSION_KDE
#define IS_KDE 1
#else
#define IS_KDE 0
#endif

typedef struct {
  WnckHandle *handle;
  WnckScreen *screen;
  xcb_connection_t *xcb_conn;
  xcb_screen_t *xcb_screen;
  TreeNode *tree;
} WindowArranger;

gboolean on_window_opened(gpointer data);

#endif
