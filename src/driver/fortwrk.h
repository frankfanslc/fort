#ifndef FORTWRK_H
#define FORTWRK_H

#include "fortdrv.h"

#define FORT_WORKER_REAUTH 0x01
#define FORT_WORKER_PSTREE 0x02

typedef void (NTAPI *FORT_WORKER_FUNC)(void);

typedef struct fort_worker
{
    UCHAR volatile id_bits;

    FORT_WORKER_FUNC reauth_func;
    FORT_WORKER_FUNC pstree_func;

    PIO_WORKITEM item;
} FORT_WORKER, *PFORT_WORKER;

#if defined(__cplusplus)
extern "C" {
#endif

FORT_API void fort_worker_queue(PFORT_WORKER worker, UCHAR work_id, FORT_WORKER_FUNC worker_func);

FORT_API NTSTATUS fort_worker_register(PDEVICE_OBJECT device, PFORT_WORKER worker);

FORT_API void fort_worker_unregister(PFORT_WORKER worker);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // FORTWRK_H
