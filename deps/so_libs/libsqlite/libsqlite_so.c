#include <so_common/so_logger.h>
#include <so_common/so_utils.h>
#include <so_linker/so_linker.h>
#include <sqlite3.h>

// ==================== libsqlite.so ====================
// SQLite 数据库库
static so_symbol libsqlite_symbols[] = {
    SO_SYMBOL_FUNC("sqlite3_open", &sqlite3_open),
    SO_SYMBOL_FUNC("sqlite3_close", &sqlite3_close),
    SO_SYMBOL_FUNC("sqlite3_prepare_v2", &sqlite3_prepare_v2),
    SO_SYMBOL_FUNC("sqlite3_exec", &sqlite3_exec),
    SO_SYMBOL_FUNC("sqlite3_errmsg", &sqlite3_errmsg),
    SO_SYMBOL_FUNC("sqlite3_finalize", &sqlite3_finalize),
    SO_SYMBOL_FUNC("sqlite3_step", &sqlite3_step),
    SO_SYMBOL_FUNC("sqlite3_column_count", &sqlite3_column_count),
    SO_SYMBOL_FUNC("sqlite3_column_name", &sqlite3_column_name),
    SO_SYMBOL_FUNC("sqlite3_column_type", &sqlite3_column_type),
    SO_SYMBOL_FUNC("sqlite3_column_int", &sqlite3_column_int),
    SO_SYMBOL_FUNC("sqlite3_column_double", &sqlite3_column_double),
    SO_SYMBOL_FUNC("sqlite3_column_text", &sqlite3_column_text),
};

so_library libsqlite_so = {
    .soname      = "libsqlite.so",
    .symbols     = libsqlite_symbols,
    .num_symbols = sizeof(libsqlite_symbols) / sizeof(libsqlite_symbols[0]),
    .prepare     = NULL,
};
