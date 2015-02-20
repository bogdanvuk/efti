#include "hw_config.h"
#include "hereboy.h"
#include "tree.h"
#include "xil_printf.h"
#include <stdlib.h>
#include <math.h>
#include "xil_io.h"
#include "xaxidma.h"
#include "util.h"
#include "xparameters.h"
#include "timing.h"

#define rand_norm() ((rand() % 10000) / 10000.0)

#define DMA_DEV_ID		0

#define TIMING_HB_ID					0
#define TIMING_FITNESS_CALC_ID			1

#define AXI_DMA_TRANSFER_SIZE	0x3fff
#define MEM_BASE_ADDR		0x11000000

#define RX_BD_SPACE_BASE	(MEM_BASE_ADDR + 0x00000000)
#define RX_BD_SPACE_HIGH	(MEM_BASE_ADDR + 0x00001FFF)
#define RX_BUFFER_BASE		(MEM_BASE_ADDR + 0x00300000)
#define RX_BUFFER_HIGH		(MEM_BASE_ADDR + 0x004FFFFF)

XAxiDma AxiDma;
u8 * volatile RxBufferPtr;

#define DT_HW_AXI_BASE_ADDR				0x40000000

#define DT_HW_AXI_CONF_STAT_OFFSET		0x70000
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

#define INST_MEM_REGION_BASE	0x00000
#define CATEG_MEM_REGION_BASE	0x60000
//#define INST_MEM_BASE(inst_num)		((unsigned int*) (DT_HW_AXI_BASE_ADDR | INST_MEM_REGION_BASE | (inst_num << INST_MEM_BANKS_ADDR_WIDTH << 2)))

#define INST_MEM_ADDR(inst_num, bank_num) ((unsigned int*) (DT_HW_AXI_BASE_ADDR | INST_MEM_REGION_BASE | ((bank_num << INST_MEM_ADDR_WIDTH | inst_num) << 2)))
#define CATEG_MEM_ADDR(inst_num) ((unsigned int*) (DT_HW_AXI_BASE_ADDR | CATEG_MEM_REGION_BASE | (inst_num << 2)))

#define DT_MEM_REGION_BASE		0x40000

#define DT_MEM_COEF_ADDR(level, node_num, bank_num) ((unsigned int*) (DT_HW_AXI_BASE_ADDR | DT_MEM_REGION_BASE | ((level << DT_MEM_TOT_ADDR_WIDTH | bank_num << DT_MEM_ADDR_WIDTH | node_num) << 2)))

#define DT_MEM_CLS_PTR_ADDR(level, node_num) ((unsigned int*) (DT_HW_AXI_BASE_ADDR | DT_MEM_REGION_BASE | ((level << DT_MEM_TOT_ADDR_WIDTH | DT_MEM_COEF_BANKS_NUM << DT_MEM_ADDR_WIDTH | node_num) << 2)))

Hb_Conf_t *hb_conf;

#define COEF_BANKS_MAX_NUM 		64

uint32_t attr_cnt;
uint32_t inst_cnt;
uint32_t categ_max;
uint32_t coef_packed_mem[COEF_BANKS_MAX_NUM];
int32_t instances[NUM_INST_MAX][NUM_ATTRIBUTES];
uint32_t categories[NUM_INST_MAX];
uint32_t current_iter;

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

uint32_t non_eval_ticks = 0;

#define TOPO_CHILDREN_ADDED			1
#define TOPO_LEFT_CHILD_REMOVED		2
#define TOPO_RIGHT_CHILD_REMOVED	3

//void __attribute__((optimize("O0"))) HbAssert(uint32_t expression)
void HbAssert(uint32_t expression)
{
	if (!expression)
	{
		xil_printf("Assert failed! Iter: %d", current_iter);
		while(1)
		{
		}
	}
}

void get_bank_bit(uint32_t attr, uint32_t bit_pos, uint32_t* bank, uint32_t* bank_bit_pos)
{
	*bank = attr * COEF_RES / 32;
	*bank_bit_pos = (attr*COEF_RES + bit_pos) % 32;
}

int pack_coefs(int32_t coefs[], uint32_t coef_cnt, uint32_t coef_res, uint32_t coef_mem[])
{
	uint32_t bank_cnt = 0;
	uint32_t bank_space_left = 32;
	uint32_t bank_space_start = 0;
	uint32_t i;
	uint32_t coef_rescaled;

	coef_mem[0] = 0;

	for (i = 0; i < coef_cnt; i++)
	{
//		coef_rescaled = ((float) coefs[i]) / 0x8000 * (1 << (coef_res - 1));

//		coef_rescaled = coefs[i] * (1 << (coef_res - 1)) / 0x8000;

		coef_rescaled = (uint16_t) coefs[i];

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
			coef_mem[bank_cnt] = 0;
		}
		else
		{
			bank_cnt++;
			coef_mem[bank_cnt] |= (coef_rescaled >> bank_space_left);
			bank_space_left = 32;
			bank_space_start = 0;
			coef_mem[bank_cnt] = 0;
		}
	}

	return bank_cnt;

}

void random_hiperplane(int32_t weights[])
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

    for (i = attr_cnt; i < NUM_ATTRIBUTES; i++)
    {
    	weights[i] = 0;
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

int hb_load_instance(int32_t* instance, uint_fast16_t category)
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


//#if ((HB_SW == 1) || (HB_HW_SW_FITNESS == 1))
	Status = XAxiDma_SimpleTransfer(&AxiDma,(u32) RxBufferPtr,
			AXI_DMA_TRANSFER_SIZE, XAXIDMA_DEVICE_TO_DMA);
//#endif

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

#if (DT_USE_LOOP_UNFOLD == 1)
#include "loop_unfold.cpp"
#endif

uint32_t classify(tree_node* dt, int32_t attributes[])
{
    tree_node* cur_node;
    int16_t res_scaled;
    int64_t res;
    int j;

    cur_node = dt;

	while (cur_node->left != NULL)
	{

#if (DT_USE_LOOP_UNFOLD == 1)
        res = vector_mult_loop_unfold(this->node_weights[cur_node], attributes, this->attr_cnt);
#else
        res = 0;
        for (j = 0; j < attr_cnt; j++)
        {
            res += cur_node->weights[j] * attributes[j];
        }
#endif

        res_scaled = res >> ATTRIBUTE_RES >> DT_ADDER_TREE_DEPTH;

        if (res_scaled >= cur_node->weights[NUM_ATTRIBUTES])
		{
			cur_node = cur_node->right;
		}
		else
		{
			cur_node = cur_node->left;
		}

	}

	return cur_node->id;
}

void hw_start(uint32_t get_classes)
{
	// Start the HW
	Xil_Out32(DT_HW_CONF1_ADDR, 0x00000001);

//	if (get_classes)
//	{
		// Start DMA (Could certanly be wrapped to look less ugly): reset the number of words to transmit
		*(u32* volatile) 0x40400058 = AXI_DMA_TRANSFER_SIZE;

//	}

	Xil_Out32(DT_HW_CONF1_ADDR, 0x00000000);
}

void find_node_distribution(tree_node* dt, volatile u32* rxBuf)
{
	unsigned i;

	for (i = 0 ; i < inst_cnt; i++)
	{
		uint32_t node;

#if (HB_SW == 1)
		node = classify(dt, instances[i]);
#if (HB_HW_SW_FITNESS == 1)
		HbAssert(node == (*rxBuf++));
#endif
#else
		node = (*rxBuf++);
#endif
		uint32_t categ = categories[i];
		node_categories_distrib[node][categ]++;
	}
}

float dt_eval(tree_node* dt)
{
	uint32_t hits = 0;
	unsigned i;
	volatile u32* rxBuf = (u32*) RxBufferPtr;
	*rxBuf = 0;

#if (HB_HW == 1)


    hw_start(1);
#endif

#if (HB_HW == 1)
    while (*rxBuf == 0)
	{
	}
#endif

    for (i = 0 ; i < inst_cnt; i++)
	{
		uint32_t node;

#if (HB_SW == 1)
		node = classify(dt, instances[i]);
#if (HB_HW_SW_FITNESS == 1)
		HbAssert(node == (*rxBuf++));
#endif
#else
		node = (*rxBuf++);
#endif
		uint32_t categ = categories[i];

		if (categ == node)
		{
			hits++;
		}

	}

    return ((float) hits)/inst_cnt;
}

void assign_classes(tree_node* dt)
{
	unsigned i,j;
	volatile u32* rxBuf = (u32*) RxBufferPtr;
    *rxBuf = 0;

#if (HB_HW == 1)


    hw_start(1);
#endif

    for (i = 1; i <= leaves_cnt; i++)
	{
		memset(node_categories_distrib[i], 0, sizeof(uint_fast16_t)*attr_cnt);
	}

#if (HB_HW == 1)
    while (*rxBuf == 0)
	{
	}
#endif

    find_node_distribution(dt, rxBuf);

	for (i = 1; i <= leaves_cnt; i++)
	{
		uint_fast16_t* node_distrib = &node_categories_distrib[i][1];
		uint_fast16_t dominant_category_cnt = *node_distrib;
		uint_fast16_t dominant_category = 1;

		for (j = 1; j < categ_max; j++)
		{
			uint_fast16_t categ_cnt = *(++node_distrib);
			if (dominant_category_cnt < categ_cnt)
			{
				dominant_category = j+1;
				dominant_category_cnt = categ_cnt;
			}
		}

		leaves[i-1]->id = dominant_category;
	}
}

float fitness_calc(tree_node* dt)
{
    uint_fast16_t hits;
    uint32_t status;
    float accuracy;
    //uint32_t accuracy;

#if ((HB_SW == 1) || (HB_HW_SW_FITNESS == 1))
    unsigned i, j;
#endif

//#if (HB_HW_SW_FITNESS == 1)
    volatile u32* rxBuf = (u32*) RxBufferPtr;
    *rxBuf = 0;
//#endif

#if (HB_HW == 1)
    hw_start(HB_HW_SW_FITNESS);
#endif

#if ((HB_SW == 1) || (HB_HW_SW_FITNESS == 1))
    for (i = 1; i <= leaves_cnt; i++)
	{
		memset(node_categories_distrib[i], 0, sizeof(uint_fast16_t)*attr_cnt);
	}

#if (HB_HW_SW_FITNESS == 1)
    while (*rxBuf == 0)
	{
	}
#endif

    find_node_distribution(dt, rxBuf);

	hits = 0;
	for (i = 1; i <= leaves_cnt; i++)
	{
		uint_fast16_t* node_distrib = &node_categories_distrib[i][1];
		uint_fast16_t dominant_category_cnt = *node_distrib;

		for (j = 1; j < categ_max; j++)
		{
			uint_fast16_t categ_cnt = *(++node_distrib);
			if (dominant_category_cnt < categ_cnt)
			{
				dominant_category_cnt = categ_cnt;
			}
		}

		hits += dominant_category_cnt;
	}
#endif

#if ((HB_HW == 1))
	status = 0;
    while ((status & 0xffff0000) == 0)
    {
    	status = Xil_In32(DT_HW_STAT1_ADDR);
    }

#if (HB_SW == 1)
    HbAssert((status >> 16) == hits);
#else
    hits = status >> 16;
#endif

#endif

    accuracy = ((float) hits)/inst_cnt;
    accuracy *= (hb_conf->complexity_weight*(((float) categ_max) - ((float) leaves_cnt))/((float)categ_max) + 1);

    return accuracy;

}

void hw_set_whole_tree()
{
	unsigned i, k;
	uint32_t class_ptr;

	for (i = 0; i < nonleaves_cnt; i++)
	{
		tree_node* node = nonleaves[i];

//		pack_coefs(node->weights, NUM_ATTRIBUTES + 1, COEF_RES, node->banks);

		for (k = 0; k < DT_MEM_COEF_BANKS_NUM; k++)
		{
			*(DT_MEM_COEF_ADDR(node->level, node->id, k)) = node->banks[k];
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

//void hw_apply_topo_

void delete_trimmed_subtree(uint32_t topology_mutated, tree_node* temp_mut_hang_tree, tree_node* topo_mut_node)
{
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
}

void hw_apply_mutation(tree_node* mut_nodes[], uint32_t mut_attr[], uint32_t mut_bit[], uint32_t weights_mutation_cnt)
{
	unsigned i;

	for (i = 0; i < weights_mutation_cnt; i++)
	{
		mut_nodes[i]->weights[mut_attr[i]] ^= (1 << mut_bit[i]);
	}
}

tree_node* hb_run(float* fitness, uint32_t* dt_leaves_cnt, uint32_t* dt_nonleaves_cnt, float* t_hb, float* t_fitness_calc_avg)
{
	tree_node* dt_best;
	tree_node* dt_cur;
	uint32_t returned_to_best_iter;
	uint64_t ticks_fitness_average = 0;
	uint64_t ticks_hb = 0;

	uint32_t weights_mutation_cnt;
	uint32_t stagnation_iter;

	uint_fast16_t mut_banks[MAX_WEIGHT_MUTATIONS];
	tree_node* mut_nodes[MAX_WEIGHT_MUTATIONS];
	uint32_t mut_masks[MAX_WEIGHT_MUTATIONS];
	uint32_t mut_attr[MAX_WEIGHT_MUTATIONS];
	int32_t mut_attr_val[MAX_WEIGHT_MUTATIONS];
	uint32_t mut_bit[MAX_WEIGHT_MUTATIONS];
	uint32_t mut_bank_val[MAX_WEIGHT_MUTATIONS];

	uint32_t topology_mutated;
	tree_node* temp_mut_hang_tree;
	tree_node* topo_mut_node;
	tree_node* topo_mut_sibling;

	float topo_mutation_probability;
	float return_to_best_probability;
	float search_probability;

	float fitness_best, fitness_cur, fitness_new;
	unsigned i;

	timing_reset(TIMING_HB_ID);
	timing_start(TIMING_HB_ID);

	Xil_Out32(DT_HW_INST_NUM_ADDR, inst_cnt - 1);
	tree_init();

	dt_cur = tree_create();
	tree_create_child(dt_cur, CHILD_LEFT);
	tree_create_child(dt_cur, CHILD_RIGHT);
	random_hiperplane(dt_cur->weights);
	pack_coefs(dt_cur->weights, NUM_ATTRIBUTES + 1, COEF_RES, dt_cur->banks);

	dt_best = tree_copy(dt_cur);

	extract_hierarcy(dt_cur);

#if (HB_HW == 1)
	hw_set_whole_tree(dt_cur);
#endif

	fitness_best = fitness_cur = fitness_calc(dt_cur);
	stagnation_iter = 0;
	returned_to_best_iter = 0;
	temp_mut_hang_tree = NULL;
	topo_mut_node = NULL;
	topo_mut_sibling = NULL;

	for (current_iter = 0; current_iter < hb_conf->max_iterations; current_iter++)
	{
#if (HB_PRINT_PROGRESS_INTERVAL != 0)
		if (current_iter % HB_PRINT_PROGRESS_INTERVAL == 0) {
			ser_printf("Current iteration: %d\n", current_iter);
		}
#endif

		// If HB Timing counter is about to overflow (it's only 16-bit counter), save the value and reset the counter
		if (timing_count_get(TIMING_HB_ID) > 0xf000)
		{
			ticks_hb += timing_count_get(TIMING_HB_ID);
			timing_reset(TIMING_HB_ID);
		}

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
				random_hiperplane(topo_mut_node->weights);
				pack_coefs(topo_mut_node->weights, NUM_ATTRIBUTES + 1, COEF_RES, topo_mut_node->banks);
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
#if (HB_HW == 1)
			hw_set_whole_tree(dt_cur);
#endif
		}

		weights_mutation_cnt = 1 + hb_conf->weights_mutation_rate *
							   (1 + stagnation_iter*hb_conf->weight_mutation_rate_raise_due_to_stagnation_step) *
							   nonleaves_cnt;

		for (i = 0; i < weights_mutation_cnt; i++)
		{
			uint32_t mut_mask_bit;
			mut_nodes[i] = nonleaves[rand() % nonleaves_cnt];

			if (rand() % 2)
			{
				mut_attr[i] = rand() % attr_cnt;
			}
			else
			{
				mut_attr[i] = NUM_ATTRIBUTES;
			}

			mut_bit[i] = rand() % COEF_RES;
//			mut_banks[i] = rand() % DT_MEM_COEF_BANKS_NUM;
//			mut_masks[i] = 1 << (rand() % 32);

#if (HB_HW == 1)
			get_bank_bit(mut_attr[i], mut_bit[i], &mut_banks[i], &mut_mask_bit);
			mut_masks[i] = 1 << mut_mask_bit;
//			pack_coefs(mut_nodes[i]->weights, NUM_ATTRIBUTES + 1, COEF_RES, coef_packed_mem);
			mut_bank_val[i] = mut_nodes[i]->banks[mut_banks[i]];
			mut_nodes[i]->banks[mut_banks[i]] ^= mut_masks[i];
			*(DT_MEM_COEF_ADDR(mut_nodes[i]->level, mut_nodes[i]->id, mut_banks[i])) = mut_nodes[i]->banks[mut_banks[i]];
#endif

#if (HB_SW == 1)
			mut_attr_val[i] = mut_nodes[i]->weights[mut_attr[i]];
			uint16_t weight_temp = mut_nodes[i]->weights[mut_attr[i]];
			mut_nodes[i]->weights[mut_attr[i]] = (int16_t) (weight_temp ^ (1 << mut_bit[i]));
#endif
		}

		timing_reset(TIMING_FITNESS_CALC_ID);
		timing_start(TIMING_FITNESS_CALC_ID);

		fitness_new = fitness_calc(dt_cur);

		timing_stop(TIMING_FITNESS_CALC_ID);
		ticks_fitness_average += timing_count_get(TIMING_FITNESS_CALC_ID);

		if ((fitness_new - fitness_cur) > 1e-6)
		{
			stagnation_iter = 0;
//#if ((HB_HW == 1) && (HB_SW == 0))
//			hw_apply_mutation(mut_nodes, mut_attr, mut_bit, weights_mutation_cnt);
//#endif

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

				delete_trimmed_subtree(topology_mutated, temp_mut_hang_tree, topo_mut_node);

				fitness_cur = fitness_best;
				returned_to_best_iter = current_iter;
				stagnation_iter = 0;
#if (HB_PRINT_STATS == 1)
				xil_printf("RB: i=%d\n\r", current_iter);
#endif
			}
			else if (topology_mutated && (rand_norm() < search_probability))
			{
//#if ((HB_HW == 1) && (HB_SW == 0))
//				hw_apply_mutation(mut_nodes, mut_attr, mut_bit, weights_mutation_cnt);
//#endif
				delete_trimmed_subtree(topology_mutated, temp_mut_hang_tree, topo_mut_node);

				fitness_cur = fitness_new;
			}
			else //We failed to advance in fitness :(
			{

				for (i = 0; i < weights_mutation_cnt; i++)
				{
#if (HB_SW == 1)
					mut_nodes[i]->weights[mut_attr[i]] = mut_attr_val[i];
#endif
#if (HB_HW == 1)
					mut_nodes[i]->banks[mut_banks[i]] = mut_bank_val[i];
#endif
				}

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
#if (HB_HW == 1)
					hw_set_whole_tree(dt_cur);
#endif
				}
				else
				{
#if (HB_HW == 1)
					for (i = 0; i < weights_mutation_cnt; i++)
					{
						*(DT_MEM_COEF_ADDR(mut_nodes[i]->level, mut_nodes[i]->id, mut_banks[i])) = mut_bank_val[i];
					}
#endif
				}

			}
		}

	}

	tree_delete_node(dt_cur);
	extract_hierarcy(dt_best);
	hw_set_whole_tree(dt_best);
	assign_classes(dt_best);

	uint32_t ticks = (uint32_t) ticks_fitness_average/hb_conf->max_iterations;
	ticks_hb += timing_count_get(TIMING_HB_ID);

	*t_fitness_calc_avg = (ticks / (111111111.0 / 2));
	*t_hb = (ticks_hb / (111111111.0 / 65536.0));

	*fitness = fitness_best;
	*dt_leaves_cnt = leaves_cnt;
	*dt_nonleaves_cnt = nonleaves_cnt;

	return dt_best;
}

// CAUTION! This function implies that the decision tree is already setup in hardware and that
// extract_hierarcy and assign_classes has been called upon it. In other words it is meant to
// be called only after hb_run has finished.
float hb_eval(tree_node* dt)
{
	hw_set_whole_tree(dt);
	return dt_eval(dt);
}

void hb_reset(Hb_Conf_t *conf, int attribute_cnt, int maximum_category)
{
	attr_cnt = attribute_cnt;
	categ_max = maximum_category;
	hb_conf = conf;
	inst_cnt = 0;
}

void hb_init()
{
	hw_init();
	// Reset the HW
	Xil_Out32(DT_HW_CONF1_ADDR, 0x00000002);
	timing_init(TIMING_FITNESS_CALC_ID, 0xffff, 0);
	timing_init(TIMING_HB_ID, 0xffff, 0xf);
}

void hb_close()
{
	timing_close(TIMING_FITNESS_CALC_ID);
	timing_close(TIMING_HB_ID);
}

