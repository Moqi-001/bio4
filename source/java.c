#include <falso_jni/FalsoJNI_Impl.h>
#include <falso_jni/FalsoJNI.h>
#include "utils/logger.h"
#include "patch.h"
#include <dirent.h>
#include <malloc.h>
#include <psp2/kernel/threadmgr.h>
/*
 * JNI Methods
*/

// jlong GetCurrentTime(jmethodID id, va_list args) {
//     int time=current_timestamp_ms();
//     debugPrintf("GetCurrentTime(%d)",time);
//     return (int64_t) time;
// }

jlong GetCurrentTime(jmethodID id, va_list args)
{
    return (int64_t)current_timestamp_ms();
}

void Native_Exit(jmethodID id, va_list args)
{
    debugPrintf("Native_Exit() requested via JNI.");
    exit(0);
}

void vibrate(jmethodID id, va_list args)
{
    debugPrintf("vibrate()");
}

jobject getLocale(jmethodID id, va_list args)
{
    debugPrintf("getLocale()");
    return "en";
}

jobject getCommonDir(jmethodID id, va_list args)
{
    debugPrintf("getCommonDir()");
    return "ux0:data/Acrodea/assets/ggee/";
}

void appFinish(jmethodID id, va_list args)
{
    debugPrintf("appFinish()");
}

void updateResourceCheck(jmethodID id, va_list args)
{
    const char *value = va_arg(args, char *);
    debugPrintf("updateResourceCheck(%s)", value);
}

void invisibleRuntimeLogo(jmethodID id, va_list args)
{
    debugPrintf("invisibleRuntimeLogo()");
}

jint stereoIsAvailability(jmethodID id, va_list args)
{
    debugPrintf("stereoIsAvailability()");
    return 1;
}

void stereoSetView(jmethodID id, va_list args)
{
    debugPrintf("stereoSetView()");
}

jint isUseRecord(jmethodID id, va_list args)
{
    debugPrintf("isUseRecord()");
    return 0;
}

float Depth=0;
jfloat stereoGetDepth(jmethodID id, va_list args)
{
    debugPrintf("stereoGetDepth()");
    if(Depth==1)
       Depth=0;
    else 
      Depth=1;
    return Depth;
}

jobject getCommonAppDir(jmethodID id, va_list args)
{
    const char *value = va_arg(args, char *);
    debugPrintf("getCommonAppDir(%s)", value);
    return DATA_PATH;
}

void launchRuntime(jmethodID id, va_list args)
{
    const char *value = va_arg(args, char *);
    debugPrintf("launchRuntime(%s)", value);
}

jobject getEnv(jmethodID id, va_list args)
{
    const char *value = va_arg(args, char *);
    debugPrintf("getEnv(%s)", value);
    if (strcmp(value, "DEVICE_VENDOR") == 0)
    {
        return "Sony";
    }
    else if (strcmp(value, "DEVICE_MODEL") == 0)
    {
        return "Psvita";
    }
    else if (strcmp(value, "OS_VERSION") == 0)
    {
        return "2.1";
    }
    else if (strcmp(value, "OS") == 0)
    {
        return "Android";
    }
    else if (strcmp(value, "GL_RENDERER") == 0)
    {
        return "Adreno";
    }

    return "";
}

void trialSetTime(jmethodID id, va_list args)
{
    int time = va_arg(args, int);
    debugPrintf("trialSetTime(%d)", time);
}

void jniTrackerRemoveAttribute(jmethodID id, va_list args)
{
    int value = va_arg(args, int);
    debugPrintf("jniTrackerRemoveAttribute(%d)", value);
}

void jniTrackerSetPage(jmethodID id, va_list args)
{
    const char *value = va_arg(args, char *);
    debugPrintf("jniTrackerSetPage(%s)", value);
}

void jniTrackerSetAttribute(jmethodID id, va_list args)
{
    const char *value = va_arg(args, char *);
    debugPrintf("jniTrackerSetAttribute(%s)", value);
}

void jniTrackerEvent(jmethodID id, va_list args)
{
    const char *value = va_arg(args, char *);
    const char *value2 = va_arg(args, char *);
    debugPrintf("jniTrackerEvent(%s,%s)", value, value2);
}

void jniTrackerAssignAttribute(jmethodID id, va_list args)
{
    const char *value = va_arg(args, char *);
    debugPrintf("jniTrackerAssignAttribute(%s)", value);
}

typedef struct {
    int i;
    jstring str;
    int UserPtr;
    int kw;
} MyHttp;

MyHttp httpObj;

jobject MyHttp_constructor(jmethodID id, va_list args)
{
    int i = va_arg(args, int);
    debugPrintf("MyHttp_constructor(%p)", i);
    httpObj.i = i;
    return (jobject)&httpObj;
}

int code=0;

jint MyHttp_obj_init(jmethodID id, va_list args)
{
    int ERROR_INIT = -18;
    int NOT_HTTP_URL = -17;
    int REQUEST_OK = 0;

    httpObj.str = va_arg(args, jstring); // boot_from=1760app/1.0.0/top/topUrlInformation
    int i1 = va_arg(args, int); // 1
    int i2 = va_arg(args, int); // 0

    // 这个值要存起来, 实际通过 init 函数传递给了 k.kw
    httpObj.kw = va_arg(args, int); // 0
    
    debugPrintf("MyHttp_obj_init(%s, %d, %d, %d)", httpObj.str, i1, i2, httpObj.kw);

    // 判定 URL 是否以 "http" 开头
    if (httpObj.str == NULL || strncmp(httpObj.str, "http", 4) != 0) {
        // 正常 Java 层代码返回的 NOT_HTTP_URL
        return REQUEST_OK;
    }

    // 现在服务器不通, 正常应该返回 NOT_HTTP_URL
    return REQUEST_OK;
}

jint MyHttp_obj_setUserPtr(jmethodID id, va_list args)
{
    // 这个值继续存起来, 后面要用, 实际存到了 k.kv
    httpObj.UserPtr = va_arg(args, int);
    debugPrintf("MyHttp_obj_setUserPtr(%p)", httpObj.UserPtr);

    // Java 层永远返回 0
    return 0;
}

typedef struct {
    int arg1;
    int arg2;
    int arg3;
    int arg4;
    int userPtr;
    int kw;
    int i;
} ThreadArgs;

SceUID thread_id=-1;
jint MyHttp_obj_connect(jmethodID id, va_list args)
{
    
    // Java 层正确处理返回 0
    return 0;
}

jint MyHttp_destroy(jmethodID id, va_list args)
{
    return 0;
}

jint getWifiState(jmethodID id, va_list args)
{
    debugPrintf("getWifiState()");
    return 0;
}

jint getCellState(jmethodID id, va_list args)
{
    debugPrintf("getCellState()");
    return 0;
}

jobject getShareData(jmethodID id, va_list args)
{
    const char *value = va_arg(args, char *);
    debugPrintf("getShareData(%s)", value);
    return "true";
}

jint setPropertycv(jmethodID id, va_list args)
{
    int i = va_arg(args, int);
    const char *value = va_arg(args, char *);
    debugPrintf("setPropertycv(%d,%s)",i, value);
    return 0;
}

jint show(jmethodID id, va_list args)
{
    debugPrintf("show()");
    return 0;
}

jobject getText(jmethodID id, va_list args)
{
    debugPrintf("getText()");
    return "1";
}

jint getWidth(jmethodID id, va_list args)
{
    const char *value = va_arg(args, char *);
    debugPrintf("getWidth(%s)",value);
    return strlen(value);
}

void getFontInfo(jmethodID id, va_list args)
{
    debugPrintf("getFontInfo()");
}

jobject getAutoLoginID(jmethodID id, va_list args)
{
    debugPrintf("getAutoLoginID()");
    return "111000";
}

jobject getExtensonByName(jmethodID id, va_list args)
{
    debugPrintf("getExtensonByName()");
    return "MyExtensionInterface";
}

jobject getImei(jmethodID id, va_list args)
{
    debugPrintf("getImei()");
    return "IMEI:";
}

jobject getImsi(jmethodID id, va_list args)
{
    debugPrintf("getImsi()");
    return "000000000000000";
}

jint setProgress(jmethodID id, va_list args)
{
    int i = va_arg(args, int);
    debugPrintf("setProgress(%d)",i);
    return 0;
}

jobject drawStringEx(jmethodID id, va_list args)
{
    const char *value = va_arg(args, char *);
    debugPrintf("drawStringEx(%s)",value);
    return NULL;
}

jobject drawString(jmethodID id, va_list args)
{
    const char *value = va_arg(args, char *);
    debugPrintf("drawString(%s)",value);
    return NULL;
}

jint getHeightEx(jmethodID id, va_list args)
{
    const char *value = va_arg(args, char *);
    debugPrintf("getHeightEx(%s)",value);
    return 1;
}

void setMessage(jmethodID id, va_list args)
{
    const char *value = va_arg(args, char *);
    debugPrintf("setMessage(%s)",value);
}

jint playVibrateTimer(jmethodID id, va_list args)
{
    //debugPrintf("playVibrateTimer()");
    return 0;
}

int downloadApplicationNum=0;
jobject getDownloadApplicationTypeInfoFromIndex(jmethodID id, va_list args)
{
    int i = va_arg(args, int);
    int i2 = va_arg(args, int);
    debugPrintf("getDownloadApplicationTypeInfoFromIndex(%d,%d)",i,i2);
    if(i>=downloadApplicationNum)
    {
        return "";
    }
    switch (i2) 
    {
        case 0:
          return "1760";
        case 1:
          return "DevilMayCry4";
        case 2:
          return "CAPCOM";
        default:
          return "";
     
     }
    return "";
}

int uninstallApplicationNum=0;
jobject getUninstallApplicationTypeInfoFromIndex(jmethodID id, va_list args)
{
    int i = va_arg(args, int);
    int i2 = va_arg(args, int);
    debugPrintf("getUninstallApplicationTypeInfoFromIndex(%d,%d)",i,i2);
    if(i>=uninstallApplicationNum)
    {
        return "";
    }
    switch (i2) 
    {
        case 0:
          return "1760";
        default:
          return "";
    }
    return "";
}

jint getUninstallApplicationCount(jmethodID id, va_list args)
{
    debugPrintf("getUninstallApplicationCount()");
    return uninstallApplicationNum;
}

jint applicationDownloadGetPauseStatus(jmethodID id, va_list args)
{
    debugPrintf("applicationDownloadGetPauseStatus()");
    return 0;
}

jint applicationDownloadGetNetowrkPauseStatus(jmethodID id, va_list args)
{
    debugPrintf("applicationDownloadGetNetowrkPauseStatus()");
    return 0;
}

int index=0;
jint getDownloadApplicationInfoFromIndex(jmethodID id, va_list args)
{
    int i = va_arg(args, int);
    debugPrintf("getDownloadApplicationInfoFromIndex(%d)",i);
    return -1;

    if(i<downloadApplicationNum)
    {
        if(index>=100)
        {
          //Install(&jni,0,"/runtime/","/RuntimeApps/","/RuntimeApps/","","","");
          index=0;
        }
        else 
        {
           index+=20;
        }
        
        return index;
    }
    
    return -1;
}

jint getDownloadApplicationBodySizeFromIndex(jmethodID id, va_list args)
{
    int i = va_arg(args, int);
    debugPrintf("getDownloadApplicationBodySizeFromIndex(%d)",i);
    if(i<downloadApplicationNum)
    {
        return 100;
    }
    
    return -1;
}

jint getDownloadApplicationSizeFromIndex(jmethodID id, va_list args)
{
    int i = va_arg(args, int);
    debugPrintf("getDownloadApplicationSizeFromIndex(%d)",i);
    if(i<downloadApplicationNum)
    {
        return index;
    }
    
    return -1;
}

jint return0(jmethodID id, va_list args)
{
    debugPrintf("return0()");
    return 0;
}

NameToMethodID nameToMethodId[] = {
    {0, "NULL", METHOD_TYPE_VOID, return0},                              
    {1, "Native_Exit", METHOD_TYPE_VOID, Native_Exit},                              // ()V
    {2, "vibrate", METHOD_TYPE_VOID, vibrate},                                      // ()V
    {3, "launchRuntime", METHOD_TYPE_VOID, launchRuntime},                          // (I)V
    {4, "appFinish", METHOD_TYPE_VOID, appFinish},                                  // (I)V
    {5, "updateResourceCheck", METHOD_TYPE_LONG, updateResourceCheck},              // ()J
    {6, "invisibleRuntimeLogo", METHOD_TYPE_INT, invisibleRuntimeLogo},             // ()I
    {7, "isUseRecord", METHOD_TYPE_VOID, isUseRecord},                              // (Ljava/lang/String;)V
    {8, "getCommonAppDir", METHOD_TYPE_OBJECT, getCommonAppDir},                    // ()Ljava/lang/String;
    {9, "jniTrackerSetPage", METHOD_TYPE_INT, jniTrackerSetPage},                   // ([BII)I
    {10, "getEnv", METHOD_TYPE_INT, getEnv},                                        // ([BII)I
    {11, "getCommonDir", METHOD_TYPE_INT, getCommonDir},                            // ([BII)I
    {12, "jniTrackerEvent", METHOD_TYPE_INT, jniTrackerEvent},                      // ([BII)I
    {13, "jniTrackerAssignAttribute", METHOD_TYPE_INT, jniTrackerAssignAttribute},  // ([BII)I
    {14, "jniTrackerRemoveAttribute", METHOD_TYPE_LONG, jniTrackerRemoveAttribute}, // ([BII)I
    {15, "getLocale", METHOD_TYPE_INT, getLocale},                                  // ([BII)I
    {16, "trialSetTime", METHOD_TYPE_INT, trialSetTime},                            // ([BII)I
    {17, "jniTrackerSetAttribute", METHOD_TYPE_VOID, jniTrackerSetAttribute},
    {18, "stereoIsAvailability", METHOD_TYPE_VOID, stereoIsAvailability},
    {19, "stereoSetView", METHOD_TYPE_VOID, stereoSetView},
    {20, "stereoGetDepth", METHOD_TYPE_VOID, stereoGetDepth},

    // Temp MyHttp fake hook
    {21, "com/acrodea/vividruntime/launcher/extension/MyHttp/<init>", METHOD_TYPE_OBJECT, MyHttp_constructor},
    {22, "init", METHOD_TYPE_INT, MyHttp_obj_init},
    {23, "setUserPtr", METHOD_TYPE_INT, MyHttp_obj_setUserPtr},
    {24, "connect", METHOD_TYPE_INT, MyHttp_obj_connect},
    {25, "destroy", METHOD_TYPE_INT, MyHttp_destroy},

    // MyClass
    {26, "getWifiState", METHOD_TYPE_INT, getWifiState},
    {27, "getCellState", METHOD_TYPE_INT, getCellState},
    {28, "getShareData", METHOD_TYPE_INT, getShareData},
    {29, "setPropertycv", METHOD_TYPE_INT, setPropertycv},
    {30, "show", METHOD_TYPE_INT, show},
    {31, "getText", METHOD_TYPE_INT, getText},
    {32, "getWidth", METHOD_TYPE_INT, getWidth},
    {33, "getFontInfo", METHOD_TYPE_INT, getFontInfo},
    {34, "getAutoLoginID", METHOD_TYPE_INT, getAutoLoginID},
    {35, "setProgress", METHOD_TYPE_INT, setProgress},
    {36, "getImei", METHOD_TYPE_INT, getImei},
    {37, "getImsi", METHOD_TYPE_INT, getImsi},
    {38, "getExtensonByName", METHOD_TYPE_INT, getExtensonByName},
    {39, "getHeightEx", METHOD_TYPE_INT, getHeightEx},
    {40, "drawString", METHOD_TYPE_INT, drawString},
    {41, "drawStringEx", METHOD_TYPE_INT, drawStringEx},
    {42, "setMessage", METHOD_TYPE_INT, setMessage},

    // My App
    {43, "playVibrateTimer", METHOD_TYPE_INT, playVibrateTimer},
    {44, "getDownloadApplicationTypeInfoFromIndex", METHOD_TYPE_INT, getDownloadApplicationTypeInfoFromIndex},
    {45, "getUninstallApplicationCount", METHOD_TYPE_INT, getUninstallApplicationCount},
    {46, "getDownloadApplicationInfoFromIndex", METHOD_TYPE_INT, getDownloadApplicationInfoFromIndex},
    {47, "applicationDownloadGetPauseStatus", METHOD_TYPE_INT, applicationDownloadGetPauseStatus},
    {48, "applicationDownloadGetNetowrkPauseStatus", METHOD_TYPE_INT, applicationDownloadGetNetowrkPauseStatus},
    {49, "getDownloadApplicationBodySizeFromIndex", METHOD_TYPE_INT, getDownloadApplicationBodySizeFromIndex},
    {50, "getDownloadApplicationSizeFromIndex", METHOD_TYPE_INT, getDownloadApplicationSizeFromIndex},
    {50, "getUninstallApplicationTypeInfoFromIndex", METHOD_TYPE_INT, getUninstallApplicationTypeInfoFromIndex},
};

MethodsBoolean methodsBoolean[] = {};
MethodsByte methodsByte[] = {};
MethodsChar methodsChar[] = {};
MethodsDouble methodsDouble[] = {

};
MethodsFloat methodsFloat[] = {
    {20, stereoGetDepth},
};
MethodsInt methodsInt[] = {};
MethodsLong methodsLong[] = {
    {100, GetCurrentTime},
};
MethodsObject methodsObject[] = {};
MethodsShort methodsShort[] = {};
MethodsVoid methodsVoid[] = {};

/*
 * JNI Fields
*/

// System-wide constant that applications sometimes request
// https://developer.android.com/reference/android/content/Context.html#WINDOW_SERVICE
char WINDOW_SERVICE[] = "window";

// System-wide constant that's often used to determine Android version
// https://developer.android.com/reference/android/os/Build.VERSION.html#SDK_INT
// Possible values: https://developer.android.com/reference/android/os/Build.VERSION_CODES
const int SDK_INT = 19;     // Android 4.4 / KitKat
const int descriptor = 110; // Android 4.4 / KitKat

const int mSurface = 0;       // Android 4.4 / KitKat
const int mNativeSurface = 0; // Android 4.4 / KitKat
const int mNativeObject = 0;  // Android 4.4 / KitKat

NameToFieldID nameToFieldId[] = {
    {0, "WINDOW_SERVICE", FIELD_TYPE_OBJECT},
    {1, "SDK_INT", FIELD_TYPE_INT},
    {2, "descriptor", FIELD_TYPE_INT},
    {3, "mSurface", FIELD_TYPE_INT},
    {4, "mNativeSurface", FIELD_TYPE_INT},
    {5, "mNativeObject", FIELD_TYPE_INT},
};

FieldsBoolean fieldsBoolean[] = {};
FieldsByte fieldsByte[] = {};
FieldsChar fieldsChar[] = {};
FieldsDouble fieldsDouble[] = {};
FieldsFloat fieldsFloat[] = {};
FieldsInt fieldsInt[] = {
    {1, SDK_INT},
    {2, descriptor},
    {3, mSurface},
    {4, mNativeSurface},
    {5, mNativeObject},
};
FieldsObject fieldsObject[] = {
    {0, WINDOW_SERVICE},
};
FieldsLong fieldsLong[] = {};
FieldsShort fieldsShort[] = {};

__FALSOJNI_IMPL_CONTAINER_SIZES
