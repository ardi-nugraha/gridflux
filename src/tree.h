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

#ifndef TREE
#define TREE

#define WNCK_I_KNOW_THIS_IS_UNSTABLE
#include <libwnck/libwnck.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum { SPLIT_HORIZONTAL, SPLIT_VERTICAL } SplitDirection;

// Node in the window tree
typedef struct TreeNode {
  struct TreeNode *left;
  struct TreeNode *right;
  struct TreeNode *parent;
  WnckWindow *window;
  SplitDirection split;
  double ratio;      // Split ratio (0.0 - 1.0)
  int x, y;          // Position
  int width, height; // Dimensions
} TreeNode;

typedef struct {
  TreeNode *root;
  int screen_width;
  int screen_height;
} WindowTree;

TreeNode *create_node(WnckWindow *window);
WindowTree *init_window_tree(int screen_width, int screen_height);
void calculate_dimensions(TreeNode *node, int depth);
TreeNode *insert_window(WindowTree *tree, WnckWindow *window);
void apply_tree_layout(TreeNode *node);
void free_tree(TreeNode *node);
int compare_tree(TreeNode *firstTree, TreeNode *secondTree);
TreeNode *copy_tree(TreeNode *node);

#endif
