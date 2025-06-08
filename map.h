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
        UT_hash_handle hh;
    } sgMapEntry;

    typedef struct
    {
        size_t valSize;
        sgMapEntry *hash;
    } sgMapStr;

    /*
     * @brief   Creates new string-keyed map instance.
     * @param   valSize the size of the value to be stored
     * @return  The pointer to the string-keyed map instance.
     */
    sgMapStr *sgMapStrCreate(size_t valSize)
    {
        sgMapStr *map = (sgMapStr *)malloc(sizeof(sgMapStr));
        map->valSize = valSize;
        map->hash = NULL;
        return map;
    }

    /*
     * @brief   Adds new entry to the map.
     * @param   map the string map pointer
     * @param   key the key
     * @param   val the pointer to the value
     * @return  `SG_ERR_NULLPTR` if an argument is a `NULL`. `SG_ERR_ALLOC` when allocation is failed. `SG_OK` when successfully added.
     */
    sgReturnType sgMapStrAdd(sgMapStr *map, const char *key, void *val)
    {
        if (map == NULL || key == NULL || val == NULL)
            return SG_ERR_NULLPTR;

        sgMapEntry *e = (sgMapEntry *)malloc(sizeof(sgMapEntry));
        if (!e)
            return SG_ERR_ALLOC;

        size_t len = strlen(key);
        e->key = malloc(len);
        e->val = malloc(map->valSize);

        strcpy(e->key, key);
        memcpy(e->val, val, map->valSize);
        HASH_ADD_KEYPTR(hh, map->hash, e->key, len, e);

        return SG_OK;
    }

    /*
     * @brief   Finds the value corresponded with the key.
     * @param   map the string map pointer
     * @param   key the desired key
     * @param   buf the buffer to hold the value, can be set to `NULL`
     * @return  `SG_ERR_NULLPTR` if an map/key is a `NULL`. `SG_NOTHING` if does not exist. `SG_OK` if ok.
     */
    sgReturnType sgMapStrFind(sgMapStr *map, const char *key, void *buf)
    {
        if (map == NULL || key == NULL || buf == NULL)
            return SG_ERR_NULLPTR;

        sgMapEntry *e = NULL;
        HASH_FIND(hh, map->hash, key, strlen(key), e);

        if (e == NULL)
            SG_NOTHING;

        if (buf != NULL)
            memcpy(buf, e->val, map->valSize);

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