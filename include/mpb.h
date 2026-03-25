/* mpb.h -- Mainframe Package Bureau
 *
 * Package manager for mainframe languages. COBOL, HLASM,
 * PL/I, REXX — they all deserve nice things.
 *
 * (c) 2026 Zane Hambly. Apache 2.0.
 */

#ifndef MPB_H
#define MPB_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---- Limits ---- */

#define MPB_MAX_PKG    256     /* max packages in registry index */
#define MPB_MAX_FILES  32      /* max files per package */
#define MPB_NAME_LEN   64
#define MPB_VER_LEN    16
#define MPB_DESC_LEN   256
#define MPB_PATH_LEN   512
#define MPB_BUF_SZ     65536

/* ---- Registry URL ---- */

#define MPB_REGISTRY   "https://raw.githubusercontent.com/Zaneham/mpb-registry/master"

/* ---- Package languages ---- */

#define MPB_COBOL   1
#define MPB_HLASM   2
#define MPB_PLI     3
#define MPB_REXX    4
#define MPB_JCL     5
#define MPB_FORTRAN 6
#define MPB_ALGOL   7
#define MPB_NEWP    8
#define MPB_WFL     9

/* ---- Package metadata ---- */

/* ---- License classification ---- */

#define MPB_LIC_UNKNOWN    0   /* no license specified — danger */
#define MPB_LIC_PERMISSIVE 1   /* Apache, MIT, BSD, ISC, Unlicense */
#define MPB_LIC_WEAK_COPY  2   /* LGPL, MPL, EPL — file-level copyleft */
#define MPB_LIC_COPYLEFT   3   /* GPL, AGPL — full copyleft */
#define MPB_LIC_PROPRI     4   /* proprietary / all rights reserved */

typedef struct {
    char     name[MPB_NAME_LEN];
    char     version[MPB_VER_LEN];
    char     description[MPB_DESC_LEN];
    char     author[MPB_NAME_LEN];
    char     license[MPB_NAME_LEN];      /* SPDX identifier */
    char     repository[MPB_PATH_LEN];   /* source repo URL */
    char     download[MPB_PATH_LEN];     /* direct download URL for files */
    uint8_t  language;
    char     files[MPB_MAX_FILES][MPB_NAME_LEN];
    int      n_files;
} mpb_pkg_t;

/* ---- Registry index entry ---- */

typedef struct {
    char     name[MPB_NAME_LEN];
    char     version[MPB_VER_LEN];
    char     description[MPB_DESC_LEN];
    uint8_t  language;
} mpb_idx_t;

/* ---- Commands ---- */

int  mpb_install(const char *name, const char *dest);
int  mpb_remove(const char *name, const char *dir);
int  mpb_update(const char *name, const char *dir);
int  mpb_search(const char *query);
int  mpb_search_all(void);
int  mpb_list(const char *dir);
int  mpb_init(const char *name, uint8_t lang);
int  mpb_info(const char *name);

/* ---- Utilities ---- */

int  mpb_http(const char *url, char *buf, int bufsz);
int  mpb_pkg_parse(const char *json, mpb_pkg_t *pkg);
const char *mpb_lang_str(uint8_t lang);
int  mpb_lic_class(const char *spdx);
const char *mpb_lic_warn(int cls);

#endif /* MPB_H */
