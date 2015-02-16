#ifndef __DECISION_TREE_H
#define __DECISION_TREE_H

#include <stdint.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <string>
#include <stdlib.h>

#define NODE_ID_INVALID     0xffff
#define DT_USE_LOOP_UNFOLD  1

using namespace std;

typedef uint_fast16_t   T_Node_Weight;
typedef uint_fast16_t   T_Attributes;

class DecisionTree 
{
        int32_t**       node_weights;
        bool*           node_is_leaf;
        int32_t*        node_thresholds;
        uint_fast16_t** node_categories_distrib;
//        uint_fast16_t 	node_categories_distrib[256][64];
        uint_fast16_t*  node_categories;
        uint_fast16_t*  classify_path;
        uint_fast16_t*  traverse_path;
        uint_fast16_t*  mirror_path;
        uint_fast16_t   traverse_cnt;
        uint32_t*		classes;
        
        uint_fast16_t*  active_nodes_array;
        uint_fast16_t   active_nodes_cnt;         
        uint_fast16_t*  leaves_array;
        uint_fast16_t   leaves_cnt;
        uint_fast16_t*  non_leaves_array;
        uint_fast16_t   non_leaves_cnt;
        
        uint_fast16_t   attr_cnt; 
        uint_fast16_t   nodes_max;
        uint_fast16_t   max_tree_depth;
        uint_fast16_t   categ_cnt;
        
        bool            classify_out_flag;
        
        void allocate_nodes(int nodes_max, int attr_cnt, int categ_cnt);
    public:
        DecisionTree(int attr_cnt, int categ_max, int max_tree_depth);
        ~DecisionTree();
        
        void dump_to_oc1_file(const char* oc1_file_name);
        
        int classify(int32_t attributes[], uint_fast16_t &leaf_node);
        int find_category_distribution(int32_t* instances[], uint_fast16_t categories[], uint_fast16_t inst_cnt);
        int assign_categories_based_on_distribution(int32_t* instances[], uint_fast16_t categories[], uint_fast16_t inst_cnt);
        int hw_eval(int32_t* instances[], uint_fast16_t categories[], uint_fast16_t inst_cnt);
        void traverse(uint_fast16_t root, uint_fast16_t traverse_path[], uint_fast16_t &traverse_cnt, bool include_leaves, bool include_non_leaves, bool make_mirror_path=false, uint_fast16_t mirror_node=0, uint_fast16_t* mirror_path=NULL);
        void add_children_to_leaf_node(uint_fast16_t node);
        void remove_leaf_node(uint_fast16_t node);
        void find_active_nodes(void);
        
        uint_fast16_t nodes_max_get(void) const {return nodes_max;}
        uint_fast16_t categ_cnt_get(void) const {return categ_cnt;}
        uint_fast16_t active_nodes_cnt_get(void) const {return active_nodes_cnt;}
        uint_fast16_t active_node_get(uint_fast16_t id) const {return active_nodes_array[id];}
        uint_fast16_t leaves_cnt_get(void) const {return leaves_cnt;}
        uint_fast16_t leaves_node_get(uint_fast16_t id) const {return leaves_array[id];}
        uint_fast16_t non_leaves_cnt_get(void) const {return non_leaves_cnt;}
        uint_fast16_t non_leaves_node_get(uint_fast16_t id) const {return non_leaves_array[id];}
        uint_fast16_t node_categories_distrib_get(uint_fast16_t node, uint_fast16_t categ) { return node_categories_distrib[node][categ];}
        
        uint_fast16_t node_weight_get(uint_fast16_t node, uint_fast16_t attr) const {return node_weights[node][attr];}
        uint_fast16_t node_threshold_get(uint_fast16_t node) const {return node_thresholds[node];}
        uint_fast16_t node_is_leaf_get(uint_fast16_t node) const {return node_is_leaf[node];}
        
        void node_weight_set(uint_fast16_t node, uint_fast16_t attr, uint_fast16_t val) 
        {
            node_weights[node][attr] = val & 0x0000ffff;
            if (val & 0x8000)
            {
                node_weights[node][attr] |= 0xffff0000;
            }
        }
        void node_threshold_set(uint_fast16_t node, int32_t val) {
            node_thresholds[node] = val & 0x0000ffff;
            if (val & 0x8000)
            {
                node_thresholds[node] |= 0xffff0000;
            }
        }
        
        DecisionTree& operator=( const DecisionTree& other );

};

#endif
