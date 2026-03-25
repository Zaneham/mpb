/* mpb_cmd.c -- MPB commands: install, search, info, list, init
 *
 * (c) 2026 Zane Hambly. Apache 2.0.
 */

#include "mpb.h"

/* ---- JSON field extraction from a block of text ----
 * Searches for "key": "value" with tolerance for whitespace
 * after the colon. Copies value into out, max bytes.
 * Returns pointer to value start, or NULL on miss.
 * One function to rule them all, instead of hardcoded
 * offsets scattered across 500 lines. */

static const char *
jfld(const char *block, int scope, const char *key,
     char *out, int max)
{
    char pat[128];
    const char *p, *end;
    int klen, slen;

    klen = snprintf(pat, sizeof(pat), "\"%s\":", key);
    if (klen <= 0) return NULL;

    p = strstr(block, pat);
    if (!p || (scope > 0 && p - block > scope)) return NULL;
    p += klen;

    /* Skip whitespace */
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;

    /* Expect opening quote */
    if (*p != '"') return NULL;
    p++;

    /* Find closing quote */
    end = strchr(p, '"');
    if (!end) return NULL;

    slen = (int)(end - p);
    if (slen >= max) slen = max - 1;
    memcpy(out, p, (size_t)slen);
    out[slen] = '\0';
    return p;
}

#ifdef _WIN32
#include <direct.h>
#define mpb_mkdir(p) _mkdir(p)
#else
#include <sys/stat.h>
#define mpb_mkdir(p) mkdir(p, 0755)
#endif

/* ---- install: fetch package from registry ---- */

int
mpb_install(const char *name, const char *dest)
{
    char url[MPB_PATH_LEN];
    char buf[MPB_BUF_SZ];
    mpb_pkg_t pkg;
    int i;

    printf("mpb: fetching %s...\n", name);

    /* Fetch package.json from registry */
    snprintf(url, sizeof(url), "%s/packages/%s/package.json",
             MPB_REGISTRY, name);

    if (mpb_http(url, buf, MPB_BUF_SZ) <= 0) {
        fprintf(stderr, "mpb: package '%s' not found\n", name);
        return -1;
    }

    if (mpb_pkg_parse(buf, &pkg) != 0) {
        fprintf(stderr, "mpb: invalid package.json for '%s'\n", name);
        return -1;
    }

    printf("mpb: %s v%s (%s) by %s\n",
           pkg.name, pkg.version, mpb_lang_str(pkg.language), pkg.author);
    printf("mpb: %s\n", pkg.description);

    /* License check — warn before installing copyleft into your bank */
    {
        int lcls = mpb_lic_class(pkg.license);
        printf("mpb: license: %s — %s\n",
               pkg.license[0] ? pkg.license : "(none)",
               mpb_lic_warn(lcls));

        if (lcls == MPB_LIC_COPYLEFT || lcls == MPB_LIC_PROPRI ||
            lcls == MPB_LIC_UNKNOWN) {
            char yn[8];
            printf("mpb: proceed? [y/N] ");
            fflush(stdout);
            if (!fgets(yn, sizeof(yn), stdin) ||
                (yn[0] != 'y' && yn[0] != 'Y')) {
                printf("mpb: install cancelled\n");
                return -1;
            }
        }
    }

    /* If package has a repository URL and no file list,
     * clone the repo instead of downloading individual files.
     * Most real packages are full repos, not single files. */
    if (pkg.n_files == 0 && pkg.repository[0]) {
        char cmd[MPB_PATH_LEN];
        char dir[MPB_PATH_LEN];
        snprintf(dir, sizeof(dir), "%s/%s", dest, name);
        snprintf(cmd, sizeof(cmd), "git clone --depth 1 \"%s\" \"%s\" 2>&1",
                 pkg.repository, dir);
        printf("mpb: cloning from %s\n", pkg.repository);
        {
            int rc = system(cmd);
            if (rc != 0) {
                fprintf(stderr, "mpb: clone failed\n");
                return -1;
            }
        }
        /* Write package.json into the cloned directory so
         * list/update/remove can find it later */
        {
            char ppath[MPB_PATH_LEN];
            FILE *pf;
            snprintf(ppath, sizeof(ppath), "%s/%s/package.json",
                     dest, name);
            pf = fopen(ppath, "w");
            if (pf) {
                fprintf(pf, "{\n");
                fprintf(pf, "  \"name\": \"%s\",\n", pkg.name);
                fprintf(pf, "  \"version\": \"%s\",\n", pkg.version);
                fprintf(pf, "  \"description\": \"%s\",\n",
                        pkg.description);
                fprintf(pf, "  \"author\": \"%s\",\n", pkg.author);
                fprintf(pf, "  \"license\": \"%s\",\n", pkg.license);
                fprintf(pf, "  \"language\": \"%s\",\n",
                        mpb_lang_str(pkg.language));
                fprintf(pf, "  \"repository\": \"%s\"\n",
                        pkg.repository);
                fprintf(pf, "}\n");
                fclose(pf);
            }
        }
        printf("mpb: installed %s v%s\n", pkg.name, pkg.version);
        return 0;
    }

    if (pkg.n_files == 0) {
        fprintf(stderr, "mpb: no files and no repository URL\n");
        return -1;
    }

    /* Create destination directory */
    {
        char dir[MPB_PATH_LEN];
        snprintf(dir, sizeof(dir), "%s/%s", dest, name);
        mpb_mkdir(dir);
    }

    /* Download each file from the source repo, not the registry.
     * The download URL points to the raw files in the author's repo. */
    {
        const char *base = pkg.download[0] ? pkg.download : MPB_REGISTRY;

        for (i = 0; i < pkg.n_files; i++) {
            char furl[MPB_PATH_LEN];
            char fpath[MPB_PATH_LEN];
            FILE *fp;

            if (pkg.download[0])
                snprintf(furl, sizeof(furl), "%s/%s", base, pkg.files[i]);
            else
                snprintf(furl, sizeof(furl), "%s/packages/%s/%s",
                         base, name, pkg.files[i]);

            snprintf(fpath, sizeof(fpath), "%s/%s/%s",
                     dest, name, pkg.files[i]);

            printf("  %s", pkg.files[i]);
            fflush(stdout);

            if (mpb_http(furl, buf, MPB_BUF_SZ) <= 0) {
                printf(" FAILED\n");
                continue;
            }

            fp = fopen(fpath, "w");
            if (fp) {
                fputs(buf, fp);
                fclose(fp);
                printf(" ok\n");
            } else {
                printf(" FAILED (can't write)\n");
            }
        }
    }

    /* Save package.json locally */
    {
        char ppath[MPB_PATH_LEN];
        FILE *fp;
        snprintf(ppath, sizeof(ppath), "%s/%s/package.json", dest, name);
        fp = fopen(ppath, "w");
        if (fp) {
            fprintf(fp, "{\n");
            fprintf(fp, "  \"name\": \"%s\",\n", pkg.name);
            fprintf(fp, "  \"version\": \"%s\",\n", pkg.version);
            fprintf(fp, "  \"description\": \"%s\",\n", pkg.description);
            fprintf(fp, "  \"author\": \"%s\",\n", pkg.author);
            fprintf(fp, "  \"language\": \"%s\",\n", mpb_lang_str(pkg.language));
            fprintf(fp, "  \"repository\": \"%s\"\n", pkg.repository);
            fprintf(fp, "}\n");
            fclose(fp);
        }
    }

    printf("mpb: installed %s v%s (%d files)\n",
           pkg.name, pkg.version, pkg.n_files);
    return 0;
}

/* ---- search: find packages matching query ---- */

int
mpb_search(const char *query)
{
    char url[MPB_PATH_LEN];
    char buf[MPB_BUF_SZ];
    const char *p;
    int count = 0;

    /* Fetch the registry index */
    snprintf(url, sizeof(url), "%s/index.json", MPB_REGISTRY);

    if (mpb_http(url, buf, MPB_BUF_SZ) <= 0) {
        fprintf(stderr, "mpb: can't reach registry\n");
        return -1;
    }

    printf("mpb: searching for '%s'...\n\n", query);

    p = buf;
    while ((p = strstr(p, "\"name\":")) != NULL) {
        char name[MPB_NAME_LEN] = {0};
        char desc[MPB_DESC_LEN] = {0};
        char ver[MPB_VER_LEN] = {0};
        char lang[32] = {0};

        jfld(p, 500, "name", name, MPB_NAME_LEN);
        jfld(p, 500, "description", desc, MPB_DESC_LEN);
        jfld(p, 500, "version", ver, MPB_VER_LEN);
        jfld(p, 500, "language", lang, 32);

        if (strstr(name, query) || strstr(desc, query)) {
            printf("  %-20s %8s  [%s]  %s\n", name, ver, lang, desc);
            count++;
        }

        p++;
    }

    if (count == 0)
        printf("  no packages found\n");
    else
        printf("\n  %d package(s) found\n", count);

    return count;
}

/* ---- info: show package details ---- */

int
mpb_info(const char *name)
{
    char url[MPB_PATH_LEN];
    char buf[MPB_BUF_SZ];
    mpb_pkg_t pkg;
    int i;

    snprintf(url, sizeof(url), "%s/packages/%s/package.json",
             MPB_REGISTRY, name);

    if (mpb_http(url, buf, MPB_BUF_SZ) <= 0) {
        fprintf(stderr, "mpb: package '%s' not found\n", name);
        return -1;
    }

    if (mpb_pkg_parse(buf, &pkg) != 0) {
        fprintf(stderr, "mpb: invalid package\n");
        return -1;
    }

    printf("\n  %s v%s\n", pkg.name, pkg.version);
    printf("  %s\n", pkg.description);
    printf("  author:   %s\n", pkg.author);
    printf("  language: %s\n", mpb_lang_str(pkg.language));
    printf("  license:  %s (%s)\n",
           pkg.license[0] ? pkg.license : "(none)",
           mpb_lic_warn(mpb_lic_class(pkg.license)));
    if (pkg.repository[0])
        printf("  repo:     %s\n", pkg.repository);
    printf("  files:\n");
    for (i = 0; i < pkg.n_files; i++)
        printf("    %s\n", pkg.files[i]);
    printf("\n");

    return 0;
}

/* ---- list: show locally installed packages ---- */

int
mpb_list(const char *dir)
{
    /* Just check for package.json files in subdirectories.
     * Cross-platform: use popen with ls/dir */
    char cmd[MPB_PATH_LEN];
    FILE *fp;
    char line[256];
    int count = 0;

    /* Use ls everywhere — even on Windows we compile under
     * Git Bash, so popen runs through bash, not cmd.exe. */
    snprintf(cmd, sizeof(cmd), "ls -1 \"%s\" 2>/dev/null", dir);

    fp = popen(cmd, "r");
    if (!fp) {
        printf("mpb: no packages installed\n");
        return 0;
    }

    printf("mpb: installed packages in %s/\n\n", dir);

    while (fgets(line, sizeof(line), fp)) {
        char *nl = strchr(line, '\n');
        if (nl) *nl = '\0';
        nl = strchr(line, '\r');
        if (nl) *nl = '\0';
        if (line[0] == '\0') continue;

        /* Check if it has a package.json */
        {
            char ppath[MPB_PATH_LEN];
            FILE *pf;
            snprintf(ppath, sizeof(ppath), "%s/%s/package.json", dir, line);
            pf = fopen(ppath, "r");
            if (pf) {
                char buf[MPB_BUF_SZ];
                mpb_pkg_t pkg;
                int n = (int)fread(buf, 1, MPB_BUF_SZ - 1, pf);
                buf[n] = '\0';
                fclose(pf);
                if (mpb_pkg_parse(buf, &pkg) == 0) {
                    printf("  %-20s %8s  [%s]  %s\n",
                           pkg.name, pkg.version,
                           mpb_lang_str(pkg.language), pkg.description);
                    count++;
                }
            }
        }
    }

    pclose(fp);

    if (count == 0)
        printf("  no packages installed\n");
    else
        printf("\n  %d package(s)\n", count);

    return count;
}

/* ---- init: create a new package.json ---- */

int
mpb_init(const char *name, uint8_t lang)
{
    FILE *fp = fopen("package.json", "w");
    if (!fp) {
        fprintf(stderr, "mpb: can't create package.json\n");
        return -1;
    }

    fprintf(fp, "{\n");
    fprintf(fp, "  \"name\": \"%s\",\n", name);
    fprintf(fp, "  \"version\": \"0.1.0\",\n");
    fprintf(fp, "  \"description\": \"\",\n");
    fprintf(fp, "  \"author\": \"\",\n");
    fprintf(fp, "  \"language\": \"%s\",\n", mpb_lang_str(lang));
    fprintf(fp, "  \"files\": []\n");
    fprintf(fp, "}\n");

    fclose(fp);
    printf("mpb: created package.json for '%s' (%s)\n",
           name, mpb_lang_str(lang));
    return 0;
}

/* ---- remove: delete an installed package ---- */

int
mpb_remove(const char *name, const char *dir)
{
    char path[MPB_PATH_LEN];
    char cmd[MPB_PATH_LEN];
    char pjson[MPB_PATH_LEN];
    FILE *fp;

    snprintf(pjson, sizeof(pjson), "%s/%s/package.json", dir, name);
    fp = fopen(pjson, "r");
    if (!fp) {
        fprintf(stderr, "mpb: '%s' is not installed\n", name);
        return -1;
    }
    fclose(fp);

    snprintf(path, sizeof(path), "%s/%s", dir, name);

#ifdef _WIN32
    snprintf(cmd, sizeof(cmd), "rmdir /s /q \"%s\" 2>nul", path);
#else
    snprintf(cmd, sizeof(cmd), "rm -rf \"%s\" 2>/dev/null", path);
#endif

    printf("mpb: removing %s...\n", name);
    {
        int rc = system(cmd);
        if (rc != 0) {
            fprintf(stderr, "mpb: failed to remove '%s'\n", name);
            return -1;
        }
    }

    printf("mpb: removed %s\n", name);
    return 0;
}

/* ---- update: git pull on an installed package ---- */

int
mpb_update(const char *name, const char *dir)
{
    char path[MPB_PATH_LEN];
    char cmd[MPB_PATH_LEN];
    char gitdir[MPB_PATH_LEN];
    FILE *fp;

    /* Check it exists */
    snprintf(path, sizeof(path), "%s/%s/package.json", dir, name);
    fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "mpb: '%s' is not installed\n", name);
        return -1;
    }
    fclose(fp);

    /* Check it's a git repo (has .git/HEAD) */
    snprintf(gitdir, sizeof(gitdir), "%s/%s/.git/HEAD", dir, name);
    fp = fopen(gitdir, "r");
    if (!fp) {
        fprintf(stderr,
            "mpb: '%s' was not installed via git, use remove + install\n",
            name);
        return -1;
    }
    fclose(fp);

    snprintf(cmd, sizeof(cmd),
             "cd \"%s/%s\" && git pull --ff-only 2>&1", dir, name);

    printf("mpb: updating %s...\n", name);
    {
        int rc = system(cmd);
        if (rc != 0) {
            fprintf(stderr, "mpb: git pull failed for '%s'\n", name);
            return -1;
        }
    }

    printf("mpb: updated %s\n", name);
    return 0;
}

/* ---- search_all: list every package in the registry ---- */

int
mpb_search_all(void)
{
    char url[MPB_PATH_LEN];
    char buf[MPB_BUF_SZ];
    const char *p;
    int count = 0;

    snprintf(url, sizeof(url), "%s/index.json", MPB_REGISTRY);

    if (mpb_http(url, buf, MPB_BUF_SZ) <= 0) {
        fprintf(stderr, "mpb: can't reach registry\n");
        return -1;
    }

    printf("mpb: all packages in registry\n\n");

    p = buf;
    while ((p = strstr(p, "\"name\":")) != NULL) {
        char name[MPB_NAME_LEN] = {0};
        char desc[MPB_DESC_LEN] = {0};
        char ver[MPB_VER_LEN] = {0};
        char lang[32] = {0};
        char lic[MPB_NAME_LEN] = {0};

        jfld(p, 500, "name", name, MPB_NAME_LEN);
        jfld(p, 500, "description", desc, MPB_DESC_LEN);
        jfld(p, 500, "version", ver, MPB_VER_LEN);
        jfld(p, 500, "language", lang, 32);
        jfld(p, 500, "license", lic, MPB_NAME_LEN);

        if (name[0]) {
            printf("  %-20s %8s  [%-5s]  %-10s  %s\n",
                   name, ver, lang, lic, desc);
            count++;
        }

        p++;
    }

    printf("\n  %d package(s) in registry\n", count);
    return count;
}
