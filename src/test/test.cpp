
// tests.cpp
#include "crossvalid.h"
#include <gtest/gtest.h>

TEST(CrossvalidTest, OutOfBound) {
    Cv_Status_T* cv_stat;
    char dselect[8] = "ctg";
    int ensembles = 17;
    int folds = 5;
    int dsets = 1;
    int runs = 1;
    int i,n,k,c;

    cv_stat = crossvalid_init(dselect, ensembles, 29, folds, runs);

    for (i=0; i < dsets; i++) {
        for (n=0; n < runs; n++) {
            for (k=0; k < folds; k++) {
                for (c=0; c < ensembles; c++) {
                    cv_stat = crossvalid_next_conf();
                    printf("$cv_pc_run:dataset=\"%s\",run=%d,id=%d,train_set_size=%d,chunk_range=(%d,%d),"
                           "chunk_size=%d,fold_range=(%d,%d),fold_chunk_size=%d,"
                           "ensemble_size=%d,cur_ensemble=%d \n",
                           cv_stat->dataset->name,
                           n,
                           k,
                           cv_stat->train_set_size,
                           cv_stat->chunk_start,
                           cv_stat->chunk_end,
                           cv_stat->chunk_size,
                           cv_stat->fold_start,
                           cv_stat->fold_start + cv_stat->fold_chunk_size,
                           cv_stat->fold_chunk_size,
                           cv_stat->cur_ensemble_size,
                           cv_stat->cur_ensemble
                        );

                    ASSERT_LE(cv_stat->chunk_end, cv_stat->dataset->inst_cnt);
                    ASSERT_LE(cv_stat->fold_start + cv_stat->fold_chunk_size, cv_stat->dataset->inst_cnt);
                }
            }
        }

        // int train_num =
    }
    // ASSERT_EQ(6, squareRoot(36.0));
    // ASSERT_EQ(18.0, squareRoot(324.0));
    // ASSERT_EQ(25.4, squareRoot(645.16));
    // ASSERT_EQ(0, squareRoot(0.0));
}

// TEST(SquareRootTest, NegativeNos) {
//     ASSERT_EQ(-1.0, squareRoot(-15.0));
//     ASSERT_EQ(-1.0, squareRoot(-0.2));
// }

int main(int argc, char **argv) {
    printf("HERE!");
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
