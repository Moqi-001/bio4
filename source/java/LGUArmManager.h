#ifndef LGU_ARM_MANAGER_H
#define LGU_ARM_MANAGER_H

#include "android/AActivity.h"
#include "android/AContext.h"

#define SERVICE_CONNECT   1
#define SERVICE_FAIL      2
#define SERVICE_NOT_EXIST 3

typedef struct LGUArmListener {
    void (*onArmResult)(struct LGUArmListener *thiz);
} LGUArmListener;

typedef struct ArmConnection ArmConnection;
typedef struct ArmInterface  ArmInterface;

typedef struct LGUArmManager {
    // public
    ArmConnection *armCon;
    bool           bSevice;
    int            netState;
    int            resCode;
    char          *resMsg;
    ArmInterface  *service;

    // private
    char     *AID;
    AContext *ctx;
    char     *vName;
    char     *countryName;
    char     *providerName;
    char     *serviceName;
    char     *pvName;
} LGUArmManager;

void           LGUArmManager_ctor(LGUArmManager *thiz, AContext *ctx);
LGUArmManager *LGUArmManager_new(AActivity *activity);
bool           LGUArmManager_ARM_Plugin_ExecuteARM(LGUArmManager *thiz, char *str);
void           LGUArmManager_setArmListener(LGUArmManager *thiz, LGUArmListener *listener);

#endif