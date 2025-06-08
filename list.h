#ifndef __SEGO_LIST_H
#define __SEGO_LIST_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "enums.h"

    typedef struct __sgListItem
    {
        void *data;
        struct __sgListItem *next;
        struct __sgListItem *prev;
    } sgListItem;

    typedef struct
    {
        size_t len;
        size_t itemSize;
        sgListItem *head;
        sgListItem *tail;
    } sgList;

    /*
     * @brief   Creates new list.
     * @param   itemSize the size of each entry in the list
     * @return  The pointer to the list instance.
     */
    sgList *sgListCreate(size_t itemSize)
    {
        sgList *list = (sgList *)malloc(sizeof(sgList));
        if (list == NULL)
            return NULL;

        list->len = 0UL;
        list->itemSize = itemSize;
        list->head = NULL;
        list->tail = NULL;
        return list;
    }

    /*
     * @brief   Creates new list item.
     * @param   data the pointer to the data
     * @param   size the size of the data
     * @return  The pointer to the list item instance.
     */
    sgListItem *__sgListItemCreate(void *data, size_t size)
    {
        sgListItem *item = (sgListItem *)malloc(sizeof(sgListItem));
        if (item == NULL)
            return NULL;

        item->data = malloc(size);
        if (item->data == NULL)
        {
            free(item);
            return NULL;
        }

        item->prev = NULL;
        item->next = NULL;

        memcpy(item->data, data, size);
        return item;
    }

    /*
     * @brief   Pushes new item to the front of the list.
     * @param   list the pointer to the list instance
     * @param   data the data to be pushed
     * @return  `SG_ERR_NULLPTR` if list/data is `NULL`. `SG_ERR_ALLOC` if memory allocation failed. `SG_OK` if added successfully.
     */
    sgReturnType sgListPushFront(sgList *list, void *data)
    {
        if (list == NULL || data == NULL)
            return SG_ERR_NULLPTR;

        sgListItem *item = __sgListItemCreate(data, list->itemSize);
        if (item == NULL)
            return SG_ERR_ALLOC;

        if (list->len == 0UL)
        {
            list->head = item;
            list->tail = item;
        }
        else
        {
            item->next = list->head;
            list->head->prev = item;
            list->head = item;
        }

        list->len += 1UL;
        return SG_OK;
    }

    /*
     * @brief   Pushes new item to the back of the list.
     * @param   list the pointer to the list instance
     * @param   data the data to be pushed
     * @return  `SG_ERR_NULLPTR` if list/data is `NULL`. `SG_ERR_ALLOC` if memory allocation failed. `SG_OK` if added successfully.
     */
    sgReturnType sgListPushBack(sgList *list, void *data)
    {
        if (list == NULL || data == NULL)
            return SG_ERR_NULLPTR;

        sgListItem *item = __sgListItemCreate(data, list->itemSize);
        if (item == NULL)
            return SG_ERR_ALLOC;

        if (list->len == 0UL)
        {
            list->head = item;
            list->tail = item;
        }
        else
        {
            item->prev = list->tail;
            list->tail->next = item;
            list->tail = item;
        }

        list->len += 1UL;
        return SG_OK;
    }

    /*
     * @brief   Inserts new item at an index.
     * @param   list the pointer to the list instance
     * @param   idx the insert index
     * @param   data the data to be inserted
     * @return  `SG_ERR_NULLPTR` if list/data is `NULL`. `SG_ERR_INVALID` if the index is invalid. `SG_ERR_ALLOC` if memory allocation failed. `SG_OK` if added successfully.
     */
    sgReturnType sgListInsert(sgList *list, size_t idx, void *data)
    {
        if (list == NULL || data == NULL)
            return SG_ERR_NULLPTR;

        if (idx > list->len)
            return SG_ERR_INVALID;
        else if (idx == 0UL)
            return sgListPushFront(list, data);
        else if (idx == list->len)
            return sgListPushBack(list, data);

        sgListItem *item = __sgListItemCreate(data, list->itemSize);
        if (item == NULL)
            return SG_ERR_ALLOC;

        sgListItem *tmp;
        if (idx <= (list->len / 2UL))
        {
            tmp = list->head;
            for (size_t i = 0; i < idx; ++i)
                tmp = tmp->next;
        }
        else
        {
            tmp = list->tail;
            for (size_t i = list->len - 1; i > idx; --i)
                tmp = tmp->prev;
        }

        item->prev = tmp->prev;
        item->next = tmp;
        tmp->prev->next = item;
        tmp->prev = item;

        list->len += 1;
        return SG_OK;
    }

    /*
     * @brief   Pops the item at front.
     * @param   list the pointer to the list instance
     * @param   buf the buffer to holds the item, can be set to `NULL`
     * @return  `SG_ERR_NULLPTR` if list is `NULL`. `SG_NOTHING` if nothing can be done. `SG_OK` if ok.
     */
    sgReturnType sgListPopFront(sgList *list, void *buf)
    {
        if (list == NULL)
            return SG_ERR_NULLPTR;

        if (list->len == 0UL)
            return SG_NOTHING;

        if (buf != NULL)
            memcpy(buf, list->head->data, list->itemSize);

        if (list->len == 1UL)
        {
            free(list->head->data);
            free(list->head);
            list->head = NULL;
            list->tail = NULL;
        }
        else
        {
            sgListItem *prevHead = list->head;
            list->head = list->head->next;
            list->head->prev = NULL;

            free(prevHead->data);
            free(prevHead);
        }

        list->len -= 1UL;
        return SG_OK;
    }

    /*
     * @brief   Pops the item at back.
     * @param   list the pointer to the list instance
     * @param   buf the buffer to holds the item, can be set to `NULL`
     * @return  `SG_ERR_NULLPTR` if list is `NULL`. `SG_NOTHING` if nothing can be done. `SG_OK` if ok.
     */
    sgReturnType sgListPopBack(sgList *list, void *buf)
    {
        if (list == NULL)
            return SG_ERR_NULLPTR;

        if (list->len == 0UL)
            return SG_NOTHING;

        if (buf != NULL)
            memcpy(buf, list->tail->data, list->itemSize);

        if (list->len == 1UL)
        {
            free(list->tail->data);
            free(list->tail);
            list->head = NULL;
            list->tail = NULL;
        }
        else
        {
            sgListItem *prevTail = list->tail;
            list->tail = list->tail->prev;
            list->tail->next = NULL;

            free(prevTail->data);
            free(prevTail);
        }

        list->len -= 1UL;
        return SG_OK;
    }

    /*
     * @brief   Removes item at an index.
     * @param   list the pointer to the list instance
     * @param   idx the index to the item
     * @param   buf the buffer to holds the item, can be set to `NULL`
     * @return  `SG_ERR_NULLPTR` if list is `NULL`. `SG_ERR_INVALID` if the index is invalid. `SG_NOTHING` if nothing can be done. `SG_OK` if ok.
     */
    sgReturnType sgListRemove(sgList *list, size_t idx, void *buf)
    {
        if (list == NULL)
            return SG_ERR_NULLPTR;

        if (idx >= list->len)
            return SG_ERR_INVALID;
        else if (idx == 0)
            return sgListPopFront(list, buf);
        else if (idx == list->len - 1)
            return sgListPopBack(list, buf);

        sgListItem *item;
        if (idx <= (list->len / 2UL))
        {
            item = list->head;
            for (size_t i = 0; i < idx; ++i)
                item = item->next;
        }
        else
        {
            item = list->tail;
            for (size_t i = list->len - 1; i > idx; --i)
                item = item->prev;
        }

        if (buf != NULL)
            memcpy(buf, item->data, list->itemSize);

        item->prev->next = item->next;
        item->next->prev = item->prev;
        free(item->data);
        free(item);

        list->len -= 1;
        return SG_OK;
    }

    /*
     * @brief   Retrieves an item at the front.
     * @param   list the pointer to the list instance
     * @param   buf the buffer to holds the item
     * @return  `SG_ERR_NULLPTR` if list/buf is `NULL`. `SG_NOTHING` if nothing can be done. `SG_OK` if ok.
     */
    sgReturnType sgListGetFront(sgList *list, void *buf)
    {
        if (list == NULL || buf == NULL)
            return SG_ERR_NULLPTR;

        if (list->len == 0UL)
            return SG_NOTHING;

        memcpy(buf, list->head->data, list->itemSize);
        return SG_OK;
    }

    /*
     * @brief   Retrieves an item at the back.
     * @param   list the pointer to the list instance
     * @param   buf the buffer to holds the item
     * @return  `SG_ERR_NULLPTR` if list/buf is `NULL`. `SG_NOTHING` if nothing can be done. `SG_OK` if ok.
     */
    sgReturnType sgListGetBack(sgList *list, void *buf)
    {
        if (list == NULL || buf == NULL)
            return SG_ERR_NULLPTR;

        if (list->len == 0UL)
            return SG_NOTHING;

        memcpy(buf, list->tail->data, list->itemSize);
        return SG_OK;
    }

    /*
     * @brief   Retrieves an item at an index.
     * @param   list the pointer to the list instance
     * @oaram   idx the index of the item
     * @param   buf the buffer to holds the item
     * @return  `SG_ERR_NULLPTR` if list/buf is `NULL`. `SG_ERR_INVALID` if the index is invalid. `SG_NOTHING` if nothing can be done. `SG_OK` if ok.
     */
    sgReturnType sgListGet(sgList *list, size_t idx, void *buf)
    {
        if (list == NULL || buf == NULL)
            return SG_ERR_NULLPTR;

        if (idx >= list->len)
            return SG_ERR_INVALID;

        sgListItem *item;
        if (idx <= (list->len / 2UL))
        {
            item = list->head;
            for (size_t i = 0; i < idx; ++i)
                item = item->next;
        }
        else
        {
            item = list->tail;
            for (size_t i = list->len - 1; i > idx; --i)
                item = item->prev;
        }

        memcpy(buf, item->data, list->itemSize);
        return SG_OK;
    }

    /*
     * @brief   Destroys the list instance.
     * @param   list the pointer to the list instance
     * @return  None.
     */
    void sgListDestroy(sgList *list)
    {
        if (list == NULL)
            return;

        sgListItem *item = list->head;
        while (item != NULL)
        {
            sgListItem *tmp = item;
            item = item->next;
            free(tmp->data);
            free(tmp);
        }

        list->len = 0UL;
        list->itemSize = 0UL;
        free(list);
    }

#ifdef __cplusplus
}
#endif

#endif