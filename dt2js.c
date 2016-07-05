#include "dt2js.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

int find_node_id(tree_node* n, tree_node* ids[], int ids_cnt) {
	int i;
	for (i = 0; i < ids_cnt; i++) {
		if (ids[i] == n) {
			return i;
		}
	}
	return 0;
}

void _dt2json_rec(char** js, tree_node* dt, T_Dataset* ds, tree_node* ids[], int ids_cnt){
	const char *fmt = "\"%d\": {\"lvl\": %d, "
						       	"\"id\": %d,"
							    "\"cls\": %d,"
							    "\"left\": \"%d\","
							    "\"right\": \"%d\","
							    "\"thr\": %0.5f,"
							    "\"coeffs\": [%s]},";

	int left = -1;
	int right = -1;
	char coeffs[256]= "";
	char *coef_cur;
	int cls = 0;
	int i;
	float thr = 0;

	if (dt->left) {
		left = find_node_id(dt->left, ids, ids_cnt);
	}

	if (dt->right) {
		right = find_node_id(dt->right, ids, ids_cnt);
	}

	if (!(dt->left) && !(dt->right)) {
		cls = dt->cls;
	} else {
		thr = ((float) (dt->weights[NUM_ATTRIBUTES] << (DT_ADDER_TREE_DEPTH + 1))) / (1 << 15);
		coef_cur = coeffs;
		for (i = 0; i < ds->attr_cnt; i++) {
			if (i != 0) {
				*coef_cur = ',';
				coef_cur++;
			}

			coef_cur += sprintf(coef_cur, "%0.5f", ((float) dt->weights[i]) / (1 << 15));
		}
	}

	int id = find_node_id(dt, ids, ids_cnt);
	*js += sprintf(*js, fmt, id,
							dt->level,
							id,
							cls,
							left,
							right,
							thr,
							coeffs
			);

	if (dt->left) {
		_dt2json_rec(js, dt->left, ds, ids, ids_cnt);
		_dt2json_rec(js, dt->right, ds, ids, ids_cnt);
	}

}

void dt2json(char** js, tree_node* dt, T_Dataset* ds){
	tree_node* ids[1 << MAX_TREE_DEPTH];
	tree_node* cur;
	int ids_cnt = 0;
	int nodes_processed = 0;

	ids[ids_cnt++] = dt;
	while (ids_cnt > nodes_processed) {
		cur = ids[nodes_processed];
		if (cur->left) {
			ids[ids_cnt++] = cur->left;
			ids[ids_cnt++] = cur->right;
		}
		nodes_processed++;
	}

	_dt2json_rec(js, dt, ds, ids, ids_cnt);
}

char json_buff[2048];

void dump_dt2json(char* fn, tree_node* dt, T_Dataset* ds) {
	char *js;
	json_buff[0] = '{';
	js = &json_buff[1];
	dt2json(&js, dt, ds);

	json_buff[strlen(json_buff) - 1] = 0; //Remove last ',' in string
	strcat(json_buff, "}");

	FILE *fp = fopen(fn, "w");
    if (fp != NULL)
    {
        fputs(json_buff, fp);
        fclose(fp);
    }
}
