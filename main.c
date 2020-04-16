#define TEST

#include "vlist.h"
#include <stdlib.h>
#include <time.h>

#ifdef TEST
#include <string.h>

#define TEST_SIZE 10000000

/**> TEST 관련 매크로 설정 부분 */
// #define RANDOMIZE_TEST

#ifdef TEST
#define STR_SIZE 256
#endif

#endif

void test(void);

int main(void)
{
#ifdef TEST
        test();
#endif
        return 0;
}

#ifdef TEST
void test(void)
{
        struct vlist *vlist, *vlist2;
        int splice_position;
        int *int_arr = (int *)malloc(TEST_SIZE * sizeof(int));
        char **str_arr = (char **)malloc(TEST_SIZE * sizeof(char *));
        for (int i = 0; i < TEST_SIZE; i++) {
                str_arr[i] = (char *)malloc(STR_SIZE * sizeof(char));
        }

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
                        .is_primitive = true,
                        .ivalue = value,
                };
                vlist_add_sublist_node(vlist, &node);

                if (i == splice_position) {
                        vlist2 = vlist_alloc(vlist->head);
                }

                int_arr[i] = value;
        }

        pr_info("ADD TEST 1 PASSED....(vlist size: " SIZE_FORMAT ")\n",
                vlist_size(vlist));

        for (int i = 0; i < TEST_SIZE; i++) {
                int index = rand() % TEST_SIZE;
                int value = vlist_get_sublist_node(vlist, index)->ivalue;
                if (value != int_arr[TEST_SIZE - index - 1]) {
                        pr_info("[ERROR] %d location %d <=> %d\n", index, value,
                                int_arr[TEST_SIZE - index - 1]);
                }
        }

        pr_info("COMPARE TEST 1 PASSED....(vlist size: " SIZE_FORMAT ")\n",
                vlist_size(vlist));

        for (int index = TEST_SIZE - 1; index >= 0; index--) {
                int value1 =
                        vlist_get_sublist_node(vlist, TEST_SIZE - (1 + index))
                                ->ivalue;
                if (index <= splice_position) {
                        int value2 = vlist_get_sublist_node(
                                             vlist2, splice_position - index)
                                             ->ivalue;
                        if (value1 != value2) {
                                pr_info("[ERROR] %d location %d <=> %d (size:" SIZE_FORMAT
                                        ")\n",
                                        index, value1, value2,
                                        vlist_size(vlist2));
                        }
                }
        }

        pr_info("REFERENCE TEST PASSED....(vlist2 size: " SIZE_FORMAT ")\n",
                vlist_size(vlist2));

        for (int i = 0; i < TEST_SIZE; i++) {
                int value = rand();
                struct sublist_node node = {
                        .size = sizeof(int),
                        .is_primitive = true,
                        .ivalue = value,
                };
                vlist_add_sublist_node(vlist2, &node);
                int_arr[i] = value;
        }

        pr_info("ADD TEST 2 PASSED....(vlist2 size: " SIZE_FORMAT ")\n",
                vlist_size(vlist2));

        for (int i = 0; i < 100000; i++) {
                int index = rand() % TEST_SIZE;
                int value = vlist_get_sublist_node(vlist2, index)->ivalue;
                if (value != int_arr[TEST_SIZE - index - 1]) {
                        pr_info("[ERROR] %d location %d <=> %d\n", index, value,
                                int_arr[TEST_SIZE - index - 1]);
                }
        }

        pr_info("COMPARE TEST 2 PASSED....(vlist2 size: " SIZE_FORMAT ")\n",
                vlist_size(vlist2));

        for (int size = TEST_SIZE; size > 0; size--) {
#ifdef RANDOMIZE_TEST
                int index = rand() % size;
                vlist_remove_sublist_node(&vlist, index);
#else
                vlist_remove_sublist_node(&vlist, 0);
#endif
        }

        pr_info("REMOVE TEST 1 PASSED....(vlist size: " SIZE_FORMAT ", %p)\n",
                vlist_size(vlist), (void *)vlist);

        for (int i = 0; i < TEST_SIZE; i++) {
                char *buffer = (char *)malloc(256 * sizeof(char));
                sprintf(buffer, "value ==> %d", i);
                struct sublist_node node = {
                        .size = sizeof(int),
                        .is_primitive = false,
                        .buffer = buffer,
                };
                vlist_add_sublist_node(vlist, &node);
                sprintf(str_arr[i], "value ==> %d", i);
        }

        pr_info("RE-ADD TEST 1 PASSED....(vlist size: " SIZE_FORMAT ")\n",
                vlist_size(vlist));

        for (int i = 0; i < TEST_SIZE; i++) {
                int index = i;
                const char *value =
                        vlist_get_sublist_node(vlist, index)->buffer;
                if (strcmp(value, str_arr[TEST_SIZE - index - 1])) {
                        pr_info("[ERROR] %d location %s <=> %s\n", index, value,
                                str_arr[TEST_SIZE - index - 1]);
                }
        }

        pr_info("RE-COMPARE TEST 1 PASSED....(vlist size: " SIZE_FORMAT ")\n",
                vlist_size(vlist));

        struct sublist_node *node;
        for (int size = TEST_SIZE; size > 0; size--) {
                int remove_index = 0, find_index;
#ifdef RANDOMIZE_TEST
                remove_index = rand() % size;
#endif
                vlist_remove_sublist_node(&vlist, remove_index);
                if (size - 1 > 0) {
                        find_index = rand() % (size - 1);
                        node = vlist_get_sublist_node(vlist, find_index);
                        if (node == NULL) {
                                pr_info("%s", "NULL detected\n");
                        }
                }
        }
        pr_info("REMOVE AND GET TEST PASSED...(vlist size: " SIZE_FORMAT ")\n",
                vlist_size(vlist));

        vlist_dealloc(vlist);
        vlist_dealloc(vlist2);

        free(int_arr);
        for (int i = 0; i < TEST_SIZE; i++) {
                free(str_arr[i]);
        }
        free(str_arr);
}
#endif
