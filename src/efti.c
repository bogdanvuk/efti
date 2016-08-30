#include "efti_conf.h"
#include "hw_config.h"
#include "efti.h"
#include "tree.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "util.h"
#include "timing.h"
#include "dataset.h"
#include "dt2js.h"
#include <stdio.h>

#if (DT_USE_LOOP_UNFOLD == 1)
#include "loop_unfold.h"
#endif

#if EFTI_HW == 1

#include "xil_io.h"
#include "xaxidma.h"
#include "xparameters.h"
#include "xil_mmu.h"

#define DMA_DEV_ID		0

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

#define INST_MEM_ADDR(inst_num, bank_num) ((volatile unsigned int*) (DT_HW_AXI_BASE_ADDR | INST_MEM_REGION_BASE | ((bank_num << INST_MEM_ADDR_WIDTH | inst_num) << 2)))
#define CATEG_MEM_ADDR(inst_num) ((volatile unsigned int*) (DT_HW_AXI_BASE_ADDR | CATEG_MEM_REGION_BASE | (inst_num << 2)))

#define DT_MEM_REGION_BASE		0x40000

#define DT_MEM_COEF_ADDR(level, node_num, bank_num) ((volatile unsigned int*) (DT_HW_AXI_BASE_ADDR | DT_MEM_REGION_BASE | ((level << DT_MEM_TOT_ADDR_WIDTH | bank_num << DT_MEM_ADDR_WIDTH | node_num) << 2)))

#define DT_MEM_CLS_PTR_ADDR(level, node_num) ((volatile unsigned int*) (DT_HW_AXI_BASE_ADDR | DT_MEM_REGION_BASE | ((level << DT_MEM_TOT_ADDR_WIDTH | DT_MEM_COEF_BANKS_NUM << DT_MEM_ADDR_WIDTH | node_num) << 2)))

#define TLB_SHARABLE	(1 << 16)

#define TBL_AP(val)		((val) << 10)
#define TBL_AP_RW_FULL	(TBL_AP(0x3))

#define TBL_TEX(val)	((val) << 12)
#define TBL_TEX_STRONGLY_ORDERED	TBL_TEX(0x0)
#define TBL_TEX_NON_SHARABLE_DEVICE	TBL_TEX(0x2)
#define TBL_XN			(1 << 4)
#define TBL_C			(1 << 3)
#define TBL_B			(1 << 2)
#define TBL_RES			(0x0002)

#endif

const Efti_Conf_t *efti_conf;

#define rand_norm() ((rand_r(seedp) % 10000) / 10000.0)
#define TIMING_EFTI_ID					0
#define TIMING_FITNESS_CALC_ID			1

#define COEF_BANKS_MAX_NUM 		64
#define LEAVES_MAX				NUM_NODES
#define NONLEAVES_MAX			NUM_NODES
#define MAX_WEIGHT_MUTATIONS	NUM_NODES

unsigned int *seedp;
uint32_t attr_cnt;
uint32_t inst_cnt;
uint32_t categ_max;
T_Dataset* dataset;
//uint32_t coef_packed_mem[COEF_BANKS_MAX_NUM];
int32_t instances[NUM_INST_MAX][NUM_ATTRIBUTES];
#define CLK_FREQ 	225e+6
#define CLOCKS_PER_SEC (CLK_FREQ / 4 / 10)
tree_node* dt_best;
tree_node* dt_cur;
uint32_t weights_mutation_cnt;
uint32_t topology_mutated;
uint_fast16_t mut_banks[MAX_WEIGHT_MUTATIONS];
tree_node* mut_nodes[MAX_WEIGHT_MUTATIONS];
uint32_t mut_masks[MAX_WEIGHT_MUTATIONS];
uint32_t mut_attr[MAX_WEIGHT_MUTATIONS];
int32_t mut_attr_val[MAX_WEIGHT_MUTATIONS];
uint32_t mut_bit[MAX_WEIGHT_MUTATIONS];
uint32_t mut_bank_val[MAX_WEIGHT_MUTATIONS];
uint32_t categories[NUM_INST_MAX];
uint32_t current_iter;

float accuracy;
float oversize;
uint32_t leaves_cnt;
tree_node* leaves[LEAVES_MAX];
float tot_impurity;
float impurity;
uint_fast16_t leaves_total_inst_cnt[LEAVES_MAX];
uint32_t nonleaves_cnt;
tree_node* nonleaves[NONLEAVES_MAX];
tree_node* node_hierarchy[MAX_TREE_DEPTH][NUM_NODES];
uint32_t node_hierarchy_cnt[MAX_TREE_DEPTH];
//Enumeration of leaves starts from 1, so it is cheaper to ignore the first
//row to distribution matrix, and hence have one more in total
uint_fast16_t node_categories_distrib[LEAVES_MAX+1][NUM_ATTRIBUTES];

uint32_t non_eval_ticks = 0;

#define TOPO_CHILDREN_ADDED			1
#define TOPO_LEFT_CHILD_REMOVED		2
#define TOPO_RIGHT_CHILD_REMOVED	3
#define TOPO_ROOT_CHILD_REMOVED 	4

//void __attribute__((optimize("O0"))) HbAssert(uint32_t expression)
void HbAssert(uint32_t expression)
{
	if (!expression)
	{
		efti_printf("Assert failed! Iter: %d", current_iter);
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
//
//int pack_coefs(int32_t coefs[], uint32_t coef_cnt, uint32_t coef_res, uint32_t coef_mem[])
//{
//	uint32_t bank_cnt = 0;
//	uint32_t bank_space_left = 32;
//	uint32_t bank_space_start = 0;
//	uint32_t i;
//	uint32_t coef_rescaled;
//
//	coef_mem[0] = 0;
//
//	for (i = 0; i < coef_cnt; i++)
//	{
////		coef_rescaled = ((float) coefs[i]) / 0x8000 * (1 << (coef_res - 1));
//
////		coef_rescaled = coefs[i] * (1 << (coef_res - 1)) / 0x8000;
//
//		coef_rescaled = (uint16_t) coefs[i];
//
//		coef_mem[bank_cnt] |= (coef_rescaled << bank_space_start);
//
//		if (bank_space_left > coef_res)
//		{
//			bank_space_left -= coef_res;
//			bank_space_start += coef_res;
//		}
//		else if (bank_space_left == coef_res)
//		{
//			bank_cnt++;
//			bank_space_left = 32;
//			bank_space_start = 0;
//			coef_mem[bank_cnt] = 0;
//		}
//		else
//		{
//			bank_cnt++;
//			coef_mem[bank_cnt] |= (coef_rescaled >> bank_space_left);
//			bank_space_left = 32;
//			bank_space_start = 0;
//			coef_mem[bank_cnt] = 0;
//		}
//	}
//
//	return bank_cnt;
//
//}

void random_hiperplane(int32_t weights[])
{
    uint_fast16_t inst_i;
    uint_fast16_t inst_j;
    int64_t res_i;
    int64_t res_j;
    float delta;
    int i;

    inst_i = rand_r(seedp) % inst_cnt;
    inst_j = inst_i;
    while (categories[inst_i] == categories[inst_j])
    {
        inst_j = rand_r(seedp) % inst_cnt;
    }

    for (i = 0; i < attr_cnt; i++)
    {
        weights[i] = instances[inst_i][i] - instances[inst_j][i];
    }

//    efti_printf("w = np.array([");
//    for (i = 0; i < attr_cnt; i++)
//    {
//        efti_printf("%d,", weights[i]);
//    }
//    efti_printf("])\n");
//
//    efti_printf("x1 = np.array([");
//    for (i = 0; i < attr_cnt; i++)
//    {
//        efti_printf("%d,", instances[inst_i][i]);
//    }
//    efti_printf("])\n");
//
//    efti_printf("x2 = np.array([");
//    for (i = 0; i < attr_cnt; i++)
//    {
//        efti_printf("%d,", instances[inst_j][i]);
//    }
//    efti_printf("])\n");

    delta = ((float) (rand_r(seedp) % 90) + 5) / 100;
//    efti_printf("inti: %d, instj: %d, delta: %f\n", inst_i, inst_j, delta);

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

//    efti_printf("th = %d\n", weights[NUM_ATTRIBUTES]);

//    res_i >>= 15;
//    res_j >>= 15;
//
//    weights[attr_cnt] = (delta*res_i + (1 - delta)*res_j) / attr_cnt/ 2;

}

int efti_clear_instances()
{
	inst_cnt = 0;

	return 0;
}

int efti_load_instance(const int32_t* instance, uint_fast16_t category)
{
	unsigned i;

	for (i = 0; i < attr_cnt; i++)
	{
		instances[inst_cnt][i] = (instance[i] + 1);
	}

//	Xil_Out32(INST_MEM_BASE(inst_cnt), coef_packed_mem[0]);
//	Xil_Out32(INST_MEM_BASE(inst_cnt) + 1, coef_packed_mem[1]);

	//S = 0

//	Xil_SetTlbAttributes(0x40000000, TBL_RES | TBL_TEX_STRONGLY_ORDERED | TBL_AP_RW_FULL | TBL_B );

//	memcpy(INST_MEM_BASE(inst_cnt), coef_packed_mem, 4*banks_cnt);
//	memcpy(coef_packed_mem, INST_MEM_BASE(inst_cnt), 4*banks_cnt);

#if EFTI_HW == 1
	unsigned banks_cnt;

	banks_cnt = pack_coefs(instance, attr_cnt, ATTRIBUTE_RES, coef_packed_mem);

	for (i = 0; i < INST_MEM_BANKS_NUM; i++)
	{
		*(INST_MEM_ADDR(inst_cnt, i)) = coef_packed_mem[i];
	}

	*(CATEG_MEM_ADDR(inst_cnt)) = category;
#endif

	categories[inst_cnt] = category;

	inst_cnt++;

	return 0;
}

#if EFTI_HW == 1

int hw_init()
{
	XAxiDma_Config *CfgPtr;
	int Status;

	/* Initialize the XAxiDma device.
		 */
	CfgPtr = XAxiDma_LookupConfig(DMA_DEV_ID);
	if (!CfgPtr) {
		efti_printf("No config found for %d\r\n", DMA_DEV_ID);
		return XST_FAILURE;
	}

	Status = XAxiDma_CfgInitialize(&AxiDma, CfgPtr);
	if (Status != XST_SUCCESS) {
		efti_printf("Initialization failed %d\r\n", Status);
		return XST_FAILURE;
	}

	XAxiDma_Reset(&AxiDma);

	if(XAxiDma_HasSg(&AxiDma)){
		efti_printf("Device configured as SG mode \r\n");
		return XST_FAILURE;
	}

	/* Disable interrupts, we use polling mode
	 */
	XAxiDma_IntrDisable(&AxiDma, XAXIDMA_IRQ_ALL_MASK,
						XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrDisable(&AxiDma, XAXIDMA_IRQ_ALL_MASK,
						XAXIDMA_DMA_TO_DEVICE);

	RxBufferPtr = (u8 *)RX_BUFFER_BASE;

//	Status = XAxiDma_SimpleTransfer(&AxiDma,(uint32_t) RxBufferPtr,
//				dt_hw_config.inst_num*4, XAXIDMA_DEVICE_TO_DMA);


//#if ((EFTI_SW == 1) || (EFTI_HW_SW_FITNESS == 1))
	Status = XAxiDma_SimpleTransfer(&AxiDma,(uint32_t) RxBufferPtr,
			AXI_DMA_TRANSFER_SIZE, XAXIDMA_DEVICE_TO_DMA);
//#endif

	// Reset the HW
	Xil_Out32(DT_HW_CONF1_ADDR, 0x00000002);

	//Xil_SetTlbAttributes(0x40000000, TBL_RES | TBL_TEX_STRONGLY_ORDERED | TBL_AP_RW_FULL | TBL_B );
	//Xil_SetTlbAttributes(0x40000000, TBL_RES | TBL_TEX_NON_SHARABLE_DEVICE | TBL_AP_RW_FULL | TBL_C );
	Xil_SetTlbAttributes(0x40000000, 0xC02 );

	return Status;
}

#endif

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

tree_node* find_dt_leaf_for_inst(tree_node* dt, int32_t attributes[])
{
    tree_node* cur_node;
    int16_t res_scaled;
    int64_t res;
    int j;

    cur_node = dt;

	while (cur_node->left != NULL)
	{

#if (DT_USE_LOOP_UNFOLD == 1)
        res = evaluate_node_test(cur_node->weights, attributes, attr_cnt);
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

	return cur_node;
}

#if EFTI_HW == 1

void hw_start(uint32_t get_classes)
{
	// Start the HW
	Xil_Out32(DT_HW_CONF1_ADDR, 0x00000001);

//	if (get_classes)
//	{
		// Start DMA (Could certanly be wrapped to look less ugly): reset the number of words to transmit
		*(uint32_t* volatile) 0x80400058 = AXI_DMA_TRANSFER_SIZE;

//	}

	Xil_Out32(DT_HW_CONF1_ADDR, 0x00000000);
}

#endif

void find_node_distribution(tree_node* dt, volatile uint32_t* rxBuf)
{
	unsigned i;


	for (i = 0 ; i < inst_cnt; i++)
	{
        uint32_t node;

#if (EFTI_SW == 1)

		node = find_dt_leaf_for_inst(dt, instances[i])->id;
#if (EFTI_HW_SW_FITNESS == 1)
		HbAssert(node == (*rxBuf++));
#endif
#else
		node_id = (*rxBuf++);
#endif
		uint32_t categ = categories[i];
		node_categories_distrib[node][categ]++;
	}
}

float ensemble_eval(tree_node* dt[], int ensemble_size) {
	uint32_t hits = 0;
	unsigned i, j;
    uint32_t vote[categ_max+1];

    for (i = 0 ; i < inst_cnt; i++)
	{
        for (j = 1; j <= categ_max; j++)
        {
            vote[j] = 0;
        }

        for (j = 0 ; j < ensemble_size; j++)
        {
            tree_node* node;

            node = find_dt_leaf_for_inst(dt[j], instances[i]);

            vote[node->cls]++;
        }

        uint32_t max_vote = 1;

        for (j = 2; j <= categ_max; j++)
        {
            if (vote[j] > vote[max_vote]) {
                max_vote = j;
            }
        }

		uint32_t categ = categories[i];

		if (categ == max_vote)
		{
			hits++;
		}
    }

    return ((float) hits)/inst_cnt;
        
}

float dt_eval(tree_node* dt)
{
	uint32_t hits = 0;
	unsigned i;
#if (EFTI_HW == 1)
	volatile uint32_t* rxBuf = (uint32_t*) RxBufferPtr;
	*rxBuf = 0;

    hw_start(1);
#endif

#if (EFTI_HW == 1)
    while (*rxBuf == 0)
	{
	}
#endif

    for (i = 0 ; i < inst_cnt; i++)
	{
		uint32_t node;

#if (EFTI_SW == 1)
		node = find_dt_leaf_for_inst(dt, instances[i])->id;
#if (EFTI_HW_SW_FITNESS == 1)
		HbAssert(node == (*rxBuf++));
#endif
#else
		node = (*rxBuf++);
#endif
		uint32_t categ = categories[i];

		if (categ == leaves[node-1]->cls)
		{
			hits++;
		}

	}

    return ((float) hits)/inst_cnt;
}

void assign_classes(tree_node* dt)
{
	unsigned i,j;
#if (EFTI_HW == 1)
	volatile uint32_t* rxBuf = (uint32_t*) RxBufferPtr;
	*rxBuf = 0;

    hw_start(1);
#endif

    for (i = 1; i <= leaves_cnt; i++)
	{
		memset(node_categories_distrib[i], 0, sizeof(uint_fast16_t)*(categ_max + 1));
	}

#if (EFTI_HW == 1)
    while (*rxBuf == 0)
	{
	}

    find_node_distribution(dt, rxBuf);
#else
    find_node_distribution(dt, NULL);
#endif

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

		leaves[i-1]->cls = dominant_category;
	}
}

float fitness_eval(tree_node* dt)
{
    uint_fast16_t hits;
    uint32_t status;
    float fitness;
    //uint32_t accuracy;

#if ((EFTI_SW == 1) || (EFTI_HW_SW_FITNESS == 1))
    int i, j;
#endif

#if (EFTI_HW == 1)
    volatile uint32_t* rxBuf = (uint32_t*) RxBufferPtr;
    *rxBuf = 0;

    hw_start(EFTI_HW_SW_FITNESS);
#endif

#if ((EFTI_SW == 1) || (EFTI_HW_SW_FITNESS == 1))
    for (i = 1; i <= leaves_cnt; i++)
	{
		memset(node_categories_distrib[i], 0, sizeof(uint_fast16_t)*(categ_max+1));
	}

#if (EFTI_HW_SW_FITNESS == 1)
    while (*rxBuf == 0)
	{
	}

    find_node_distribution(dt, rxBuf);
#else
    find_node_distribution(dt, NULL);
#endif

	hits = 0;
	tot_impurity = 0;
	for (i = 1; i <= leaves_cnt; i++)
	{
		uint_fast16_t* node_distrib = &node_categories_distrib[i][1];
		uint_fast16_t dominant_category_cnt = *node_distrib;
		uint_fast16_t dominant_category = 1;
		uint_fast16_t total_leaf_inst_cnt = *node_distrib;

		for (j = 1; j < categ_max; j++)
		{
			uint_fast16_t categ_cnt = *(++node_distrib);
			total_leaf_inst_cnt  += categ_cnt;
			if (dominant_category_cnt < categ_cnt)
			{
				dominant_category = j+1;
				dominant_category_cnt = categ_cnt;
			}
		}

		hits += dominant_category_cnt;
		leaves[i-1]->cls = dominant_category;
		leaves[i-1]->impurity = ((float) (total_leaf_inst_cnt - dominant_category_cnt)) / (total_leaf_inst_cnt + 1); // +1 not to divise by zero when no instance in leaf

        tot_impurity += leaves[i-1]->impurity;

		leaves_total_inst_cnt[i-1] = total_leaf_inst_cnt;
	}

	for (i = nonleaves_cnt - 1; i >= 0; i--) {
		nonleaves[i]->impurity = nonleaves[i]->left->impurity + nonleaves[i]->right->impurity;
	}

#endif

#if ((EFTI_HW == 1))
	status = 0;
    while ((status & 0xffff0000) == 0)
    {
    	status = Xil_In32(DT_HW_STAT1_ADDR);
    }

#if (EFTI_SW == 1)
    HbAssert((status >> 16) == hits);
#else
    hits = status >> 16;
#endif

#endif

    accuracy = ((float) hits)/inst_cnt;
    oversize = (((float) leaves_cnt) - ((float) categ_max))/((float)categ_max);
    impurity = tot_impurity / leaves_cnt;

    fitness = accuracy * (1 - efti_conf->complexity_weight*oversize) * (1 - efti_conf->impurity_weight*impurity);

    return fitness;

}

#if EFTI_HW == 1

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

#endif

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

tree_node* efti(float* fitness, uint32_t* dt_leaves_cnt,
                uint32_t* dt_nonleaves_cnt, float* t_hb, unsigned int *seed)
{
	uint32_t returned_to_best_iter;

	uint32_t stagnation_iter;

	tree_node* temp_mut_hang_tree;
	tree_node* topo_mut_node;
	tree_node* topo_mut_sibling;
	uint16_t weight_temp;

	float topo_mutation_probability;
	float return_to_best_probability;
	float search_probability;

	float fitness_best, fitness_cur, fitness_new;
	unsigned i, j;
	char fn[512];
	char* fn_fmt = "/data/projects/rst/examples/doktorat/source/images/efti_overview_dts/json/%d.js";

	uint32_t exec_time = timing_get();

	seedp = seed;
#if EFTI_HW == 1
	Xil_Out32(DT_HW_INST_NUM_ADDR, inst_cnt - 1);
#endif

	tree_init();

	dt_cur = tree_create();
	tree_create_child(dt_cur, CHILD_LEFT);
	tree_create_child(dt_cur, CHILD_RIGHT);
	random_hiperplane(dt_cur->weights);
#if (EFTI_HW == 1)
	pack_coefs(dt_cur->weights, NUM_ATTRIBUTES + 1, COEF_RES, dt_cur->banks);
#endif

	dt_best = tree_copy(dt_cur);

	extract_hierarcy(dt_cur);

#if (EFTI_HW == 1)
	hw_set_whole_tree(dt_cur);
#endif

	fitness_best = fitness_cur = fitness_eval(dt_cur);
	stagnation_iter = 0;
	returned_to_best_iter = 0;
	temp_mut_hang_tree = NULL;
	topo_mut_node = NULL;
	topo_mut_sibling = NULL;

//	sprintf(fn, fn_fmt, 0);
//	dump_dt2json(fn, dt_cur, dataset);
//	efti_printf("iter: %06d, fitness: %.3f, size: %d, accuracy: %.3f\n", 0, fitness_cur, leaves_cnt, accuracy);

	for (current_iter = 0; current_iter < efti_conf->max_iterations; current_iter++)
	{
#if (EFTI_PRINT_PROGRESS_INTERVAL != 0)
		if (current_iter % EFTI_PRINT_PROGRESS_INTERVAL == 0) {
			efti_printf("Current iteration: %d\n", current_iter);
		}

		if (current_iter == 912823) {
			efti_printf("Current iteration: %d\n", current_iter);
//			extract_hierarcy(dt_cur);
		}
#endif

		topology_mutated = 0;

		topo_mutation_probability = efti_conf->topology_mutation_rate * leaves_cnt;

		topo_mutation_probability *= 1 + stagnation_iter*efti_conf->topo_mutation_rate_raise_due_to_stagnation_step;

		if (topo_mutation_probability > rand_norm())
		{
			float tot_inv_fullness = 0;
			if (efti_conf->use_impurity_topo_mut) {
				tot_impurity = 0;
				for (i = 0; i < leaves_cnt; i++) {
					tot_impurity += leaves[i]->impurity;
					tot_inv_fullness = 1.0 / (leaves_total_inst_cnt[i] + 1); // +1 not to divise by zero when no instance in leaf
				}
			}

			while (topology_mutated == 0) {
				if (!efti_conf->use_impurity_topo_mut) {
					topo_mut_node = leaves[rand_r(seedp) % leaves_cnt];
				} else {
					topo_mut_node = NULL;
				}

				if (rand_r(seedp) % 2) {
					if (efti_conf->use_impurity_topo_mut) {
						float rand_scaled = (float) rand_r(seedp) / RAND_MAX * tot_impurity;

						for (i = 0; i < leaves_cnt; i++) {
							rand_scaled -= leaves[i]->impurity;
							if (rand_scaled <= 0) {
								topo_mut_node = leaves[i];
								break;
							}
						}
					}

					if (topo_mut_node->level < (MAX_TREE_DEPTH - 1))
					{
						topology_mutated = TOPO_CHILDREN_ADDED;
						tree_create_child(topo_mut_node, CHILD_LEFT);
						tree_create_child(topo_mut_node, CHILD_RIGHT);
						random_hiperplane(topo_mut_node->weights);
#if (EFTI_HW == 1)
						pack_coefs(topo_mut_node->weights, NUM_ATTRIBUTES + 1, COEF_RES, topo_mut_node->banks);
#endif
					}
				} else {
					if (efti_conf->use_impurity_topo_mut) {
						float rand_scaled = (float) rand_r(seedp) / RAND_MAX * tot_inv_fullness;

						for (i = 0; i < leaves_cnt; i++) {
							rand_scaled -= 1.0 / (leaves_total_inst_cnt[i] + 1);
							if (rand_scaled <= 0) {
								topo_mut_node = leaves[i];
								break;
							}
						}
					}

					// Make impossible to remove the root if its the only node
					if (leaves_cnt > 2) {
						temp_mut_hang_tree = topo_mut_node->parent;
						topo_mut_sibling = tree_get_sibling(topo_mut_node);

						if (temp_mut_hang_tree == dt_cur) {
							topology_mutated = TOPO_ROOT_CHILD_REMOVED;
							dt_cur = topo_mut_sibling;
						}
						else if (temp_mut_hang_tree->parent->left == temp_mut_hang_tree)
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
				}
			}

			extract_hierarcy(dt_cur);
#if (EFTI_HW == 1)
			hw_set_whole_tree(dt_cur);
#endif
		}

		weights_mutation_cnt = 1 + efti_conf->weights_mutation_rate *
							   (1 + stagnation_iter*efti_conf->weight_mutation_rate_raise_due_to_stagnation_step) *
							   nonleaves_cnt;

		for (i = 0; i < weights_mutation_cnt; i++)
		{
			uint32_t mut_mask_bit;
			if (!efti_conf->use_impurity_weight_mut) {
				mut_nodes[i] = nonleaves[rand_r(seedp) % nonleaves_cnt];
			} else {
				float tot_node_impurity = 0;
				for (j = 0; j < nonleaves_cnt; j++) {
					tot_node_impurity += nonleaves[j]->impurity;
				}

				float rand_scaled = (float) rand_r(seedp) / RAND_MAX * tot_node_impurity;

				for (j = 0; j < nonleaves_cnt; j++) {
					rand_scaled -= nonleaves[j]->impurity;
					if (rand_scaled <= 0) {
						mut_nodes[i] = nonleaves[j];
						break;
					}
				}
			}

			if (rand_r(seedp) % 2)
			{
				mut_attr[i] = rand_r(seedp) % attr_cnt;
			}
			else
			{
				mut_attr[i] = NUM_ATTRIBUTES;
			}

			mut_bit[i] = rand_r(seedp) % COEF_RES;
//			mut_banks[i] = rand_r(seedp) % DT_MEM_COEF_BANKS_NUM;
//			mut_masks[i] = 1 << (rand_r(seedp) % 32);

#if (EFTI_HW == 1)
			get_bank_bit(mut_attr[i], mut_bit[i], &mut_banks[i], &mut_mask_bit);
			mut_masks[i] = 1 << mut_mask_bit;
//			pack_coefs(mut_nodes[i]->weights, NUM_ATTRIBUTES + 1, COEF_RES, coef_packed_mem);
			mut_bank_val[i] = mut_nodes[i]->banks[mut_banks[i]];
			mut_nodes[i]->banks[mut_banks[i]] ^= mut_masks[i];
			*(DT_MEM_COEF_ADDR(mut_nodes[i]->level, mut_nodes[i]->id, mut_banks[i])) = mut_nodes[i]->banks[mut_banks[i]];
#endif

#if (EFTI_SW == 1)
			mut_attr_val[i] = mut_nodes[i]->weights[mut_attr[i]];
			weight_temp = mut_nodes[i]->weights[mut_attr[i]];
			mut_nodes[i]->weights[mut_attr[i]] = (int16_t) (weight_temp ^ (1 << mut_bit[i]));
#endif
		}

		fitness_new = fitness_eval(dt_cur);

		if ((fitness_new - fitness_cur) > 1e-6)
		{
			stagnation_iter = 0;
//#if ((EFTI_HW == 1) && (EFTI_SW == 0))
//			hw_apply_mutation(mut_nodes, mut_attr, mut_bit, weights_mutation_cnt);
//#endif

			fitness_cur = fitness_new;
			delete_trimmed_subtree(topology_mutated, temp_mut_hang_tree, topo_mut_node);
			if ((fitness_cur - fitness_best) > 1e-6)
			{
				tree_delete_node(dt_best);
				dt_best = tree_copy(dt_cur);

				returned_to_best_iter = current_iter;
				fitness_best = fitness_cur;
//				sprintf(fn, fn_fmt, current_iter);
//				dump_dt2json(fn, dt_cur, dataset);
//				efti_printf("iter: %06d, fitness: %.3f, size: %d, accuracy: %.3f\n", current_iter, fitness_cur, leaves_cnt, accuracy);
#if (EFTI_PRINT_STATS == 1)
				efti_printf("AB: i=%d,f=%f,s=%d\n", current_iter, fitness_cur, leaves_cnt);
#endif
			}
			else
			{
#if (EFTI_PRINT_STATS == 1)
				efti_printf("CB: i=%d,f=%f,s=%d\n", current_iter, fitness_cur, leaves_cnt);
#endif
			}
		}
		else
		{
			stagnation_iter++;

			return_to_best_probability = efti_conf->return_to_best_prob_iteration_increment *
										(current_iter - returned_to_best_iter);

			search_probability = efti_conf->search_probability * (1 + stagnation_iter*efti_conf->search_probability_raise_due_to_stagnation_step);

			/* Should we return to the best yet solution since we are wondering without improvement for a long time? */
			if (rand_norm() < return_to_best_probability)
			{
				delete_trimmed_subtree(topology_mutated, temp_mut_hang_tree, topo_mut_node);
				tree_delete_node(dt_cur);
				dt_cur = tree_copy(dt_best);
				extract_hierarcy(dt_cur);
#if EFTI_HW == 1
				hw_set_whole_tree(dt_cur);
#endif
				fitness_cur = fitness_best;
				returned_to_best_iter = current_iter;
				stagnation_iter = 0;
#if (EFTI_PRINT_STATS == 1)
				efti_printf("RB: i=%d\n\r", current_iter);
#endif
			}
			else if (topology_mutated && (rand_norm() < search_probability))
			{
//#if ((EFTI_HW == 1) && (EFTI_SW == 0))
//				hw_apply_mutation(mut_nodes, mut_attr, mut_bit, weights_mutation_cnt);
//#endif
#if (EFTI_PRINT_STATS == 1)
				efti_printf("SP: i=%d\n\r", current_iter);
#endif
				delete_trimmed_subtree(topology_mutated, temp_mut_hang_tree, topo_mut_node);

				fitness_cur = fitness_new;
			}
			else //We failed to advance in fitness :(
			{

				for (i = 0; i < weights_mutation_cnt; i++)
				{
#if (EFTI_SW == 1)
					mut_nodes[i]->weights[mut_attr[i]] = mut_attr_val[i];
#endif
#if (EFTI_HW == 1)
					mut_nodes[i]->banks[mut_banks[i]] = mut_bank_val[i];
#endif
				}

				// If we mutated topology, dt_cur was changed and we need to revert the changes
				if (topology_mutated)
				{
					if (topology_mutated == TOPO_ROOT_CHILD_REMOVED)
					{
						dt_cur = temp_mut_hang_tree;
					}
					else if (topology_mutated == TOPO_RIGHT_CHILD_REMOVED)
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
#if (EFTI_HW == 1)
					hw_set_whole_tree(dt_cur);
#endif
				}
				else
				{
#if (EFTI_HW == 1)
					for (i = 0; i < weights_mutation_cnt; i++)
					{
						*(DT_MEM_COEF_ADDR(mut_nodes[i]->level, mut_nodes[i]->id, mut_banks[i])) = mut_bank_val[i];
					}
#endif
				}

			}
		}

	}

#if EFTI_PROFILING == 0

	tree_delete_node(dt_cur);
	extract_hierarcy(dt_best);
#if EFTI_HW == 1
	hw_set_whole_tree(dt_best);
#endif
	assign_classes(dt_best);

	*t_hb = timing_tick2sec(timing_get() - exec_time);

	*fitness = fitness_best;
	*dt_leaves_cnt = leaves_cnt;
	*dt_nonleaves_cnt = nonleaves_cnt;
#endif

	return dt_best;
}

// CAUTION! This function implies that the decision tree is already setup in hardware and that
// extract_hierarcy and assign_classes has been called upon it. In other words it is meant to
// be called only after efti_run has finished.
float efti_eval(tree_node* dt)
{
#if EFTI_HW == 1
	hw_set_whole_tree(dt);
#endif
	return dt_eval(dt);
}

void efti_reset(const Efti_Conf_t *conf, T_Dataset* ds)
{
	attr_cnt = ds->attr_cnt;
	categ_max = ds->categ_max;
	dataset = ds;
	efti_conf = conf;
	inst_cnt = 0;
}

void efti_init()
{
#if EFTI_HW == 1
	hw_init();
#endif

	timing_init(TIMING_EFTI_ID, 0xffff, 0xf);
}

void efti_close()
{
	timing_close(TIMING_EFTI_ID);
}

