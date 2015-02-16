#include "hw_config.h"
#include "hereboy.h"
#include "tree.h"
#include "xil_printf.h"
#include <stdlib.h>
#include <math.h>
#include "xil_io.h"
#include "xaxidma.h"
#include "util.h"

#define rand_norm() ((rand() % 10000) / 10000.0)

#define DMA_DEV_ID		0

#define MEM_BASE_ADDR		0x11000000

#define RX_BD_SPACE_BASE	(MEM_BASE_ADDR + 0x00000000)
#define RX_BD_SPACE_HIGH	(MEM_BASE_ADDR + 0x00001FFF)
#define RX_BUFFER_BASE		(MEM_BASE_ADDR + 0x00300000)
#define RX_BUFFER_HIGH		(MEM_BASE_ADDR + 0x004FFFFF)

XAxiDma AxiDma;
u8 * volatile RxBufferPtr;

#define DT_HW_AXI_BASE_ADDR				0x40000000

#define DT_HW_AXI_CONF_STAT_OFFSET		0x00000000
#define DT_HW_AXI_CONF_STAT_BASE_ADDR	(DT_HW_AXI_BASE_ADDR + DT_HW_AXI_CONF_STAT_OFFSET)

#define DT_HW_CONF1_OFFSET				0x00000000
#define DT_HW_CONF1_ADDR				(DT_HW_CONF1_OFFSET + DT_HW_AXI_CONF_STAT_BASE_ADDR)
#define DT_HW_CONF1_RUN_BIT				0
#define DT_HW_CONF1_RST_BIT				1

#define DT_HW_INST_NUM_OFFSET			0x00000004
#define DT_HW_INST_NUM_ADDR				(DT_HW_INST_NUM_OFFSET + DT_HW_AXI_CONF_STAT_BASE_ADDR)

#define DT_HW_AXI_PROC_CNT_LOW_OFFSET	0x00000010
#define DT_HW_AXI_PROC_CNT_LOW_ADDR		(DT_HW_AXI_PROC_CNT_LOW_OFFSET + DT_HW_AXI_CONF_STAT_BASE_ADDR)

#define DT_HW_AXI_PROC_CNT_HIGH_OFFSET	0x00000014
#define DT_HW_AXI_PROC_CNT_HIGH_ADDR	(DT_HW_AXI_PROC_CNT_HIGH_OFFSET + DT_HW_AXI_CONF_STAT_BASE_ADDR)

#define DT_HW_STAT1_OFFSET				0x00000018
#define DT_HW_STAT1_ADDR				(DT_HW_STAT1_OFFSET + DT_HW_AXI_CONF_STAT_BASE_ADDR)
#define DT_HW_STAT1_FINISHED_BIT		0

#define INST_MEM_REGION_BASE	0x10000
#define CATEG_MEM_REGION_BASE	0x30000
//#define INST_MEM_BASE(inst_num)		((unsigned int*) (DT_HW_AXI_BASE_ADDR | INST_MEM_REGION_BASE | (inst_num << INST_MEM_BANKS_ADDR_WIDTH << 2)))

#define INST_MEM_ADDR(inst_num, bank_num) ((unsigned int*) (DT_HW_AXI_BASE_ADDR | INST_MEM_REGION_BASE | ((bank_num << INST_MEM_ADDR_WIDTH | inst_num) << 2)))
#define CATEG_MEM_ADDR(inst_num) ((unsigned int*) (DT_HW_AXI_BASE_ADDR | CATEG_MEM_REGION_BASE | (inst_num << 2)))

#define DT_MEM_REGION_BASE		0x20000

#define DT_MEM_COEF_ADDR(level, node_num, bank_num) ((unsigned int*) (DT_HW_AXI_BASE_ADDR | DT_MEM_REGION_BASE | ((level << DT_MEM_TOT_ADDR_WIDTH | bank_num << DT_MEM_ADDR_WIDTH | node_num) << 2)))

#define DT_MEM_CLS_PTR_ADDR(level, node_num) ((unsigned int*) (DT_HW_AXI_BASE_ADDR | DT_MEM_REGION_BASE | ((level << DT_MEM_TOT_ADDR_WIDTH | DT_MEM_COEF_BANKS_NUM << DT_MEM_ADDR_WIDTH | node_num) << 2)))

Hb_Conf_t *hb_conf;

#define COEF_BANKS_MAX_NUM 		64

uint32_t attr_cnt;
uint32_t inst_cnt;
uint32_t categ_max;
uint32_t coef_packed_mem[COEF_BANKS_MAX_NUM];
uint32_t instances[NUM_INST_MAX][NUM_ATTRIBUTES];
uint32_t categories[NUM_INST_MAX];

#define LEAVES_MAX				256
#define NONLEAVES_MAX			256
#define MAX_WEIGHT_MUTATIONS	128

uint32_t leaves_cnt;
tree_node* leaves[LEAVES_MAX];
uint32_t nonleaves_cnt;
tree_node* nonleaves[NONLEAVES_MAX];
tree_node* node_hierarchy[MAX_TREE_DEPTH][NUM_NODES];
uint32_t node_hierarchy_cnt[MAX_TREE_DEPTH];
uint_fast16_t node_categories_distrib[LEAVES_MAX][NUM_ATTRIBUTES];

#define TOPO_CHILDREN_ADDED			1
#define TOPO_LEFT_CHILD_REMOVED		2
#define TOPO_RIGHT_CHILD_REMOVED	3

//void get_bank_bit(uint32_t attr, uint32_t bit_pos, uint32_t* bank, uint32_t* bank_bit_pos)
//{
//	bank =
//}

int pack_coefs(int16_t coefs[], uint32_t coef_cnt, uint32_t coef_res, uint32_t coef_mem[])
{
	uint32_t bank_cnt = 0;
	uint32_t bank_space_left = 32;
	uint32_t bank_space_start = 0;
	uint32_t i;
	int32_t coef_rescaled;

	for (i = 0; i < coef_cnt; i++)
	{
		coef_mem[i] = 0;
	}

	for (i = 0; i < coef_cnt; i++)
	{
		coef_rescaled = ((float) coefs[i]) / 0x8000 * (1 << (coef_res - 1));

		coef_mem[bank_cnt] |= (coef_rescaled << bank_space_start);

		if (bank_space_left > coef_res)
		{
			bank_space_left -= coef_res;
			bank_space_start += coef_res;
		}
		else if (bank_space_left == coef_res)
		{
			bank_cnt++;
			bank_space_left = 32;
			bank_space_start = 0;
		}
		else
		{
			bank_cnt++;
			coef_mem[bank_cnt] |= (coef_rescaled >> bank_space_left);
			bank_space_left = 32;
			bank_space_start = 0;
		}
	}

	return bank_cnt;

}

void random_hiperplane(int16_t weights[])
{
    uint_fast16_t inst_i;
    uint_fast16_t inst_j;
    int64_t res_i;
    int64_t res_j;
    float delta;
    int i;

    inst_i = rand() % inst_cnt;
    inst_j = inst_i;
    while (categories[inst_i] == categories[inst_j])
    {
        inst_j = rand() % inst_cnt;
    }

    for (i = 0; i < attr_cnt; i++)
    {
        weights[i] = instances[inst_i][i] - instances[inst_j][i];
    }

    delta = ((float) (rand() % 100)) / 100;

    res_i = 0;
    res_j = 0;
    for (i = 0; i < attr_cnt; i++)
    {
        res_i += ((int64_t) weights[i]) * instances[inst_i][i];
        res_j += ((int64_t) weights[i]) * instances[inst_j][i];
    }

    weights[NUM_ATTRIBUTES] = ((int64_t) (delta*res_i + (1 - delta)*res_j)) >> ATTRIBUTE_RES >> DT_ADDER_TREE_DEPTH;

//    res_i >>= 15;
//    res_j >>= 15;
//
//    weights[attr_cnt] = (delta*res_i + (1 - delta)*res_j) / attr_cnt/ 2;

}

int hb_clear_instances()
{
	inst_cnt = 0;

	return 0;
}

int hb_load_instance(int16_t* instance, uint_fast16_t category)
{
	unsigned i;
	unsigned banks_cnt;

	for (i = 0; i < attr_cnt; i++)
	{
		instances[inst_cnt][i] = instance[i];
	}

	banks_cnt = pack_coefs(instance, attr_cnt, ATTRIBUTE_RES, coef_packed_mem);

//	Xil_Out32(INST_MEM_BASE(inst_cnt), coef_packed_mem[0]);
//	Xil_Out32(INST_MEM_BASE(inst_cnt) + 1, coef_packed_mem[1]);

//	Xil_SetTlbAttributes(0x40000000, 0xC06);

//	memcpy(INST_MEM_BASE(inst_cnt), coef_packed_mem, 4*banks_cnt);
//	memcpy(coef_packed_mem, INST_MEM_BASE(inst_cnt), 4*banks_cnt);

	for (i = 0; i < banks_cnt; i++)
	{
		*(INST_MEM_ADDR(inst_cnt, i)) = coef_packed_mem[i];
	}

	*(CATEG_MEM_ADDR(inst_cnt)) = category;

	categories[inst_cnt] = category;

	inst_cnt++;

	return 0;
}

int hw_init()
{
	XAxiDma_Config *CfgPtr;
	int Status;

	/* Initialize the XAxiDma device.
		 */
	CfgPtr = XAxiDma_LookupConfig(DMA_DEV_ID);
	if (!CfgPtr) {
		xil_printf("No config found for %d\r\n", DMA_DEV_ID);
		return XST_FAILURE;
	}

	Status = XAxiDma_CfgInitialize(&AxiDma, CfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Initialization failed %d\r\n", Status);
		return XST_FAILURE;
	}

	XAxiDma_Reset(&AxiDma);

	if(XAxiDma_HasSg(&AxiDma)){
		xil_printf("Device configured as SG mode \r\n");
		return XST_FAILURE;
	}

	/* Disable interrupts, we use polling mode
	 */
	XAxiDma_IntrDisable(&AxiDma, XAXIDMA_IRQ_ALL_MASK,
						XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrDisable(&AxiDma, XAXIDMA_IRQ_ALL_MASK,
						XAXIDMA_DMA_TO_DEVICE);

	RxBufferPtr = (u8 *)RX_BUFFER_BASE;

//	Status = XAxiDma_SimpleTransfer(&AxiDma,(u32) RxBufferPtr,
//				dt_hw_config.inst_num*4, XAXIDMA_DEVICE_TO_DMA);

	Status = XAxiDma_SimpleTransfer(&AxiDma,(u32) RxBufferPtr,
						0x1000, XAXIDMA_DEVICE_TO_DMA);

	return Status;
}

int _extract_hierarcy(tree_node* dt, uint32_t level)
{
	if (level >= MAX_TREE_DEPTH)
	{
		level = MAX_TREE_DEPTH - 1;
	}

	dt->level = level;

	if (dt->left == NULL)
	{
		leaves[leaves_cnt] = dt;
		leaves_cnt++;
		dt->id = leaves_cnt;
	}
	else
	{
		dt->id = node_hierarchy_cnt[level];

		node_hierarchy[level][node_hierarchy_cnt[level]] = dt;
		node_hierarchy_cnt[level]++;

		nonleaves[nonleaves_cnt] = dt;
		nonleaves_cnt++;
		_extract_hierarcy(dt->left, level + 1);
		_extract_hierarcy(dt->right, level + 1);
	}

	return 0;
}

int extract_hierarcy(tree_node* dt)
{
	unsigned i;

	leaves_cnt = 0;
	nonleaves_cnt = 0;

	for (i = 0; i < MAX_TREE_DEPTH; i++)
	{
		node_hierarchy_cnt[i] = 0;
	}

	return _extract_hierarcy(dt, 0);
}

float fitness_calc()
{
    uint_fast16_t hits;
    uint32_t status;
    float accuracy;
    //unsigned i, j;
    //u32* rxBuf;

    // Start the HW
    Xil_Out32(DT_HW_CONF1_ADDR, 0x00000001);

//    *(u32* volatile) 0x40400058 = 0x1000;
//
//    for (i = 1; i <= leaves_cnt; i++)
//	{
//		memset(node_categories_distrib[i], 0, sizeof(uint_fast16_t)*attr_cnt);
//	}

    Xil_Out32(DT_HW_CONF1_ADDR, 0x00000000);

    hits = 0;
    while (hits == 0)
    {
    	status = Xil_In32(DT_HW_STAT1_ADDR);
    	hits = status >> 16;
    }

#if 0
	rxBuf = (u32*) RxBufferPtr;
	for (i = 0 ; i < inst_cnt; i++)
	{
		uint32_t node = (*rxBuf++);
		uint32_t categ = categories[i];
		node_categories_distrib[node][categ]++;
	}

	hits = 0;
	for (i = 1; i <= leaves_cnt; i++)
	{
		uint_fast16_t* node_distrib = &node_categories_distrib[i][1];
		uint_fast16_t dominant_category_cnt = *node_distrib;

		for (j = 0; j < categ_max; j++)
		{
			uint_fast16_t categ_cnt = *(node_distrib++);
			if (dominant_category_cnt < categ_cnt)
			{
				dominant_category_cnt = categ_cnt;
			}
		}

		hits += dominant_category_cnt;
	}
#endif

    accuracy = ((float) hits)/inst_cnt;

//    tree_size = dt.active_nodes_cnt_get();
//
    accuracy *= (hb_conf->complexity_weight*(((float) categ_max) - ((float) nonleaves_cnt))/((float)categ_max) + 1);

    return accuracy;

}

void hw_set_whole_tree()
{
	unsigned i, k;
	uint32_t class_ptr;

	for (i = 0; i < nonleaves_cnt; i++)
	{
		tree_node* node = nonleaves[i];

		for (k = 0; k < DT_MEM_COEF_BANKS_NUM; k++)
		{
			*(DT_MEM_COEF_ADDR(node->level, node->id, k)) = node->weights[k];
		}

		class_ptr = 0;

		if (node->left->left != NULL)
		{
			class_ptr |= (node->left->id << DT_CHILD_PTR_DATA_WIDTH);
		}
		else
		{
			class_ptr |= (node->left->id << CLASS_RES) << 2*DT_CHILD_PTR_DATA_WIDTH;
		}

		if (node->right->left != NULL)
		{
			class_ptr |= node->right->id;
		}
		else
		{
			class_ptr |= node->right->id << 2*DT_CHILD_PTR_DATA_WIDTH;
		}


		*(DT_MEM_CLS_PTR_ADDR(node->level, node->id)) = class_ptr;

	}

}

int hb_run()
{
	tree_node* dt_best;
	tree_node* dt_cur;
	uint32_t current_iter;
	uint32_t returned_to_best_iter;

	uint32_t weights_mutation_cnt;
	uint32_t stagnation_iter;
	int16_t unpacked_weights[NUM_ATTRIBUTES + 1];

	uint_fast16_t mut_banks[MAX_WEIGHT_MUTATIONS];
	tree_node* mut_nodes[MAX_WEIGHT_MUTATIONS];
	uint32_t mut_masks[MAX_WEIGHT_MUTATIONS];

	uint32_t topology_mutated;
	tree_node* temp_mut_hang_tree;
	tree_node* topo_mut_node;
	tree_node* topo_mut_sibling;

	float topo_mutation_probability;
	float return_to_best_probability;
	float search_probability;

	float fitness_best, fitness_cur, fitness_new;
	unsigned i;

	Xil_Out32(DT_HW_INST_NUM_ADDR, inst_cnt);

	dt_cur = tree_create();
	tree_create_child(dt_cur, CHILD_LEFT);
	tree_create_child(dt_cur, CHILD_RIGHT);
	random_hiperplane(unpacked_weights);
	pack_coefs(unpacked_weights, attr_cnt + 1, COEF_RES, dt_cur->weights);

	dt_best = tree_copy(dt_cur);

	extract_hierarcy(dt_cur);
	hw_set_whole_tree(dt_cur);

	fitness_best = fitness_cur = fitness_calc();
	stagnation_iter = 0;
	returned_to_best_iter = 0;
	temp_mut_hang_tree = NULL;
	topo_mut_node = NULL;
	topo_mut_sibling = NULL;

	for (current_iter = 0; current_iter < hb_conf->max_iterations; current_iter++)
	{
#if (HB_PRINT_PROGRESS_INTERVAL != 0)
		if (current_iter % HB_PRINT_PROGRESS_INTERVAL == 0) {
			xil_printf("Current iteration: %d\n\r", current_iter);
		}
#endif

		topology_mutated = 0;

		topo_mutation_probability = hb_conf->topology_mutation_rate * leaves_cnt;

		topo_mutation_probability *= 1 + stagnation_iter*hb_conf->topo_mutation_rate_raise_due_to_stagnation_step;

		if (topo_mutation_probability > rand_norm())
		{
			topo_mut_node = leaves[rand() % leaves_cnt];

			/* 50% chance to add or delete a node */
			if ((topo_mut_node->level < (MAX_TREE_DEPTH - 1)) &&
				((rand() % 2) || (topo_mut_node == dt_cur->left) || (topo_mut_node == dt_cur->right)))
			{
//				xil_printf("MA: i=%d, n=%x\n\r", current_iter, (uint32_t) topo_mut_node);
				topology_mutated = TOPO_CHILDREN_ADDED;
				tree_create_child(topo_mut_node, CHILD_LEFT);
				tree_create_child(topo_mut_node, CHILD_RIGHT);
				random_hiperplane(unpacked_weights);
				pack_coefs(unpacked_weights, attr_cnt + 1, COEF_RES, topo_mut_node->weights);
			}
			else
			{
				temp_mut_hang_tree = topo_mut_node->parent;
				topo_mut_sibling = tree_get_sibling(topo_mut_node);

				if (temp_mut_hang_tree->parent->left == temp_mut_hang_tree)
				{
					topology_mutated = TOPO_LEFT_CHILD_REMOVED;
					temp_mut_hang_tree->parent->left = topo_mut_sibling;
					topo_mut_sibling->parent = temp_mut_hang_tree->parent;
				}
				else
				{
					topology_mutated = TOPO_RIGHT_CHILD_REMOVED;
					temp_mut_hang_tree->parent->right = topo_mut_sibling;
					topo_mut_sibling->parent = temp_mut_hang_tree->parent;
				}

			}

			extract_hierarcy(dt_cur);
			hw_set_whole_tree(dt_cur);
		}

		weights_mutation_cnt = 1 + hb_conf->weights_mutation_rate *
							   (1 + stagnation_iter*hb_conf->weight_mutation_rate_raise_due_to_stagnation_step) *
							   nonleaves_cnt;

		for (i = 0; i < weights_mutation_cnt; i++)
		{
			mut_nodes[i] = nonleaves[rand() % nonleaves_cnt];
			mut_banks[i] = rand() % DT_MEM_COEF_BANKS_NUM;
			mut_masks[i] = 1 << (rand() % 32);

			*(DT_MEM_COEF_ADDR(mut_nodes[i]->level, mut_nodes[i]->id, mut_banks[i])) = mut_nodes[i]->weights[mut_banks[i]] ^ mut_masks[i];

		}

		fitness_new = fitness_calc();

		if ((fitness_new - fitness_cur) > 1e-6)
		{
			stagnation_iter = 0;

			for (i = 0; i < weights_mutation_cnt; i++)
			{
				mut_nodes[i]->weights[mut_banks[i]] ^= mut_masks[i];
			}

			fitness_cur = fitness_new;

			if ((fitness_cur - fitness_best) > 1e-6)
			{
				tree_delete_node(dt_best);
				dt_best = tree_copy(dt_cur);

				returned_to_best_iter = current_iter;
				fitness_best = fitness_cur;
#if (HB_PRINT_STATS == 1)
				xil_printf("AB: i=%d,f=", current_iter);
				print_float(fitness_cur, 100);
				xil_printf(",s=%d", leaves_cnt);
				xil_printf("\n\r");
#endif
			}
			else
			{
#if (HB_PRINT_STATS == 1)
				xil_printf("CB: i=%d,f=", current_iter);
				print_float(fitness_cur, 100);
				xil_printf(",s=%d", leaves_cnt);
				xil_printf("\n\r");
#endif
			}
		}
		else
		{
			stagnation_iter++;

			return_to_best_probability = hb_conf->return_to_best_prob_iteration_increment *
										(current_iter - returned_to_best_iter);

			search_probability = hb_conf->search_probability * (1 + stagnation_iter*hb_conf->search_probability_raise_due_to_stagnation_step);

			/* Should we return to the best yet solution since we are wondering without improvement for a long time? */
			if (rand_norm() < return_to_best_probability)
			{
				tree_delete_node(dt_cur);
				dt_cur = tree_copy(dt_best);
				extract_hierarcy(dt_cur);
				hw_set_whole_tree(dt_cur);

				if ((topology_mutated) && ((topology_mutated == TOPO_RIGHT_CHILD_REMOVED) || (topology_mutated == TOPO_LEFT_CHILD_REMOVED)))
				{
					if (temp_mut_hang_tree->left == topo_mut_node)
					{
						temp_mut_hang_tree->right = NULL;
					}
					else
					{
						temp_mut_hang_tree->left = NULL;
					}

					tree_delete_node(temp_mut_hang_tree);
				}

				fitness_cur = fitness_best;
				returned_to_best_iter = current_iter;
				stagnation_iter = 0;
#if (HB_PRINT_STATS == 1)
				xil_printf("RB: i=%d\n\r", current_iter);
#endif
			}
			else if (topology_mutated && (rand_norm() < search_probability))
			{
				for (i = 0; i < weights_mutation_cnt; i++)
				{
					mut_nodes[i]->weights[mut_banks[i]] ^= mut_masks[i];
				}

				if ((topology_mutated == TOPO_RIGHT_CHILD_REMOVED) || (topology_mutated == TOPO_LEFT_CHILD_REMOVED))
				{
					if (temp_mut_hang_tree->left == topo_mut_node)
					{
						temp_mut_hang_tree->right = NULL;
					}
					else
					{
						temp_mut_hang_tree->left = NULL;
					}

					tree_delete_node(temp_mut_hang_tree);
				}

				fitness_cur = fitness_new;
			}
			else //We failed to advance in fitness :(
			{
				// If we mutated topology, dt_cur was changed and we need to revert the changes
				if (topology_mutated)
				{
					if (topology_mutated == TOPO_RIGHT_CHILD_REMOVED)
					{
						temp_mut_hang_tree->parent->right = temp_mut_hang_tree;
						topo_mut_sibling->parent = temp_mut_hang_tree;
					}
					else if (topology_mutated == TOPO_LEFT_CHILD_REMOVED)
					{
						temp_mut_hang_tree->parent->left = temp_mut_hang_tree;
						topo_mut_sibling->parent = temp_mut_hang_tree;
					}
					else if (topology_mutated == TOPO_CHILDREN_ADDED)
					{
						tree_delete_child(topo_mut_node, CHILD_LEFT);
						tree_delete_child(topo_mut_node, CHILD_RIGHT);
					}

					extract_hierarcy(dt_cur);
					hw_set_whole_tree(dt_cur);
				}
				else
				{
					for (i = 0; i < weights_mutation_cnt; i++)
					{
						*(DT_MEM_COEF_ADDR(mut_nodes[i]->level, mut_nodes[i]->id, mut_banks[i])) = mut_nodes[i]->weights[mut_banks[i]];
					}
				}

			}
		}

	}

#if (HB_PRINT_FINAL == 1)
	xil_printf("FINAL: f=");
	print_float(fitness_best, 100);
	xil_printf(",s=%d", nonleaves_cnt);
	xil_printf("\n\r");
#endif

	return fitness_best;
}

int hb_init(Hb_Conf_t *conf, int attribute_cnt, int maximum_category) //(int32_t** instances, int inst_cnt, uint_fast16_t* categories, int attr_cnt, int categ_max, int max_tree_depth, Hb_Conf_t hb_conf, int seed, uint32_t hits[], bool hw_sw) :dt(attr_cnt, categ_max, max_tree_depth), dt_new(attr_cnt, categ_max, max_tree_depth), dt_best(attr_cnt, categ_max, max_tree_depth)
{
	hw_init();
	// Reset the HW
	Xil_Out32(DT_HW_CONF1_ADDR, 0x00000002);
	inst_cnt = 0;
	attr_cnt = attribute_cnt;
	categ_max = maximum_category;
	hb_conf = conf;
	return 0;

//	this->max_iterations = hb_conf.max_iterations;
//	this->topology_mutation_rate = hb_conf.topology_mutation_rate;
//	this->weights_mutation_rate = hb_conf.weights_mutation_rate;
//	this->search_probability = hb_conf.search_probability;
//	this->search_probability_raise_due_to_stagnation_step = hb_conf.search_probability_raise_due_to_stagnation_step;
//	this->topo_mutation_rate_raise_due_to_stagnation_step = hb_conf.topo_mutation_rate_raise_due_to_stagnation_step;
//	this->weight_mutation_rate_raise_due_to_stagnation_step = hb_conf.weight_mutation_rate_raise_due_to_stagnation_step;
//	this->return_to_best_prob_iteration_increment = hb_conf.return_to_best_prob_iteration_increment;
//	this->complexity_weight = hb_conf.complexity_weight;
//
//	this->inst_cnt = inst_cnt;
//	this->attr_cnt = attr_cnt;
//
//	this->instances = instances;
//	this->categories = categories;
//
//	srand (seed);
//
//	this->init_node_with_random_hiperplane(this->dt, 0);
//
//	this->hw_sw = hw_sw;
//	this->hits = hits;
//	this->current_iter = 0;
//
//	this->fitness_best = this->fitness_cur = this->fitness_calc(dt);
//
//	dt_best = dt;
}
#if 0
void init_node_with_random_hiperplane(DecisionTree &dt, uint_fast16_t node)
{
    int32_t weights[this->attr_cnt];
    int32_t threshold;
    
    random_hiperplane(weights, threshold);
 
    dt.node_threshold_set(node, threshold);
    for (int i = 0; i < this->attr_cnt; i++)
    {
        dt.node_weight_set(node, i, weights[i]);
    }
    
}

double Hereboy::fitness_calc(DecisionTree &dt)
{
    uint_fast16_t hits;
    double tree_size;
    double accuracy;
    
    if (this->hw_sw)
    {
    	dt.hw_eval(this->instances, this->categories, this->inst_cnt);
    	hits = this->hits[this->current_iter];

    	if (hits > 1000)
    		hits = 250;
    }
    else
    {
    	hits = dt.assign_categories_based_on_distribution(this->instances, this->categories, this->inst_cnt);
    	this->hits[this->current_iter] = hits;
    }

    accuracy = double(hits)/this->inst_cnt;
    
    tree_size = dt.active_nodes_cnt_get();
    
    accuracy *= (this->complexity_weight*(dt.categ_cnt_get() - tree_size)/dt.categ_cnt_get() + 1);
    
    //accuracy -= tree_size*this->complexity_weight;
    
    return accuracy;
    
}

void Hereboy::random_hiperplane(int32_t weights[], int32_t &threshold)
{
    uint_fast16_t inst_i;
    uint_fast16_t inst_j;
    int64_t res_i;
    int64_t res_j;
    double delta;
    
    inst_i = rand() % this->inst_cnt;
    inst_j = inst_i;
    while (this->categories[inst_i] == this->categories[inst_j])
    {
        inst_j = rand() % this->inst_cnt;
    }

    for (int i = 0; i < this->attr_cnt; i++)
    {
        weights[i] = this->instances[inst_i][i] - this->instances[inst_j][i];
    }
    
    delta = double(rand() % 100)/100;
    
    res_i = 0;
    res_j = 0;
    for (int i = 0; i < this->attr_cnt; i++)
    {
        res_i += weights[i] * this->instances[inst_i][i];
        res_j += weights[i] * this->instances[inst_j][i];
    }
    
    res_i >>= 15;
    res_j >>= 15;
    
    threshold = round(delta*res_i + (1 - delta)*res_j) / this->attr_cnt;
   
}

double Hereboy::run(void)
{
    //uint_fast16_t hits;
    double fitness_new;
    double topo_mutation_probability;
    double return_to_best_probability;
    uint_fast16_t weights_mutation_cnt;
    double search_probability;
    bool topology_mutated;
    uint_fast16_t cout_refresh_step;
    
    cout_refresh_step = this->max_iterations/100;

   current_iter = 0;
   stagnation_iter = 0;
   best_accepted_iter = 0;
   returned_to_best_iter = 0;
    
    for (this->current_iter = 0; this->current_iter < this->max_iterations; this->current_iter++)
    {
//        if ((this->output_verbose) && (this->current_iter % cout_refresh_step == 0))
//        {
//            cout << " Progress: " << setw(3) << (int) floor(double(this->current_iter)/this->max_iterations*100) << "%  |";
//            cout << " Fitness: " << fixed << setprecision(3) << this->fitness_best << " |";
//            cout << " Size: " << dt_best.leaves_cnt_get();
//            cout << endl;
//            cout << "\e[A\e[A\r" << endl;
//        }

    	if (this->current_iter % 50000 == 0) {
    		xil_printf("Current iteration: %d\n\r", this->current_iter);
    	}
        
        dt_new = dt;
        
        topology_mutated = false;
        
        weights_mutation_cnt = 1 + this->weights_mutation_rate * 
                               (1 + this->stagnation_iter*weight_mutation_rate_raise_due_to_stagnation_step) *
                               dt_new.non_leaves_cnt_get();
        
        for (int i = 0; i < weights_mutation_cnt; i++)
        {
            uint_fast16_t mut_node = dt_new.non_leaves_node_get(rand() % dt_new.non_leaves_cnt_get());
            uint_fast16_t mut_weight = rand() % this->attr_cnt + 1;     //+1 since we want to include threshold also
            uint_fast16_t mut_mask = 1 << (rand() % 16);
            
            if (mut_weight < this->attr_cnt)
            {
                uint_fast16_t weight_old = dt_new.node_weight_get(mut_node, mut_weight);
                dt_new.node_weight_set(mut_node, mut_weight, weight_old ^ mut_mask);
            }
            else    //we are mutating threshold
            {
                uint_fast16_t threshold_old = dt_new.node_threshold_get(mut_node);
                dt_new.node_threshold_set(mut_node, threshold_old ^ mut_mask);
            }
        }

        topo_mutation_probability = this->topology_mutation_rate * dt_new.leaves_cnt_get();

        topo_mutation_probability *= 1 + this->stagnation_iter*topo_mutation_rate_raise_due_to_stagnation_step;
        
        if (topo_mutation_probability > rand_norm())
        {
            uint_fast16_t mut_node = dt_new.leaves_node_get(rand() % dt_new.leaves_cnt_get());
            
            topology_mutated = true;
            
            /* 50% chance to add or delete a node */
            if ((rand() % 2) || (mut_node <= 2))
            {
                dt_new.add_children_to_leaf_node(mut_node);
                this->init_node_with_random_hiperplane(this->dt_new, mut_node);
            }
            else
            {
                dt_new.remove_leaf_node(mut_node);
            }
            
        }
        
        fitness_new = this->fitness_calc(dt_new);
        
        if ((fitness_new - this->fitness_cur) > 1e-6)
        {
            this->stagnation_iter = 0;
            dt = dt_new;
            this->fitness_cur = fitness_new;
            
            if ((this->fitness_cur - this->fitness_best) > 1e-6)
            {
                dt_best = dt;
                this->best_accepted_iter = this->current_iter;
                this->returned_to_best_iter = this->current_iter;
                this->fitness_best = this->fitness_cur;

                this->log("FB", "");
            }
            else
            {
                this->log("FN", "");
            }
        }
        else
        {
            this->stagnation_iter++;
            
            return_to_best_probability = this->return_to_best_prob_iteration_increment * 
                                        (this->current_iter - this->returned_to_best_iter);
            
            /* Should we return to the best yet solution since we are wondering without improvement for a long time? */
            if (rand_norm() < return_to_best_probability)
            {
                dt = dt_best;
                this->fitness_cur = this->fitness_best;
                this->returned_to_best_iter = this->current_iter;
                this->stagnation_iter = 0;
                
                this->log("RB", "");
            }
            else if (topology_mutated)
            {
                search_probability = this->search_probability * (1 + this->stagnation_iter*search_probability_raise_due_to_stagnation_step);
            
                if (rand_norm() < search_probability)
                {
                    dt = dt_new;
                    this->fitness_cur = fitness_new;
                    
                    this->log("RB", "");
                }
            }
        }
         
    }
    
    return this->fitness_best;
}
#endif
