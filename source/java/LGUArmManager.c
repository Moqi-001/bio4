#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include <so_common/so_logger.h>

#include "LGUArmManager.h"

#include "android/AActivity.h"
#include "android/AContext.h"
#include "android/AIBinder.h"

static const char    *TAG = "LGUArmManger";
static LGUArmListener mArmListener;

void ArmConnection_ctor(ArmConnection *thiz) {
}

void ArmConnection_ctor2(ArmConnection *thiz, LGUArmManager *lGUArmManager, ArmConnection *armConnection) {
    ArmConnection_ctor(thiz);
}

// @Override
void ArmConnection_onServiceConnected(ArmConnection *thiz, AComponentName *name, AIBinder *boundService) {
    so_logd("%s onServiceConnected", TAG);
    /*
        if (LGUArmManager.this.service == null) {
            LGUArmManager.this.service = ArmInterface.Stub.asInterface(boundService);
        }
        LGUArmManager.this.netState = 1;
        LGUArmManager.this.getLicenseStatus(LGUArmManager.this.AID);
    */
}

// @Override
void ArmConnection_onServiceDisconnected(ArmConnection *thiz, AComponentName *name) {
    so_logd("%s onServiceDisconnected", TAG);
}

void LGUArmManager_ctor(LGUArmManager *thiz, AContext *ctx) {
    so_logd("%s Create", TAG);

    thiz->ctx          = ctx;
    thiz->countryName  = "KR";
    thiz->providerName = "LGU";
    thiz->serviceName  = "OZS";
    thiz->pvName       = "000001";
}

LGUArmManager *LGUArmManager_new(AActivity *activity) {
    LGUArmManager *mg = (LGUArmManager *)calloc(1, sizeof(LGUArmManager));
    if (!mg) return NULL;

    LGUArmManager_ctor(mg, NULL);
    return mg;
}

bool LGUArmManager_ARM_Plugin_ExecuteARM(LGUArmManager *thiz, char *str) {
    thiz->netState = 1;
    mArmListener.onArmResult(&mArmListener);
    return true;
}

void LGUArmManager_setArmListener(LGUArmManager *thiz, LGUArmListener *listener) {
    memcpy(&mArmListener, listener, sizeof(LGUArmListener));
}
