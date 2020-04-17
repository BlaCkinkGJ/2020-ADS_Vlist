#ifdef __cplusplus
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <vector>
#include <list>
#include <deque>
#include <forward_list>
#else
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#endif
#include "vlist.h"

#define PRINT_RATIO 25000
// #define PRINT_FORMAT "%lu/%lu""th %lfs\n", i, N, (double)(end_time - start_time) /CLOCKS_PER_SEC
#define PRINT_FORMAT                                                           \
        "%lu\t%lf\n", i, (double)(end_time - start_time) / CLOCKS_PER_SEC

// #define VLIST
// #define LIST
// #define FORWARD_LIST
// #define VECTOR
// #define DEQUE

int main()
{
        FILE *fp = NULL, *fp_w = NULL;

        clock_t start_time, end_time;
        char command[256];

        size_t N;
#ifdef VLIST
        struct vlist *vlist = vlist_alloc(NULL);

        fp = fopen("test.inp", "r");
        fp_w = fopen("test.out", "w");

        fscanf(fp, SIZE_FORMAT, &N);

        start_time = clock();
        for (size_t i = 0; i < N; i++) {
                fscanf(fp, "%s", command);
                if (i % PRINT_RATIO == 0) {
                        end_time = clock();
                        printf(PRINT_FORMAT);
                }
                if (!strcmp(command, "INSERT")) {
                        int student_id = 0;
                        struct sublist_node node;

                        fscanf(fp, "%d", &student_id);

                        node.is_primitive = true;
                        node.size = sizeof(int);
                        node.ivalue = student_id;

                        vlist_add_sublist_node(vlist, &node);
                } else if (!strcmp(command, "SEARCH")) {
                        int pos = 0;
                        struct sublist_node *node = NULL;
                        fscanf(fp, "%d", &pos);
                        node = vlist_get_sublist_node(vlist, pos);
                        if (node) {
                                fprintf(fp_w, "%d\n", node->ivalue);
                        }
                } else if (!strcmp(command, "DEL")) {
                        vlist_remove_sublist_node(&vlist, 0);
                }
        }

        end_time = clock();
        vlist_dealloc(vlist);
        fclose(fp);
        fclose(fp_w);
        printf("vlist %lfs\n",
               (double)(end_time - start_time) / CLOCKS_PER_SEC);
#endif

#ifdef __cplusplus
#if defined(LIST) || defined(FORWARD_LIST)
        fp = fopen("test.inp", "r");
        fp_w = fopen("list.out", "w");
        fscanf(fp, SIZE_FORMAT, &N);

        start_time = clock();
#ifdef LIST
        std::list<int> list;
#else
        std::forward_list<int> list;
#endif
        for (size_t i = 0; i < N; i++) {
                fscanf(fp, "%s", command);
                if (i % PRINT_RATIO == 0) {
                        end_time = clock();
                        printf(PRINT_FORMAT);
                }
                if (!strcmp(command, "INSERT")) {
                        int student_id = 0;

                        fscanf(fp, "%d", &student_id);

#ifdef LIST
                        list.push_back(student_id);
#else
                        list.push_front(student_id);
#endif
                } else if (!strcmp(command, "SEARCH")) {
#ifdef LIST
                        int i = list.size() - 1;
#else
                        int i = 0;
#endif
                        int pos = 0;
                        fscanf(fp, "%d", &pos);
                        for (const auto &value : list) {
                                if (i == pos) {
                                        fprintf(fp_w, "%d\n", value);
                                        break;
                                }
#ifdef LIST
                                i--;
#else
                                i++;
#endif
                        }
                } else if (!strcmp(command, "DEL")) {
#ifdef LIST
                        list.pop_back();
#else
                        list.pop_front();
#endif
                }
        }

        end_time = clock();
#ifdef LIST
        printf("list %lfs\n", (double)(end_time - start_time) / CLOCKS_PER_SEC);
#endif
#ifdef FORWARD_LIST
        printf("forward list %lfs\n",
               (double)(end_time - start_time) / CLOCKS_PER_SEC);
#endif
        fclose(fp);
        fclose(fp_w);
#endif

#ifdef VECTOR
        fp = fopen("test.inp", "r");
        fp_w = fopen("list.out", "w");
        fscanf(fp, SIZE_FORMAT, &N);

        start_time = clock();
        std::vector<int> list;
        for (size_t i = 0; i < N; i++) {
                fscanf(fp, "%s", command);
                if (i % PRINT_RATIO == 0) {
                        end_time = clock();
                        printf(PRINT_FORMAT);
                }
                if (!strcmp(command, "INSERT")) {
                        int student_id = 0;

                        fscanf(fp, "%d", &student_id);

                        list.push_back(student_id);
                } else if (!strcmp(command, "SEARCH")) {
                        int i = list.size() - 1;
                        int pos = 0;
                        fscanf(fp, "%d", &pos);
                        fprintf(fp_w, "%d\n", list.at(i - pos));
                } else if (!strcmp(command, "DEL")) {
                        list.pop_back();
                }
        }

        end_time = clock();
        printf("vector %lfs\n",
               (double)(end_time - start_time) / CLOCKS_PER_SEC);
        fclose(fp);
        fclose(fp_w);
#endif

#ifdef DEQUE
        fp = fopen("test.inp", "r");
        fp_w = fopen("list.out", "w");
        fscanf(fp, SIZE_FORMAT, &N);

        start_time = clock();
        std::deque<int> list;
        for (size_t i = 0; i < N; i++) {
                fscanf(fp, "%s", command);
                if (i % PRINT_RATIO == 0) {
                        end_time = clock();
                        printf(PRINT_FORMAT);
                }
                if (!strcmp(command, "INSERT")) {
                        int student_id = 0;

                        fscanf(fp, "%d", &student_id);

                        list.push_back(student_id);
                } else if (!strcmp(command, "SEARCH")) {
                        int i = list.size() - 1;
                        int pos = 0;
                        fscanf(fp, "%d", &pos);
                        fprintf(fp_w, "%d\n", list.at(i - pos));
                } else if (!strcmp(command, "DEL")) {
                        list.pop_back();
                }
        }

        end_time = clock();
        printf("deque %lfs\n",
               (double)(end_time - start_time) / CLOCKS_PER_SEC);
        fclose(fp);
        fclose(fp_w);
#endif
#endif
        return 0;
}
