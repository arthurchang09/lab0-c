#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
struct list_head *q_new()
{
    struct list_head *q = malloc(sizeof(struct list_head));
    if (!q) {
        return NULL;
    }
    q->prev = q;
    q->next = q;
    return q;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l) {
        return;
    }
    l->prev->next = NULL;
    while (l->next) {
        // cppcheck-suppress nullPointer
        element_t *rm = list_entry(l->next, element_t, list);
        l->next = l->next->next;
        q_release_element(rm);
    }
    free(l);
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head) {
        return false;
    }
    size_t s_len = strlen(s) + 1;
    element_t *new_ele_node = NULL;
    if (!(new_ele_node = malloc(sizeof(element_t)))) {
        return false;
    }
    if (!(new_ele_node->value = (char *) malloc(s_len))) {
        free(new_ele_node);
        return false;
    }
    memcpy(new_ele_node->value, s, s_len);
    struct list_head *node_list = &new_ele_node->list;
    new_ele_node->list.prev = head;
    new_ele_node->list.next = head->next;
    head->next->prev = node_list;
    head->next = node_list;
    return true;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head) {
        return false;
    }
    size_t s_len = strlen(s) + 1;
    element_t *new_ele_node = NULL;
    if (!(new_ele_node = malloc(sizeof(element_t)))) {
        return false;
    }
    if (!(new_ele_node->value = (char *) malloc(s_len))) {
        free(new_ele_node);
        return false;
    }
    memcpy(new_ele_node->value, s, s_len);
    struct list_head *node_list = &new_ele_node->list;
    new_ele_node->list.prev = head->prev;
    new_ele_node->list.next = head;
    head->prev->next = node_list;
    head->prev = node_list;

    return true;
}

/*
 * Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * REF:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || !head->next || head->next == head) {
        return NULL;
    }
    // cppcheck-suppress nullPointer
    element_t *rm = list_entry(head->next, element_t, list);
    if (sp) {
        strncpy(sp, rm->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    rm->list.next->prev = head;
    head->next = head->next->next;

    return rm;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || !head->next || head->next == head) {
        return NULL;
    }
    // cppcheck-suppress nullPointer
    element_t *rm = list_entry(head->prev, element_t, list);
    if (sp) {
        strncpy(sp, rm->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    rm->list.prev->next = head;
    head->prev = head->prev->prev;
    return rm;
}

/*
 * WARN: This is for external usage, don't modify it
 * Attempt to release element.
 */
void q_release_element(element_t *e)
{
    free(e->value);
    free(e);
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(struct list_head *head)
{
    int lengh = 0;
    if (!head) {
        return 0;
    }
    struct list_head *curr = head->next;
    while (curr != head) {
        lengh++;
        curr = curr->next;
    }
    return lengh;
}

/*
 * Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return true if successful.
 * Return false if list is NULL or empty.
 */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || q_size(head) == 0) {
        return false;
    }
    struct list_head **indir = &head;
    for (struct list_head *fast = head->next;
         fast != head && fast->next != head; fast = fast->next->next) {
        indir = &(*indir)->next;
    }
    struct list_head *del = *indir;
    del = del->next;
    del->prev->next = del->next;
    del->next->prev = del->prev;
    // cppcheck-suppress nullPointer
    element_t *del_n = list_entry(del, element_t, list);
    q_release_element(del_n);
    return true;
}

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head) {
        return false;
    }
    struct list_head *node, *next;
    bool next_same_val = false;
    for (node = (head)->next; node != (head);) {
        next = node->next;
        // cppcheck-suppress nullPointer
        element_t *del = list_entry(node, element_t, list);
        if (next != head &&
            strcmp(del->value,
                   // cppcheck-suppress nullPointer
                   list_entry(next, element_t, list)->value) == 0) {
            printf("delete %s ", del->value);
            next_same_val = true;
            node->prev->next = next;
            next->prev = node->prev;
            q_release_element(del);
        } else if (next_same_val) {
            printf("delete %s ", del->value);
            next_same_val = false;
            node->prev->next = next;
            next->prev = node->prev;
            q_release_element(del);
        }
        node = next;
    }
    return true;
}

/*
 * Attempt to swap every two adjacent nodes.
 */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    struct list_head *node;
    for (node = (head)->next; node != (head) && node->next != head;
         node = node->next) {
        struct list_head *tmp_head = node->next;
        list_del(node);
        list_add(node, tmp_head);
    }
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(struct list_head *head)
{
    if (!head || !head->next || head->next->next == head) {
        return;
    }
    struct list_head *curr = head->next;
    struct list_head *temp = NULL;
    while (curr != head) {
        temp = curr->next;
        curr->next = curr->prev;
        curr->prev = temp;
        curr = temp;
    }
    temp = head->next;
    head->next = head->prev;
    head->prev = temp;
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
struct list_head *merge(struct list_head *l1,
                        struct list_head *l2,
                        struct list_head *head)
{
    if (!l1 || l1 == head) {
        return l2;
    }
    if (!l2 || l2 == head) {
        return l1;
    }
    struct list_head *tmp_head, *curr;
    // cppcheck-suppress nullPointer
    element_t *l1_node = list_entry(l1, element_t, list);
    // cppcheck-suppress nullPointer
    element_t *l2_node = list_entry(l2, element_t, list);
    /*
    if (strcmp(l1_node->value,l2_node->value) < 0) {
        l1->next = merge(l1->next, l2, head);
        l1->next->prev = l1;
        l1->prev = head;
        return l1;
    }
    else {
        l2->next = merge(l2->next, l1, head);
        l2->next->prev = l2;
        l2->prev = head;
        return l2;
    }*/
    if (strcmp(l1_node->value, l2_node->value) < 0) {
        tmp_head = l1;
        l1->prev = head;
        l1 = l1->next;
    } else {
        tmp_head = l2;
        l2->prev = head;
        l2 = l2->next;
    }
    curr = tmp_head;
    while (l1 != head && l2 != head) {
        // cppcheck-suppress nullPointer
        l1_node = list_entry(l1, element_t, list);
        // cppcheck-suppress nullPointer
        l2_node = list_entry(l2, element_t, list);
        if (strcmp(l1_node->value, l2_node->value) < 0) {
            curr->next = l1;
            l1->prev = curr;
            l1 = l1->next;
            // return l1;
        } else {
            curr->next = l2;
            l2->prev = curr;
            l2 = l2->next;
            // return l2;
        }
        curr = curr->next;
    }
    if (l1 == head) {
        curr->next = l2;
        l2->prev = curr;
    } else {
        curr->next = l1;
        l1->prev = curr;
    }
    return tmp_head;
}

struct list_head *merge_sort_list(struct list_head *head,
                                  struct list_head *real_head)
{
    if (!head || !head->next || head->next == real_head || head == real_head) {
        return head;
    }

    struct list_head *fast = head;
    struct list_head *slow = head;
    while (fast != real_head && fast->next != real_head) {
        fast = fast->next->next;
        slow = slow->next;
    }

    fast = slow;
    slow = slow->prev;
    slow->next = real_head;
    fast->prev = real_head;

    struct list_head *l1 = merge_sort_list(head, real_head);
    struct list_head *l2 = merge_sort_list(fast, real_head);
    // struct list_head tmp, *ptr_tmp = &tmp;
    return merge(l1, l2, real_head);
}

void q_sort(struct list_head *head)
{
    if (!head || !head->next) {
        return;
    }
    head->next = merge_sort_list(head->next, head);
    struct list_head *tail = head;
    while (tail->next != head) {
        tail = tail->next;
    }
    head->prev = tail;
}
