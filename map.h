#ifndef __SEGO_MAP_H
#define __SEGO_MAP_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <string.h>
#include "uthash.h"
#include "enums.h"

    typedef struct
    {
        char *key;
        void *val;
        size_t keySize;
        size_t valSize;
        UT_hash_handle hh;
    } sgMapEntry;

    typedef struct
    {
        sgMapEntry *hash;
    } sgMapStr;

    /*
     * @brief   Creates new string-keyed map instance.
     * @param   none
     * @return  The pointer to the string-keyed map instance.
     */
    sgMapStr *sgMapStrCreate()
    {
        sgMapStr *map = (sgMapStr *)malloc(sizeof(sgMapStr));
        map->hash = NULL;
        return map;
    }

    /*
     * @brief   Adds new entry to the map.
     * @param   map the string map pointer
     * @param   key the key
     * @param   val the pointer to the value
     * @param   valSize the size of the value
     * @return  `SG_ERR_NULLPTR` if an argument is a `NULL`. `SG_ERR_ALLOC` when allocation is failed. `SG_OK` when successfully added.
     */
    sgReturnType sgMapStrAdd(sgMapStr *map, const char *key, void *val, size_t valSize)
    {
        if (map == NULL || key == NULL || val == NULL)
            return SG_ERR_NULLPTR;

        sgMapEntry *e = (sgMapEntry *)malloc(sizeof(sgMapEntry));
        if (!e)
            return SG_ERR_ALLOC;

        size_t len = strlen(key);
        e->key = malloc(len);
        e->val = malloc(valSize);
        e->keySize = len;
        e->valSize = valSize;

        strcpy(e->key, key);
        memcpy(e->val, val, valSize);
        HASH_ADD_KEYPTR(hh, map->hash, e->key, e->keySize, e);

        return SG_OK;
    }

    /*
     * @brief   Seeks or checks for existence of a key.
     * @param   map the string map pointer
     * @param   key the desired key
     * @return  The size of the value stored within the key. Returns `0` if the key does not exist.
     */
    size_t sgMapStrSeek(sgMapStr *map, const char *key)
    {
        if (map == NULL || key == NULL)
            return 0UL;

        sgMapEntry *e = NULL;
        HASH_FIND(hh, map->hash, key, strlen(key), e);

        if (e == NULL)
            return 0UL;
        else
            return e->valSize;
    }

    /*
     * @brief   Finds the value corresponded with the key.
     * @param   map the string map pointer
     * @param   key the desired key
     * @param   buf the buffer to hold the value
     * @return  `SG_ERR_NULLPTR` if an argument is a `NULL`. `SG_NOTHING` if the finding failed. `SG_OK` if the value copied to the `buf`.
     */
    sgReturnType sgMapStrFind(sgMapStr *map, const char *key, void *buf)
    {
        if (map == NULL || key == NULL || buf == NULL)
            return SG_ERR_NULLPTR;

        sgMapEntry *e = NULL;
        HASH_FIND(hh, map->hash, key, strlen(key), e);

        if (e == NULL)
            SG_NOTHING;

        memcpy(buf, e->val, e->valSize);
        return SG_OK;
    }

    /*
     * @brief   Deletes an entry from the map.
     * @param   map the string map pointer
     * @param   key the desired key
     * @return  `SG_ERR_NULLPTR` if an argument is a `NULL`. `SG_NOTHING` if key does not exist. `SG_OK` if the entry successfully deleted.
     */
    sgReturnType sgMapStrDelete(sgMapStr *map, const char *key)
    {
        if (map == NULL || key == NULL)
            return SG_ERR_NULLPTR;

        sgMapEntry *e = NULL;
        HASH_FIND(hh, map->hash, key, strlen(key), e);

        if (e == NULL)
            return SG_NOTHING;

        HASH_DEL(map->hash, e);
        free(e->key);
        free(e->val);
        free(e);

        return SG_OK;
    }

    /*
     * @brief   Destroys the string map.
     * @param   map the pointer to the map.
     * @return  None.
     */
    void sgMapStrDestroy(sgMapStr *map)
    {
        if (map == NULL)
            return;

        sgMapEntry *cur, *tmp;
        HASH_ITER(hh, map->hash, cur, tmp)
        {
            HASH_DEL(map->hash, cur);
            free(cur->key);
            free(cur->val);
            free(cur);
        }
    }

#ifdef __cplusplus
}
#endif

#endif