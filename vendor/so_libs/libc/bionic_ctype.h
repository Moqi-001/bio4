#ifndef BIONIC_CTYPE_H
#define BIONIC_CTYPE_H

#ifdef __cplusplus
extern "C" {
#endif

#define CTYPE_NUM_CHARS 256

extern const char  BIONIC_C_ctype_[1 + CTYPE_NUM_CHARS];
extern const short BIONIC_C_tolower_[1 + CTYPE_NUM_CHARS];
extern const short BIONIC_C_toupper_[1 + CTYPE_NUM_CHARS];

#ifdef __cplusplus
};
#endif

#endif