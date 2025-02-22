#include "control.h"

namespace Control {

#define CASE_STRING(id)                                                                            \
    case (id):                                                                                     \
        return #id;

const char *const commandString(Command cmd)
{
    switch (cmd) {
        CASE_STRING(CommandNone)

        CASE_STRING(Prog)

        CASE_STRING(Rpc_Result_Ok)
        CASE_STRING(Rpc_Result_Error)
        CASE_STRING(Rpc_RpcManager_initClient)

        CASE_STRING(Rpc_AppInfoManager_lookupAppInfo)
        CASE_STRING(Rpc_AppInfoManager_checkLookupFinished)

        CASE_STRING(Rpc_ConfManager_save)
        CASE_STRING(Rpc_ConfManager_addApp)
        CASE_STRING(Rpc_ConfManager_deleteApp)
        CASE_STRING(Rpc_ConfManager_purgeApps)
        CASE_STRING(Rpc_ConfManager_updateApp)
        CASE_STRING(Rpc_ConfManager_updateAppBlocked)
        CASE_STRING(Rpc_ConfManager_updateAppName)
        CASE_STRING(Rpc_ConfManager_addZone)
        CASE_STRING(Rpc_ConfManager_deleteZone)
        CASE_STRING(Rpc_ConfManager_updateZone)
        CASE_STRING(Rpc_ConfManager_updateZoneName)
        CASE_STRING(Rpc_ConfManager_updateZoneEnabled)
        CASE_STRING(Rpc_ConfManager_checkPassword)
        CASE_STRING(Rpc_ConfManager_confChanged)
        CASE_STRING(Rpc_ConfManager_appAlerted)
        CASE_STRING(Rpc_ConfManager_appChanged)
        CASE_STRING(Rpc_ConfManager_appUpdated)
        CASE_STRING(Rpc_ConfManager_zoneAdded)
        CASE_STRING(Rpc_ConfManager_zoneRemoved)
        CASE_STRING(Rpc_ConfManager_zoneUpdated)

        CASE_STRING(Rpc_DriverManager_updateState)

        CASE_STRING(Rpc_QuotaManager_alert)

        CASE_STRING(Rpc_StatManager_deleteStatApp)
        CASE_STRING(Rpc_StatManager_deleteConn)
        CASE_STRING(Rpc_StatManager_deleteConnAll)
        CASE_STRING(Rpc_StatManager_resetAppTrafTotals)
        CASE_STRING(Rpc_StatManager_clearTraffic)
        CASE_STRING(Rpc_StatManager_trafficCleared)
        CASE_STRING(Rpc_StatManager_appStatRemoved)
        CASE_STRING(Rpc_StatManager_appCreated)
        CASE_STRING(Rpc_StatManager_trafficAdded)
        CASE_STRING(Rpc_StatManager_connChanged)
        CASE_STRING(Rpc_StatManager_appTrafTotalsResetted)

        CASE_STRING(Rpc_ServiceInfoManager_trackService)
        CASE_STRING(Rpc_ServiceInfoManager_revertService)

        CASE_STRING(Rpc_TaskManager_runTask)
        CASE_STRING(Rpc_TaskManager_abortTask)
        CASE_STRING(Rpc_TaskManager_taskStarted)
        CASE_STRING(Rpc_TaskManager_taskFinished)

    default:
        return nullptr;
    }
}

const char *const rpcManagerString(RpcManager rpcManager)
{
    switch (rpcManager) {
        CASE_STRING(Rpc_NoneManager)
        CASE_STRING(Rpc_AppInfoManager)
        CASE_STRING(Rpc_ConfManager)
        CASE_STRING(Rpc_DriverManager)
        CASE_STRING(Rpc_QuotaManager)
        CASE_STRING(Rpc_StatManager)
        CASE_STRING(Rpc_ServiceInfoManager)
        CASE_STRING(Rpc_TaskManager)
    default:
        return nullptr;
    }
}

RpcManager managerByCommand(Command cmd)
{
    static const RpcManager g_commandManagers[] = {
        Rpc_NoneManager, // CommandNone = 0,

        Rpc_NoneManager, // Prog,

        Rpc_NoneManager, // Rpc_Result_Ok,
        Rpc_NoneManager, // Rpc_Result_Error,

        Rpc_NoneManager, // Rpc_RpcManager_initClient,

        Rpc_AppInfoManager, // Rpc_AppInfoManager_lookupAppInfo,
        Rpc_AppInfoManager, // Rpc_AppInfoManager_checkLookupFinished,

        Rpc_ConfManager, // Rpc_ConfManager_save,
        Rpc_ConfManager, // Rpc_ConfManager_addApp,
        Rpc_ConfManager, // Rpc_ConfManager_deleteApp,
        Rpc_ConfManager, // Rpc_ConfManager_purgeApps,
        Rpc_ConfManager, // Rpc_ConfManager_updateApp,
        Rpc_ConfManager, // Rpc_ConfManager_updateAppBlocked,
        Rpc_ConfManager, // Rpc_ConfManager_updateAppName,
        Rpc_ConfManager, // Rpc_ConfManager_addZone,
        Rpc_ConfManager, // Rpc_ConfManager_deleteZone,
        Rpc_ConfManager, // Rpc_ConfManager_updateZone,
        Rpc_ConfManager, // Rpc_ConfManager_updateZoneName,
        Rpc_ConfManager, // Rpc_ConfManager_updateZoneEnabled,
        Rpc_ConfManager, // Rpc_ConfManager_checkPassword,
        Rpc_ConfManager, // Rpc_ConfManager_confChanged,
        Rpc_ConfManager, // Rpc_ConfManager_appEndTimesUpdated,
        Rpc_ConfManager, // Rpc_ConfManager_appAlerted,
        Rpc_ConfManager, // Rpc_ConfManager_appChanged,
        Rpc_ConfManager, // Rpc_ConfManager_appUpdated,
        Rpc_ConfManager, // Rpc_ConfManager_zoneAdded,
        Rpc_ConfManager, // Rpc_ConfManager_zoneRemoved,
        Rpc_ConfManager, // Rpc_ConfManager_zoneUpdated,

        Rpc_DriverManager, // Rpc_DriverManager_updateState,

        Rpc_QuotaManager, // Rpc_QuotaManager_alert,

        Rpc_StatManager, // Rpc_StatManager_deleteStatApp,
        Rpc_StatManager, // Rpc_StatManager_deleteConn,
        Rpc_StatManager, // Rpc_StatManager_deleteConnAll,
        Rpc_StatManager, // Rpc_StatManager_resetAppTrafTotals,
        Rpc_StatManager, // Rpc_StatManager_clearTraffic,
        Rpc_StatManager, // Rpc_StatManager_trafficCleared,
        Rpc_StatManager, // Rpc_StatManager_appStatRemoved,
        Rpc_StatManager, // Rpc_StatManager_appCreated,
        Rpc_StatManager, // Rpc_StatManager_trafficAdded,
        Rpc_StatManager, // Rpc_StatManager_connChanged,
        Rpc_StatManager, // Rpc_StatManager_appTrafTotalsResetted,

        Rpc_ServiceInfoManager, // Rpc_ServiceInfoManager_trackService,
        Rpc_ServiceInfoManager, // Rpc_ServiceInfoManager_revertService,

        Rpc_TaskManager, // Rpc_TaskManager_runTask,
        Rpc_TaskManager, // Rpc_TaskManager_abortTask,
        Rpc_TaskManager, // Rpc_TaskManager_taskStarted,
        Rpc_TaskManager, // Rpc_TaskManager_taskFinished,
    };

    return g_commandManagers[cmd];
}

bool commandRequiresValidation(Command cmd)
{
    static const qint8 g_commandValidations[] = {
        0, // CommandNone = 0,

        0, // Prog,

        0, // Rpc_Result_Ok,
        0, // Rpc_Result_Error,

        0, // Rpc_RpcManager_initClient,

        true, // Rpc_AppInfoManager_lookupAppInfo,
        0, // Rpc_AppInfoManager_checkLookupFinished,

        true, // Rpc_ConfManager_save,
        true, // Rpc_ConfManager_addApp,
        true, // Rpc_ConfManager_deleteApp,
        true, // Rpc_ConfManager_purgeApps,
        true, // Rpc_ConfManager_updateApp,
        true, // Rpc_ConfManager_updateAppBlocked,
        true, // Rpc_ConfManager_updateAppName,
        true, // Rpc_ConfManager_addZone,
        true, // Rpc_ConfManager_deleteZone,
        true, // Rpc_ConfManager_updateZone,
        true, // Rpc_ConfManager_updateZoneName,
        true, // Rpc_ConfManager_updateZoneEnabled,
        0, // Rpc_ConfManager_checkPassword,
        0, // Rpc_ConfManager_confChanged,
        0, // Rpc_ConfManager_appEndTimesUpdated,
        0, // Rpc_ConfManager_appAlerted,
        0, // Rpc_ConfManager_appChanged,
        0, // Rpc_ConfManager_appUpdated,
        0, // Rpc_ConfManager_zoneAdded,
        0, // Rpc_ConfManager_zoneRemoved,
        0, // Rpc_ConfManager_zoneUpdated,

        0, // Rpc_DriverManager_updateState,

        0, // Rpc_QuotaManager_alert,

        true, // Rpc_StatManager_deleteStatApp,
        true, // Rpc_StatManager_deleteConn,
        true, // Rpc_StatManager_deleteConnAll,
        true, // Rpc_StatManager_resetAppTrafTotals,
        true, // Rpc_StatManager_clearTraffic,
        0, // Rpc_StatManager_trafficCleared,
        0, // Rpc_StatManager_appStatRemoved,
        0, // Rpc_StatManager_appCreated,
        0, // Rpc_StatManager_trafficAdded,
        0, // Rpc_StatManager_connChanged,
        0, // Rpc_StatManager_appTrafTotalsResetted,

        true, // Rpc_TaskManager_runTask,
        true, // Rpc_TaskManager_abortTask,
        0, // Rpc_TaskManager_taskStarted,
        0, // Rpc_TaskManager_taskFinished,
    };

    return g_commandValidations[cmd];
}

QDebug operator<<(QDebug debug, Command cmd)
{
    debug << commandString(cmd);
    return debug;
}

QDebug operator<<(QDebug debug, RpcManager rpcManager)
{
    debug << rpcManagerString(rpcManager);
    return debug;
}

}
