/* mpb_pkg.c -- Package parsing and language helpers
 *
 * (c) 2026 Zane Hambly. Apache 2.0.
 */

#include "mpb.h"

const char *
mpb_lang_str(uint8_t lang)
{
    switch (lang) {
    case MPB_COBOL: return "cobol";
    case MPB_HLASM: return "hlasm";
    case MPB_PLI:   return "pli";
    case MPB_REXX:  return "rexx";
    case MPB_JCL:   return "jcl";
    default:        return "unknown";
    }
}

static uint8_t
lang_from_str(const char *s)
{
    if (strcmp(s, "cobol") == 0) return MPB_COBOL;
    if (strcmp(s, "hlasm") == 0) return MPB_HLASM;
    if (strcmp(s, "pli") == 0)   return MPB_PLI;
    if (strcmp(s, "rexx") == 0)  return MPB_REXX;
    if (strcmp(s, "jcl") == 0)   return MPB_JCL;
    return 0;
}

/* Ghetto JSON value extractor — same approach as Freiberg.
 * Good enough for package.json files. */

static const char *
jstr(const char *json, const char *key, char *out, int max)
{
    char pat[128];
    const char *p, *end;
    int klen, slen;

    klen = snprintf(pat, sizeof(pat), "\"%s\":", key);
    if (klen <= 0) return NULL;

    p = strstr(json, pat);
    if (!p) return NULL;
    p += klen;
    /* Skip whitespace and opening quote */
    while (*p == ' ' || *p == '\t') p++;
    if (*p == '"') p++;
    else return NULL;

    end = strchr(p, '"');
    if (!end) return NULL;

    slen = (int)(end - p);
    if (slen >= max) slen = max - 1;
    memcpy(out, p, (size_t)slen);
    out[slen] = '\0';
    return out;
}

int
mpb_pkg_parse(const char *json, mpb_pkg_t *pkg)
{
    char lang[32];

    memset(pkg, 0, sizeof(*pkg));

    jstr(json, "name", pkg->name, MPB_NAME_LEN);
    jstr(json, "version", pkg->version, MPB_VER_LEN);
    jstr(json, "description", pkg->description, MPB_DESC_LEN);
    jstr(json, "author", pkg->author, MPB_NAME_LEN);
    jstr(json, "license", pkg->license, MPB_NAME_LEN);
    jstr(json, "repository", pkg->repository, MPB_PATH_LEN);
    jstr(json, "download", pkg->download, MPB_PATH_LEN);

    if (jstr(json, "language", lang, sizeof(lang)))
        pkg->language = lang_from_str(lang);

    /* Parse files array — look for "files":["a","b","c"] */
    {
        const char *fa = strstr(json, "\"files\":");
        if (fa) { while (*fa && *fa != '[') fa++; }
        if (fa) {
            const char *p = fa + 1;
            while (*p && *p != ']' && pkg->n_files < MPB_MAX_FILES) {
                if (*p == '"') {
                    p++;
                    {
                        const char *end = strchr(p, '"');
                        if (end) {
                            int len = (int)(end - p);
                            if (len >= MPB_NAME_LEN) len = MPB_NAME_LEN - 1;
                            memcpy(pkg->files[pkg->n_files], p, (size_t)len);
                            pkg->files[pkg->n_files][len] = '\0';
                            pkg->n_files++;
                            p = end + 1;
                        } else break;
                    }
                } else {
                    p++;
                }
            }
        }
    }

    return pkg->name[0] ? 0 : -1;
}

/* ---- License classification ----
 * SPDX identifiers → risk level. Not legal advice,
 * just a heads-up before you mix GPL into your bank's
 * proprietary COBOL. Ask your lawyer, not your compiler. */

int
mpb_lic_class(const char *spdx)
{
    if (!spdx || !spdx[0]) return MPB_LIC_UNKNOWN;

    /* Permissive */
    if (strcmp(spdx, "Apache-2.0") == 0) return MPB_LIC_PERMISSIVE;
    if (strcmp(spdx, "MIT") == 0)        return MPB_LIC_PERMISSIVE;
    if (strcmp(spdx, "BSD-2-Clause") == 0) return MPB_LIC_PERMISSIVE;
    if (strcmp(spdx, "BSD-3-Clause") == 0) return MPB_LIC_PERMISSIVE;
    if (strcmp(spdx, "ISC") == 0)        return MPB_LIC_PERMISSIVE;
    if (strcmp(spdx, "Unlicense") == 0)  return MPB_LIC_PERMISSIVE;
    if (strcmp(spdx, "0BSD") == 0)       return MPB_LIC_PERMISSIVE;
    if (strcmp(spdx, "CC0-1.0") == 0)    return MPB_LIC_PERMISSIVE;
    if (strcmp(spdx, "Zlib") == 0)       return MPB_LIC_PERMISSIVE;

    /* Weak copyleft */
    if (strcmp(spdx, "LGPL-2.1") == 0)   return MPB_LIC_WEAK_COPY;
    if (strcmp(spdx, "LGPL-3.0") == 0)   return MPB_LIC_WEAK_COPY;
    if (strcmp(spdx, "MPL-2.0") == 0)    return MPB_LIC_WEAK_COPY;
    if (strcmp(spdx, "EPL-2.0") == 0)    return MPB_LIC_WEAK_COPY;

    /* Strong copyleft */
    if (strcmp(spdx, "GPL-2.0") == 0)    return MPB_LIC_COPYLEFT;
    if (strcmp(spdx, "GPL-3.0") == 0)    return MPB_LIC_COPYLEFT;
    if (strcmp(spdx, "AGPL-3.0") == 0)   return MPB_LIC_COPYLEFT;

    /* Proprietary */
    if (strcmp(spdx, "PROPRIETARY") == 0)  return MPB_LIC_PROPRI;
    if (strcmp(spdx, "UNLICENSED") == 0)   return MPB_LIC_PROPRI;

    return MPB_LIC_UNKNOWN;
}

const char *
mpb_lic_warn(int cls)
{
    switch (cls) {
    case MPB_LIC_PERMISSIVE: return "permissive (safe for commercial use)";
    case MPB_LIC_WEAK_COPY:  return "weak copyleft (modifications must be shared)";
    case MPB_LIC_COPYLEFT:   return "COPYLEFT (derivative works must be open source)";
    case MPB_LIC_PROPRI:     return "PROPRIETARY (check terms before use)";
    default:                 return "UNKNOWN LICENSE (review before use)";
    }
}
