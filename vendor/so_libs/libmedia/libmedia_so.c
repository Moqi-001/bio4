#include <so_common/so_logger.h>
#include <so_common/so_utils.h>
#include <so_linker/so_linker.h>

#include "AudioTrack.h"

int AAudioRecord_fake(void) {
    so_logd("[Error] Android AudioRecord function called: unsupport!");
    return 0;
}
// ==================== libmedia.so ====================
// Android 多媒体库
static so_symbol libmedia_symbols[] = {
    // ===== AudioRecord =====
    SO_SYMBOL_FUNC("_ZN7android11AudioRecordC1Ev", &AAudioRecord_fake),
    SO_SYMBOL_FUNC("_ZN7android11AudioRecordD1Ev", &AAudioRecord_fake),
    SO_SYMBOL_FUNC("_ZN7android11AudioRecord5startEv", &AAudioRecord_fake),
    SO_SYMBOL_FUNC("_ZN7android11AudioRecord4stopEv", &AAudioRecord_fake),
    SO_SYMBOL_FUNC("_ZN7android11AudioRecord3setE14audio_source_tj14audio_format_tjiPFviPvS3_ES3_ibiNS0_13transfer_typeE19audio_input_flags_t", &AAudioRecord_fake),
    SO_SYMBOL_FUNC("_ZN7android11AudioRecord3setE14audio_source_tj14audio_format_tjiNS0_12record_flagsEPFviPvS4_ES4_ibi", &AAudioRecord_fake),
    SO_SYMBOL_FUNC("_ZN7android11AudioRecord3setEijijijPFviPvS1_ES1_ibi", &AAudioRecord_fake),
    SO_SYMBOL_FUNC("_ZN7android11AudioRecord3setEijijijPFviPvS1_ES1_ib", &AAudioRecord_fake),

    // ===== AudioTrack =====
    SO_SYMBOL_FUNC("_ZN7android10AudioTrackC1Ev", &AAudioTrack_ctor),
    SO_SYMBOL_FUNC("_ZN7android10AudioTrackD1Ev", &AAudioTrack_dtor),
    SO_SYMBOL_FUNC("_ZNK7android10AudioTrack9initCheckEv", &AAudioTrack_initCheck),
    SO_SYMBOL_FUNC("_ZN7android10AudioTrack5startEv", &AAudioTrack_start),
    SO_SYMBOL_FUNC("_ZN7android10AudioTrack5pauseEv", &AAudioTrack_pause),
    SO_SYMBOL_FUNC("_ZN7android10AudioTrack4stopEv", &AAudioTrack_stop),
    SO_SYMBOL_FUNC("_ZN7android10AudioTrack9setVolumeEff", &AAudioTrack_setVolume),
    SO_SYMBOL_FUNC("_ZN7android10AudioTrack3setE19audio_stream_type_tj14audio_format_tji20audio_output_flags_tPFviPvS4_ES4_iRKNS_2spINS_7IMemoryEEEbi", &AAudioTrack_set),
};

so_library libmedia_so = {
    .soname      = "libmedia.so",
    .symbols     = libmedia_symbols,
    .num_symbols = sizeof(libmedia_symbols) / sizeof(libmedia_symbols[0]),
    .prepare     = NULL,
};
