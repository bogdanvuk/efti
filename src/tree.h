/*
 * tree.h
 *
 *  Created on: Feb 9, 2015
 *      Author: bvukobratovic
 */

#ifndef TREE_H_
#define TREE_H_

#include <stdint.h>

#include "hw_config.h"

#define NODE_POOL_MAX		256
#define WEIGHTS_MAX_NUM 	(NUM_ATTRIBUTES + 1)
#define NODE_SIZE_BARE		(WEIGHTS_MAX_NUM*4 + DT_MEM_BANKS_NUM*4 + 8 + 12 + 4)
#define NODE_SIZE_POW2		512

#define CHILD_LEFT			0
#define CHILD_RIGHT			1

struct tree_el{
	TAttr weights[WEIGHTS_MAX_NUM];
	uint32_t banks[DT_MEM_BANKS_NUM];
	uint32_t level;
	uint32_t id;
	uint32_t cls;
	float impurity;
	float disbalance;
	uint32_t inst_num;
	uint32_t to_bottom;
	struct tree_el* left;
	struct tree_el* right;
	struct tree_el* parent;
//	char padding[NODE_SIZE_POW2 - NODE_SIZE_BARE];
};

typedef struct tree_el tree_node;

#define is_child_of(parent, child) (((child) == (parent)->left) || ((child) == (parent)->right))

tree_node* tree_create();
int tree_create_child(tree_node* node, uint32_t child);
int tree_delete_node(tree_node* node);
tree_node* tree_copy(tree_node* src);
int tree_delete_child(tree_node* node, uint32_t child);
tree_node* tree_get_sibling(tree_node* node);
void tree_init();
tree_node* node_alloc();
/* int is_child_of(tree_node* parent, tree_node* child); */

#endif /* TREE_H_ */
