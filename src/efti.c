
#include "efti_conf.h"
#include "rand.h"
#include <assert.h>
#include "hw_config.h"
#include "efti.h"
#include "tree.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "util.h"
#include "timing.h"
#include "dataset.h"
/* #include "dt2js.h" */
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

#define TIMING_EFTI_ID					0
#define TIMING_FITNESS_CALC_ID			1

#define COEF_BANKS_MAX_NUM 		64
#define LEAVES_MAX				NUM_NODES
#define NONLEAVES_MAX			NUM_NODES
#define MAX_WEIGHT_MUTATIONS	NUM_NODES

uint32_t attr_cnt;
uint32_t inst_cnt;
uint32_t categ_max;
T_Dataset* dataset;
//uint32_t coef_packed_mem[COEF_BANKS_MAX_NUM];
int32_t instances[NUM_INST_MAX][NUM_ATTRIBUTES];

typedef struct {
    tree_node* path[MAX_TREE_DEPTH];
    int64_t sum[MAX_TREE_DEPTH];
} T_Last_Classification;

T_Last_Classification last_iter_classification[NUM_INST_MAX];

//tree_node* last_classification_path[NUM_INST_MAX][MAX_TREE_DEPTH];
//int64_t last_eval_sum[NUM_INST_MAX][MAX_TREE_DEPTH];
int64_t changed_eval_sums[NUM_INST_MAX*MAX_TREE_DEPTH];
tree_node* changed_paths[NUM_INST_MAX*MAX_TREE_DEPTH];
int32_t changed_instances[NUM_INST_MAX*MAX_TREE_DEPTH];
int32_t changed_depths[NUM_INST_MAX*MAX_TREE_DEPTH];
int32_t changed_nodes_num;
uint64_t total_nodes_traversed;
uint64_t needed_nodes_traversed;
float perc_recalc;

#define CLK_FREQ 	225e+6
#define CLOCKS_PER_SEC (CLK_FREQ / 4 / 10)
/* tree_node* dt_best; */
/* tree_node* dt_cur; */

DT_t dt_best;
DT_t dt_cur;

uint32_t weights_mutation_cnt;
uint32_t topology_mutated;
tree_node* temp_mut_hang_tree;
tree_node* topo_mut_node;
tree_node* topo_mut_sibling;
uint_fast16_t mut_banks[MAX_WEIGHT_MUTATIONS];
tree_node* mut_nodes[MAX_WEIGHT_MUTATIONS];
uint32_t mut_masks[MAX_WEIGHT_MUTATIONS];
uint32_t mut_attr[MAX_WEIGHT_MUTATIONS];
int32_t mut_attr_val[MAX_WEIGHT_MUTATIONS];
uint32_t mut_bit[MAX_WEIGHT_MUTATIONS];
uint32_t mut_bank_val[MAX_WEIGHT_MUTATIONS];
uint32_t categories[NUM_INST_MAX];
uint32_t current_iter;
uint32_t returned_to_best_iter;
float fitness_best;
uint32_t stagnation_iter;
uint32_t delta_on;
uint32_t searching;

uint32_t node_hierarchy_cnt[MAX_TREE_DEPTH];
//Enumeration of leaves starts from 1, so it is cheaper to ignore the first
//row to distribution matrix, and hence have one more in total
uint_fast16_t node_categories_distrib[LEAVES_MAX+1][NUM_ATTRIBUTES];

uint32_t non_eval_ticks = 0;

#define TOPO_CHILDREN_ADDED			1
#define TOPO_LEFT_CHILD_REMOVED		2
#define TOPO_RIGHT_CHILD_REMOVED	3
#define TOPO_ROOT_CHILD_REMOVED 	4

#define MAX_ATTR_VAL ((1 << (COEF_RES - 1)) - 1)

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

#include <float.h>
double norm(double mu, double sigma)
{
    const double epsilon = -DBL_MAX;
    const double two_pi = 2.0*3.14159265358979323846;

    static double z0, z1;
    static unsigned generate;
    generate = !generate;

    if (!generate)
        return z1 * sigma + mu;

    double u1, u2;
    do
    {
        u1 = rand_norm();
        u2 = rand_norm();
    }
    while ( u1 <= epsilon );

    z0 = sqrt(-2.0 * log(u1)) * cos(two_pi * u2);
    z1 = sqrt(-2.0 * log(u1)) * sin(two_pi * u2);
    return z0 * sigma + mu;
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
    uint_fast16_t i;

    inst_i = rand_imax(inst_cnt);
    inst_j = inst_i;
    while (categories[inst_i] == categories[inst_j])
    {
        inst_j = rand_imax(inst_cnt);
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

    delta = ((float) (rand_imax(90)) + 5) / 100;
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

int dt_compare(tree_node* dt1, tree_node* dt2){
    unsigned i;

    assert(dt1->id == dt2->id);

    if (dt1->left == NULL){
        assert(dt2->left == NULL);
    } else if (dt2->left == NULL) {
        assert(0);
    } else {
        dt_compare(dt1->left, dt2->left);
        dt_compare(dt1->right, dt2->right);

        for (i = 0; i < attr_cnt; i++)
        {
            assert(dt1->weights[i] == dt2->weights[i]);
        }
        assert(dt1->weights[NUM_ATTRIBUTES] == dt2->weights[NUM_ATTRIBUTES]);
    }

    return 1;
}

int _extract_hierarcy(DT_t* dt, tree_node* node, uint32_t level)
{
    if (level >= MAX_TREE_DEPTH)
    {
        level = MAX_TREE_DEPTH - 1;
    }

    node->level = level;

    if (node->left == NULL)
    {
        dt->leaves[dt->leaves_cnt++] = node;
        // ID of the leaf is bigger by 1 from its position index, since ID=0 has
        // special meaning for the EFTIP co-processor
        node->id = dt->leaves_cnt;
    }
    else
    {
        node->id = node_hierarchy_cnt[level];

        node_hierarchy_cnt[level]++;

        dt->nonleaves[dt->nonleaves_cnt++] = node;
        _extract_hierarcy(dt, node->left, level + 1);
        _extract_hierarcy(dt, node->right, level + 1);
    }

    if (dt->depth < level) {
        dt->depth = level;
    }

    return 0;
}

int extract_hierarcy(DT_t* dt)
{
    unsigned i;

    dt->leaves_cnt = 0;
    dt->nonleaves_cnt = 0;
    dt->depth = 0;

    for (i = 0; i < MAX_TREE_DEPTH; i++)
    {
        node_hierarchy_cnt[i] = 0;
    }

    return _extract_hierarcy(dt, dt->root, 0);
}

void apply_single_path_change(T_Last_Classification* last_classification, int depth, int64_t res, tree_node* cur_node) {
    last_classification->sum[depth] = res;
    last_classification->path[depth] = cur_node;
}

/* #if (DT_USE_LOOP_UNFOLD == 1) */
/* #include "loop_unfold.c" */
/* #endif */

tree_node* find_dt_leaf_for_inst(tree_node* dt, int32_t attributes[], int32_t inst_id, uint32_t total_recalc_all)
{
    tree_node* cur_node;
    int16_t res_scaled;
    int64_t res;
    uint_fast16_t j;
#if (DELTA_CLASSIFICATION == 1)
    int path_diverged = total_recalc_all | (!delta_on);
#else
    int path_diverged = 1;
#endif
    int depth = 0;
    int cur_node_mutated = 0;
    int cur_node_manipulated = 0;
    T_Last_Classification* last_classification = &last_iter_classification[inst_id];
    cur_node = dt;

    /* if (topology_mutated || recalc_all){//if this isn't a first classification, and topology has not been mutated */
    /*     path_diverged = 1; */
    /* } */

    while (cur_node->left != NULL)
    {
        tree_node* prev_node = cur_node;
        cur_node_manipulated = 0;
        cur_node_mutated = 0;

        if (!path_diverged){
            if (topology_mutated) {
                if (cur_node == topo_mut_node) {
                    cur_node_manipulated = 1;
                } else if (temp_mut_hang_tree != NULL) {
                    if (cur_node == temp_mut_hang_tree->parent) {
                        cur_node_manipulated = 1;
                    } else if (topology_mutated == TOPO_ROOT_CHILD_REMOVED) {
                        cur_node_manipulated = 1;
                    }
                }
            }

            if (cur_node_manipulated) {
                needed_nodes_traversed++;
                res = evaluate_node_test(cur_node->weights, attributes, attr_cnt);
            } else {
                for(j = 0; j < weights_mutation_cnt; j++) {
                    if (cur_node == mut_nodes[j]) {
                        if (!cur_node_mutated) {
                            res = last_classification->sum[depth];
                        }
                        cur_node_mutated = 1;
                        if (mut_attr[j] != NUM_ATTRIBUTES) {
                            res -= mut_attr_val[j]*attributes[mut_attr[j]];
                            res += cur_node->weights[mut_attr[j]] * attributes[mut_attr[j]];
                        }
                    }
                }

                if (cur_node_mutated)
                    assert(res == evaluate_node_test(cur_node->weights, attributes, attr_cnt));
            }

            if (cur_node_mutated || cur_node_manipulated) {
                path_diverged = 1;
            } else {
                cur_node = last_classification->path[depth];
                assert(cur_node->parent == prev_node);
            }
        } else {
#if (DT_USE_LOOP_UNFOLD == 1)
            needed_nodes_traversed++;
            res = evaluate_node_test(cur_node->weights, attributes, attr_cnt);
#else
            res = 0;
            for (j = 0; j < attr_cnt; j++)
            {
                res += cur_node->weights[j] * attributes[j];
            }
#endif
        }

        if (path_diverged) {
            res_scaled = res >> ATTRIBUTE_RES >> DT_ADDER_TREE_DEPTH;

            if (res_scaled >= cur_node->weights[NUM_ATTRIBUTES])
            {
                cur_node = cur_node->right;
            }
            else
            {
                cur_node = cur_node->left;
            }
#if (DELTA_CLASSIFICATION == 1)

            /* path_maybe_merged_back = save_path_change_temp(cur_node, depth, inst_id, res, last_classification); */
                // Watch for orphan subtrees that are still listed in saved paths
            if ((delta_on) && (cur_node_mutated) && is_child_of(prev_node, cur_node)) {
                if (cur_node == last_classification->path[depth]) {
                    path_diverged = 0;
                }
            }
#endif
        }

#if (DELTA_CLASSIFICATION == 1)
        if (total_recalc_all && delta_on) {
            apply_single_path_change(last_classification, depth, res, cur_node);
        }
#endif

        depth++;
        total_nodes_traversed++;

    }

    return cur_node;
}

void recalculate_path(DT_t* dt) {
    for (uint32_t i = 0 ; i < inst_cnt; i++)
    {
        find_dt_leaf_for_inst(dt->root, instances[i], i, 1);
    }
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

void find_node_distribution(DT_t* dt, uint32_t recalc_all)
{
    unsigned i;
    int16_t res;

    changed_nodes_num = 0;
    if (dt->depth == 1) {
        for (i = 0 ; i < inst_cnt; i++)
        {
            uint32_t categ = categories[i];
            res = evaluate_node_test(dt->root->weights, instances[i], attr_cnt) >> ATTRIBUTE_RES >> DT_ADDER_TREE_DEPTH;

            if (res >= dt->root->weights[NUM_ATTRIBUTES])
            {
                /* assert(find_dt_leaf_for_inst(dt->root, instances[i], i, 1)->id == 2); */
                node_categories_distrib[2][categ]++;
            }
            else
            {
                /* assert(find_dt_leaf_for_inst(dt->root, instances[i], i, 1)->id == 1); */
                node_categories_distrib[1][categ]++;
            }
        }
    } else {
        for (i = 0 ; i < inst_cnt; i++)
        {
            uint32_t node;

#if (EFTI_SW == 1)

            node = find_dt_leaf_for_inst(dt->root, instances[i], i, recalc_all)->id;
#if (EFTI_HW_SW_FITNESS == 1)
            HbAssert(node == (*rxBuf++));
#endif
#else
            node_id = (*rxBuf++);
#endif
            uint32_t categ = categories[i];

            assert(node <= LEAVES_MAX);
            assert(categ <= categ_max);
            node_categories_distrib[node][categ]++;

            /* if (current_iter == 12) { */
            /*     efti_printf("Classify %d into %d\n", i, node); */
            /* } */
        }
    }
}

float ensemble_eval(DT_t* dt[], int ensemble_size) {
    uint32_t hits = 0;
    unsigned i, j;
    uint32_t vote[categ_max+1];

    for (i = 0 ; i < inst_cnt; i++)
    {
        for (j = 1; j <= categ_max; j++)
        {
            vote[j] = 0;
        }

        for (j = 0 ; j < (unsigned)ensemble_size; j++)
        {
            tree_node* node;

            node = find_dt_leaf_for_inst(dt[j]->root, instances[i],i,1);

            vote[node->cls]++;
        }

        uint32_t max_vote = rand_imax(categ_max) + 1;

        for (j = 1; j <= categ_max; j++)
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

float accuracy_calc(DT_t* dt, uint32_t recalc_all_paths) {
    uint_fast16_t hits;

    for (uint_fast16_t i = 1; i <= dt->leaves_cnt; i++)
    {
        memset(node_categories_distrib[i], 0, sizeof(uint_fast16_t)*(categ_max+1));
    }


    find_node_distribution(dt, recalc_all_paths);

    hits = 0;

    for (uint_fast16_t i = 1; i <= dt->leaves_cnt; i++)
    {
        uint_fast16_t* node_distrib = &node_categories_distrib[i][1];
        uint_fast16_t dominant_category_cnt = *node_distrib;
        uint_fast16_t dominant_category = 1;
        uint_fast16_t total_leaf_inst_cnt = *node_distrib;

        for (uint_fast16_t j = 1; j < categ_max; j++)
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
        dt->leaves[i-1]->cls = dominant_category;

#if (IMPURITY_CALC == 1)
        leaves[i-1]->impurity = ((float) (total_leaf_inst_cnt - dominant_category_cnt)) / (total_leaf_inst_cnt + 1); // +1 not to divide by zero when no instance in leaf
#endif
    }

#if (IMPURITY_CALC == 1)
    for (int i = nonleaves_cnt - 1; i >= 0; i--) {
        nonleaves[i]->impurity = nonleaves[i]->left->impurity + nonleaves[i]->right->impurity;
    }
#endif

    return ((float) hits)/inst_cnt;

}

void fitness_eval(DT_t* dt, uint32_t recalc_all_paths)
{
#if ((EFTI_SW == 1) || (EFTI_HW_SW_FITNESS == 1))

    dt->accuracy = accuracy_calc(dt, recalc_all_paths);

#elif ((EFTI_HW == 1))

    hw_start(EFTI_HW_SW_FITNESS);

    uint32_t status = 0;
    while ((status & 0xffff0000) == 0)
    {
        status = Xil_In32(DT_HW_STAT1_ADDR);
    }

    hits = status >> 16;
    accuracy = ((float) hits)/inst_cnt;

#endif

    dt->oversize = (((float) dt->leaves_cnt) - ((float) categ_max))/((float)categ_max);

#if (IMPURITY_CALC == 1)
    float tot_impurity = 0;
    for (int i = 0; i < leaves_cnt; i++) {
        tot_impurity += leaves[i]->impurity;
    }
    impurity = tot_impurity / leaves_cnt;
#endif

    dt->fit = dt->accuracy * (1 - efti_conf->complexity_weight*dt->oversize*dt->oversize) * (1 - efti_conf->impurity_weight*dt->impurity);

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

void mutation(DT_t* dt) {

    float topo_mutation_probability;

    topology_mutated = 0;

    /* topo_mutation_probability = efti_conf->topology_mutation_rate * leaves_cnt; */

    /* topo_mutation_probability *= 1 + stagnation_iter*efti_conf->topo_mutation_rate_raise_due_to_stagnation_step; */
    if (dt->leaves_cnt < categ_max) {
        topo_mutation_probability = 0.2;
    } else {
        topo_mutation_probability = 0.5;
    }

    /* topo_mutation_probability = 0.5; */
    if (topo_mutation_probability > rand_norm())
    {
        while (topology_mutated == 0) {
            if (!efti_conf->use_impurity_topo_mut) {
                topo_mut_node = dt->leaves[rand_imax(dt->leaves_cnt)];
            } else {
                topo_mut_node = NULL;
            }

            double add_chance;
            if (dt->leaves_cnt < categ_max) {
                add_chance = 0.3;
            } else {
                add_chance = 0.5;
            }

            /* if (rand_r(seedp) % 2) { */
            if (rand_norm() < add_chance) {
                /* if (rand_norm() > 0.5) { */
                /* if ((rand_norm() < 0.3 ? (leaves_cnt < categ_max) : 0.5)) { */
                if (efti_conf->use_impurity_topo_mut) {
                    float rand_scaled = rand_norm() * dt->impurity;

                    for (uint_fast16_t i = 0; i < dt->leaves_cnt; i++) {
                        rand_scaled -= dt->leaves[i]->impurity;
                        if (rand_scaled <= 0) {
                            topo_mut_node = dt->leaves[i];
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
                    temp_mut_hang_tree = NULL;
                    topo_mut_sibling = NULL;
#if (EFTI_HW == 1)
                    pack_coefs(topo_mut_node->weights, NUM_ATTRIBUTES + 1, COEF_RES, topo_mut_node->banks);
#endif
                }
            } else {
                if (efti_conf->use_impurity_topo_mut) {
                    topo_mut_node = dt->leaves[rand_imax(dt->leaves_cnt)];
                }

                // Make impossible to remove the root if its the only node
                if (dt->leaves_cnt > 2) {
                    temp_mut_hang_tree = topo_mut_node->parent;
                    topo_mut_sibling = tree_get_sibling(topo_mut_node);

                    if (temp_mut_hang_tree == dt->root) {
                        topology_mutated = TOPO_ROOT_CHILD_REMOVED;
                        dt->root = topo_mut_sibling;
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

        extract_hierarcy(dt);
#if (EFTI_HW == 1)
        hw_set_whole_tree(dt);
#endif
    }
    /* weights_mutation_cnt = 1 + (int) (dt->nonleaves_cnt*attr_cnt*efti_conf->weights_mutation_rate/categ_max); */
    /* if (weights_mutation_cnt > 1) { */
    /*     efti_printf("Next\n"); */
    /* } */
    weights_mutation_cnt = 1 + stagnation_iter*efti_conf->weight_mutation_rate_raise_due_to_stagnation_step;
    if (weights_mutation_cnt > 2) {
        weights_mutation_cnt = 2;
    }
    /* if (attr_cnt < 15) { */
    /*     if (dt->nonleaves_cnt < categ_max) { */
    /*         weights_mutation_cnt = 1; */
    /*     } else { */
    /*         weights_mutation_cnt = 1; */
    /*     } */
    /* } else { */
    /*     if (dt->nonleaves_cnt < categ_max) { */
    /*         weights_mutation_cnt = 1; */
    /*     } else { */
    /*         weights_mutation_cnt = 2; */
    /*     } */
    /* } */

    for (uint_fast16_t i = 0; i < weights_mutation_cnt; i++)
    {
        int unique_mutation = 0;
        while (!unique_mutation) {
            if (!efti_conf->use_impurity_weight_mut) {
                mut_nodes[i] = dt->nonleaves[rand_imax(dt->nonleaves_cnt)];
            } else {
                float tot_node_impurity = 0;
                for (uint_fast16_t j = 0; j < dt->nonleaves_cnt; j++) {
                    tot_node_impurity += dt->nonleaves[j]->impurity;
                }

                float rand_scaled = (float) rand_norm() * tot_node_impurity;

                for (uint_fast16_t j = 0; j < dt->nonleaves_cnt; j++) {
                    rand_scaled -= dt->nonleaves[j]->impurity;
                    if (rand_scaled <= 0) {
                        mut_nodes[i] = dt->nonleaves[j];
                        break;
                    }
                }
            }

            if (rand_norm() > 0.3)
            {
                mut_attr[i] = rand_imax(attr_cnt);
            }
            else
            {
                mut_attr[i] = NUM_ATTRIBUTES;
            }

            unique_mutation = 1;
            /* if ((mut_nodes[i] == topo_mut_node) && (topology_mutated)) { */
            /*     unique_mutation = 0; */
            /* } else { */
                for (uint_fast16_t j = 0; j < i; j++) {
                    if ((mut_nodes[j] == mut_nodes[i]) &&
                         (mut_attr[j] == mut_attr[i]))  {
                    /* if (mut_nodes[j] == mut_nodes[i]) { */
                        /* efti_printf("Failed at %d", mut_nodes[i]->id); */
                        unique_mutation = 0;
                        break;
                    }
                }
            /* } */
        }

        mut_bit[i] = rand_imax(COEF_RES);
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
        /* mut_nodes[i]->weights[mut_attr[i]] = (int16_t) (mut_attr_val[i] ^ (1 << mut_bit[i])); */
        double sigma = MAX_ATTR_VAL/5; //mut_nodes[i]->to_bottom;
        double delta = norm(0, sigma);
        if (mut_attr_val[i] + delta >= MAX_ATTR_VAL) {
            mut_nodes[i]->weights[mut_attr[i]] = MAX_ATTR_VAL;
        } else if (mut_attr_val[i] + delta <= -MAX_ATTR_VAL) {
            mut_nodes[i]->weights[mut_attr[i]] = -MAX_ATTR_VAL;
        } else {
            mut_nodes[i]->weights[mut_attr[i]] += delta;
        }

#endif
    }
}

void dt_copy(DT_t* src, DT_t* dest) {
    if (dest->root) {
        tree_delete_node(dest->root);
    }

    *dest = *src;
    dest->root = tree_copy(src->root);
    extract_hierarcy(dest);
}

void dt_free(DT_t* dt) {
    tree_delete_node(dt->root);
}

float selection(float fit, DT_t* dt_mut, DT_t* dt_best) {
    float return_to_best_probability;
    float search_probability;
    if ((dt_mut->fit - fit) > 1e-6)
    {
        stagnation_iter = 0;
#if (DELTA_CLASSIFICATION == 1)
        if (delta_on) recalculate_path(dt_mut);
#endif
        delete_trimmed_subtree(topology_mutated, temp_mut_hang_tree, topo_mut_node);
        if ((dt_mut->fit - dt_best->fit) > 1e-6)
        {
            dt_copy(dt_mut, dt_best);
            returned_to_best_iter = current_iter;
#if (EFTI_PRINT_STATS == 1)
            if (searching) {
                efti_printf("$event:name=\"SAB\",iter=%d,fit=%f,size=%d\n", current_iter, dt_mut->fit, dt_mut->leaves_cnt);
            } else {
                efti_printf("$event:name=\"AB\",iter=%d,fit=%f,size=%d\n", current_iter, dt_mut->fit, dt_mut->leaves_cnt);
            }
#endif
            searching = 0;
        }
        else
        {
#if (EFTI_PRINT_STATS == 1)
            efti_printf("$event:name=\"CB\",iter=%d,fit=%f,size=%d\n", current_iter, dt_mut->fit, dt_mut->leaves_cnt);
#endif
        }
    }
    else
    {
        stagnation_iter++;

        return_to_best_probability = efti_conf->return_to_best_prob_iteration_increment; // *
            // (current_iter - returned_to_best_iter);

        /* search_probability = efti_conf->search_probability * (1 + stagnation_iter*efti_conf->search_probability_raise_due_to_stagnation_step); */
        float dist = (fit - dt_mut->fit)/fit;
        /* search_probability = efti_conf->search_probability * */
        /*     (1 + stagnation_iter*efti_conf->search_probability_raise_due_to_stagnation_step) * */
        search_probability = stagnation_iter *
            efti_conf->search_probability_raise_due_to_stagnation_step *
            exp(-dist*10);

        float should_return = (rand_norm() < return_to_best_probability);
        float should_search = (rand_norm() < search_probability);

        /* Should we return to the best yet solution since we are wondering without improvement for a long time? */
        if ((searching) && (should_return))
        {
            searching = 0;
            delete_trimmed_subtree(topology_mutated, temp_mut_hang_tree, topo_mut_node);
            dt_copy(dt_best, dt_mut);
            if (delta_on) recalculate_path(dt_mut);
#if EFTI_HW == 1
            hw_set_whole_tree(dt_mut);
#endif
            returned_to_best_iter = current_iter;
            stagnation_iter = 0;
#if (EFTI_PRINT_STATS == 1)
            efti_printf("$event:name=\"RB\",iter=%d,fit=%f,size=%d\n", current_iter, dt_best->fit, dt_best->leaves_cnt);
#endif
        }
        /* else if (topology_mutated && (rand_norm() < search_probability)) */
        else if ((!searching) && (should_search))
        /* else if (rand_norm() < search_probability) */
        {
            searching = 1;
//#if ((EFTI_HW == 1) && (EFTI_SW == 0))
//				hw_apply_mutation(mut_nodes, mut_attr, mut_bit, weights_mutation_cnt);
//#endif
#if (EFTI_PRINT_STATS == 1)
            efti_printf("$event:name=\"SP\",iter=%d,fit=%f,size=%d\n", current_iter, dt_mut->fit, dt_mut->leaves_cnt);
            /* efti_printf("SP: i=%d\n\r", current_iter); */
#endif
            delete_trimmed_subtree(topology_mutated, temp_mut_hang_tree, topo_mut_node);
            /* apply_path_changes(); */
#if (DELTA_CLASSIFICATION == 1)
            if (delta_on) recalculate_path(dt_mut);
#endif
        }
        else //We failed to advance in fitness :(
        {
            dt_mut->fit = fit;
            for (int i = weights_mutation_cnt; i > 0; i--)
            {
#if (EFTI_SW == 1)
                mut_nodes[i-1]->weights[mut_attr[i-1]] = mut_attr_val[i-1];
#endif
#if (EFTI_HW == 1)
                mut_nodes[i-1]->banks[mut_banks[i-1]] = mut_bank_val[i-1];
#endif
            }

            // If we mutated topology, dt_cur was changed and we need to revert the changes
            if (topology_mutated)
            {
                if (topology_mutated == TOPO_ROOT_CHILD_REMOVED)
                {
                    dt_mut->root = temp_mut_hang_tree;
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

                extract_hierarcy(dt_mut);
#if (EFTI_HW == 1)
                hw_set_whole_tree(dt_mut);
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

            /* dt_compare(dt_cur, dt_best); */
        }
    }

    return fit;
}

void dt_init(DT_t* dt) {
    dt->fit = 0;
    dt->accuracy = 0;
    dt->depth = 0;
    dt->leaves_cnt = 0;
    dt->nonleaves_cnt = 0;
    dt->root = NULL;
    dt->oversize = 0;
}

DT_t* efti(float* t_hb)
{

    float fit;

    /* char fn[512]; */
    /* char* fn_fmt = "/data/projects/rst/examples/doktorat/source/images/efti_overview_dts/json/%d.js"; */

    struct timeval exec_time = timing_get();

#if EFTI_HW == 1
    Xil_Out32(DT_HW_INST_NUM_ADDR, inst_cnt - 1);
#endif

    dt_init(&dt_cur);
    dt_init(&dt_best);
    dt_cur.root = tree_create();
    tree_create_child(dt_cur.root, CHILD_LEFT);
    tree_create_child(dt_cur.root, CHILD_RIGHT);
    random_hiperplane(dt_cur.root->weights);
#if (EFTI_HW == 1)
    pack_coefs(dt_cur->weights, NUM_ATTRIBUTES + 1, COEF_RES, dt_cur->banks);
#endif

    extract_hierarcy(&dt_cur);

#if (EFTI_HW == 1)
    hw_set_whole_tree(dt_cur);
#endif

    delta_on = 0;
    fitness_eval(&dt_cur, delta_on);
    dt_copy(&dt_cur, &dt_best);
    /* recalculate_path(dt_cur); */
    stagnation_iter = 0;
    returned_to_best_iter = 0;
    temp_mut_hang_tree = NULL;
    topo_mut_node = NULL;
    topo_mut_sibling = NULL;

//	sprintf(fn, fn_fmt, 0);
//	dump_dt2json(fn, dt_cur, dataset);
//	efti_printf("iter: %06d, fitness: %.3f, size: %d, accuracy: %.3f\n", 0, fit, leaves_cnt, accuracy);

    for (current_iter = 0; current_iter < efti_conf->max_iterations; current_iter++)
    {
#if (EFTI_PRINT_PROGRESS_INTERVAL != 0)
        if (current_iter % EFTI_PRINT_PROGRESS_INTERVAL == 0) {
            efti_printf("Current iteration: %d\n", current_iter);
        }
#endif
        mutation(&dt_cur);
        fit = dt_cur.fit;
        fitness_eval(&dt_cur, 0);
        selection(fit, &dt_cur, &dt_best);
#if (DELTA_CLASSIFICATION == 1)
        if (!delta_on) {
            if (dt_cur.depth >= DELTA_ON_DEPTH_THR) {
                /* efti_printf("DELTA ON, depth: %d\n", dt_cur.depth); */
                delta_on = 1;
                recalculate_path(&dt_cur);
            }
        } else {
            if (dt_cur.depth <= DELTA_OFF_DEPTH_THR) {
                /* efti_printf("DELTA OFF, depth: %d\n", dt_cur.depth); */
                delta_on = 0;
            }
        }
#endif
    }

    dt_free(&dt_cur);
#if EFTI_HW == 1
    hw_set_whole_tree(dt_best);
#endif

    *t_hb = timing_tick2sec(exec_time);

    return &dt_best;
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

