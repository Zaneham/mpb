/* main.c -- MPB entry point
 *
 * Mainframe Package Bureau. Because COBOL developers
 * deserve a package manager too.
 *
 * (c) 2026 Zane Hambly. Apache 2.0.
 */

#include "mpb.h"

#define MPB_LOCAL "mpb_packages"

static void
usage(void)
{
    printf("mpb -- mainframe package bureau\n\n");
    printf("usage: mpb <command> [args]\n\n");
    printf("commands:\n");
    printf("  install <pkg>       install a package\n");
    printf("  remove <pkg>        uninstall a package\n");
    printf("  update <pkg>        pull latest version\n");
    printf("  search <query>      search the registry\n");
    printf("  search              list all packages\n");
    printf("  info <pkg>          show package details\n");
    printf("  list                show installed packages\n");
    printf("  init <name> <lang>  create a new package.json\n\n");
    printf("languages: cobol, hlasm, pli, rexx, jcl\n\n");
    printf("examples:\n");
    printf("  mpb install dateutil\n");
    printf("  mpb install zblas\n");
    printf("  mpb search date\n");
    printf("  mpb init mylib cobol\n");
}

int
main(int argc, char **argv)
{
    if (argc < 2) {
        usage();
        return 0;
    }

    if (strcmp(argv[1], "install") == 0) {
        if (argc < 3) {
            fprintf(stderr, "mpb: install requires a package name\n");
            return 1;
        }
        return mpb_install(argv[2], MPB_LOCAL) == 0 ? 0 : 1;
    }

    if (strcmp(argv[1], "remove") == 0) {
        if (argc < 3) {
            fprintf(stderr, "mpb: remove requires a package name\n");
            return 1;
        }
        return mpb_remove(argv[2], MPB_LOCAL) == 0 ? 0 : 1;
    }

    if (strcmp(argv[1], "update") == 0) {
        if (argc < 3) {
            fprintf(stderr, "mpb: update requires a package name\n");
            return 1;
        }
        return mpb_update(argv[2], MPB_LOCAL) == 0 ? 0 : 1;
    }

    if (strcmp(argv[1], "search") == 0) {
        if (argc < 3)
            return mpb_search_all() >= 0 ? 0 : 1;
        return mpb_search(argv[2]) >= 0 ? 0 : 1;
    }

    if (strcmp(argv[1], "info") == 0) {
        if (argc < 3) {
            fprintf(stderr, "mpb: info requires a package name\n");
            return 1;
        }
        return mpb_info(argv[2]) == 0 ? 0 : 1;
    }

    if (strcmp(argv[1], "list") == 0) {
        return mpb_list(MPB_LOCAL) >= 0 ? 0 : 1;
    }

    if (strcmp(argv[1], "init") == 0) {
        uint8_t lang = MPB_COBOL;
        const char *name = argc >= 3 ? argv[2] : "mypackage";
        if (argc >= 4) {
            if (strcmp(argv[3], "hlasm") == 0) lang = MPB_HLASM;
            else if (strcmp(argv[3], "pli") == 0) lang = MPB_PLI;
            else if (strcmp(argv[3], "rexx") == 0) lang = MPB_REXX;
            else if (strcmp(argv[3], "jcl") == 0) lang = MPB_JCL;
        }
        return mpb_init(name, lang) == 0 ? 0 : 1;
    }

    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
        usage();
        return 0;
    }

    fprintf(stderr, "mpb: unknown command '%s'\n", argv[1]);
    return 1;
}
