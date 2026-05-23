#ifndef LGU_ARM_LISTENER_H
#define LGU_ARM_LISTENER_H

#define SERVICE_CONNECT   1
#define SERVICE_FAIL      2
#define SERVICE_NOT_EXIST 3

typedef struct LGUArmListener {
    void (*onArmResult)(struct LGUArmListener *thiz);
} LGUArmListener;

#endif