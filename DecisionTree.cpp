#include "DecisionTree.h"
#include <stdio.h>
#include "dt_hw.h"

#define node_is_left_child(node)    (((node) & 0x0001) != 0)
#define node_is_right_child(node)   (!(node_is_left_child(node)))
#define node_sibling_get(node)      ((node_is_left_child(node)) ? (node) + 1 : (node -1))
#define node_parent_get(node)       (((node) - 1) >> 1)
#define node_left_child_get(node)   (((node) << 1) + 1)
#define node_right_child_get(node)  (((node) << 1) + 2)

#if (DT_USE_LOOP_UNFOLD == 1)
#include "loop_unfold.cpp"
#endif

void DecisionTree::allocate_nodes(int max_tree_depth, int attr_cnt, int categ_cnt)
{
    int nodes_max = (1 << max_tree_depth) - 1;
    const char* err_msg;
    
    this->node_weights              = new int32_t*[nodes_max];
//  	this->node_categories_distrib   = new uint_fast16_t*[nodes_max];
    this->node_categories           = new uint_fast16_t[nodes_max];
    this->node_thresholds           = new int32_t[nodes_max];
    this->node_is_leaf              = new bool[nodes_max];
    this->classify_path             = new uint_fast16_t[max_tree_depth];
    this->traverse_path             = new uint_fast16_t[nodes_max];
    this->mirror_path               = new uint_fast16_t[nodes_max];
    this->active_nodes_array        = new uint_fast16_t[nodes_max];
    this->leaves_array              = new uint_fast16_t[nodes_max];
    this->non_leaves_array          = new uint_fast16_t[nodes_max];
    
    this->classes					= NULL;

    for (int i = 0; i < nodes_max; ++i)
    {
        this->node_weights[i]            = new int32_t[attr_cnt];
//        this->node_categories_distrib[i] = new uint_fast16_t[categ_cnt];
    }
    
    this->max_tree_depth = max_tree_depth;
    this->nodes_max = nodes_max;
    this->attr_cnt = attr_cnt;
    this->categ_cnt = categ_cnt;
}

int DecisionTree::classify(int32_t attributes[], uint_fast16_t &leaf_node)
{
    uint_fast16_t depth_cnt;
    uint_fast16_t cur_node;
    int32_t res_scaled;
    int64_t res;
    
    depth_cnt = 0;
    cur_node = 0;
    
    while (!this->node_is_leaf[cur_node])
    {

#if 0    
        if (depth_cnt > 3)
        {
            cout << cur_node << endl;
            for (int j = 0; j < 5; j++)
                cout << this->classify_path[j] << endl;
        }
#endif        
        
        this->classify_path[depth_cnt++] = cur_node;
        
        //cout << cur_node << endl;
        
#if (DT_USE_LOOP_UNFOLD == 1)
        res = vector_mult_loop_unfold(this->node_weights[cur_node], attributes, this->attr_cnt);
#else
        res = 0;
        for (int j = 0; j < this->attr_cnt; j++)
        {
            res += this->node_weights[cur_node][j] * attributes[j];
        }
#endif
        
        res_scaled = res >> 15;
#if 0        
        cout << " res_scaled = " << res_scaled << endl;
        cout << " threshold = " << (int32_t) this->attr_cnt*this->node_thresholds[cur_node] << endl;
#endif        
        
        if (res_scaled >= (int32_t) this->attr_cnt*this->node_thresholds[cur_node])
        {
            cur_node = (cur_node << 1) + 2;
        }
        else
        {
            cur_node = (cur_node << 1) + 1;
        }
    }
    
    leaf_node = cur_node;
    
    return this->node_categories[leaf_node];
}

int DecisionTree::hw_eval(int32_t* instances[], uint_fast16_t categories[], uint_fast16_t inst_cnt)
{
	uint_fast16_t hits;

//	if (this->classes == NULL)
//	{
//		this->classes = new uint32_t[inst_cnt];
//	}

//	for (unsigned i = 0; i < this->leaves_cnt; i++)
//	{
//		uint32_t node = this->leaves_array[i];
//		memset(node_categories_distrib[node], 0, sizeof(uint_fast16_t)*8);
//	}

	return dt_hw_eval(this->leaves_array, this->leaves_cnt, categories);

//	hits = 0;
//	for (unsigned i = 0; i < this->leaves_cnt; i++)
//	{
//		uint32_t node = this->leaves_array[i];
//		uint_fast16_t* node_distrib = this->node_categories_distrib[node];
////		uint_fast16_t dominant_category_id = 0;
//		uint_fast16_t dominant_category_cnt = *node_distrib;
//
//		for (unsigned j = 1; j < this->categ_cnt; j++)
//		{
//			uint_fast16_t categ_cnt = *(node_distrib++);
//			if (dominant_category_cnt < categ_cnt)
//			{
////				dominant_category_id = j;
//				dominant_category_cnt = categ_cnt;
//			}
//		}
//
//		hits += dominant_category_cnt;
////		this->node_categories[i] = dominant_category_id;
//	}

//	for (unsigned i = 0; i < this->nodes_max; i++)
//	{
//		memset(this->node_categories_distrib[i], 0, sizeof(uint_fast16_t)*this->categ_cnt);
//	}

//	for (unsigned i = 0; i < inst_cnt; i++)
//	{
//		uint32_t node = this->classes[i] & 0x7;
//		uint32_t categ = categories[i];
//
//		this->node_categories_distrib[node][categ]++;
//	}

//	hits = 0;
//	for (unsigned i = 0; i < this->nodes_max; i++)
//	{
//		uint_fast16_t* node_distrib = this->node_categories_distrib[i];
//		uint_fast16_t dominant_category_cnt = *node_distrib;
//
//		for (unsigned j = this->categ_cnt - 1; j != 0; j--)
//		{
//			uint_fast16_t categ_cnt = (*node_distrib)++;
//			if (dominant_category_cnt < categ_cnt)
//			{
//				dominant_category_cnt = categ_cnt;
//			}
//		}
//
//		hits += dominant_category_cnt;
//	}

	return hits;
}

int DecisionTree::assign_categories_based_on_distribution(int32_t* instances[], uint_fast16_t categories[], uint_fast16_t inst_cnt)
{
    uint_fast16_t hits;
    
    this->find_category_distribution(instances, categories, inst_cnt);
    
    hits = 0;
	for (unsigned i = 0; i < this->leaves_cnt; i++)
	{
		uint32_t node = this->leaves_array[i];
		uint_fast16_t* node_distrib = this->node_categories_distrib[node];
//		uint_fast16_t dominant_category_id = 0;
		uint_fast16_t dominant_category_cnt = *node_distrib;

		for (unsigned j = 1; j < this->categ_cnt; j++)
		{
			uint_fast16_t categ_cnt = *(node_distrib++);
			if (dominant_category_cnt < categ_cnt)
			{
//				dominant_category_id = j;
				dominant_category_cnt = categ_cnt;
			}
		}

		hits += dominant_category_cnt;
//		this->node_categories[i] = dominant_category_id;
	}
    
    return hits;
}

int DecisionTree::find_category_distribution(int32_t* instances[], uint_fast16_t categories[], uint_fast16_t inst_cnt)
{
    uint_fast16_t end_node;
    
//    for (unsigned i = 0; i < this->nodes_max; i++)
//    {
//        memset(this->node_categories_distrib[i], 0, sizeof(uint_fast16_t)*this->categ_cnt);
//    }

    for (unsigned i = 0; i < this->active_nodes_cnt; i++)
	{
		uint32_t node = this->active_nodes_array[i];
		memset(this->node_categories_distrib[node], 0, sizeof(uint_fast16_t)*8);
	}
    
    for (unsigned i = 0; i < inst_cnt; i++)
    {
        this->classify(instances[i], end_node);
        this->node_categories_distrib[end_node][categories[i]]++;
    }
}

void DecisionTree::traverse(uint_fast16_t root, uint_fast16_t traverse_path[], uint_fast16_t &traverse_cnt, bool include_leaves, bool include_non_leaves, bool make_mirror_path, uint_fast16_t mirror_node, uint_fast16_t* mirror_path)
{
    bool traversed;
    uint_fast16_t cur_node;
    uint_fast16_t cur_mirror_node;
    
    traverse_cnt = 0;
    cur_node = root;
    cur_mirror_node = mirror_node;
    traversed = false;
    while (!traversed)
    {
        //cout << cur_node << endl;
        if (this->node_is_leaf[cur_node])
        {
            if (include_leaves)
            {
                if (make_mirror_path) mirror_path[traverse_cnt] = cur_mirror_node;
                traverse_path[traverse_cnt++] = cur_node;
            }
            
            if (cur_node == root)
            {
                traversed = true;
            }
            else if (node_is_left_child(cur_node))
            {
                cur_node = node_sibling_get(cur_node);
                if (make_mirror_path) cur_mirror_node = node_sibling_get(cur_mirror_node);
            }
            else
            {
                while ((!node_is_left_child(cur_node)) && (cur_node != root))
                {
                    cur_node = node_parent_get(cur_node);
                    if (make_mirror_path) cur_mirror_node = node_parent_get(cur_mirror_node);
                }
                
                if (cur_node == root)
                {
                    traversed = true;
                }
                else
                {
                    cur_node = node_sibling_get(cur_node);
                    if (make_mirror_path) cur_mirror_node = node_sibling_get(cur_mirror_node);
                }
            }
        }
        else
        {
            if (include_non_leaves)
            {
                if (make_mirror_path) mirror_path[traverse_cnt] = cur_mirror_node;
                traverse_path[traverse_cnt++] = cur_node;
            }
            
            cur_node = node_left_child_get(cur_node);
            if (make_mirror_path) cur_mirror_node = node_left_child_get(cur_mirror_node);
        }
    }
    
    //traverse_path[traverse_cnt] = NODE_ID_INVALID;
   
}

void DecisionTree::add_children_to_leaf_node(uint_fast16_t node)
{
    if (node_left_child_get(node) < this->nodes_max)
    {
        this->node_is_leaf[node] = false;
        this->node_is_leaf[node_left_child_get(node)] = true;
        this->node_is_leaf[node_right_child_get(node)] = true;
        this->find_active_nodes();
    }
}
void DecisionTree::remove_leaf_node(uint_fast16_t node)
{
    int32_t weights[this->attr_cnt];
    int32_t threshold;
    
    traverse(node_sibling_get(node), this->traverse_path, this->traverse_cnt, true, true, true, node_parent_get(node), this->mirror_path);
    
    int32_t weigths_old[this->traverse_cnt][this->attr_cnt];
    int32_t threshold_old[this->traverse_cnt];
    bool    node_leaf_status_old[this->traverse_cnt];
    
    for (int i = 0; i < this->traverse_cnt; i++)
    {
        uint_fast16_t cur_node; 
        
        cur_node = this->traverse_path[i];
        
        threshold_old[i] = this->node_thresholds[cur_node];
        node_leaf_status_old[i] = this->node_is_leaf[cur_node];
        for (int j = 0; j < this->attr_cnt; j++)
        {
            weigths_old[i][j] = this->node_weights[cur_node][j];
        }
    }
    
    for (int i = 0; i < this->traverse_cnt; i++)
    {
        uint_fast16_t cur_node;
        uint_fast16_t cur_mirror_node;
        
        cur_node = this->traverse_path[i];
        cur_mirror_node = this->mirror_path[i];
        
        this->node_thresholds[cur_mirror_node] = threshold_old[i];
        this->node_is_leaf[cur_mirror_node] = node_leaf_status_old[i];
        for (int j = 0; j < this->attr_cnt; j++)
        {
            this->node_weights[cur_mirror_node][j] = weigths_old[i][j];
        }
    }
    
    this->find_active_nodes();
}

void DecisionTree::find_active_nodes(void)
{
    this->traverse(0, this->active_nodes_array, this->active_nodes_cnt, true, true);
    
    this->leaves_cnt = 0;
    this->non_leaves_cnt = 0;
    
    for (int i = 0; i < this->active_nodes_cnt; i++)
    {
        uint_fast16_t cur_node = this->active_nodes_array[i];
        
        if (this->node_is_leaf[cur_node])
        {
            this->leaves_array[this->leaves_cnt++] = cur_node;
        }
        else
        {
            this->non_leaves_array[this->non_leaves_cnt++] = cur_node;
        }
    }
}

DecisionTree& DecisionTree::operator=( const DecisionTree& other )
{
    this->active_nodes_cnt  = other.active_nodes_cnt;
    this->leaves_cnt        = other.leaves_cnt;
    this->non_leaves_cnt    = other.non_leaves_cnt;
    
    for (int i = 0; i < other.active_nodes_cnt; i++)
    {
        uint_fast16_t cur_node = other.active_nodes_array[i];
        
        for (int j = 0; j < this->attr_cnt; j++)
        {
            this->node_weights[cur_node][j] = other.node_weights[cur_node][j];
        }
        
        this->node_thresholds[cur_node] = other.node_thresholds[cur_node];
        this->node_is_leaf[cur_node]    = other.node_is_leaf[cur_node];
        this->active_nodes_array[i]     = cur_node;
    }
    
    for (int i = 0; i < other.leaves_cnt; i++)
    {
        this->leaves_array[i] = other.leaves_array[i];
    }
    
    for (int i = 0; i < other.non_leaves_cnt; i++)
    {
        this->non_leaves_array[i] = other.non_leaves_array[i];
    }
    
    return *this;
}

void DecisionTree::dump_to_oc1_file(const char* oc1_file_name)
{
    this->find_active_nodes();
    
    FILE * pFile;

    pFile = fopen (oc1_file_name,"w");
    
    fprintf (pFile, "Training set: bc_norm_1_test.dat, Dimensions: %d, Categories: %d\n\n",this->attr_cnt, this->categ_cnt);
    
    for (int i = 0; i < this->active_nodes_cnt; i++)
    {
        uint_fast16_t cur_node;
        
        cur_node = this->active_nodes_array[i];
        
        if (!this->node_is_leaf[cur_node])
        {
            char node_pos[this->max_tree_depth];
            uint_fast16_t node_pos_cnt;
            uint_fast16_t cur_node_ancestor;
            
            node_pos_cnt = 0;
            
            cur_node_ancestor = cur_node;
            
            while (cur_node_ancestor != 0)
            {
                if (node_is_left_child(cur_node_ancestor))
                {
                    node_pos[node_pos_cnt] = 'l';
                }
                else
                {
                    node_pos[node_pos_cnt] = 'r';
                }
                node_pos_cnt++;
                
                cur_node_ancestor = node_parent_get(cur_node_ancestor);
            }
            
            if (node_pos_cnt == 0)
            {
                fprintf (pFile, "Root");
            }
            else
            {
                for (int i = node_pos_cnt - 1; i >= 0; i--)
                {
                    fprintf (pFile, "%c", node_pos[i]);
                }
            }
            
            fprintf(pFile, " Hyperplane: ");
            
            fprintf(pFile, "Left = [");
            
            for (int j = 0; j < this->categ_cnt; j++)
            {
                fprintf(pFile, "%d", node_categories_distrib[node_left_child_get(cur_node)][j]);
                if (j < this->categ_cnt - 1)
                {
                    fprintf(pFile, ",");
                }
            }
            
            fprintf(pFile, "], Right = [");
            
            for (int j = 0; j < this->categ_cnt; j++)
            {
                fprintf(pFile, "%d", node_categories_distrib[node_right_child_get(cur_node)][j]);
                if (j < this->categ_cnt - 1)
                {
                    fprintf(pFile, ",");
                }
            }
            
            fprintf(pFile, "]\n");
            
            for (int j = 0; j < this->attr_cnt; j++)
            {
                fprintf(pFile, "%f x[%d]", double(this->node_weights[cur_node][j])/0x8000, j+1);
                fprintf(pFile, " + ");
            }
            
            fprintf(pFile, "%f = 0\n\n", -double(this->node_thresholds[cur_node])*this->attr_cnt/0x8000);
        }
    }
    
    fclose (pFile);
}

DecisionTree::~DecisionTree()
{
    
    for (int i = 0; i < this->nodes_max; ++i)
    {
        delete [] this->node_weights[i];
        delete [] this->node_categories_distrib[i];
    }
    
    delete [] this->node_categories_distrib;
    delete [] this->classify_path;
    delete [] this->node_weights;
    delete [] this->node_categories;
    delete [] this->node_thresholds;
    delete [] this->node_is_leaf;
    delete [] this->traverse_path;
    delete [] this->mirror_path;
    delete [] this->active_nodes_array;
    delete [] this->leaves_array;
    delete [] this->non_leaves_array;
}

DecisionTree::DecisionTree(int attr_cnt, int categ_max, int max_tree_depth)
{
    this->allocate_nodes(max_tree_depth, attr_cnt, categ_max + 1);

    this->node_is_leaf[0] = false;
    this->node_is_leaf[1] = true;
    this->node_is_leaf[2] = true;

    this->find_active_nodes();
}
    
