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

#include "tree.h"
#include <stdio.h>
TreeNode *create_node(WnckWindow *window) {
  TreeNode *node = (TreeNode *)malloc(sizeof(TreeNode));
  node->left = NULL;
  node->right = NULL;
  node->parent = NULL;
  node->window = window;
  node->ratio = 0.5; // Default to 50/50 split
  node->x = 0;
  node->y = 0;
  node->width = 0;
  node->height = 0;
  return node;
}

WindowTree *init_window_tree(int screen_width, int screen_height) {
  WindowTree *tree = (WindowTree *)malloc(sizeof(WindowTree));
  tree->root = NULL;
  tree->screen_width = screen_width;
  tree->screen_height = screen_height - 5;
  return tree;
}

void calculate_dimensions(TreeNode *node, int depth) {
  if (!node)
    return;

  if (node->left && node->right) {
    if (node->split == SPLIT_HORIZONTAL) {
      int split_pos = (int)(node->height * node->ratio);

      // Left child (top)
      node->left->x = node->x;
      node->left->y = node->y;
      node->left->width = node->width;
      node->left->height = split_pos;

      // Right child (bottom) with optional gap
      node->right->x = node->x;
      node->right->y = node->y + split_pos;
      node->right->width = node->width;
      node->right->height = node->height - split_pos;
    } else {
      int split_pos = (int)(node->width * node->ratio);

      // Left child
      node->left->x = node->x;
      node->left->y = node->y;
      node->left->width = split_pos - 5;
      node->left->height = node->height;

      // Right child
      node->right->x = node->x + split_pos;
      node->right->y = node->y;
      node->right->width = node->width - split_pos;
      node->right->height = node->height;
    }

    calculate_dimensions(node->left, depth + 1);
    calculate_dimensions(node->right, depth + 1);
  }
}

int compare_tree(TreeNode *firstTree, TreeNode *secondTree) {

  if (!firstTree && !secondTree)
    return 1;

  if (!firstTree || !secondTree)
    return 0;

  if (firstTree->x != secondTree->x || firstTree->y != secondTree->y ||
      firstTree->width != secondTree->width ||
      firstTree->height != secondTree->height) {
    return 0;
  }

  int leftEqual = compare_tree(firstTree->left, secondTree->left);
  int rightEqual = compare_tree(firstTree->right, secondTree->right);

  return leftEqual && rightEqual;
}

TreeNode *copy_tree(TreeNode *node) {
  if (!node)
    return NULL;

  TreeNode *new_node = (TreeNode *)malloc(sizeof(TreeNode));
  new_node->x = node->x;
  new_node->y = node->y;
  new_node->width = node->width;
  new_node->height = node->height;
  new_node->split = node->split;
  new_node->ratio = node->ratio;

  new_node->left = copy_tree(node->left);
  new_node->right = copy_tree(node->right);

  return new_node;
}

TreeNode *insert_window(WindowTree *tree, WnckWindow *window) {
  TreeNode *new_node = create_node(window);

  if (!tree->root) {
    // First window takes full screen
    tree->root = new_node;
    new_node->width = tree->screen_width;
    new_node->height = tree->screen_height;
    return new_node;
  }

  // Find a leaf node to split
  TreeNode *current = tree->root;
  while (current->left || current->right) {
    if (!current->right)
      current = current->left;
    else if (!current->left)
      current = current->right;
    else
      current = current->right;
  }

  // Create new parent node
  TreeNode *new_parent = create_node(NULL);
  new_parent->width = current->width;
  new_parent->height = current->height;
  new_parent->x = current->x;
  new_parent->y = current->y;

  // Decide split direction based on dimensions
  new_parent->split =
      (current->width > current->height) ? SPLIT_VERTICAL : SPLIT_HORIZONTAL;

  // Set up the split
  if (current == tree->root) {
    tree->root = new_parent;
  } else {
    if (current->parent->left == current)
      current->parent->left = new_parent;
    else
      current->parent->right = new_parent;
  }

  new_parent->parent = current->parent;
  new_parent->left = current;
  new_parent->right = new_node;
  current->parent = new_parent;
  new_node->parent = new_parent;

  // Calculate new dimensions
  calculate_dimensions(tree->root, 0);

  return new_node;
}

// Apply window layout from tree
void apply_tree_layout(TreeNode *node) {
  if (!node)
    return;

  if (node->window) {
    wnck_window_unmaximize(node->window);
    wnck_window_set_geometry(node->window, WNCK_WINDOW_GRAVITY_STATIC,
                             WNCK_WINDOW_CHANGE_X | WNCK_WINDOW_CHANGE_Y |
                                 WNCK_WINDOW_CHANGE_WIDTH |
                                 WNCK_WINDOW_CHANGE_HEIGHT,
                             node->x, node->y, node->width, node->height);
  }

  apply_tree_layout(node->left);
  apply_tree_layout(node->right);
}

void free_tree(TreeNode *node) {
  if (!node)
    return;
  free_tree(node->left);
  free_tree(node->right);
  free(node);
}
