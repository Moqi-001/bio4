#include <stdlib.h>
#include <string.h>

#include <so_common/so_logger.h>

#include "configuration.h"

#define VITA_SCREEN_WIDTH  960
#define VITA_SCREEN_HEIGHT 544
#define VITA_DENSITY_DPI   220  // 约 220 DPI
#define VITA_SDK_VERSION   19   // Android 4.4

struct AConfiguration {
    uint16_t mcc;
    uint16_t mnc;
    char     language[2];
    char     country[2];
    uint8_t  orientation;
    uint8_t  touchscreen;
    uint16_t density;
    uint8_t  keyboard;
    uint8_t  navigation;
    uint8_t  keysHidden;
    uint8_t  navHidden;
    uint16_t sdkVersion;
    uint16_t minorVersion;
    uint16_t screenWidth;
    uint16_t screenHeight;
    uint8_t  screenSize;
    uint8_t  screenLong;
    uint8_t  uiModeType;
    uint8_t  uiModeNight;
    uint16_t screenWidthDp;
    uint16_t screenHeightDp;
    uint16_t smallestScreenWidthDp;
    uint8_t  layoutDirection;
};

AConfiguration *AConfiguration_new() {
    AConfiguration *config = malloc(sizeof(AConfiguration));
    memset(config, 0, sizeof(AConfiguration));
    so_logd("AConfiguration_new(): %p", config);
    return config;
}

void AConfiguration_delete(AConfiguration *config) {
    so_logd("AConfiguration_delete(%p)", config);
    free(config);
}

void AConfiguration_fromAssetManager(AConfiguration *out, AAssetManager *am) {
    out->mcc = 0;  // 移动国家码
    out->mnc = 0;  // 移动网络码

    out->language[0] = 'e';
    out->language[1] = 'n';
    out->country[0]  = 'U';
    out->country[1]  = 'S';

    out->orientation  = ACONFIGURATION_ORIENTATION_LAND;    // 屏幕方向 (横屏)
    out->screenSize   = ACONFIGURATION_SCREENSIZE_NORMAL;   // 屏幕尺寸 (标准: 4-6英寸)
    out->screenLong   = ACONFIGURATION_SCREENLONG_YES;      // 是否长屏 (是)
    out->density      = VITA_DENSITY_DPI;                   // 像素密度 (220 dpi)
    out->touchscreen  = ACONFIGURATION_TOUCHSCREEN_FINGER;  // 触屏类型 (手指)
    out->keyboard     = ACONFIGURATION_KEYBOARD_NOKEYS;     // 物理键盘 (无)
    out->navigation   = ACONFIGURATION_NAVIGATION_DPAD;     // 非触摸导航方式 (方向键)
    out->keysHidden   = ACONFIGURATION_KEYSHIDDEN_YES;      // 键盘是否隐藏 (是)
    out->navHidden    = ACONFIGURATION_NAVHIDDEN_NO;        // 导航设备是否隐藏 (否)
    out->sdkVersion   = VITA_SDK_VERSION;                   // Android 4.4 API 19
    out->minorVersion = 0;
    out->uiModeType   = ACONFIGURATION_UI_MODE_TYPE_NORMAL;  // UI 类型 (标准应用)
    out->uiModeNight  = ACONFIGURATION_UI_MODE_NIGHT_NO;     // 夜间模式 (否)

    // DP = 像素 / (DPI / 160)
    out->screenWidthDp         = (uint16_t)(VITA_SCREEN_WIDTH * 160 / VITA_DENSITY_DPI);   // 屏幕宽度DP ≈ 698
    out->screenHeightDp        = (uint16_t)(VITA_SCREEN_HEIGHT * 160 / VITA_DENSITY_DPI);  // 屏幕高度DP ≈ 395
    out->smallestScreenWidthDp = (uint16_t)(VITA_SCREEN_HEIGHT * 160 / VITA_DENSITY_DPI);  // 最小屏幕宽度DP ≈ 395

    out->layoutDirection = ACONFIGURATION_LAYOUTDIR_LTR;  // 布局方向 (从左到右)

    so_logd("AConfiguration_fromAssetManager(%p,%p)", out, am);
}

void AConfiguration_copy(AConfiguration *dest, AConfiguration *src) {
    *dest = *src;
    so_logd("AConfiguration_copy(%p,%p)", dest, src);
}

int32_t AConfiguration_getMcc(AConfiguration *config) {
    so_logd("AConfiguration_getMcc(%p): %d", config, config->mcc);
    return config->mcc;
}

int32_t AConfiguration_getMnc(AConfiguration *config) {
    so_logd("AConfiguration_getMnc(%p): %d", config, config->mnc);
    return config->mnc;
}

void AConfiguration_getLanguage(AConfiguration *config, char *outLanguage) {
    outLanguage[0] = config->language[0];
    outLanguage[1] = config->language[1];
    so_logd("AConfiguration_getLanguage(%p): %s", config, outLanguage);
}

void AConfiguration_getCountry(AConfiguration *config, char *outCountry) {
    outCountry[0] = config->country[0];
    outCountry[1] = config->country[1];
    so_logd("AConfiguration_getCountry(%p): %s", config, outCountry);
}

int32_t AConfiguration_getOrientation(AConfiguration *config) {
    so_logd("AConfiguration_getOrientation(%p): %d", config, config->orientation);
    return config->orientation;
}

int32_t AConfiguration_getTouchscreen(AConfiguration *config) {
    so_logd("AConfiguration_getTouchscreen(%p): %d", config, config->touchscreen);
    return config->touchscreen;
}

int32_t AConfiguration_getDensity(AConfiguration *config) {
    so_logd("AConfiguration_getDensity(%p): %d", config, config->density);
    return config->density;
}

int32_t AConfiguration_getKeyboard(AConfiguration *config) {
    so_logd("AConfiguration_getKeyboard(%p): %d", config, config->keyboard);
    return config->keyboard;
}

int32_t AConfiguration_getNavigation(AConfiguration *config) {
    so_logd("AConfiguration_getNavigation(%p): %d", config, config->navigation);
    return config->navigation;
}

int32_t AConfiguration_getKeysHidden(AConfiguration *config) {
    so_logd("AConfiguration_getKeysHidden(%p): %d", config, config->keysHidden);
    return config->keysHidden;
}

int32_t AConfiguration_getNavHidden(AConfiguration *config) {
    so_logd("AConfiguration_getNavHidden(%p): %d", config, config->navHidden);
    return config->navHidden;
}

int32_t AConfiguration_getSdkVersion(AConfiguration *config) {
    so_logd("AConfiguration_getSdkVersion(%p): %d", config, config->sdkVersion);
    return config->sdkVersion;
}

int32_t AConfiguration_getScreenSize(AConfiguration *config) {
    so_logd("AConfiguration_getScreenSize(%p): %d", config, config->screenSize);
    return config->screenSize;
}

int32_t AConfiguration_getScreenLong(AConfiguration *config) {
    so_logd("AConfiguration_getScreenLong(%p): %d", config, config->screenLong);
    return config->screenLong;
}

int32_t AConfiguration_getUiModeType(AConfiguration *config) {
    so_logd("AConfiguration_getUiModeType(%p): %d", config, config->uiModeType);
    return config->uiModeType;
}

int32_t AConfiguration_getUiModeNight(AConfiguration *config) {
    so_logd("AConfiguration_getUiModeNight(%p): %d", config, config->uiModeNight);
    return config->uiModeNight;
}

int32_t AConfiguration_getScreenWidthDp(AConfiguration *config) {
    so_logd("AConfiguration_getScreenWidthDp(%p): %d", config, config->screenWidthDp);
    return config->screenWidthDp;
}

int32_t AConfiguration_getScreenHeightDp(AConfiguration *config) {
    so_logd("AConfiguration_getScreenHeightDp(%p): %d", config, config->screenHeightDp);
    return config->screenHeightDp;
}

int32_t AConfiguration_getSmallestScreenWidthDp(AConfiguration *config) {
    so_logd("AConfiguration_getSmallestScreenWidthDp(%p): %d", config, config->smallestScreenWidthDp);
    return config->smallestScreenWidthDp;
}

int32_t AConfiguration_getLayoutDirection(AConfiguration *config) {
    so_logd("AConfiguration_getLayoutDirection(%p): %d", config, config->layoutDirection);
    return config->layoutDirection;
}

// ----------------------------------------------------------------------

void AConfiguration_setMcc(AConfiguration *config, int32_t mcc) {
    config->mcc = mcc;
    so_logd("AConfiguration_setMcc(%p,%d)", config, mcc);
}

void AConfiguration_setMnc(AConfiguration *config, int32_t mnc) {
    config->mnc = mnc;
    so_logd("AConfiguration_setMnc(%p,%d)", config, mnc);
}

void AConfiguration_setLanguage(AConfiguration *config, const char *language) {
    config->language[0] = language[0];
    config->language[1] = language[1];
    so_logd("AConfiguration_setLanguage(%p,%s)", config, language);
}

void AConfiguration_setCountry(AConfiguration *config, const char *country) {
    config->country[0] = country[0];
    config->country[1] = country[1];
    so_logd("AConfiguration_setCountry(%p,%s)", config, country);
}

void AConfiguration_setOrientation(AConfiguration *config, int32_t orientation) {
    config->orientation = orientation;
    so_logd("AConfiguration_setOrientation(%p,%d)", config, orientation);
}

void AConfiguration_setTouchscreen(AConfiguration *config, int32_t touchscreen) {
    config->touchscreen = touchscreen;
    so_logd("AConfiguration_setTouchscreen(%p,%d)", config, touchscreen);
}

void AConfiguration_setDensity(AConfiguration *config, int32_t density) {
    config->density = density;
    so_logd("AConfiguration_setDensity(%p,%d)", config, density);
}

void AConfiguration_setKeyboard(AConfiguration *config, int32_t keyboard) {
    so_logd("AConfiguration_setKeyboard(%p,%d)", config, keyboard);
    config->keyboard = keyboard;
}

void AConfiguration_setNavigation(AConfiguration *config, int32_t navigation) {
    config->navigation = navigation;
    so_logd("AConfiguration_setNavigation(%p,%d)", config, navigation);
}

void AConfiguration_setKeysHidden(AConfiguration *config, int32_t keysHidden) {
    config->keysHidden = keysHidden;
    so_logd("AConfiguration_setKeysHidden(%p,%d)", config, keysHidden);
}

void AConfiguration_setNavHidden(AConfiguration *config, int32_t navHidden) {
    config->navHidden = navHidden;
    so_logd("AConfiguration_setNavHidden(%p,%d)", config, navHidden);
}

void AConfiguration_setSdkVersion(AConfiguration *config, int32_t sdkVersion) {
    config->sdkVersion = sdkVersion;
    so_logd("AConfiguration_setSdkVersion(%p,%d)", config, sdkVersion);
}

void AConfiguration_setScreenSize(AConfiguration *config, int32_t screenSize) {
    config->screenSize = screenSize;
    so_logd("AConfiguration_setScreenSize(%p,%d)", config, screenSize);
}

void AConfiguration_setScreenLong(AConfiguration *config, int32_t screenLong) {
    config->screenLong = screenLong;
    so_logd("AConfiguration_setScreenLong(%p,%d)", config, screenLong);
}

void AConfiguration_setUiModeType(AConfiguration *config, int32_t uiModeType) {
    config->uiModeType = uiModeType;
    so_logd("AConfiguration_setUiModeType(%p,%d)", config, uiModeType);
}

void AConfiguration_setUiModeNight(AConfiguration *config, int32_t uiModeNight) {
    config->uiModeNight = uiModeNight;
    so_logd("AConfiguration_setUiModeNight(%p,%d)", config, uiModeNight);
}

void AConfiguration_setScreenWidthDp(AConfiguration *config, int32_t value) {
    config->screenWidthDp = value;
    so_logd("AConfiguration_setScreenWidthDp(%p,%d)", config, value);
}

void AConfiguration_setScreenHeightDp(AConfiguration *config, int32_t value) {
    config->screenHeightDp = value;
    so_logd("AConfiguration_setScreenHeightDp(%p,%d)", config, value);
}

void AConfiguration_setSmallestScreenWidthDp(AConfiguration *config, int32_t value) {
    config->smallestScreenWidthDp = value;
    so_logd("AConfiguration_setSmallestScreenWidthDp(%p,%d)", config, value);
}

void AConfiguration_setLayoutDirection(AConfiguration *config, int32_t value) {
    config->layoutDirection = value;
    so_logd("AConfiguration_setLayoutDirection(%p,%d)", config, value);
}

// ----------------------------------------------------------------------

int32_t AConfiguration_diff(AConfiguration *config1, AConfiguration *config2) {
    int32_t result = 0;
    so_logd("AConfiguration_diff(%p,%p): %d", config1, config2, result);
    return result;
}

int32_t AConfiguration_match(AConfiguration *base, AConfiguration *requested) {
    int32_t result = 0;
    so_logd("AConfiguration_match(%p,%p): %d", base, requested, result);
    return result;
}

int32_t AConfiguration_isBetterThan(AConfiguration *base, AConfiguration *test,
                                    AConfiguration *requested) {
    int32_t result = 0;
    so_logd("AConfiguration_isBetterThan(%p,%p,%p): %d", base, test, requested, result);
    return result;
}
