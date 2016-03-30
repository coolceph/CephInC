/*
  Red Black Trees
  (C) 1999  Andrea Arcangeli <andrea@suse.de>
  (C) 2002  David Woodhouse <dwmw2@infradead.org>
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  linux/lib/rbtree.c
*/

#include "common/rbtree.h"

static void __cceph_rb_rotate_left(cceph_rb_node *node, cceph_rb_root *root) {
	cceph_rb_node *right = node->rb_right;
	cceph_rb_node *parent = cceph_rb_parent(node);

	if ((node->rb_right = right->rb_left)) {
		cceph_rb_set_parent(right->rb_left, node);
    }
	right->rb_left = node;

	cceph_rb_set_parent(right, parent);

	if (parent) {
		if (node == parent->rb_left) {
			parent->rb_left = right;
        } else {
			parent->rb_right = right;
        }
	} else {
		root->rb_node = right;
    }
	cceph_rb_set_parent(node, right);
}

static void __cceph_rb_rotate_right(cceph_rb_node *node, cceph_rb_root *root) {
	cceph_rb_node *left = node->rb_left;
	cceph_rb_node *parent = cceph_rb_parent(node);

	if ((node->rb_left = left->rb_right)) {
		cceph_rb_set_parent(left->rb_right, node);
    }
	left->rb_right = node;

	cceph_rb_set_parent(left, parent);

	if (parent) {
		if (node == parent->rb_right) {
			parent->rb_right = left;
        } else {
			parent->rb_left = left;
        }
	} else {
		root->rb_node = left;
    }
	cceph_rb_set_parent(node, left);
}

void cceph_rb_insert_color(cceph_rb_node *node, cceph_rb_root *root) {
	cceph_rb_node *parent, *gparent;

	while ((parent = cceph_rb_parent(node)) && cceph_rb_is_red(parent)) {
		gparent = cceph_rb_parent(parent);

		if (parent == gparent->rb_left) {
			{
				register cceph_rb_node *uncle = gparent->rb_right;
				if (uncle && cceph_rb_is_red(uncle)) {
					cceph_rb_set_black(uncle);
					cceph_rb_set_black(parent);
					cceph_rb_set_red(gparent);
					node = gparent;
					continue;
				}
			}

			if (parent->rb_right == node) {
				register cceph_rb_node *tmp;
				__cceph_rb_rotate_left(parent, root);
				tmp = parent;
				parent = node;
				node = tmp;
			}

			cceph_rb_set_black(parent);
			cceph_rb_set_red(gparent);
			__cceph_rb_rotate_right(gparent, root);
		} else {
			{
				register cceph_rb_node *uncle = gparent->rb_left;
				if (uncle && cceph_rb_is_red(uncle)) {
					cceph_rb_set_black(uncle);
					cceph_rb_set_black(parent);
					cceph_rb_set_red(gparent);
					node = gparent;
					continue;
				}
			}

			if (parent->rb_left == node) {
				register cceph_rb_node *tmp;
				__cceph_rb_rotate_right(parent, root);
				tmp = parent;
				parent = node;
				node = tmp;
			}

			cceph_rb_set_black(parent);
			cceph_rb_set_red(gparent);
			__cceph_rb_rotate_left(gparent, root);
		}
	}

	cceph_rb_set_black(root->rb_node);
}

static void __cceph_rb_erase_color(
        cceph_rb_node *node,
        cceph_rb_node *parent,
        cceph_rb_root *root) {

	cceph_rb_node *other;

	while ((!node || cceph_rb_is_black(node)) && node != root->rb_node) {
		if (parent->rb_left == node) {
			other = parent->rb_right;
			if (cceph_rb_is_red(other)) {
				cceph_rb_set_black(other);
				cceph_rb_set_red(parent);
				__cceph_rb_rotate_left(parent, root);
				other = parent->rb_right;
			}
			if ((!other->rb_left || cceph_rb_is_black(other->rb_left)) &&
			    (!other->rb_right || cceph_rb_is_black(other->rb_right))) {
				cceph_rb_set_red(other);
				node = parent;
				parent = cceph_rb_parent(node);
			} else {
				if (!other->rb_right || cceph_rb_is_black(other->rb_right)) {
					cceph_rb_set_black(other->rb_left);
					cceph_rb_set_red(other);
					__cceph_rb_rotate_right(other, root);
					other = parent->rb_right;
				}
				cceph_rb_set_color(other, cceph_rb_color(parent));
				cceph_rb_set_black(parent);
				cceph_rb_set_black(other->rb_right);
				__cceph_rb_rotate_left(parent, root);
				node = root->rb_node;
				break;
			}
		} else {
			other = parent->rb_left;
			if (cceph_rb_is_red(other)) {
				cceph_rb_set_black(other);
				cceph_rb_set_red(parent);
				__cceph_rb_rotate_right(parent, root);
				other = parent->rb_left;
			}
			if ((!other->rb_left || cceph_rb_is_black(other->rb_left)) &&
			    (!other->rb_right || cceph_rb_is_black(other->rb_right))) {
				cceph_rb_set_red(other);
				node = parent;
				parent = cceph_rb_parent(node);
			} else {
				if (!other->rb_left || cceph_rb_is_black(other->rb_left)) {
					cceph_rb_set_black(other->rb_right);
					cceph_rb_set_red(other);
					__cceph_rb_rotate_left(other, root);
					other = parent->rb_left;
				}
				cceph_rb_set_color(other, cceph_rb_color(parent));
				cceph_rb_set_black(parent);
				cceph_rb_set_black(other->rb_left);
				__cceph_rb_rotate_right(parent, root);
				node = root->rb_node;
				break;
			}
		}
	}
	if (node) {
		cceph_rb_set_black(node);
    }
}

void cceph_rb_erase(cceph_rb_node *node, cceph_rb_root *root) {
	cceph_rb_node *child, *parent;
	int color;

	if (!node->rb_left) {
		child = node->rb_right;
    } else if (!node->rb_right) {
		child = node->rb_left;
    } else {
		cceph_rb_node *old = node, *left;

		node = node->rb_right;
		while ((left = node->rb_left) != NULL)
			node = left;

		if (cceph_rb_parent(old)) {
			if (cceph_rb_parent(old)->rb_left == old) {
				cceph_rb_parent(old)->rb_left = node;
            } else {
				cceph_rb_parent(old)->rb_right = node;
            }
		} else {
			root->rb_node = node;
        }

		child = node->rb_right;
		parent = cceph_rb_parent(node);
		color = cceph_rb_color(node);

		if (parent == old) {
			parent = node;
		} else {
			if (child) {
				cceph_rb_set_parent(child, parent);
            }
			parent->rb_left = child;

			node->rb_right = old->rb_right;
			cceph_rb_set_parent(old->rb_right, node);
		}

		node->rb_parent_color = old->rb_parent_color;
		node->rb_left = old->rb_left;
		cceph_rb_set_parent(old->rb_left, node);

		goto color;
	}

	parent = cceph_rb_parent(node);
	color  = cceph_rb_color(node);

	if (child) {
		cceph_rb_set_parent(child, parent);
    }
	if (parent) {
		if (parent->rb_left == node) {
			parent->rb_left = child;
        } else {
			parent->rb_right = child;
        }
    } else {
		root->rb_node = child;
    }

 color:
	if (color == CCEPH_RB_BLACK) {
		__cceph_rb_erase_color(child, parent, root);
    }
}

/*
 * This function returns the first node (in sort order) of the tree.
 */
cceph_rb_node *cceph_rb_first(const cceph_rb_root *root) {
	cceph_rb_node	*n;

	n = root->rb_node;
	if (!n) {
		return NULL;
    }
	while (n->rb_left) {
		n = n->rb_left;
    }
	return n;
}

cceph_rb_node *cceph_rb_last(const cceph_rb_root *root) {
	cceph_rb_node	*n;

	n = root->rb_node;
	if (!n) {
		return NULL;
    }
	while (n->rb_right) {
		n = n->rb_right;
    }
	return n;
}

cceph_rb_node *cceph_rb_next(const cceph_rb_node *node) {
	cceph_rb_node *parent;

	if (cceph_rb_parent(node) == node) {
		return NULL;
    }

	/* If we have a right-hand child, go down and then left as far
	   as we can. */
	if (node->rb_right) {
		node = node->rb_right;
		while (node->rb_left) {
			node = node->rb_left;
        }
		return (cceph_rb_node *)node;
	}

	/* No right-hand children.  Everything down and left is
	   smaller than us, so any 'next' node must be in the general
	   direction of our parent. Go up the tree; any time the
	   ancestor is a right-hand child of its parent, keep going
	   up. First time it's a left-hand child of its parent, said
	   parent is our 'next' node. */
	while ((parent = cceph_rb_parent(node)) && node == parent->rb_right) {
		node = parent;
    }

	return parent;
}

cceph_rb_node *cceph_rb_prev(const cceph_rb_node *node) {
	cceph_rb_node *parent;

	if (cceph_rb_parent(node) == node) {
		return NULL;
    }

	/* If we have a left-hand child, go down and then right as far
	   as we can. */
	if (node->rb_left) {
		node = node->rb_left;
		while (node->rb_right) {
			node = node->rb_right;
        }
		return (cceph_rb_node *)node;
	}

	/* No left-hand children. Go up till we find an ancestor which
	   is a right-hand child of its parent */
	while ((parent = cceph_rb_parent(node)) && node == parent->rb_left) {
		node = parent;
    }

	return parent;
}

void cceph_rb_replace_node(
        cceph_rb_node *victim,
        cceph_rb_node *new,
        cceph_rb_root *root) {

	cceph_rb_node *parent = cceph_rb_parent(victim);

	/* Set the surrounding nodes to point to the replacement */
	if (parent) {
		if (victim == parent->rb_left) {
			parent->rb_left = new;
        } else {
			parent->rb_right = new;
        }
	} else {
		root->rb_node = new;
	}
	if (victim->rb_left) {
		cceph_rb_set_parent(victim->rb_left, new);
    }
	if (victim->rb_right) {
		cceph_rb_set_parent(victim->rb_right, new);
    }

	/* Copy the pointers/colour from the victim to the replacement */
	*new = *victim;
}
