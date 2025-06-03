#ifndef __SEGO_ENUMS_H
#define __SEGO_ENUMS_H

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        SG_OK,
        SG_NOTHING,
        SG_TIMEOUT,
        SG_ERR_NULLPTR,
        SG_ERR_ALLOC,
        SG_ERR_PTHREAD
    } sgReturnType;

    typedef enum
    {
        SG_CTX_LOWERED,
        SG_CTX_RAISED,
        SG_CTX_ERROR
    } sgContextFlag;

    typedef enum
    {
        SG_TIME_NS = 1L,
        SG_TIME_US = 1000L,
        SG_TIME_MS = 1000000L,
        SG_TIME_S = 1000000000L
    } sgTimeUnit;

#ifdef __cplusplus
}
#endif

#endif