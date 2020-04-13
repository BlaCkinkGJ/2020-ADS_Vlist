#include "vlist.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief sublist를 할당하도록 한다.
 * 
 * @param nr_nodes sublist가 가지는 element 갯수를 의미한다.
 * 
 * @return struct sublist* 새롭게 할당한 sublist를 반환하도록 한다.
 * 
 * @exception NULL 메모리 할당을 실패했을 경우 반환된다.
 */
static struct sublist *sublist_alloc(const size_t nr_nodes)
{
        const size_t node_size = sizeof(struct sublist_node) * nr_nodes;
        struct sublist *new_sublist = NULL;

        new_sublist =
                (struct sublist *)malloc(sizeof(struct sublist) + node_size);
        if (!new_sublist) {
                pr_info("%s", get_err_msg(ALLOC_FAILED));
                return NULL;
        }
        new_sublist->size = nr_nodes;
        new_sublist->current_offset = nr_nodes;
        new_sublist->next_offset = 0; /**< 끝을 나타낸다. */
        new_sublist->ref_count = 0;
        new_sublist->next = NULL;

        return new_sublist;
}

/**
 * @brief sublist node에 대한 할당을 해제하도록 한다.
 * 
 * @param node 할당 해제의 대상이 되는 노드 
 * 
 * @note node->gc_state가 false이고, buffer가 빈 경우에도 동작은 한다.
 */
static void sublist_node_dealloc(struct sublist_node *node)
{
#ifdef DEBUG
        if (node->gc_state == false) {
                pr_info("[WARN] %s", get_err_msg(NODE_GC_STATE_IS_FALSE));
        }
#endif

        node->size = 0;
        node->gc_state = false;
        if (node->is_primitive && !node->buffer) {
#ifdef DEBUG
                pr_info("[WARN] %s", get_err_msg(BUFFER_IS_EMPTY));
#endif
                return;
        }
        free(node->buffer);
}

/**
 * @brief sublist의 각 node의 할당을 해제하도록 한다.
 *
 * @return int deallocation 성공 여부 정보를 반환한다.
 * @param list sublist 정보를 가진다
 */
static int sublist_dealloc(struct sublist *list)
{
        const size_t size = list->size;
        size_t node_index = 0;

        if (list->ref_count > 0) {
                return DEALLOC_FAILED;
        }

        for (node_index = 0; node_index < size; node_index++) {
                struct sublist_node *node = &list->nodes[node_index];
                sublist_node_dealloc(node);
        }

        return NO_ERR;
}

/**
 * @brief vlist를 할당을 해주도록 한다.
 * 
 * @param list sublist를 참조하는 경우 NULL이 아닌 sublist 값을 주면 된다.
 * @return struct vlist* 새롭게 할당한 vlist를 반환하도록 한다.
 * 
 * @exception NULL 메모리 할당을 실패한 경우 반환된다.
 */
struct vlist *vlist_alloc(struct sublist *list)
{
        const size_t initial_sublist_size = 1;
        const size_t vlist_size = sizeof(struct vlist);
        struct vlist *new_vlist = (struct vlist *)malloc(vlist_size);

        new_vlist->use_checkpoint = false;
        new_vlist->head = list;

        if (list == NULL) {
                struct sublist *head_sublist = NULL;

                head_sublist = sublist_alloc(initial_sublist_size);
                if (!head_sublist) {
                        pr_info("[ERROR] %s", get_err_msg(ALLOC_FAILED));
                        return NULL;
                }
                new_vlist->head = head_sublist;
                new_vlist->sublist_offset = &(new_vlist->head->current_offset);
        } else {
                new_vlist->use_checkpoint = true;
                new_vlist->checkpoint_offset = list->current_offset;
        }

        new_vlist->head->ref_count++;
        new_vlist->last_sublist_size = &(new_vlist->head->size);

        return new_vlist;
}

/**
 * @brief 할당된 vlist 및 sublist의 메모리를 해제한다.
 * 
 * @param vlist 할당을 해제할 vlist를 가리킨다.
 * @return int deallocation fail 여부를 반환한다.
 * 
 * @warning 강제로 ref_count를 0으로 해서 연관된 모든 vlist의 할당을 해제하므로 사용을 자제해야 한다.
 */
int vlist_dealloc(struct vlist *vlist)
{
        struct sublist *next_sublist = NULL;
        int ret = NO_ERR;

        while (vlist->head) {
                next_sublist = vlist->head->next;
                vlist->head->ref_count = 0;
                ret = sublist_dealloc(vlist->head);
                if (ret != NO_ERR) {
                        pr_info("[ERROR] %s", get_err_msg(ret));
                        return ret;
                }
                vlist->head = next_sublist;
        }

        free(vlist);

        return ret;
}

/**
 * @brief vlist의 크기를 구하도록 한다.
 * 
 * @param vlist vlist의 정보를 가지는 포인터
 * @return size_t vlist의 크기를 반환한다.
 */
size_t vlist_size(struct vlist *vlist)
{
        struct sublist *list_ptr = vlist->head;
        size_t size_of_vlist =
                *(vlist->last_sublist_size) - vlist->checkpoint_offset;

        if (likely(!vlist->use_checkpoint)) {
                size_of_vlist =
                        *(vlist->last_sublist_size) - *(vlist->sublist_offset);
        }

#ifdef DEBUG
        pr_info("[INFO] %I64d %I64d\n", *(vlist->last_sublist_size),
                *(vlist->sublist_offset));
#endif

        while (list_ptr->next != NULL) {
#ifdef DEBUG
                pr_info("[INFO] %I64d %I64d\n", list_ptr->next->size,
                        list_ptr->next_offset);
#endif
                size_of_vlist += list_ptr->next->size - list_ptr->next_offset;
                list_ptr = list_ptr->next;
        }

        return size_of_vlist;
}

/**
 * @brief vlist를 통해서 sublist node를 획득한다.
 * 
 * @param vlist 현재 찾고자하는 vlist의 위치를 가진다.
 * @param index 0 부터 시작하는 vlist의 index를 지칭한다.
 * @return struct sublist_node* sublist를 못 찾은 경우에는 NULL을 반환한다.
 */
struct sublist_node *vlist_get_sublist_node(struct vlist *vlist, size_t index)
{
        const size_t total_vlist_size = vlist_size(vlist);

        struct sublist *list_ptr = vlist->head, *next_list_ptr;
        struct sublist_node *node = NULL;
        size_t sublist_size = (list_ptr->size - vlist->checkpoint_offset);
        size_t current_offset = vlist->checkpoint_offset;

        if (likely(!vlist->use_checkpoint)) {
                sublist_size = (list_ptr->size - list_ptr->current_offset);
                current_offset = list_ptr->current_offset;
        }

        if (index > total_vlist_size) {
                pr_info("[ERROR] %s", get_err_msg(FIND_FAILED));
                goto ret;
        }

        if (index < sublist_size) {
                node = &list_ptr->nodes[current_offset + index];
                goto ret;
        }

        index = index - sublist_size;

        while (list_ptr->next != NULL) {
                next_list_ptr = list_ptr->next;
                sublist_size = next_list_ptr->size - list_ptr->next_offset;
                if (index < sublist_size) {
                        node = &next_list_ptr
                                        ->nodes[list_ptr->next_offset + index];
                        goto ret;
                }

                index = index - sublist_size;
                list_ptr = next_list_ptr;
        }

ret:
        return node;
}

/**
 * @brief node 값을 vlist의 sublist에 추가한다.
 * 
 * @param vlist 값을 넣고자하는 vlist이다.
 * @param node 값을 넣을 node이다.
 * @return int 성공한 경우 NO_ERR이 반환된다.
 */
int vlist_add_sublist_node(struct vlist *vlist, const struct sublist_node *node)
{
        struct sublist *list_ptr = vlist->head;
        struct sublist_node *target_node = NULL;

        if (unlikely(vlist->use_checkpoint) || *(vlist->sublist_offset) == 0) {
                list_ptr = sublist_alloc(*(vlist->last_sublist_size) << 1);
                if (list_ptr == NULL) {
                        return ALLOC_FAILED;
                }
                list_ptr->next_offset = vlist->checkpoint_offset;
                if (likely(!vlist->use_checkpoint))
                        list_ptr->next_offset = *(vlist->sublist_offset);
                list_ptr->next = vlist->head;
                list_ptr->ref_count++;

                vlist->last_sublist_size = &(list_ptr->size);
                vlist->sublist_offset = &(list_ptr->current_offset);
                vlist->head = list_ptr;
                vlist->use_checkpoint = false;
        }

        list_ptr->current_offset = list_ptr->current_offset - 1;
        target_node = &(vlist->head->nodes[*(vlist->sublist_offset)]);

        memcpy(target_node, node, sizeof(struct sublist_node));
        target_node->gc_state = false;

        return NO_ERR;
}