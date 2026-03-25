/* mpb_http.c -- HTTP fetch for MPB
 *
 * (c) 2026 Zane Hambly. Apache 2.0.
 */

#include "mpb.h"

int
mpb_http(const char *url, char *buf, int bufsz)
{
    char cmd[1024];
    FILE *fp;
    int n;

    snprintf(cmd, sizeof(cmd),
             "curl -sL -m 15 -H \"User-Agent: mpb/0.1\" \"%s\" 2>/dev/null",
             url);

    fp = popen(cmd, "r");
    if (!fp) return -1;

    n = (int)fread(buf, 1, (size_t)(bufsz - 1), fp);
    pclose(fp);

    if (n <= 0) return -1;
    buf[n] = '\0';
    return n;
}
