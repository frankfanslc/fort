/* Fort Firewall Configuration */

#include "fortcnf.h"

#define FORT_ZONES_POOL_TAG 'ZwfF'

/* Synchronize with tommy_hashdyn_node! */
typedef struct fort_conf_exe_node
{
    struct fort_conf_exe_node *next;
    struct fort_conf_exe_node *prev;

    PFORT_APP_ENTRY app_entry; /* tommy_hashdyn_node::data */

    tommy_key_t path_hash; /* tommy_hashdyn_node::index */
} FORT_CONF_EXE_NODE, *PFORT_CONF_EXE_NODE;

static FORT_TIME fort_current_time(void)
{
    TIME_FIELDS tf;
    LARGE_INTEGER system_time, local_time;

    KeQuerySystemTime(&system_time);
    ExSystemTimeToLocalTime(&system_time, &local_time);
    RtlTimeToTimeFields(&local_time, &tf);

    FORT_TIME time;
    time.hour = (UCHAR) tf.Hour;
    time.minute = (UCHAR) tf.Minute;

    return time;
}

FORT_API void fort_device_conf_open(PFORT_DEVICE_CONF device_conf)
{
    KeInitializeSpinLock(&device_conf->ref_lock);
}

FORT_API UCHAR fort_device_flag_set(PFORT_DEVICE_CONF device_conf, UCHAR flag, BOOL on)
{
    return on ? InterlockedOr8(&device_conf->flags, flag)
              : InterlockedAnd8(&device_conf->flags, ~flag);
}

static UCHAR fort_device_flags(PFORT_DEVICE_CONF device_conf)
{
    return fort_device_flag_set(device_conf, 0, TRUE);
}

FORT_API UCHAR fort_device_flag(PFORT_DEVICE_CONF device_conf, UCHAR flag)
{
    return fort_device_flags(device_conf) & flag;
}

static PFORT_CONF_EXE_NODE fort_conf_ref_exe_find_node(
        PFORT_CONF_REF conf_ref, const PVOID path, UINT32 path_len, tommy_key_t path_hash)
{
    PFORT_CONF_EXE_NODE node =
            (PFORT_CONF_EXE_NODE) tommy_hashdyn_bucket(&conf_ref->exe_map, path_hash);

    while (node != NULL) {
        if (node->path_hash == path_hash
                && fort_conf_app_exe_equal(node->app_entry, path, path_len)) {
            return node;
        }

        node = node->next;
    }

    return NULL;
}

FORT_API FORT_APP_FLAGS fort_conf_exe_find(const PFORT_CONF conf, const PVOID path, UINT32 path_len)
{
    PFORT_CONF_REF conf_ref = (PFORT_CONF_REF) ((char *) conf - offsetof(FORT_CONF_REF, conf));
    const tommy_key_t path_hash = (tommy_key_t) tommy_hash_u64(0, path, path_len);
    FORT_APP_FLAGS app_flags;

    KIRQL oldIrql = ExAcquireSpinLockShared(&conf_ref->conf_lock);
    {
        const PFORT_CONF_EXE_NODE node =
                fort_conf_ref_exe_find_node(conf_ref, path, path_len, path_hash);

        app_flags.v = node ? node->app_entry->flags.v : 0;
    }
    ExReleaseSpinLockShared(&conf_ref->conf_lock, oldIrql);

    return app_flags;
}

static NTSTATUS fort_conf_ref_exe_add_path_locked(PFORT_CONF_REF conf_ref, const PVOID path,
        UINT32 path_len, tommy_key_t path_hash, FORT_APP_FLAGS flags)
{
    const PFORT_CONF_EXE_NODE node =
            fort_conf_ref_exe_find_node(conf_ref, path, path_len, path_hash);

    if (node == NULL) {
        const UINT16 entry_size = (UINT16) FORT_CONF_APP_ENTRY_SIZE(path_len);
        PFORT_APP_ENTRY entry = fort_pool_malloc(&conf_ref->pool_list, entry_size);

        if (entry == NULL)
            return STATUS_INSUFFICIENT_RESOURCES;

        entry->flags = flags;
        entry->path_len = (UINT16) path_len;

        // Copy path
        {
            char *new_path = (char *) (entry + 1);
            RtlCopyMemory(new_path, path, path_len);
        }

        // Add exe node
        {
            PFORT_CONF conf = &conf_ref->conf;

            tommy_arrayof *exe_nodes = &conf_ref->exe_nodes;
            tommy_hashdyn *exe_map = &conf_ref->exe_map;

            tommy_hashdyn_node *exe_node = tommy_list_tail(&conf_ref->free_nodes);

            if (exe_node != NULL) {
                tommy_list_remove_existing(&conf_ref->free_nodes, exe_node);
            } else {
                const UINT16 index = conf->exe_apps_n;

                tommy_arrayof_grow(exe_nodes, index + 1);

                exe_node = tommy_arrayof_ref(exe_nodes, index);
            }

            tommy_hashdyn_insert(exe_map, exe_node, entry, path_hash);

            ++conf->exe_apps_n;
        }
    } else {
        if (flags.is_new)
            return FORT_STATUS_USER_ERROR;

        // Replace flags
        {
            PFORT_APP_ENTRY entry = node->app_entry;
            entry->flags = flags;
        }
    }

    return STATUS_SUCCESS;
}

FORT_API NTSTATUS fort_conf_ref_exe_add_path(
        PFORT_CONF_REF conf_ref, const PVOID path, UINT32 path_len, FORT_APP_FLAGS flags)
{
    const tommy_key_t path_hash = (tommy_key_t) tommy_hash_u64(0, path, path_len);
    NTSTATUS status;

    KIRQL oldIrql = ExAcquireSpinLockExclusive(&conf_ref->conf_lock);
    status = fort_conf_ref_exe_add_path_locked(conf_ref, path, path_len, path_hash, flags);
    ExReleaseSpinLockExclusive(&conf_ref->conf_lock, oldIrql);

    return status;
}

FORT_API NTSTATUS fort_conf_ref_exe_add_entry(
        PFORT_CONF_REF conf_ref, const PFORT_APP_ENTRY entry, BOOL locked)
{
    const PVOID path = (const PVOID)(entry + 1);
    const UINT32 path_len = entry->path_len;
    const FORT_APP_FLAGS flags = entry->flags;

    if (locked) {
        const tommy_key_t path_hash = (tommy_key_t) tommy_hash_u64(0, path, path_len);

        return fort_conf_ref_exe_add_path_locked(conf_ref, path, path_len, path_hash, flags);
    } else {
        return fort_conf_ref_exe_add_path(conf_ref, path, path_len, flags);
    }
}

static void fort_conf_ref_exe_fill(PFORT_CONF_REF conf_ref, const PFORT_CONF conf)
{
    const char *app_entries = (const char *) (conf->data + conf->exe_apps_off);

    const int count = conf->exe_apps_n;

    for (int i = 0; i < count; ++i) {
        const PFORT_APP_ENTRY entry = (const PFORT_APP_ENTRY) app_entries;

        fort_conf_ref_exe_add_entry(conf_ref, entry, TRUE);

        app_entries += FORT_CONF_APP_ENTRY_SIZE(entry->path_len);
    }
}

static void fort_conf_ref_exe_del_path(PFORT_CONF_REF conf_ref, const PVOID path, UINT32 path_len)
{
    const tommy_key_t path_hash = (tommy_key_t) tommy_hash_u64(0, path, path_len);

    KIRQL oldIrql = ExAcquireSpinLockExclusive(&conf_ref->conf_lock);
    {
        PFORT_CONF_EXE_NODE node = fort_conf_ref_exe_find_node(conf_ref, path, path_len, path_hash);

        if (node != NULL) {
            // Delete from conf
            {
                PFORT_CONF conf = &conf_ref->conf;
                --conf->exe_apps_n;
            }

            // Delete from pool
            {
                PFORT_APP_ENTRY entry = node->app_entry;
                fort_pool_free(&conf_ref->pool_list, entry);
            }

            // Delete from exe map
            tommy_hashdyn_remove_existing(&conf_ref->exe_map, (tommy_hashdyn_node *) node);

            tommy_list_insert_tail_check(&conf_ref->free_nodes, (tommy_node *) node);
        }
    }
    ExReleaseSpinLockExclusive(&conf_ref->conf_lock, oldIrql);
}

FORT_API void fort_conf_ref_exe_del_entry(PFORT_CONF_REF conf_ref, const PFORT_APP_ENTRY entry)
{
    const PVOID path = (const PVOID)(entry + 1);
    const UINT32 path_len = entry->path_len;

    fort_conf_ref_exe_del_path(conf_ref, path, path_len);
}

static void fort_conf_ref_init(PFORT_CONF_REF conf_ref)
{
    conf_ref->refcount = 0;

    fort_pool_list_init(&conf_ref->pool_list);
    tommy_list_init(&conf_ref->free_nodes);

    tommy_arrayof_init(&conf_ref->exe_nodes, sizeof(FORT_CONF_EXE_NODE));
    tommy_hashdyn_init(&conf_ref->exe_map);

    conf_ref->conf_lock = 0;
}

FORT_API PFORT_CONF_REF fort_conf_ref_new(const PFORT_CONF conf, ULONG len)
{
    const ULONG conf_len = FORT_CONF_DATA_OFF + conf->exe_apps_off;
    const ULONG ref_len = conf_len + offsetof(FORT_CONF_REF, conf);
    PFORT_CONF_REF conf_ref = tommy_malloc(ref_len);

    if (conf_ref != NULL) {
        RtlCopyMemory(&conf_ref->conf, conf, conf_len);

        fort_conf_ref_init(conf_ref);
        fort_pool_init(&conf_ref->pool_list, len - conf_len);

        fort_conf_ref_exe_fill(conf_ref, conf);
    }

    return conf_ref;
}

static void fort_conf_ref_del(PFORT_CONF_REF conf_ref)
{
    fort_pool_done(&conf_ref->pool_list);

    tommy_hashdyn_done(&conf_ref->exe_map);
    tommy_arrayof_done(&conf_ref->exe_nodes);

    tommy_free(conf_ref);
}

FORT_API void fort_conf_ref_put(PFORT_DEVICE_CONF device_conf, PFORT_CONF_REF conf_ref)
{
    KLOCK_QUEUE_HANDLE lock_queue;
    KeAcquireInStackQueuedSpinLock(&device_conf->ref_lock, &lock_queue);
    {
        const UINT32 refcount = --conf_ref->refcount;

        if (refcount == 0 && conf_ref != device_conf->ref) {
            fort_conf_ref_del(conf_ref);
        }
    }
    KeReleaseInStackQueuedSpinLock(&lock_queue);
}

FORT_API PFORT_CONF_REF fort_conf_ref_take(PFORT_DEVICE_CONF device_conf)
{
    PFORT_CONF_REF conf_ref;

    KLOCK_QUEUE_HANDLE lock_queue;
    KeAcquireInStackQueuedSpinLock(&device_conf->ref_lock, &lock_queue);
    {
        conf_ref = device_conf->ref;
        if (conf_ref != NULL) {
            ++conf_ref->refcount;
        }
    }
    KeReleaseInStackQueuedSpinLock(&lock_queue);

    return conf_ref;
}

FORT_API FORT_CONF_FLAGS fort_conf_ref_set(PFORT_DEVICE_CONF device_conf, PFORT_CONF_REF conf_ref)
{
    FORT_CONF_FLAGS old_conf_flags;

    const PFORT_CONF_REF old_conf_ref = fort_conf_ref_take(device_conf);

    if (old_conf_ref != NULL) {
        old_conf_flags = old_conf_ref->conf.flags;
    } else {
        RtlZeroMemory(&old_conf_flags, sizeof(FORT_CONF_FLAGS));
        old_conf_flags.prov_boot = fort_device_flag(device_conf, FORT_DEVICE_PROV_BOOT) != 0;
    }

    KLOCK_QUEUE_HANDLE lock_queue;
    KeAcquireInStackQueuedSpinLock(&device_conf->ref_lock, &lock_queue);
    {
        FORT_CONF_FLAGS conf_flags;

        device_conf->ref = conf_ref;

        if (conf_ref != NULL) {
            PFORT_CONF conf = &conf_ref->conf;

            conf_flags = conf->flags;
            fort_device_flag_set(device_conf, FORT_DEVICE_PROV_BOOT, conf_flags.prov_boot);
        } else {
            RtlZeroMemory((void *) &conf_flags, sizeof(FORT_CONF_FLAGS));
            conf_flags.prov_boot = fort_device_flag(device_conf, FORT_DEVICE_PROV_BOOT) != 0;
        }

        device_conf->conf_flags = conf_flags;
    }
    KeReleaseInStackQueuedSpinLock(&lock_queue);

    if (old_conf_ref != NULL) {
        fort_conf_ref_put(device_conf, old_conf_ref);
    }

    return old_conf_flags;
}

FORT_API FORT_CONF_FLAGS fort_conf_ref_flags_set(
        PFORT_DEVICE_CONF device_conf, const PFORT_CONF_FLAGS conf_flags)
{
    FORT_CONF_FLAGS old_conf_flags;

    KLOCK_QUEUE_HANDLE lock_queue;
    KeAcquireInStackQueuedSpinLock(&device_conf->ref_lock, &lock_queue);
    {
        PFORT_CONF_REF conf_ref = device_conf->ref;

        if (conf_ref != NULL) {
            PFORT_CONF conf = &conf_ref->conf;

            old_conf_flags = conf->flags;
            conf->flags = *conf_flags;

            fort_conf_app_perms_mask_init(conf, conf_flags->group_bits);

            fort_device_flag_set(device_conf, FORT_DEVICE_PROV_BOOT, conf_flags->prov_boot);

            device_conf->conf_flags = *conf_flags;
        } else {
            RtlZeroMemory(&old_conf_flags, sizeof(FORT_CONF_FLAGS));
            old_conf_flags.prov_boot = fort_device_flag(device_conf, FORT_DEVICE_PROV_BOOT) != 0;

            device_conf->conf_flags = old_conf_flags;
        }
    }
    KeReleaseInStackQueuedSpinLock(&lock_queue);

    return old_conf_flags;
}

FORT_API BOOL fort_conf_ref_period_update(PFORT_DEVICE_CONF device_conf, BOOL force, int *periods_n)
{
    PFORT_CONF_REF conf_ref = fort_conf_ref_take(device_conf);

    if (conf_ref == NULL)
        return FALSE;

    BOOL res = FALSE;
    PFORT_CONF conf = &conf_ref->conf;

    if (conf->app_periods_n != 0) {
        const FORT_TIME time = fort_current_time();
        const UINT16 period_bits = fort_conf_app_period_bits(conf, time, periods_n);

        if (force || device_conf->conf_flags.group_bits != period_bits) {
            device_conf->conf_flags.group_bits = period_bits;

            fort_conf_app_perms_mask_init(conf, period_bits);

            res = TRUE;
        }
    }

    fort_conf_ref_put(device_conf, conf_ref);

    return res;
}

FORT_API PFORT_CONF_ZONES fort_conf_zones_new(PFORT_CONF_ZONES zones, ULONG len)
{
    PFORT_CONF_ZONES conf_zones = fort_mem_alloc(len, FORT_ZONES_POOL_TAG);
    if (conf_zones != NULL) {
        RtlCopyMemory(conf_zones, zones, len);
    }
    return conf_zones;
}

static void fort_conf_zones_free(PFORT_CONF_ZONES zones)
{
    if (zones != NULL) {
        fort_mem_free(zones, FORT_ZONES_POOL_TAG);
    }
}

FORT_API void fort_conf_zones_set(PFORT_DEVICE_CONF device_conf, PFORT_CONF_ZONES zones)
{
    KIRQL oldIrql = ExAcquireSpinLockExclusive(&device_conf->zones_lock);
    fort_conf_zones_free(device_conf->zones);
    device_conf->zones = zones;
    ExReleaseSpinLockExclusive(&device_conf->zones_lock, oldIrql);
}

FORT_API void fort_conf_zone_flag_set(PFORT_DEVICE_CONF device_conf, PFORT_CONF_ZONE_FLAG zone_flag)
{
    KIRQL oldIrql = ExAcquireSpinLockExclusive(&device_conf->zones_lock);
    PFORT_CONF_ZONES zones = device_conf->zones;
    if (zones != NULL) {
        const UINT32 zone_mask = (1u << (zone_flag->zone_id - 1));
        if (zone_flag->enabled) {
            zones->enabled_mask |= zone_mask;
        } else {
            zones->enabled_mask &= ~zone_mask;
        }
    }
    ExReleaseSpinLockExclusive(&device_conf->zones_lock, oldIrql);
}

FORT_API BOOL fort_conf_zones_ip_included(
        PFORT_DEVICE_CONF device_conf, UINT32 zones_mask, UINT32 remote_ip)
{
    BOOL res = FALSE;

    KIRQL oldIrql = ExAcquireSpinLockShared(&device_conf->zones_lock);
    PFORT_CONF_ZONES zones = device_conf->zones;
    if (zones != NULL) {
        zones_mask &= (zones->mask & zones->enabled_mask);
        while (zones_mask != 0) {
            const int zone_index = bit_scan_forward(zones_mask);
            PFORT_CONF_ADDR_LIST addr_list =
                    (PFORT_CONF_ADDR_LIST) (zones->data + zones->addr_off[zone_index]);

            if (fort_conf_ip_inlist(remote_ip, addr_list)) {
                res = TRUE;
                break;
            }

            zones_mask ^= (1u << zone_index);
        }
    }
    ExReleaseSpinLockShared(&device_conf->zones_lock, oldIrql);

    return res;
}
