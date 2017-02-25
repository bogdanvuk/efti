/*
 * tree.c
 *
 *  Created on: Feb 9, 2015
 *      Author: bvukobratovic
 */

#include "tree.h"
#include <stdlib.h>

tree_node node_pool[NODE_POOL_MAX];
uint32_t node_pool_alloc[NODE_POOL_MAX];
uint32_t node_pool_next_free = 0;

void tree_init()
{
	unsigned i;
	node_pool_next_free = 0;

	for (i = 0; i < NODE_POOL_MAX; i++)
	{
		node_pool_alloc[i] = 0;
	}
}

tree_node* node_alloc()
{
    return (tree_node*) malloc(sizeof(tree_node));
//	while (node_pool_alloc[node_pool_next_free])
//	{
//		node_pool_next_free++;
//	}
//
//	node_pool_alloc[node_pool_next_free] = 1;
//	return &node_pool[node_pool_next_free];
}

void node_free(tree_node* node)
{
	free(node);
////	uint32_t node_index = (((uint32_t) node) - ((uint32_t) node_pool)) >> NODE_SIZE_BITS;
//	uint32_t node_index = node - node_pool;
//
//	node_pool_alloc[node_index] = 0;
//
//	if (node_index < node_pool_next_free)
//	{
//		node_pool_next_free = node_index;
//	}
}

tree_node* tree_create()
{
	tree_node* root;
//	root = malloc(sizeof(tree_node));
	root = node_alloc();
//	xil_printf("Malloc %x\n\r", (uint32_t) root);
	root->left = NULL;
	root->right = NULL;
  root->parent = NULL;
  root->to_bottom = 1;

	return root;
}

tree_node* tree_copy(tree_node* src)
{
	tree_node* node = node_alloc(); //malloc(sizeof(tree_node));
//	xil_printf("Malloc %x\n\r", (uint32_t) node);
	*node = *src;

	if (src->left != NULL)
	{
		node->left = tree_copy(src->left);
		node->left->parent = node;
	}
	else
	{
		node->left = NULL;
	}

	if (src->right != NULL)
	{
		node->right = tree_copy(src->right);
		node->right->parent = node;
	}
	else
	{
		node->right = NULL;
	}

	return node;
}

int tree_create_child(tree_node* node, uint32_t child)
{
	tree_node* child_node = tree_create();

	child_node->parent = node;

	if (child == CHILD_LEFT)
	{
		node->left = child_node;
	}
	else
	{
		node->right = child_node;
	}

	return 0;
}

tree_node* tree_get_sibling(tree_node* node)
{
	if (node->parent->left == node)
	{
		return node->parent->right;
	}
	else
	{
		return node->parent->left;
	}
}

int tree_delete_node(tree_node* node)
{
	if (node->left != NULL)
	{
		tree_delete_node(node->left);
	}

	if (node->right != NULL)
	{
		tree_delete_node(node->right);
	}

//	xil_printf("Free %x\n\r", (uint32_t) node);
	//free(node);
	node_free(node);

	return 0;
}

/* int is_child_of(tree_node* parent, tree_node* child) */
/* { */
/*     return (child == parent->left) || (child == parent->right); */
/* } */

int tree_delete_child(tree_node* node, uint32_t child)
{
	if (child == CHILD_LEFT)
	{
		if (node->left != NULL)
		{
			tree_delete_node(node->left);
			node->left = NULL;
		}
	}
	else
	{
		if (node->right != NULL)
		{
			tree_delete_node(node->right);
			node->right = NULL;
		}
	}

	return 0;
}



