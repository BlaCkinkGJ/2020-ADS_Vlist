#ifndef VLIST_H__
#define VLIST_H__

#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#ifdef __GNUC__
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif

#define NO_ERR 0
#define ALLOC_FAILED -1
#define DEALLOC_FAILED -2
#define NODE_GC_STATE_IS_FALSE -3
#define BUFFER_IS_EMPTY -4
#define FIND_FAILED -5

#define pr_info(msg, ...)                                                      \
        fprintf(stderr, "[{%lfs} %s(%s):%d] " msg,                             \
                ((double)clock() / CLOCKS_PER_SEC), __FILE__, __FUNCTION__,    \
                __LINE__, ##__VA_ARGS__)

#define get_err_msg(no)                                                        \
        ((const char *[]){ "No Error", "Allocation Failed",                    \
                           "Deallocation Failed", "Node gc state is false",    \
                           "Buffer is empty", "Cannot find elements" }[-(no)])

/**
 * @brief sublist의 데이터가 실질적으로 들어가는 부분
 * 
 */
struct sublist_node {
        size_t size; /**< buffer의 크기 */
        bool is_primitive; /**< primitive 형태인가? */
        bool gc_state; /**< gc 수행 여부 */
        union {
                int ivalue; /**< integer 값인가? */
                float fvalue; /**< float 값인가? */
                void *buffer; /**< buffer */
        };
};

/**
 * @brief vlist의 하위 리스트에 대한 정보를 가지는 구조체
 * 
 */
struct sublist {
        size_t size;
        size_t current_offset; /**< 현재 sublist의 offset */
        size_t next_offset; /**< 다음 sublist의 offset */

        int ref_count; /**< 현재 하위 리스트에 대한 참조 횟수 */

        struct sublist *next; /**< 다음 하위 리스트의 위치 */
        struct sublist_node nodes[0]; /**< 리스트가 가진 정보 */
};

/**
 * @brief 하나의 vlist가 가지는 정보
 * 
 */
struct vlist {
        struct sublist *head; /**< 하위 리스트의 시작점에 대한 주소를 가짐 */

        size_t *last_sublist_size; /**< 마지막 sublist_size */
        bool use_checkpoint; /**< checkpoint_offset 사용 설정이다. */
        union {
                size_t checkpoint_offset; /**< 다른 sublist에 참조 초기화를 할 때만 사용된다. */
                size_t *sublist_offset;
        };
};

struct vlist *vlist_alloc(struct sublist *list);
int vlist_dealloc(struct vlist *vlist);
size_t vlist_size(struct vlist *vlist);
struct sublist_node *vlist_get_sublist_node(struct vlist *vlist,
                                            const size_t index);
int vlist_add_sublist_node(struct vlist *vlist,
                           const struct sublist_node *node);

#endif