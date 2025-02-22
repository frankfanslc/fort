#ifndef FORTDEF_H
#define FORTDEF_H

enum FortLogType {
    FORT_LOG_TYPE_NONE = 0,
    FORT_LOG_TYPE_BLOCKED = 1,
    FORT_LOG_TYPE_BLOCKED_IP,
    FORT_LOG_TYPE_ALLOWED,
    FORT_LOG_TYPE_PROC_NEW,
    FORT_LOG_TYPE_STAT_TRAF,
    FORT_LOG_TYPE_TIME,
};

enum FortBlockReason {
    FORT_BLOCK_REASON_NONE = -1,
    FORT_BLOCK_REASON_UNKNOWN = 0,
    FORT_BLOCK_REASON_IP_INET,
    FORT_BLOCK_REASON_REAUTH,
    FORT_BLOCK_REASON_PROGRAM,
    FORT_BLOCK_REASON_APP_GROUP_FOUND,
    FORT_BLOCK_REASON_APP_GROUP_DEFAULT,
};

#endif // FORTDEF_H
