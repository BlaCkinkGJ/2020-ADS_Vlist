#include "vlist.h"
#include <stdlib.h>
#include <time.h>

//#define TEST_SIZE 100000000

#define TEST_SIZE 1000

int test_arr[TEST_SIZE];

void test(void);

int main(void)
{
        test();
        return 0;
}

void test(void)
{
        struct vlist *vlist, *vlist2;
        int splice_position;

        srand(time(NULL));

        vlist = vlist_alloc(NULL);
        if (vlist == NULL) {
                pr_info("%s", get_err_msg(ALLOC_FAILED));
        }

        splice_position = rand() % TEST_SIZE;
        for (int i = 0; i < TEST_SIZE; i++) {
                int value = rand();
                struct sublist_node node = {
                        .size = sizeof(int),
                        .ivalue = value,
                };
                vlist_add_sublist_node(vlist, &node);
                if (i == splice_position) {
                        vlist2 = vlist_alloc(vlist->head);
                }
                test_arr[i] = value;
        }

        pr_info("ADD TEST PASSED....\n");

        for (int i = 0; i < 100000; i++) {
                int index = rand() % TEST_SIZE;
                int value = vlist_get_sublist_node(vlist, index)->ivalue;
                if (value != test_arr[TEST_SIZE - index - 1]) {
                        pr_info("[ERROR] %d location %d <=> %d\n", index, value,
                                test_arr[TEST_SIZE - index - 1]);
                }
        }

        pr_info("COMPARE TEST PASSED....\n");

        for (int index = TEST_SIZE - 1; index >= 0; index--) {
                int value1 =
                        vlist_get_sublist_node(vlist, TEST_SIZE - (1 + index))
                                ->ivalue;
                if (index <= splice_position) {
                        int value2 = vlist_get_sublist_node(
                                             vlist2, splice_position - index)
                                             ->ivalue;
                        if (value1 != value2) {
                                pr_info("[ERROR] %d location %d <=> %d (size:%I64u)\n",
                                        index, value1, value2,
                                        vlist_size(vlist2));
                        }
                }
        }

        pr_info("REFERENCE TEST PASSED....\n");

        vlist_dealloc(vlist);
}