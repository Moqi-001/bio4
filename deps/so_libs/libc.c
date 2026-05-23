#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef __POSIX_VISIBLE
#define __POSIX_VISIBLE 999999
#endif

#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <libgen.h>
#include <locale.h>
#include <malloc.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <pthread.h>
#include <sched.h>
#include <semaphore.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <utime.h>
#include <wchar.h>
#include <wctype.h>

#include <sys/resource.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/unistd.h>

#include <psp2/kernel/clib.h>
#include <so_linker/so_linker.h>
#include <so_common/so_logger.h>
#include <so_common/so_utils.h>

#include "reimpl/errno.h"
#include "reimpl/io.h"
#include "reimpl/mem.h"
#include "reimpl/pthr.h"
#include "reimpl/sys.h"

// ===== ARM C ABI (aeabi) =====
extern void               __aeabi_atexit(void *object, void (*destructor)(void *), void *dso_handle);
extern double             __aeabi_d2f(float f);
extern long long          __aeabi_d2lz(double d);
extern int                __aeabi_d2iz(double d);
extern unsigned int       __aeabi_d2uiz(double d);
extern unsigned long long __aeabi_d2ulz(double d);
extern double             __aeabi_dadd(double a, double b);
extern int                __aeabi_dcmpeq(double a, double b);
extern int                __aeabi_dcmpge(double a, double b);
extern int                __aeabi_dcmpgt(double a, double b);
extern int                __aeabi_dcmple(double a, double b);
extern int                __aeabi_dcmplt(double a, double b);
extern double             __aeabi_ddiv(double dividend, double divisor);
extern double             __aeabi_dmul(double a, double b);
extern double             __aeabi_dsub(double a, double b);
extern float              __aeabi_f2d(double d);
extern int                __aeabi_f2iz(float f);
extern long long          __aeabi_f2lz(float f);
extern unsigned long long __aeabi_f2ulz(float f);
extern float              __aeabi_fadd(float a, float b);
extern int                __aeabi_fcmpeq(float a, float b);
extern int                __aeabi_fcmpge(float a, float b);
extern int                __aeabi_fcmpgt(float a, float b);
extern int                __aeabi_fcmple(float a, float b);
extern int                __aeabi_fcmplt(float a, float b);
extern int                __aeabi_fcmpun(float a, float b);
extern float              __aeabi_fdiv(float dividend, float divisor);
extern float              __aeabi_fmul(float a, float b);
extern float              __aeabi_fsub(float a, float b);
extern float              __aeabi_i2f(int i);
extern double             __aeabi_i2d(int i);
extern int                __aeabi_idiv(int dividend, int divisor);
extern long long          __aeabi_idivmod(int dividend, int divisor);
extern double             __aeabi_l2d(long long l);
extern float              __aeabi_l2f(long long l);
extern long long          __aeabi_ldivmod(long long dividend, long long divisor);
extern long long          __aeabi_lmul(long long a, long long b);
extern long long          __aeabi_llsl(long long value, int shift);
extern void               __aeabi_memclr(void *dest, size_t n);
extern void               __aeabi_memclr4(void *dest, size_t n);
extern void               __aeabi_memclr8(void *dest, size_t n);
extern void               __aeabi_memcpy(void *dest, const void *src, size_t n);
extern void               __aeabi_memcpy4(void *dest, const void *src, size_t n);
extern void               __aeabi_memcpy8(void *dest, const void *src, size_t n);
extern void               __aeabi_memmove(void *dest, const void *src, size_t n);
extern void               __aeabi_memset4(void *dest, size_t n, int c);
extern void               __aeabi_memset8(void *dest, size_t n, int c);
extern void               __aeabi_memset(void *dest, size_t n, int c);
extern double             __aeabi_ui2d(unsigned int u);
extern float              __aeabi_ui2f(unsigned int u);
extern unsigned int       __aeabi_uidiv(unsigned int dividend, unsigned int divisor);
extern unsigned long long __aeabi_uidivmod(unsigned int dividend, unsigned int divisor);
extern double             __aeabi_ul2d(unsigned long long u);
extern float              __aeabi_ul2f(unsigned long long u);
extern unsigned long long __aeabi_uldivmod(unsigned long long dividend, unsigned long long divisor);

// ===== ARM C++ Unwinding =====
extern void __aeabi_unwind_cpp_pr0(void);
extern void __aeabi_unwind_cpp_pr1(void);

// ===== ARM C++ Unwinding Extensions =====
extern void *__gnu_Unwind_Find_exidx(void *pc, int *pcount);
extern void  __gnu_unwind_frame(void *exception_obj, void *context);

// ===== C++ ABI Support =====
extern int  __cxa_atexit(void (*destructor)(void *), void *arg, void *__dso_handle);
extern void __cxa_finalize(void *__dso_handle);

// ===== C++ Runtime Support =====
extern void *__dso_handle;

// ===== Stack Protector =====
extern uintptr_t __stack_chk_guard;

// ===== Assert Internals =====
extern void __assert(const char *expr, const char *file, unsigned int line, const char *func);

// ===== Stdio Internals =====
static FILE __sF_fake[3];
extern int  __srget(FILE *fp);
extern int  __swbuf(int c, FILE *fp);

// ===== Ctype Data Tables =====
extern const char  *BIONIC_ctype_;
extern const short *BIONIC_tolower_tab_;
extern const short *BIONIC_toupper_tab_;

// ===== System Info =====
const unsigned int __page_size = PAGE_SIZE;

char *mktemp_soloader(char *template) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    char *result = mktemp(template);
#pragma GCC diagnostic pop
    return result;
}

int getnameinfo(const struct sockaddr *sa, socklen_t salen,
                char *host, socklen_t hostlen,
                char *serv, socklen_t servlen, int flags) {
    if (host && hostlen > 0) host[0] = '\0';
    if (serv && servlen > 0) serv[0] = '\0';
    return EAI_SYSTEM;
}

int uname_soloader(void *buf) {
    strcpy(buf + 195, "1.0");
    return 0;
}

int select_soloader(int n, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout) {
    so_logd("select: called");
    return 0;
}

// ==================== libc.so ====================
static so_symbol libc_symbols[] = {
    // ===== ARM C ABI (aeabi) =====
    SO_SYMBOL_FUNC("__aeabi_atexit", &__aeabi_atexit),
    SO_SYMBOL_FUNC("__aeabi_d2f", &__aeabi_d2f),
    SO_SYMBOL_FUNC("__aeabi_d2lz", &__aeabi_d2lz),
    SO_SYMBOL_FUNC("__aeabi_d2iz", &__aeabi_d2iz),
    SO_SYMBOL_FUNC("__aeabi_d2uiz", &__aeabi_d2uiz),
    SO_SYMBOL_FUNC("__aeabi_d2ulz", &__aeabi_d2ulz),
    SO_SYMBOL_FUNC("__aeabi_dadd", &__aeabi_dadd),
    SO_SYMBOL_FUNC("__aeabi_dcmpeq", &__aeabi_dcmpeq),
    SO_SYMBOL_FUNC("__aeabi_dcmpge", &__aeabi_dcmpge),
    SO_SYMBOL_FUNC("__aeabi_dcmpgt", &__aeabi_dcmpgt),
    SO_SYMBOL_FUNC("__aeabi_dcmple", &__aeabi_dcmple),
    SO_SYMBOL_FUNC("__aeabi_dcmplt", &__aeabi_dcmplt),
    SO_SYMBOL_FUNC("__aeabi_ddiv", &__aeabi_ddiv),
    SO_SYMBOL_FUNC("__aeabi_dmul", &__aeabi_dmul),
    SO_SYMBOL_FUNC("__aeabi_dsub", &__aeabi_dsub),
    SO_SYMBOL_FUNC("__aeabi_f2d", &__aeabi_f2d),
    SO_SYMBOL_FUNC("__aeabi_f2iz", &__aeabi_f2iz),
    SO_SYMBOL_FUNC("__aeabi_f2lz", &__aeabi_f2lz),
    SO_SYMBOL_FUNC("__aeabi_f2ulz", &__aeabi_f2ulz),
    SO_SYMBOL_FUNC("__aeabi_fadd", &__aeabi_fadd),
    SO_SYMBOL_FUNC("__aeabi_fcmpeq", &__aeabi_fcmpeq),
    SO_SYMBOL_FUNC("__aeabi_fcmpge", &__aeabi_fcmpge),
    SO_SYMBOL_FUNC("__aeabi_fcmpgt", &__aeabi_fcmpgt),
    SO_SYMBOL_FUNC("__aeabi_fcmple", &__aeabi_fcmple),
    SO_SYMBOL_FUNC("__aeabi_fcmplt", &__aeabi_fcmplt),
    SO_SYMBOL_FUNC("__aeabi_fcmpun", &__aeabi_fcmpun),
    SO_SYMBOL_FUNC("__aeabi_fdiv", &__aeabi_fdiv),
    SO_SYMBOL_FUNC("__aeabi_fmul", &__aeabi_fmul),
    SO_SYMBOL_FUNC("__aeabi_fsub", &__aeabi_fsub),
    SO_SYMBOL_FUNC("__aeabi_i2f", &__aeabi_i2f),
    SO_SYMBOL_FUNC("__aeabi_i2d", &__aeabi_i2d),
    SO_SYMBOL_FUNC("__aeabi_idiv", &__aeabi_idiv),
    SO_SYMBOL_FUNC("__aeabi_idivmod", &__aeabi_idivmod),
    SO_SYMBOL_FUNC("__aeabi_l2d", &__aeabi_l2d),
    SO_SYMBOL_FUNC("__aeabi_l2f", &__aeabi_l2f),
    SO_SYMBOL_FUNC("__aeabi_ldivmod", &__aeabi_ldivmod),
    SO_SYMBOL_FUNC("__aeabi_lmul", &__aeabi_lmul),
    SO_SYMBOL_FUNC("__aeabi_llsl", &__aeabi_llsl),
    SO_SYMBOL_FUNC("__aeabi_memclr", &__aeabi_memclr),
    SO_SYMBOL_FUNC("__aeabi_memclr4", &__aeabi_memclr4),
    SO_SYMBOL_FUNC("__aeabi_memclr8", &__aeabi_memclr8),
    SO_SYMBOL_FUNC("__aeabi_memcpy", &__aeabi_memcpy),
    SO_SYMBOL_FUNC("__aeabi_memcpy4", &__aeabi_memcpy4),
    SO_SYMBOL_FUNC("__aeabi_memcpy8", &__aeabi_memcpy8),
    SO_SYMBOL_FUNC("__aeabi_memmove", &__aeabi_memmove),
    SO_SYMBOL_FUNC("__aeabi_memset4", &__aeabi_memset4),
    SO_SYMBOL_FUNC("__aeabi_memset8", &__aeabi_memset8),
    SO_SYMBOL_FUNC("__aeabi_memset", &__aeabi_memset),
    SO_SYMBOL_FUNC("__aeabi_ui2d", &__aeabi_ui2d),
    SO_SYMBOL_FUNC("__aeabi_ui2f", &__aeabi_ui2f),
    SO_SYMBOL_FUNC("__aeabi_uidiv", &__aeabi_uidiv),
    SO_SYMBOL_FUNC("__aeabi_uidivmod", &__aeabi_uidivmod),
    SO_SYMBOL_FUNC("__aeabi_ul2d", &__aeabi_ul2d),
    SO_SYMBOL_FUNC("__aeabi_ul2f", &__aeabi_ul2f),
    SO_SYMBOL_FUNC("__aeabi_uldivmod", &__aeabi_uldivmod),

    // ===== ARM C++ Unwinding (Base) =====
    SO_SYMBOL_FUNC("__aeabi_unwind_cpp_pr0", &__aeabi_unwind_cpp_pr0),
    SO_SYMBOL_FUNC("__aeabi_unwind_cpp_pr1", &__aeabi_unwind_cpp_pr1),

    // ===== ARM C++ Unwinding (GNU Extensions) =====
    SO_SYMBOL_FUNC("__gnu_Unwind_Find_exidx", &so_ret_0),
    SO_SYMBOL_FUNC("__gnu_unwind_frame", &__gnu_unwind_frame),

    // ===== C++ ABI Support =====
    SO_SYMBOL_FUNC("__cxa_atexit", &__cxa_atexit),
    SO_SYMBOL_FUNC("__cxa_finalize", &__cxa_finalize),

    // ===== C++ Runtime Support =====
    SO_SYMBOL_OBJ("__dso_handle", &__dso_handle, sizeof(__dso_handle)),

    // ===== Stack Protection =====
    SO_SYMBOL_FUNC("__stack_chk_fail", &__stack_chk_fail_soloader),
    SO_SYMBOL_OBJ("__stack_chk_guard", &__stack_chk_guard, 4),

    // ===== Assertion / Debug =====
    SO_SYMBOL_FUNC("__assert", &__assert),

    // ===== Memory Management =====
    SO_SYMBOL_FUNC("malloc", &malloc),
    SO_SYMBOL_FUNC("free", &free),
    SO_SYMBOL_FUNC("calloc", &calloc),
    SO_SYMBOL_FUNC("realloc", &realloc),
    SO_SYMBOL_FUNC("memalign", &memalign),
    SO_SYMBOL_FUNC("aligned_alloc", &aligned_alloc),
    SO_SYMBOL_FUNC("valloc", &valloc),

    // ===== Memory Mapping (mmap) =====
    SO_SYMBOL_FUNC("mmap", &mmap),
    SO_SYMBOL_FUNC("munmap", &munmap),
    SO_SYMBOL_FUNC("mprotect", &mprotect),
    SO_SYMBOL_FUNC("msync", &msync),

    // ===== Memory Operations (byte/string) =====
    SO_SYMBOL_FUNC("memcmp", &memcmp),
    SO_SYMBOL_FUNC("memcpy", &sceClibMemcpy),
    SO_SYMBOL_FUNC("memmem", &memmem),
    SO_SYMBOL_FUNC("memmove", &memmove),
    SO_SYMBOL_FUNC("memset", &memset),
    SO_SYMBOL_FUNC("memchr", &memchr),
    SO_SYMBOL_FUNC("memrchr", &memrchr),

    // ===== String Operations =====
    SO_SYMBOL_FUNC("strcasecmp", &strcasecmp),
    SO_SYMBOL_FUNC("strcat", &strcat),
    SO_SYMBOL_FUNC("strchr", &strchr),
    SO_SYMBOL_FUNC("strcmp", &strcmp),
    SO_SYMBOL_FUNC("strcoll", &strcoll),
    SO_SYMBOL_FUNC("strcpy", &strcpy),
    SO_SYMBOL_FUNC("strcspn", &strcspn),
    SO_SYMBOL_FUNC("strdup", &strdup),
    SO_SYMBOL_FUNC("strlcat", &strlcat),
    SO_SYMBOL_FUNC("strlcpy", &strlcpy),
    SO_SYMBOL_FUNC("strlen", &strlen),
    SO_SYMBOL_FUNC("strncasecmp", &strncasecmp),
    SO_SYMBOL_FUNC("strncat", &strncat),
    SO_SYMBOL_FUNC("strncmp", &strncmp),
    SO_SYMBOL_FUNC("strncpy", &strncpy),
    SO_SYMBOL_FUNC("strnlen", &strnlen),
    SO_SYMBOL_FUNC("strpbrk", &strpbrk),
    SO_SYMBOL_FUNC("strrchr", &strrchr),
    SO_SYMBOL_FUNC("strspn", &strspn),
    SO_SYMBOL_FUNC("strstr", &strstr),
    SO_SYMBOL_FUNC("strtok", &strtok),
    SO_SYMBOL_FUNC("strtok_r", &strtok_r),
    SO_SYMBOL_FUNC("strxfrm", &strxfrm),

    // ===== stdio - Internal =====
    SO_SYMBOL_OBJ("__sF", &__sF_fake, sizeof(__sF_fake)),
    SO_SYMBOL_FUNC("__srget", &__srget),
    SO_SYMBOL_FUNC("__swbuf", &__swbuf),

    // ===== stdio - Formatted Input =====
    SO_SYMBOL_FUNC("fscanf", &fscanf),
    SO_SYMBOL_FUNC("scanf", &scanf),
    SO_SYMBOL_FUNC("sscanf", &sscanf),
    SO_SYMBOL_FUNC("vscanf", &vscanf),
    SO_SYMBOL_FUNC("vfscanf", &vfscanf),
    SO_SYMBOL_FUNC("vsscanf", &vsscanf),

    // ===== stdio - Formatted Output =====
    SO_SYMBOL_FUNC("printf", &sceClibPrintf),
    SO_SYMBOL_FUNC("fprintf", &fprintf),
    SO_SYMBOL_FUNC("sprintf", &sprintf),
    SO_SYMBOL_FUNC("snprintf", &snprintf),
    SO_SYMBOL_FUNC("vprintf", &vprintf),
    SO_SYMBOL_FUNC("vfprintf", &vfprintf),
    SO_SYMBOL_FUNC("vsprintf", &vsprintf),
    SO_SYMBOL_FUNC("vsnprintf", &vsnprintf),
    SO_SYMBOL_FUNC("vasprintf", &vasprintf),

    // ===== stdio - Wide Character Formatted Input =====
    SO_SYMBOL_FUNC("wscanf", &wscanf),
    SO_SYMBOL_FUNC("fwscanf", &fwscanf),
    SO_SYMBOL_FUNC("swscanf", &swscanf),
    SO_SYMBOL_FUNC("vwscanf", &vwscanf),
    SO_SYMBOL_FUNC("vfwscanf", &vfwscanf),
    SO_SYMBOL_FUNC("vswscanf", &vswscanf),

    // ===== stdio - Wide Character Formatted Output =====
    SO_SYMBOL_FUNC("wprintf", &wprintf),
    SO_SYMBOL_FUNC("fwprintf", &fwprintf),
    SO_SYMBOL_FUNC("swprintf", &swprintf),
    SO_SYMBOL_FUNC("vwprintf", &vwprintf),
    SO_SYMBOL_FUNC("vfwprintf", &vfwprintf),
    SO_SYMBOL_FUNC("vswprintf", &vswprintf),

    // ===== stdio - File Operations =====
    SO_SYMBOL_FUNC("fopen", &fopen_soloader),
    SO_SYMBOL_FUNC("freopen", &freopen),
    SO_SYMBOL_FUNC("fdopen", &fdopen),
    SO_SYMBOL_FUNC("fclose", &fclose_soloader),
    SO_SYMBOL_FUNC("fread", &fread_soloader),
    SO_SYMBOL_FUNC("fwrite", &fwrite_soloader),
    SO_SYMBOL_FUNC("fseek", &fseek_soloader),
    SO_SYMBOL_FUNC("ftell", &ftell_soloader),
    SO_SYMBOL_FUNC("fseeko", &fseeko_soloader),
    SO_SYMBOL_FUNC("ftello", &ftello_soloader),
    SO_SYMBOL_FUNC("rewind", &rewind),
    SO_SYMBOL_FUNC("feof", &feof),
    SO_SYMBOL_FUNC("ferror", &ferror),
    SO_SYMBOL_FUNC("fflush", &fflush),
    SO_SYMBOL_FUNC("fileno", &fileno),
    SO_SYMBOL_FUNC("fgetpos", &fgetpos),
    SO_SYMBOL_FUNC("fsetpos", &fsetpos),
    SO_SYMBOL_FUNC("setbuf", &setbuf),
    SO_SYMBOL_FUNC("setvbuf", &setvbuf),

    // ===== stdio - Character IO =====
    SO_SYMBOL_FUNC("fgetc", &fgetc),
    SO_SYMBOL_FUNC("fgets", &fgets),
    SO_SYMBOL_FUNC("fputc", &fputc),
    SO_SYMBOL_FUNC("fputs", &fputs),
    SO_SYMBOL_FUNC("getc", &getc),
    SO_SYMBOL_FUNC("putc", &putc),
    SO_SYMBOL_FUNC("putchar", &putchar),
    SO_SYMBOL_FUNC("puts", &puts),
    SO_SYMBOL_FUNC("ungetc", &ungetc),

    // ===== stdio - Wide Character IO =====
    SO_SYMBOL_FUNC("fgetwc", &fgetwc),
    SO_SYMBOL_FUNC("fgetws", &fgetws),
    SO_SYMBOL_FUNC("fputwc", &fputwc),
    SO_SYMBOL_FUNC("fputws", &fputws),
    SO_SYMBOL_FUNC("getwc", &getwc),
    SO_SYMBOL_FUNC("getwchar", &getwchar),
    SO_SYMBOL_FUNC("putwc", &putwc),
    SO_SYMBOL_FUNC("putwchar", &putwchar),
    SO_SYMBOL_FUNC("ungetwc", &ungetwc),

    // ===== ctype (Character Classification) =====
    SO_SYMBOL_OBJ("_ctype_", &BIONIC_ctype_, 257),
    SO_SYMBOL_OBJ("_tolower_tab_", &BIONIC_tolower_tab_, 514),
    SO_SYMBOL_OBJ("_toupper_tab_", &BIONIC_toupper_tab_, 514),
    SO_SYMBOL_FUNC("isalnum", &isalnum),
    SO_SYMBOL_FUNC("isalpha", &isalpha),
    SO_SYMBOL_FUNC("isblank", &isblank),
    SO_SYMBOL_FUNC("iscntrl", &iscntrl),
    SO_SYMBOL_FUNC("isgraph", &isgraph),
    SO_SYMBOL_FUNC("islower", &islower),
    SO_SYMBOL_FUNC("isprint", &isprint),
    SO_SYMBOL_FUNC("ispunct", &ispunct),
    SO_SYMBOL_FUNC("isspace", &isspace),
    SO_SYMBOL_FUNC("isupper", &isupper),
    SO_SYMBOL_FUNC("isxdigit", &isxdigit),
    SO_SYMBOL_FUNC("tolower", &tolower),
    SO_SYMBOL_FUNC("toupper", &toupper),

    // ===== Wide Character / wctype =====
    SO_SYMBOL_FUNC("btowc", &btowc),
    SO_SYMBOL_FUNC("iswalpha", &iswalpha),
    SO_SYMBOL_FUNC("iswcntrl", &iswcntrl),
    SO_SYMBOL_FUNC("iswctype", &iswctype),
    SO_SYMBOL_FUNC("iswdigit", &iswdigit),
    SO_SYMBOL_FUNC("iswlower", &iswlower),
    SO_SYMBOL_FUNC("iswprint", &iswprint),
    SO_SYMBOL_FUNC("iswpunct", &iswpunct),
    SO_SYMBOL_FUNC("iswspace", &iswspace),
    SO_SYMBOL_FUNC("iswupper", &iswupper),
    SO_SYMBOL_FUNC("iswxdigit", &iswxdigit),
    SO_SYMBOL_FUNC("mbrlen", &mbrlen),
    SO_SYMBOL_FUNC("mbrtowc", &mbrtowc),
    SO_SYMBOL_FUNC("towlower", &towlower),
    SO_SYMBOL_FUNC("towupper", &towupper),
    SO_SYMBOL_FUNC("wcrtomb", &wcrtomb),
    SO_SYMBOL_FUNC("wcscasecmp", &wcscasecmp),
    SO_SYMBOL_FUNC("wcscmp", &wcscmp),
    SO_SYMBOL_FUNC("wcscoll", &wcscoll),
    SO_SYMBOL_FUNC("wcscpy", &wcscpy),
    SO_SYMBOL_FUNC("wcsftime", &wcsftime),
    SO_SYMBOL_FUNC("wcslcat", &wcslcat),
    SO_SYMBOL_FUNC("wcslcpy", &wcslcpy),
    SO_SYMBOL_FUNC("wcslen", &wcslen),
    SO_SYMBOL_FUNC("wcsncasecmp", &wcsncasecmp),
    SO_SYMBOL_FUNC("wcsncpy", &wcsncpy),
    SO_SYMBOL_FUNC("wcstombs", &wcstombs),
    SO_SYMBOL_FUNC("wcsxfrm", &wcsxfrm),
    SO_SYMBOL_FUNC("wctob", &wctob),
    SO_SYMBOL_FUNC("wctype", &wctype),
    SO_SYMBOL_FUNC("wmemchr", &wmemchr),
    SO_SYMBOL_FUNC("wmemcmp", &wmemcmp),
    SO_SYMBOL_FUNC("wmemcpy", &wmemcpy),
    SO_SYMBOL_FUNC("wmemmove", &wmemmove),
    SO_SYMBOL_FUNC("wmemset", &wmemset),

    // ===== stdlib - General Utilities =====
    SO_SYMBOL_FUNC("abort", &abort_soloader),
    SO_SYMBOL_FUNC("exit", &exit_soloader),
    SO_SYMBOL_FUNC("atof", &atof),
    SO_SYMBOL_FUNC("atoi", &atoi),
    SO_SYMBOL_FUNC("atol", &atol),
    SO_SYMBOL_FUNC("atoll", &atoll),
    SO_SYMBOL_FUNC("rand", &rand),
    SO_SYMBOL_FUNC("srand", &srand),
    SO_SYMBOL_FUNC("lrand48", &lrand48),
    SO_SYMBOL_FUNC("srand48", &srand48),
    SO_SYMBOL_FUNC("qsort", &qsort),
    SO_SYMBOL_FUNC("getenv", &getenv_soloader),
    SO_SYMBOL_FUNC("setenv", &setenv_soloader),
    SO_SYMBOL_FUNC("system", &system),
    SO_SYMBOL_FUNC("sleep", &sleep),
    SO_SYMBOL_FUNC("usleep", &usleep),

    // ===== stdlib - String Conversion =====
    SO_SYMBOL_FUNC("strtod", &strtod),
    SO_SYMBOL_FUNC("strtof", &strtof),
    SO_SYMBOL_FUNC("strtoimax", &strtoimax),
    SO_SYMBOL_FUNC("strtol", &strtol),
    SO_SYMBOL_FUNC("strtold", &strtold),
    SO_SYMBOL_FUNC("strtoll", &strtoll),
    SO_SYMBOL_FUNC("strtoul", &strtoul),
    SO_SYMBOL_FUNC("strtoull", &strtoull),
    SO_SYMBOL_FUNC("strtoumax", &strtoumax),

    // ===== Time =====
    SO_SYMBOL_FUNC("clock", &clock_soloader),
    SO_SYMBOL_FUNC("clock_getres", &clock_getres_soloader),
    SO_SYMBOL_FUNC("clock_gettime", &clock_gettime_soloader),
    SO_SYMBOL_FUNC("difftime", &difftime),
    SO_SYMBOL_FUNC("gettimeofday", &gettimeofday),
    SO_SYMBOL_FUNC("gmtime", &gmtime),
    SO_SYMBOL_FUNC("gmtime_r", &gmtime_r),
    SO_SYMBOL_FUNC("localtime", &localtime),
    SO_SYMBOL_FUNC("localtime_r", &localtime_r),
    SO_SYMBOL_FUNC("mktime", &mktime),
    SO_SYMBOL_FUNC("nanosleep", &nanosleep),
    SO_SYMBOL_FUNC("strftime", &strftime),
    SO_SYMBOL_FUNC("time", &time),
    SO_SYMBOL_FUNC("tzset", &tzset),
    SO_SYMBOL_FUNC("utime", &utime),
    SO_SYMBOL_FUNC("ftime", &ftime),

    // ===== Errno =====
    SO_SYMBOL_FUNC("__errno", &__errno_soloader),
    SO_SYMBOL_FUNC("strerror", &strerror_soloader),
    SO_SYMBOL_FUNC("strerror_r", &strerror_r_soloader),

    // ===== Signal =====
    SO_SYMBOL_FUNC("bsd_signal", &signal),
    SO_SYMBOL_FUNC("raise", &raise),
    SO_SYMBOL_FUNC("sigaction", &sigaction),
    SO_SYMBOL_FUNC("kill", &kill),

    // ===== Socket =====
    SO_SYMBOL_FUNC("socket", &socket),
    SO_SYMBOL_FUNC("bind", &bind),
    SO_SYMBOL_FUNC("connect", &connect),
    SO_SYMBOL_FUNC("listen", &listen),
    SO_SYMBOL_FUNC("accept", &accept),
    SO_SYMBOL_FUNC("recv", &recv),
    SO_SYMBOL_FUNC("recvfrom", &recvfrom),
    SO_SYMBOL_FUNC("recvmsg", &recvmsg),
    SO_SYMBOL_FUNC("send", &send),
    SO_SYMBOL_FUNC("sendto", &sendto),
    SO_SYMBOL_FUNC("sendmsg", &sendmsg),
    SO_SYMBOL_FUNC("setsockopt", &setsockopt),
    SO_SYMBOL_FUNC("getsockopt", &getsockopt),
    SO_SYMBOL_FUNC("getsockname", &getsockname),
    SO_SYMBOL_FUNC("getpeername", &getpeername),
    SO_SYMBOL_FUNC("shutdown", &shutdown),
    SO_SYMBOL_FUNC("poll", &poll),
    SO_SYMBOL_FUNC("select", &select_soloader),

    // ===== Epoll =====
    SO_SYMBOL_FUNC("epoll_create", &so_ret_0),
    SO_SYMBOL_FUNC("epoll_ctl", &so_ret_0),
    SO_SYMBOL_FUNC("epoll_wait", &so_ret_0),

    // ===== Network Address =====
    SO_SYMBOL_FUNC("getaddrinfo", &getaddrinfo),
    SO_SYMBOL_FUNC("getnameinfo", &getnameinfo),
    SO_SYMBOL_FUNC("freeaddrinfo", &freeaddrinfo),
    SO_SYMBOL_FUNC("gethostbyaddr", &gethostbyaddr),
    SO_SYMBOL_FUNC("gethostbyname", &gethostbyname),
    SO_SYMBOL_FUNC("gethostname", &gethostname),
    SO_SYMBOL_FUNC("getservbyname", &getservbyname),
    SO_SYMBOL_FUNC("inet_aton", &inet_aton),
    SO_SYMBOL_FUNC("inet_ntoa", &inet_ntoa),
    SO_SYMBOL_FUNC("inet_ntop", &inet_ntop),
    SO_SYMBOL_FUNC("inet_addr", &inet_addr),

    // ===== File IO (POSIX) =====
    SO_SYMBOL_FUNC("open", &open_soloader),
    SO_SYMBOL_FUNC("close", &close_soloader),
    SO_SYMBOL_FUNC("read", &read),
    SO_SYMBOL_FUNC("write", &write),
    SO_SYMBOL_FUNC("lseek", &lseek),
    SO_SYMBOL_FUNC("unlink", &unlink),
    SO_SYMBOL_FUNC("remove", &remove),
    SO_SYMBOL_FUNC("rename", &rename),
    SO_SYMBOL_FUNC("mkdir", &mkdir),
    SO_SYMBOL_FUNC("rmdir", &rmdir),
    SO_SYMBOL_FUNC("chdir", &chdir_soloader),
    SO_SYMBOL_FUNC("getcwd", &getcwd),
    SO_SYMBOL_FUNC("access", &access),
    SO_SYMBOL_FUNC("chmod", &chmod),
    SO_SYMBOL_FUNC("stat", &stat_soloader),
    SO_SYMBOL_FUNC("lstat", &lstat),
    SO_SYMBOL_FUNC("fstat", &fstat_soloader),
    SO_SYMBOL_FUNC("dup", &dup_soloader),
    SO_SYMBOL_FUNC("fcntl", &fcntl_soloader),
    SO_SYMBOL_FUNC("fsync", &fsync_soloader),
    SO_SYMBOL_FUNC("ftruncate", &ftruncate),
    SO_SYMBOL_FUNC("truncate", &truncate),
    SO_SYMBOL_FUNC("pipe", &pipe),
    SO_SYMBOL_FUNC("ioctl", &ioctl_soloader),
    SO_SYMBOL_FUNC("statfs", &statfs_soloader),

    // ===== Directory Operations =====
    SO_SYMBOL_FUNC("opendir", &opendir_soloader),
    SO_SYMBOL_FUNC("closedir", &closedir_soloader),
    SO_SYMBOL_FUNC("readdir", &readdir_soloader),
    SO_SYMBOL_FUNC("readdir_r", &readdir_r_soloader),

    // ===== Temporary Files =====
    SO_SYMBOL_FUNC("mktemp", &mktemp_soloader),
    SO_SYMBOL_FUNC("mkstemp", &mkstemp),
    SO_SYMBOL_FUNC("tmpfile", &tmpfile),
    SO_SYMBOL_FUNC("tmpnam", &tmpnam),

    // ===== Path Operations =====
    SO_SYMBOL_FUNC("realpath", &realpath),
    SO_SYMBOL_FUNC("symlink", &symlink_soloader),
    SO_SYMBOL_FUNC("basename", &basename),
    SO_SYMBOL_FUNC("dirname", &dirname),

    // ===== Pthread =====
    SO_SYMBOL_FUNC("__pthread_cleanup_push", &__pthread_cleanup_push_soloader),
    SO_SYMBOL_FUNC("__pthread_cleanup_pop", &__pthread_cleanup_pop_soloader),
    SO_SYMBOL_FUNC("pthread_create", &pthread_create_soloader),
    SO_SYMBOL_FUNC("pthread_detach", &pthread_detach_soloader),
    SO_SYMBOL_FUNC("pthread_join", &pthread_join_soloader),
    SO_SYMBOL_FUNC("pthread_self", &pthread_self_soloader),
    SO_SYMBOL_FUNC("pthread_equal", &pthread_equal_soloader),
    SO_SYMBOL_FUNC("pthread_exit", &pthread_exit),
    SO_SYMBOL_FUNC("pthread_cancel", &pthread_cancel),
    SO_SYMBOL_FUNC("pthread_once", &pthread_once_soloader),
    SO_SYMBOL_FUNC("pthread_kill", &pthread_kill_soloader),
    SO_SYMBOL_FUNC("pthread_sigmask", &so_ret_0),

    // ===== Pthread Mutex =====
    SO_SYMBOL_FUNC("pthread_mutex_init", &pthread_mutex_init_soloader),
    SO_SYMBOL_FUNC("pthread_mutex_destroy", &pthread_mutex_destroy_soloader),
    SO_SYMBOL_FUNC("pthread_mutex_lock", &pthread_mutex_lock_soloader),
    SO_SYMBOL_FUNC("pthread_mutex_trylock", &pthread_mutex_trylock_soloader),
    SO_SYMBOL_FUNC("pthread_mutex_unlock", &pthread_mutex_unlock_soloader),
    SO_SYMBOL_FUNC("pthread_mutexattr_init", &pthread_mutexattr_init_soloader),
    SO_SYMBOL_FUNC("pthread_mutexattr_destroy", &pthread_mutexattr_destroy_soloader),
    SO_SYMBOL_FUNC("pthread_mutexattr_settype", &pthread_mutexattr_settype_soloader),

    // ===== Pthread Condition Variable =====
    SO_SYMBOL_FUNC("pthread_cond_init", &pthread_cond_init_soloader),
    SO_SYMBOL_FUNC("pthread_cond_destroy", &pthread_cond_destroy_soloader),
    SO_SYMBOL_FUNC("pthread_cond_signal", &pthread_cond_signal_soloader),
    SO_SYMBOL_FUNC("pthread_cond_broadcast", &pthread_cond_broadcast_soloader),
    SO_SYMBOL_FUNC("pthread_cond_wait", &pthread_cond_wait_soloader),
    SO_SYMBOL_FUNC("pthread_cond_timedwait", &pthread_cond_timedwait_soloader),
    SO_SYMBOL_FUNC("pthread_cond_timedwait_relative_np", &pthread_cond_timedwait_relative_np_soloader),

    // ===== Pthread Attribute =====
    SO_SYMBOL_FUNC("pthread_attr_init", &pthread_attr_init_soloader),
    SO_SYMBOL_FUNC("pthread_attr_destroy", &pthread_attr_destroy_soloader),
    SO_SYMBOL_FUNC("pthread_attr_setdetachstate", &pthread_attr_setdetachstate_soloader),
    SO_SYMBOL_FUNC("pthread_attr_getdetachstate", &pthread_attr_getdetachstate),
    SO_SYMBOL_FUNC("pthread_attr_setstacksize", &pthread_attr_setstacksize_soloader),
    SO_SYMBOL_FUNC("pthread_attr_setschedparam", &pthread_attr_setschedparam),

    // ===== Pthread Thread Specific =====
    SO_SYMBOL_FUNC("pthread_key_create", &pthread_key_create),
    SO_SYMBOL_FUNC("pthread_key_delete", &pthread_key_delete),
    SO_SYMBOL_FUNC("pthread_getspecific", &pthread_getspecific_soloader),
    SO_SYMBOL_FUNC("pthread_setspecific", &pthread_setspecific),

    // ===== Pthread Scheduling =====
    SO_SYMBOL_FUNC("pthread_getschedparam", &pthread_getschedparam_soloader),
    SO_SYMBOL_FUNC("pthread_setschedparam", &pthread_setschedparam_soloader),
    SO_SYMBOL_FUNC("pthread_setname_np", &pthread_setname_np_soloader),

    // ===== Semaphore =====
    SO_SYMBOL_FUNC("sem_init", &sem_init_soloader),
    SO_SYMBOL_FUNC("sem_destroy", &sem_destroy_soloader),
    SO_SYMBOL_FUNC("sem_wait", &sem_wait_soloader),
    SO_SYMBOL_FUNC("sem_trywait", &sem_trywait_soloader),
    SO_SYMBOL_FUNC("sem_timedwait", &sem_timedwait_soloader),
    SO_SYMBOL_FUNC("sem_post", &sem_post_soloader),
    SO_SYMBOL_FUNC("sem_getvalue", &sem_getvalue_soloader),

    // ===== Scheduling =====
    SO_SYMBOL_FUNC("sched_yield", &sched_yield),
    SO_SYMBOL_FUNC("sched_get_priority_max", &sched_get_priority_max),
    SO_SYMBOL_FUNC("sched_get_priority_min", &sched_get_priority_min),

    // ===== Atomic Operations =====
    SO_SYMBOL_FUNC("__atomic_cmpxchg", &__atomic_cmpxchg),
    SO_SYMBOL_FUNC("__atomic_dec", &__atomic_dec),
    SO_SYMBOL_FUNC("__atomic_inc", &__atomic_inc),
    SO_SYMBOL_FUNC("__atomic_swap", &__atomic_swap),

    // ===== Non-Local Jumps =====
    SO_SYMBOL_FUNC("setjmp", &setjmp),
    SO_SYMBOL_FUNC("longjmp", &longjmp),

    // ===== Process Control =====
    SO_SYMBOL_FUNC("fork", &fork),
    SO_SYMBOL_FUNC("getpid", &getpid),
    SO_SYMBOL_FUNC("getpagesize", &getpagesize),
    SO_SYMBOL_FUNC("sbrk", &sbrk),
    SO_SYMBOL_FUNC("syscall", &syscall),
    SO_SYMBOL_FUNC("waitpid", &so_ret_0),
    SO_SYMBOL_FUNC("prctl", &so_ret_0),
    SO_SYMBOL_FUNC("sysconf", &so_ret_0),
    SO_SYMBOL_FUNC("uname", &uname_soloader),

    // ===== Resource Priority =====
    SO_SYMBOL_FUNC("setpriority", &setpriority),
    SO_SYMBOL_FUNC("getpriority", &getpriority),

    // ===== Locale =====
    SO_SYMBOL_FUNC("setlocale", &setlocale),

    // ===== Bionic (Android libc) Extensions =====
    SO_SYMBOL_OBJ("__page_size", &__page_size, sizeof(__page_size)),
    SO_SYMBOL_FUNC("__system_property_get", &__system_property_get_soloader),

    // ===== Google Extensions =====
    SO_SYMBOL_FUNC("__google_potentially_blocking_region_begin", &so_ret_0),
    SO_SYMBOL_FUNC("__google_potentially_blocking_region_end", &so_ret_0),
};

static int prepare_libc(so_library *library) {
    __sF_fake[0] = *stdin;
    __sF_fake[1] = *stdout;
    __sF_fake[2] = *stderr;

    return 0;
}

so_library so_library_c = {
    .soname      = "libc.so",
    .symbols     = libc_symbols,
    .num_symbols = sizeof(libc_symbols) / sizeof(libc_symbols[0]),
    .prepare     = prepare_libc,
};
