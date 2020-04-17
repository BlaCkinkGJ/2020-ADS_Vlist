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
        new_sublist->nr_invalid = 0;
        new_sublist->next = NULL;
        new_sublist->prev = NULL;
        new_sublist->ref_count = 0;

        for (size_t i = 0; i < nr_nodes; i++) {
                new_sublist->nodes[i].is_primitive = false;
                new_sublist->nodes[i].is_invalid = false;
                new_sublist->nodes[i].buffer = NULL;
        }

        return new_sublist;
}

/**
 * @brief sublist node에 대한 할당을 해제하도록 한다.
 * 
 * @param node 할당 해제의 대상이 되는 노드 
 * 
 * @note node가 가진 정보가 primitive 형식인 경우에는 free를 수행하지 않는다.
 */
static void sublist_node_dealloc(struct sublist_node *node)
{
        node->size = 0;
        node->is_invalid = false;
        if (node->is_primitive || !node->buffer) {
#ifdef DEBUG
                pr_info("[WARN] %s\n", get_err_msg(BUFFER_IS_EMPTY));
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
 * @param force 강제로 sublist를 dealloc한다.
 */
static int sublist_dealloc(struct sublist *list, const bool force)
{
        const size_t size = list->size;
        size_t node_index = 0;
        int ret = REF_OVERFLOW;

        list->ref_count--;

        if (list->ref_count == 0) {
                if (!force && list->nr_invalid != list->size) {
                        pr_info("%s\n", get_err_msg(DEALLOC_FAILED));
                        return DEALLOC_FAILED;
                }

                for (node_index = 0; node_index < size; node_index++) {
                        struct sublist_node *node = &list->nodes[node_index];
                        if (!force && !node->is_invalid) {
                                pr_info("%s", get_err_msg(DEALLOC_FAILED));
                                return DEALLOC_FAILED;
                        }
                        sublist_node_dealloc(node);
                }

                free(list);
                ret = NO_ERR;
        }

        return ret;
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
        struct vlist *new_vlist = NULL;

        new_vlist = (struct vlist *)malloc(sizeof(struct vlist));
        if (!new_vlist) {
                pr_info("%s\n", get_err_msg(ALLOC_FAILED));
                return NULL;
        }

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

        new_vlist->head->nr_invalid = 0;
        new_vlist->head->ref_count++;
        new_vlist->sublist_nr_invalid = new_vlist->head->nr_invalid;
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
                ret = sublist_dealloc(vlist->head, true);
                if (ret ==
                    REF_OVERFLOW) { /**> sublist의 ref_count만 감소한 경우 해당 부분이 단말이므로 해제를 종료하도록 한다.*/
                        break;
                }
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
        struct sublist *list_ptr = NULL;
        size_t size_of_vlist;

        if (unlikely(vlist == NULL)) {
                return 0;
        }

        list_ptr = vlist->head;
        size_of_vlist = *(vlist->last_sublist_size) - vlist->checkpoint_offset;

        if (likely(!vlist->use_checkpoint)) {
                size_of_vlist =
                        *(vlist->last_sublist_size) - *(vlist->sublist_offset);
        }

        size_of_vlist -= vlist->sublist_nr_invalid;

        while (list_ptr->next != NULL && list_ptr->ref_count > 0) {
#ifdef DEBUG
                pr_info("[INFO] " SIZE_FORMAT " " SIZE_FORMAT "\n",
                        list_ptr->next->size, list_ptr->next_offset);
#endif
                size_of_vlist += list_ptr->next->size - list_ptr->next_offset;
                size_of_vlist -= list_ptr->next->nr_invalid;
                list_ptr = list_ptr->next;
        }

        return size_of_vlist;
}

/**
 * @brief sublist에서 offset에 기반하여 find_pos에 위치한 node를 찾도록 한다.
 *
 * @param sublist 찾고자 하는 sublist를 가리킨다.
 * @param offset sublist가 현재 쓰고 있는 위치를 가리킨다.
 * @param find_pos 사용자가 찾고자 하는 장소를 가리킨다.
 *
 * @return struct sublit_node* sublist에서 찾은 node 위치를 반환한다. 만약 찾지 못한 경우에는 NULL이 반환된다.
 */
static struct sublist_node *sublist_get_node(struct sublist *sublist,
                                             size_t offset, size_t find_pos)
{
        struct sublist_node *node = NULL;
        if (sublist->nr_invalid == 0) {
                node = &sublist->nodes[offset + find_pos];
        } else {
#ifdef RANDOMIZE_ERASE
                /**< 한 개라도 invalid 플래그가 설정된 경우 sublist 전체를 읽도록 한다. */
                size_t pos, target_pos = 0;
                for (pos = offset; pos < sublist->size; pos++) {
                        node = &sublist->nodes[pos];
                        if (node->is_invalid == false) {
                                if (target_pos == find_pos) {
                                        break;
                                }
                                target_pos++;
                        }
                } // end of for
#else
                node = &sublist->nodes[offset + find_pos + sublist->nr_invalid];
#endif
        }
        return node;
}

/**
 * @brief vlist를 통해서 sublist node를 획득한다.
 * 
 * @param vlist 현재 찾고자하는 vlist의 위치를 가진다.
 * @param index 0 부터 시작하는 vlist의 index를 지칭한다.
 * @return struct sublist_node* sublist를 못 찾은 경우에는 NULL을 반환한다.
 *
 * @exception NULL FIND_FAIL이 발생한 경우에 NULL 값이 반환된다.
 */
struct sublist_node *vlist_get_sublist_node(struct vlist *vlist,
                                            size_t find_pos)
{
        const size_t total_vlist_size = vlist_size(vlist);

        struct sublist *list_ptr = vlist->head, *next_list_ptr;
        struct sublist_node *node = NULL;
        size_t sublist_size = (list_ptr->size - vlist->checkpoint_offset -
                               vlist->sublist_nr_invalid);
        size_t current_offset = vlist->checkpoint_offset;

        if (likely(!vlist->use_checkpoint)) {
                sublist_size = (list_ptr->size - list_ptr->current_offset -
                                list_ptr->nr_invalid);
                current_offset = list_ptr->current_offset;
        }

        if (find_pos > total_vlist_size) {
                pr_info("[ERROR] %s", get_err_msg(FIND_FAILED));
                goto ret;
        }

        if (find_pos < sublist_size) {
                node = sublist_get_node(list_ptr, current_offset, find_pos);
                goto ret;
        }

        find_pos = find_pos - sublist_size;

        while (list_ptr->next != NULL) {
                next_list_ptr = list_ptr->next;
                sublist_size = (next_list_ptr->size - list_ptr->next_offset -
                                next_list_ptr->nr_invalid);
                if (find_pos < sublist_size) {
                        node = sublist_get_node(
                                next_list_ptr, list_ptr->next_offset, find_pos);
                        goto ret;
                }

                find_pos = find_pos - sublist_size;
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
int vlist_add_sublist_node(struct vlist *vlist, struct sublist_node *node)
{
        struct sublist *list_ptr = vlist->head;
        struct sublist_node *target_node = NULL;

        if (unlikely(vlist->use_checkpoint) || *(vlist->sublist_offset) == 0) {
                list_ptr = sublist_alloc(*(vlist->last_sublist_size) << 1);
                if (list_ptr == NULL) {
                        return ALLOC_FAILED;
                }
                list_ptr->next_offset = vlist->checkpoint_offset;
                if (likely(!vlist->use_checkpoint)) {
                        list_ptr->next_offset = *(vlist->sublist_offset);
                }
                list_ptr->next = vlist->head;
                vlist->head->prev = list_ptr;
                list_ptr->nr_invalid = 0;
                list_ptr->ref_count++;

                vlist->last_sublist_size = &(list_ptr->size);
                vlist->sublist_offset = &(list_ptr->current_offset);
                vlist->sublist_nr_invalid = list_ptr->nr_invalid;
                vlist->head = list_ptr;
                vlist->use_checkpoint = false;
        }

        list_ptr->current_offset = list_ptr->current_offset - 1;

        node->parent = vlist->head;
        target_node = &(vlist->head->nodes[*(vlist->sublist_offset)]);

        memcpy(target_node, node, sizeof(struct sublist_node));
        target_node->is_invalid = false;

        return NO_ERR;
}

/**
 * @brief sublist의node에 GC 플래그를 설정하고, GC 대상 sublist를 제거한다.
 *
 * @param vlist vlist를 가리키는 포인터 callback 된다.
 * @param remove_pos 삭제하고자하는 위치이다.
 *
 * @note remove_pos가 0인 경우 아닌 경우보다 훨씬 빠르게 동작한다.
 */
int vlist_remove_sublist_node(struct vlist **_vlist, const size_t remove_pos)
{
        struct vlist *vlist = *_vlist;
        struct sublist_node *node = NULL;
        node = vlist_get_sublist_node(vlist, remove_pos);
        if (!node || node->is_invalid) {
                pr_info("%s\n", get_err_msg(REMOVE_FAILED));
                return REMOVE_FAILED;
        }
        node->is_invalid = true;
        node->parent->nr_invalid++;
        if (node->parent == vlist->head) {
                vlist->sublist_nr_invalid++;
        }

        if (vlist_size(vlist) == 0) {
#ifdef DEBUG
                pr_info("%s", "[INFO] vlist deallocation\n");
#endif
                vlist_dealloc(vlist);
                *_vlist = vlist_alloc(NULL);
                if (!vlist) {
                        pr_info("%s\n", get_err_msg(ALLOC_FAILED));
                        return ALLOC_FAILED;
                }
                return NO_ERR;
        }

        return NO_ERR;
}
