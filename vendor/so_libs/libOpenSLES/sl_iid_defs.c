/*
 * OpenSL ES 接口 ID 常量定义（128 位 GUID）
 *
 * SLInterfaceID 是「指向 GUID 结构体的指针」，所以符号本身是一个指针变量；
 * 结构体字段顺序见 SLES/OpenSLES.h 的 struct SLInterfaceID_。
 * 每个 IID 用独立结构体，保证地址互不相同（接口分发按指针比对）。
 */
#include <SLES/OpenSLES.h>

static const struct SLInterfaceID_ guid_SL_IID_3DCOMMIT = { 0x4d66f9dd, 0x1a17, 0x11dc, (SLuint16)(0xB3 | (0x00 << 8)), { 0x00, 0x1D, 0xB3, 0x71, 0xD8, 0x00 } };
const SLInterfaceID SL_IID_3DCOMMIT = &guid_SL_IID_3DCOMMIT;

static const struct SLInterfaceID_ guid_SL_IID_3DDOPPLER = { 0x9e5b46e4, 0x1a17, 0x11dc, (SLuint16)(0xB3 | (0x00 << 8)), { 0x00, 0x1D, 0xB3, 0x71, 0xD8, 0x00 } };
const SLInterfaceID SL_IID_3DDOPPLER = &guid_SL_IID_3DDOPPLER;

static const struct SLInterfaceID_ guid_SL_IID_3DGROUPING = { 0x46ffbb7c, 0x1a17, 0x11dc, (SLuint16)(0xB3 | (0x00 << 8)), { 0x00, 0x1D, 0xB3, 0x71, 0xD8, 0x00 } };
const SLInterfaceID SL_IID_3DGROUPING = &guid_SL_IID_3DGROUPING;

static const struct SLInterfaceID_ guid_SL_IID_3DLOCATION = { 0xba002508, 0x1a17, 0x11dc, (SLuint16)(0xB3 | (0x00 << 8)), { 0x00, 0x1D, 0xB3, 0x71, 0xD8, 0x00 } };
const SLInterfaceID SL_IID_3DLOCATION = &guid_SL_IID_3DLOCATION;

static const struct SLInterfaceID_ guid_SL_IID_3DMACROSCOPIC = { 0x6d39c6f9, 0x1a17, 0x11dc, (SLuint16)(0xB3 | (0x00 << 8)), { 0x00, 0x1D, 0xB3, 0x71, 0xD8, 0x00 } };
const SLInterfaceID SL_IID_3DMACROSCOPIC = &guid_SL_IID_3DMACROSCOPIC;

static const struct SLInterfaceID_ guid_SL_IID_3DSOURCE = { 0x6db146c0, 0x1a17, 0x11dc, (SLuint16)(0xB3 | (0x00 << 8)), { 0x00, 0x1D, 0xB3, 0x71, 0xD8, 0x00 } };
const SLInterfaceID SL_IID_3DSOURCE = &guid_SL_IID_3DSOURCE;

static const struct SLInterfaceID_ guid_SL_IID_ANDROIDBUFFERQUEUESOURCE = { 0x788c34a8, 0x18aa, 0x11df, (SLuint16)(0x86 | (0x17 << 8)), { 0x00, 0x02, 0xA5, 0xD5, 0xC5, 0x1B } };
const SLInterfaceID SL_IID_ANDROIDBUFFERQUEUESOURCE = &guid_SL_IID_ANDROIDBUFFERQUEUESOURCE;

static const struct SLInterfaceID_ guid_SL_IID_ANDROIDCONFIGURATION = { 0xc2191c80, 0x5d55, 0x11df, (SLuint16)(0x9C | (0xC2 << 8)), { 0x00, 0x02, 0xA5, 0xD5, 0xC5, 0x1B } };
const SLInterfaceID SL_IID_ANDROIDCONFIGURATION = &guid_SL_IID_ANDROIDCONFIGURATION;

static const struct SLInterfaceID_ guid_SL_IID_ANDROIDEFFECT = { 0x3e74a4e0, 0x9c9c, 0x11df, (SLuint16)(0xA5 | (0xCD << 8)), { 0x00, 0x02, 0xA5, 0xD5, 0xC5, 0x1B } };
const SLInterfaceID SL_IID_ANDROIDEFFECT = &guid_SL_IID_ANDROIDEFFECT;

static const struct SLInterfaceID_ guid_SL_IID_ANDROIDEFFECTCAPABILITIES = { 0x2eb3c877, 0x9c9c, 0x11df, (SLuint16)(0xA9 | (0x73 << 8)), { 0x00, 0x02, 0xA5, 0xD5, 0xC5, 0x1B } };
const SLInterfaceID SL_IID_ANDROIDEFFECTCAPABILITIES = &guid_SL_IID_ANDROIDEFFECTCAPABILITIES;

static const struct SLInterfaceID_ guid_SL_IID_ANDROIDEFFECTSEND = { 0x9e5ad4f6, 0x9c9c, 0x11df, (SLuint16)(0x8C | (0x04 << 8)), { 0x00, 0x02, 0xA5, 0xD5, 0xC5, 0x1B } };
const SLInterfaceID SL_IID_ANDROIDEFFECTSEND = &guid_SL_IID_ANDROIDEFFECTSEND;

static const struct SLInterfaceID_ guid_SL_IID_ANDROIDSIMPLEBUFFERQUEUE = { 0xc9c4abe0, 0x8c93, 0x11df, (SLuint16)(0x9F | (0xCB << 8)), { 0x00, 0x02, 0xA5, 0xD5, 0xC5, 0x1B } };
const SLInterfaceID SL_IID_ANDROIDSIMPLEBUFFERQUEUE = &guid_SL_IID_ANDROIDSIMPLEBUFFERQUEUE;

static const struct SLInterfaceID_ guid_SL_IID_AUDIOENCODER = { 0xc32bf4c8, 0x1a17, 0x11dc, (SLuint16)(0xB3 | (0x00 << 8)), { 0x00, 0x1D, 0xB3, 0x71, 0xD8, 0x00 } };
const SLInterfaceID SL_IID_AUDIOENCODER = &guid_SL_IID_AUDIOENCODER;

static const struct SLInterfaceID_ guid_SL_IID_AUDIOENCODERCAPABILITIES = { 0xe07526e0, 0x1a17, 0x11dc, (SLuint16)(0xB3 | (0x00 << 8)), { 0x00, 0x1D, 0xB3, 0x71, 0xD8, 0x00 } };
const SLInterfaceID SL_IID_AUDIOENCODERCAPABILITIES = &guid_SL_IID_AUDIOENCODERCAPABILITIES;

static const struct SLInterfaceID_ guid_SL_IID_AUDIOIODEVICECAPABILITIES = { 0x6a6fc882, 0x1a17, 0x11dc, (SLuint16)(0xB3 | (0x00 << 8)), { 0x00, 0x1D, 0xB3, 0x71, 0xD8, 0x00 } };
const SLInterfaceID SL_IID_AUDIOIODEVICECAPABILITIES = &guid_SL_IID_AUDIOIODEVICECAPABILITIES;

static const struct SLInterfaceID_ guid_SL_IID_BASSBOOST = { 0x83811e60, 0x1a17, 0x11dc, (SLuint16)(0xB3 | (0x00 << 8)), { 0x00, 0x1D, 0xB3, 0x71, 0xD8, 0x00 } };
const SLInterfaceID SL_IID_BASSBOOST = &guid_SL_IID_BASSBOOST;

static const struct SLInterfaceID_ guid_SL_IID_BUFFERQUEUE = { 0x6b69fc06, 0x949e, 0x11d9, (SLuint16)(0x97 | (0x00 << 8)), { 0x00, 0x5F, 0xD4, 0xC5, 0xC9, 0xA1 } };
const SLInterfaceID SL_IID_BUFFERQUEUE = &guid_SL_IID_BUFFERQUEUE;

static const struct SLInterfaceID_ guid_SL_IID_DEVICEVOLUME = { 0x64fe56ba, 0x1a17, 0x11dc, (SLuint16)(0xB3 | (0x00 << 8)), { 0x00, 0x1D, 0xB3, 0x71, 0xD8, 0x00 } };
const SLInterfaceID SL_IID_DEVICEVOLUME = &guid_SL_IID_DEVICEVOLUME;

static const struct SLInterfaceID_ guid_SL_IID_DYNAMICINTERFACEMANAGEMENT = { 0x281ced76, 0x9594, 0x11d9, (SLuint16)(0x97 | (0x00 << 8)), { 0x00, 0x5F, 0xD4, 0xC5, 0xC9, 0xA1 } };
const SLInterfaceID SL_IID_DYNAMICINTERFACEMANAGEMENT = &guid_SL_IID_DYNAMICINTERFACEMANAGEMENT;

static const struct SLInterfaceID_ guid_SL_IID_DYNAMICSOURCE = { 0x2b49bd6e, 0x9595, 0x11d9, (SLuint16)(0x97 | (0x00 << 8)), { 0x00, 0x5F, 0xD4, 0xC5, 0xC9, 0xA1 } };
const SLInterfaceID SL_IID_DYNAMICSOURCE = &guid_SL_IID_DYNAMICSOURCE;

static const struct SLInterfaceID_ guid_SL_IID_EFFECTSEND = { 0x7b560fca, 0x1a17, 0x11dc, (SLuint16)(0xB3 | (0x00 << 8)), { 0x00, 0x1D, 0xB3, 0x71, 0xD8, 0x00 } };
const SLInterfaceID SL_IID_EFFECTSEND = &guid_SL_IID_EFFECTSEND;

static const struct SLInterfaceID_ guid_SL_IID_ENGINE = { 0x8d97c260, 0x1a16, 0x11dc, (SLuint16)(0xB3 | (0x00 << 8)), { 0x00, 0x1D, 0xB3, 0x71, 0xD8, 0x00 } };
const SLInterfaceID SL_IID_ENGINE = &guid_SL_IID_ENGINE;

static const struct SLInterfaceID_ guid_SL_IID_ENGINECAPABILITIES = { 0x5bc4a9b8, 0x1a17, 0x11dc, (SLuint16)(0xB3 | (0x00 << 8)), { 0x00, 0x1D, 0xB3, 0x71, 0xD8, 0x00 } };
const SLInterfaceID SL_IID_ENGINECAPABILITIES = &guid_SL_IID_ENGINECAPABILITIES;

static const struct SLInterfaceID_ guid_SL_IID_ENVIRONMENTALREVERB = { 0xe3da5c39, 0x1a17, 0x11dc, (SLuint16)(0xB3 | (0x00 << 8)), { 0x00, 0x1D, 0xB3, 0x71, 0xD8, 0x00 } };
const SLInterfaceID SL_IID_ENVIRONMENTALREVERB = &guid_SL_IID_ENVIRONMENTALREVERB;

static const struct SLInterfaceID_ guid_SL_IID_EQUALIZER = { 0x7d52d160, 0x1a17, 0x11dc, (SLuint16)(0xB3 | (0x00 << 8)), { 0x00, 0x1D, 0xB3, 0x71, 0xD8, 0x00 } };
const SLInterfaceID SL_IID_EQUALIZER = &guid_SL_IID_EQUALIZER;

static const struct SLInterfaceID_ guid_SL_IID_LED = { 0x3e45a4e0, 0x1a17, 0x11dc, (SLuint16)(0xB3 | (0x00 << 8)), { 0x00, 0x1D, 0xB3, 0x71, 0xD8, 0x00 } };
const SLInterfaceID SL_IID_LED = &guid_SL_IID_LED;

static const struct SLInterfaceID_ guid_SL_IID_METADATAEXTRACTION = { 0x6f05c6c8, 0x1a17, 0x11dc, (SLuint16)(0xB3 | (0x00 << 8)), { 0x00, 0x1D, 0xB3, 0x71, 0xD8, 0x00 } };
const SLInterfaceID SL_IID_METADATAEXTRACTION = &guid_SL_IID_METADATAEXTRACTION;

static const struct SLInterfaceID_ guid_SL_IID_METADATATRAVERSAL = { 0x214d7c68, 0x1a17, 0x11dc, (SLuint16)(0xB3 | (0x00 << 8)), { 0x00, 0x1D, 0xB3, 0x71, 0xD8, 0x00 } };
const SLInterfaceID SL_IID_METADATATRAVERSAL = &guid_SL_IID_METADATATRAVERSAL;

static const struct SLInterfaceID_ guid_SL_IID_MIDIMESSAGES = { 0x60b7cd01, 0x1a17, 0x11dc, (SLuint16)(0xB3 | (0x00 << 8)), { 0x00, 0x1D, 0xB3, 0x71, 0xD8, 0x00 } };
const SLInterfaceID SL_IID_MIDIMESSAGES = &guid_SL_IID_MIDIMESSAGES;

static const struct SLInterfaceID_ guid_SL_IID_MIDISOUNDS = { 0xd5304dbe, 0x1a17, 0x11dc, (SLuint16)(0xB3 | (0x00 << 8)), { 0x00, 0x1D, 0xB3, 0x71, 0xD8, 0x00 } };
const SLInterfaceID SL_IID_MIDISOUNDS = &guid_SL_IID_MIDISOUNDS;

static const struct SLInterfaceID_ guid_SL_IID_MIDITEMPO = { 0x1c5d0b46, 0x1a17, 0x11dc, (SLuint16)(0xB3 | (0x00 << 8)), { 0x00, 0x1D, 0xB3, 0x71, 0xD8, 0x00 } };
const SLInterfaceID SL_IID_MIDITEMPO = &guid_SL_IID_MIDITEMPO;

static const struct SLInterfaceID_ guid_SL_IID_MUTESOLO = { 0x2c454582, 0x1a17, 0x11dc, (SLuint16)(0xB3 | (0x00 << 8)), { 0x00, 0x1D, 0xB3, 0x71, 0xD8, 0x00 } };
const SLInterfaceID SL_IID_MUTESOLO = &guid_SL_IID_MUTESOLO;

static const struct SLInterfaceID_ guid_SL_IID_OBJECT = { 0x6b68fc04, 0x949e, 0x11d9, (SLuint16)(0x97 | (0x00 << 8)), { 0x00, 0x5F, 0xD4, 0xC5, 0xC9, 0xA1 } };
const SLInterfaceID SL_IID_OBJECT = &guid_SL_IID_OBJECT;

static const struct SLInterfaceID_ guid_SL_IID_OUTPUTMIX = { 0x4fe1b6ad, 0x1a16, 0x11dc, (SLuint16)(0xB3 | (0x00 << 8)), { 0x00, 0x1D, 0xB3, 0x71, 0xD8, 0x00 } };
const SLInterfaceID SL_IID_OUTPUTMIX = &guid_SL_IID_OUTPUTMIX;

static const struct SLInterfaceID_ guid_SL_IID_PLAY = { 0x3824e6d1, 0x1a16, 0x11dc, (SLuint16)(0xB3 | (0x00 << 8)), { 0x00, 0x1D, 0xB3, 0x71, 0xD8, 0x00 } };
const SLInterfaceID SL_IID_PLAY = &guid_SL_IID_PLAY;

static const struct SLInterfaceID_ guid_SL_IID_PLAYBACKRATE = { 0x5bc4a9b8, 0x1a17, 0x11dc, (SLuint16)(0xB3 | (0x00 << 8)), { 0x00, 0x1D, 0xB3, 0x71, 0xD8, 0x00 } };
const SLInterfaceID SL_IID_PLAYBACKRATE = &guid_SL_IID_PLAYBACKRATE;

static const struct SLInterfaceID_ guid_SL_IID_PREFETCHSTATUS = { 0x8e35efca, 0x1a17, 0x11dc, (SLuint16)(0xB3 | (0x00 << 8)), { 0x00, 0x1D, 0xB3, 0x71, 0xD8, 0x00 } };
const SLInterfaceID SL_IID_PREFETCHSTATUS = &guid_SL_IID_PREFETCHSTATUS;

static const struct SLInterfaceID_ guid_SL_IID_RATEPITCH = { 0x7ec57198, 0x1a17, 0x11dc, (SLuint16)(0xB3 | (0x00 << 8)), { 0x00, 0x1D, 0xB3, 0x71, 0xD8, 0x00 } };
const SLInterfaceID SL_IID_RATEPITCH = &guid_SL_IID_RATEPITCH;

static const struct SLInterfaceID_ guid_SL_IID_RECORD = { 0x3824e6d1, 0x1a16, 0x11dc, (SLuint16)(0xB3 | (0x00 << 8)), { 0x00, 0x1D, 0xB3, 0x71, 0xD8, 0x00 } };
const SLInterfaceID SL_IID_RECORD = &guid_SL_IID_RECORD;

static const struct SLInterfaceID_ guid_SL_IID_REVERB = { 0x83811e60, 0x1a17, 0x11dc, (SLuint16)(0xB3 | (0x00 << 8)), { 0x00, 0x1D, 0xB3, 0x71, 0xD8, 0x00 } };
const SLInterfaceID SL_IID_REVERB = &guid_SL_IID_REVERB;

static const struct SLInterfaceID_ guid_SL_IID_SEEK = { 0x3d4843c7, 0x1a16, 0x11dc, (SLuint16)(0xB3 | (0x00 << 8)), { 0x00, 0x1D, 0xB3, 0x71, 0xD8, 0x00 } };
const SLInterfaceID SL_IID_SEEK = &guid_SL_IID_SEEK;

static const struct SLInterfaceID_ guid_SL_IID_VIBRA = { 0x25650a40, 0x1a17, 0x11dc, (SLuint16)(0xB3 | (0x00 << 8)), { 0x00, 0x1D, 0xB3, 0x71, 0xD8, 0x00 } };
const SLInterfaceID SL_IID_VIBRA = &guid_SL_IID_VIBRA;

static const struct SLInterfaceID_ guid_SL_IID_VIRTUALIZER = { 0xae17e308, 0x1a17, 0x11dc, (SLuint16)(0xB3 | (0x00 << 8)), { 0x00, 0x1D, 0xB3, 0x71, 0xD8, 0x00 } };
const SLInterfaceID SL_IID_VIRTUALIZER = &guid_SL_IID_VIRTUALIZER;

static const struct SLInterfaceID_ guid_SL_IID_VISUALIZATION = { 0xba002508, 0x1a17, 0x11dc, (SLuint16)(0xB3 | (0x00 << 8)), { 0x00, 0x1D, 0xB3, 0x71, 0xD8, 0x00 } };
const SLInterfaceID SL_IID_VISUALIZATION = &guid_SL_IID_VISUALIZATION;

static const struct SLInterfaceID_ guid_SL_IID_AUDIODECODERCAPABILITIES = { 0x6a6fc882, 0x1a17, 0x11dc, (SLuint16)(0xb3 | (0x00 << 8)), { 0x1d, 0xb3, 0x71, 0xd8, 0x00, 0x00 } };
const SLInterfaceID SL_IID_AUDIODECODERCAPABILITIES = &guid_SL_IID_AUDIODECODERCAPABILITIES;

static const struct SLInterfaceID_ guid_SL_IID_MIDIMESSAGE = { 0x60b7cd01, 0x1a17, 0x11dc, (SLuint16)(0xb3 | (0x00 << 8)), { 0x1d, 0xb3, 0x71, 0xd8, 0x00, 0x00 } };
const SLInterfaceID SL_IID_MIDIMESSAGE = &guid_SL_IID_MIDIMESSAGE;

static const struct SLInterfaceID_ guid_SL_IID_MIDIMUTESOLO = { 0x2c454582, 0x1a17, 0x11dc, (SLuint16)(0xb3 | (0x00 << 8)), { 0x1d, 0xb3, 0x71, 0xd8, 0x00, 0x00 } };
const SLInterfaceID SL_IID_MIDIMUTESOLO = &guid_SL_IID_MIDIMUTESOLO;

static const struct SLInterfaceID_ guid_SL_IID_MIDITIME = { 0x1c5d0b46, 0x1a17, 0x11dc, (SLuint16)(0xb3 | (0x00 << 8)), { 0x1d, 0xb3, 0x71, 0xd8, 0x00, 0x00 } };
const SLInterfaceID SL_IID_MIDITIME = &guid_SL_IID_MIDITIME;

static const struct SLInterfaceID_ guid_SL_IID_NULL = { 0x00000000, 0x0000, 0x0000, (SLuint16)(0x00 | (0x00 << 8)), { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
const SLInterfaceID SL_IID_NULL = &guid_SL_IID_NULL;

static const struct SLInterfaceID_ guid_SL_IID_PITCH = { 0x7ec57198, 0x1a17, 0x11dc, (SLuint16)(0xb3 | (0x00 << 8)), { 0x1d, 0xb3, 0x71, 0xd8, 0x00, 0x00 } };
const SLInterfaceID SL_IID_PITCH = &guid_SL_IID_PITCH;

static const struct SLInterfaceID_ guid_SL_IID_PRESETREVERB = { 0x1427cd88, 0x1a17, 0x11dc, (SLuint16)(0xb3 | (0x00 << 8)), { 0x1d, 0xb3, 0x71, 0xd8, 0x00, 0x00 } };
const SLInterfaceID SL_IID_PRESETREVERB = &guid_SL_IID_PRESETREVERB;

static const struct SLInterfaceID_ guid_SL_IID_THREADSYNC = { 0x281ced76, 0x9594, 0x11d9, (SLuint16)(0x97 | (0x00 << 8)), { 0x5f, 0xd4, 0xc5, 0xc9, 0xa1, 0x00 } };
const SLInterfaceID SL_IID_THREADSYNC = &guid_SL_IID_THREADSYNC;

static const struct SLInterfaceID_ guid_SL_IID_VOLUME = { 0x64fe56ba, 0x1a17, 0x11dc, (SLuint16)(0xb3 | (0x00 << 8)), { 0x1d, 0xb3, 0x71, 0xd8, 0x00, 0x00 } };
const SLInterfaceID SL_IID_VOLUME = &guid_SL_IID_VOLUME;
