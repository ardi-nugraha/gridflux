#include "gridflux.h"
#include "tree.h"
#include <stdio.h>
#include <stdlib.h>

static void get_screen_size(int *width, int *height) {
  GdkDisplay *display = gdk_display_get_default();
  GdkMonitor *primary_monitor = gdk_display_get_primary_monitor(display);
  GdkRectangle monitor_geometry;
  gdk_monitor_get_geometry(primary_monitor, &monitor_geometry);
  *width = monitor_geometry.width;
  *height = monitor_geometry.height;
}

static int count_windows_in_workspace(WnckScreen *screen,
                                      WnckWorkspace *workspace) {
  GList *windows = wnck_screen_get_windows(screen);
  int count = 0;

  for (GList *l = windows; l != NULL; l = l->next) {
    WnckWindow *window = WNCK_WINDOW(l->data);
    if (!wnck_window_is_minimized(window) &&
        wnck_window_get_window_type(window) == WNCK_WINDOW_NORMAL &&
        wnck_window_get_workspace(window) == workspace) {
      count++;
    }
  }

  return count;
}

static WnckWorkspace *create_new_workspace(WindowArranger *arranger) {
  int current_count = wnck_screen_get_workspace_count(arranger->screen);

  xcb_connection_t *conn = arranger->xcb_conn;
  xcb_screen_t *screen = arranger->xcb_screen;

  xcb_intern_atom_cookie_t cookie = xcb_intern_atom(
      conn, 0, strlen("_NET_NUMBER_OF_DESKTOPS"), "_NET_NUMBER_OF_DESKTOPS");
  xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(conn, cookie, NULL);

  if (reply) {
    xcb_atom_t atom = reply->atom;
    uint32_t new_count = current_count + 1;

    // Send client message to change number of desktops
    xcb_client_message_event_t event = {.response_type = XCB_CLIENT_MESSAGE,
                                        .format = 32,
                                        .window = screen->root,
                                        .type = atom,
                                        .data.data32[0] = new_count};

    xcb_send_event(conn, 0, screen->root,
                   XCB_EVENT_MASK_STRUCTURE_NOTIFY |
                       XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT,
                   (char *)&event);
    xcb_flush(conn);
    free(reply);

    // Wait a bit for the workspace to be created
    g_usleep(500);

    WnckWorkspace *new_workspace =
        wnck_screen_get_workspace(arranger->screen, new_count);
    if (new_workspace) {
      return new_workspace;
    }
  }

  return NULL;
}

static void move_window_to_workspace(WnckWindow *window,
                                     WnckWorkspace *workspace) {
  wnck_window_move_to_workspace(window, workspace);
}

// Updated arrange_windows function using tree
static void arrange_windows(WindowArranger *arranger) {
  WnckWorkspace *active_workspace =
      wnck_screen_get_active_workspace(arranger->screen);
  if (!active_workspace)
    return;

  GList *windows = wnck_screen_get_windows(arranger->screen);
  GList *active_windows = NULL;
  int screen_width, screen_height;

  get_screen_size(&screen_width, &screen_height);

  int top_dock = 0, bot_dock = 0, left_dock = 0, right_dock = 0;
  if (IS_KDE) {
    for (GList *l = windows; l != NULL; l = l->next) {
      WnckWindow *window = WNCK_WINDOW(l->data);

      if (wnck_window_is_skip_tasklist(
              window)) { // Panels usually skip the tasklist

        if (wnck_window_is_skip_pager(window)) {
          WnckWindowType type = wnck_window_get_window_type(window);
          if (type == WNCK_WINDOW_DOCK) {

            GdkRectangle geometry;
            wnck_window_get_geometry(window, &geometry.x, &geometry.y,
                                     &geometry.width, &geometry.height);

            if (geometry.y == 0) {
              top_dock += geometry.height;
            } else if (geometry.y + geometry.height >= gdk_screen_height()) {
              bot_dock += geometry.height;
            } else if (geometry.x == 0) {
              left_dock += geometry.height;
            } else if (geometry.x + geometry.width >= gdk_screen_width()) {
              right_dock += geometry.height;
            }
          }
        }
      }
    }
    if (bot_dock > 0 && top_dock == 0) {
      screen_height = screen_height - bot_dock;
    } else if (top_dock > 0 || bot_dock > 0) {
      screen_height = screen_height - bot_dock - top_dock + 15;
    }
  }

  WindowTree *tree = init_window_tree(screen_width, screen_height);

  for (GList *l = windows; l != NULL; l = l->next) {
    WnckWindow *window = WNCK_WINDOW(l->data);

    if (!wnck_window_is_minimized(window) &&
        wnck_window_get_window_type(window) == WNCK_WINDOW_NORMAL &&
        wnck_window_get_workspace(window) == active_workspace) {
      active_windows = g_list_append(active_windows, window);

      int x, y, width, height;
      wnck_window_get_geometry(window, &x, &y, &width, &height);

      insert_window(tree, window);
      if (IS_KDE) {
        tree->root->y = top_dock > 0 ? top_dock - 15 : 15;
      }
    }
  }

  if (active_windows == NULL) {
    free_tree(tree->root);

    free(tree);
    return;
  }

  if (arranger->tree) {
    int sameTree = compare_tree(tree->root, arranger->tree);
    if (sameTree == 0) {
      apply_tree_layout(tree->root);
    }
  } else {
    apply_tree_layout(tree->root);
  }
  arranger->tree = copy_tree(tree->root);

  free_tree(tree->root);

  free(tree);
  g_list_free(active_windows);
}

gboolean on_window_opened(gpointer data) {
  WindowArranger *arranger = (WindowArranger *)data;
  WnckScreen *screen = arranger->screen;

  if (!screen) {
    g_warning("Failed to retrieve WnckScreen.");
    return FALSE;
  }

  WnckWorkspace *active_workspace = wnck_screen_get_active_workspace(screen);
  if (!active_workspace) {
    g_warning("No active workspace found.");
    return TRUE;
  }

  GList *windows = wnck_screen_get_windows(screen);
  if (!windows) {
    g_warning("No windows found.");
    return TRUE;
  }

  WnckWindow *new_window = (WnckWindow *)g_list_last(windows)->data;
  if (!new_window) {
    g_warning("Failed to retrieve the last opened window.");
    return TRUE;
  }

  GArray *free_workspaces = g_array_new(FALSE, FALSE, sizeof(WnckWorkspace *));
  GArray *overloaded_workspaces =
      g_array_new(FALSE, FALSE, sizeof(WnckWorkspace *));

  int total_workspaces = wnck_screen_get_workspace_count(screen);
  for (int i = 0; i < total_workspaces; i++) {
    WnckWorkspace *workspace = wnck_screen_get_workspace(screen, i);
    if (!workspace)
      continue;

    int count = count_windows_in_workspace(screen, workspace);
    if (count > MAX_WINDOWS_PER_WORKSPACE) {
      g_array_append_val(overloaded_workspaces, workspace);
    } else if (count < MAX_WINDOWS_PER_WORKSPACE) {
      g_array_append_val(free_workspaces, workspace);
    }
  }

  // If no free workspaces exist, create a new one
  if (free_workspaces->len == 0) {
    create_new_workspace(arranger);
    WnckWorkspace *new_workspace =
        wnck_screen_get_workspace(screen, total_workspaces);
    g_array_append_val(free_workspaces, new_workspace);
  }

  // Process overloaded workspaces
  for (int i = 0; i < overloaded_workspaces->len; i++) {
    WnckWorkspace *overloaded =
        g_array_index(overloaded_workspaces, WnckWorkspace *, i);
    int excess_windows = count_windows_in_workspace(screen, overloaded) -
                         MAX_WINDOWS_PER_WORKSPACE;

    if (excess_windows > 0) {
      GList *workspace_windows = wnck_screen_get_windows(screen);
      GList *windows_to_move = NULL;

      for (GList *l = g_list_last(workspace_windows);
           l != NULL && excess_windows > 0; l = l->prev) {
        WnckWindow *window = (WnckWindow *)l->data;
        if (!window)
          continue;

        if (wnck_window_get_workspace(window) == overloaded) {
          windows_to_move = g_list_prepend(windows_to_move, window);
          excess_windows--;
        }
      }

      for (GList *m = windows_to_move; m != NULL; m = m->next) {
        WnckWindow *window_to_move = (WnckWindow *)m->data;
        WnckWorkspace *target_workspace =
            g_array_index(free_workspaces, WnckWorkspace *, 0);

        move_window_to_workspace(window_to_move, target_workspace);

        if (count_windows_in_workspace(screen, target_workspace) >=
            MAX_WINDOWS_PER_WORKSPACE) {
          g_array_remove_index(free_workspaces, 0);
          if (free_workspaces->len == 0) {
            create_new_workspace(arranger);
            target_workspace =
                wnck_screen_get_workspace(screen, total_workspaces);
            g_array_append_val(free_workspaces, target_workspace);
          }
        }
      }

      // Free the list of windows to move
      g_list_free(windows_to_move);
    }
  }

  // Cleanup
  g_array_free(free_workspaces, TRUE);
  g_array_free(overloaded_workspaces, TRUE);

  arrange_windows(arranger);

  return TRUE;
}
