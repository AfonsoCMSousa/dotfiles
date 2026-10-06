// netspark - network RX/TX sparkline for a polybar `tail = true` module.
//
// Reads /proc/net/dev once a second and prints one line per tick:
//   ↓▁▂▅█▃▁… ↑▁▁▂▁▁▁…
// New samples enter on the right and scroll left.
//
// Bars use a fixed log scale (FLOOR_BPS .. CEIL_BPS) rather than the
// window max, so one spike doesn't flatten everything else and idle
// background noise doesn't get blown up into full bars.
//
// Build: cc -O2 -o netspark netspark.c -lm
// Usage: netspark [interface]   (default: enp6s0)

#define _POSIX_C_SOURCE 200809L
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define WIDTH     20
#define FLOOR_BPS 1e3 /* at or below this: lowest bar */
#define CEIL_BPS  1e8 /* at or above this: full bar   */

static const char *bars[] = {"▁", "▂", "▃", "▄", "▅", "▆", "▇", "█"};

static int read_counters(const char *iface, unsigned long long *rx,
                         unsigned long long *tx)
{
    FILE *f = fopen("/proc/net/dev", "r");
    if (!f)
        return -1;

    char line[512];
    size_t len = strlen(iface);
    int ret = -1;

    while (fgets(line, sizeof line, f)) {
        char *p = line;
        while (*p == ' ')
            p++;
        if (strncmp(p, iface, len) != 0 || p[len] != ':')
            continue;
        /* rx_bytes is the 1st field after the colon, tx_bytes the 9th */
        if (sscanf(p + len + 1, "%llu %*s %*s %*s %*s %*s %*s %*s %llu",
                   rx, tx) == 2)
            ret = 0;
        break;
    }

    fclose(f);
    return ret;
}

static int level(double bps)
{
    if (bps <= FLOOR_BPS)
        return 0;
    double t = log10(bps / FLOOR_BPS) / log10(CEIL_BPS / FLOOR_BPS);
    int l = (int)(t * 7 + 0.5);
    return l > 7 ? 7 : l;
}

static double seconds_between(struct timespec a, struct timespec b)
{
    return (b.tv_sec - a.tv_sec) + (b.tv_nsec - a.tv_nsec) / 1e9;
}

static void print_missing(const char *iface)
{
    printf("%%{F#5a6570}no %s%%{F-}\n", iface);
    fflush(stdout);
}

int main(int argc, char **argv)
{
    const char *iface = argc > 1 ? argv[1] : "enp6s0";
    const struct timespec tick = {1, 0};

    double rxh[WIDTH] = {0}, txh[WIDTH] = {0};
    int head = 0; /* index of the oldest sample */

    unsigned long long prx, ptx, rx, tx;
    struct timespec pt, now;

    while (read_counters(iface, &prx, &ptx) != 0) {
        print_missing(iface);
        nanosleep(&(struct timespec){5, 0}, NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &pt);

    for (;;) {
        nanosleep(&tick, NULL);

        if (read_counters(iface, &rx, &tx) != 0) {
            print_missing(iface);
            continue;
        }
        clock_gettime(CLOCK_MONOTONIC, &now);

        double dt = seconds_between(pt, now);
        /* counters going backwards means the interface was reset */
        rxh[head] = rx >= prx ? (rx - prx) / dt : 0;
        txh[head] = tx >= ptx ? (tx - ptx) / dt : 0;
        head = (head + 1) % WIDTH;
        prx = rx;
        ptx = tx;
        pt = now;

        /* %{T8} is font-7 in config.ini: the block characters' size */
        fputs("%{F#9da2ab}↓ %{T8}%{F#c9f299}", stdout);
        for (int i = 0; i < WIDTH; i++)
            fputs(bars[level(rxh[(head + i) % WIDTH])], stdout);
        fputs("%{T-}  %{F#9da2ab}↑ %{T8}%{F#ff0048}", stdout);
        for (int i = 0; i < WIDTH; i++)
            fputs(bars[level(txh[(head + i) % WIDTH])], stdout);
        fputs("%{T-}%{F-}\n", stdout);

        /* stdout is a pipe to polybar, so it's fully buffered otherwise */
        fflush(stdout);
    }
}
