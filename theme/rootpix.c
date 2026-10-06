// rootpix: read the desktop background that's actually on screen.
//
// Wallpaper setters (nitrogen, feh, ...) publish the background pixmap on the
// root window as _XROOTPMAP_ID. Reading it directly gets the wallpaper
// without the windows on top, whether or not the setter saved anything.
//
//   rootpix id          print the pixmap id
//   rootpix hash        print a fingerprint of its pixels (changes with the wallpaper)
//   rootpix dump FILE   write it as a binary PPM, downscaled 8x (palette input)
//
// Built on demand by theme/theme into ~/.cache/theme; not tracked in git.

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xcb/xcb.h>

static xcb_pixmap_t root_pixmap(xcb_connection_t *c, xcb_window_t root)
{
    xcb_intern_atom_reply_t *a = xcb_intern_atom_reply(
        c, xcb_intern_atom(c, 1, strlen("_XROOTPMAP_ID"), "_XROOTPMAP_ID"), NULL);
    if (!a || a->atom == XCB_NONE)
        return XCB_NONE;
    xcb_get_property_reply_t *p = xcb_get_property_reply(
        c, xcb_get_property(c, 0, root, a->atom, XCB_ATOM_PIXMAP, 0, 1), NULL);
    free(a);
    xcb_pixmap_t pix = XCB_NONE;
    if (p && xcb_get_property_value_length(p) == 4)
        pix = *(xcb_pixmap_t *)xcb_get_property_value(p);
    free(p);
    return pix;
}

int main(int argc, char **argv)
{
    if (argc < 2)
        return 2;
    xcb_connection_t *c = xcb_connect(NULL, NULL);
    if (xcb_connection_has_error(c))
        return 1;
    xcb_screen_t *s = xcb_setup_roots_iterator(xcb_get_setup(c)).data;
    xcb_pixmap_t pix = root_pixmap(c, s->root);
    if (pix == XCB_NONE)
        return 1;

    if (!strcmp(argv[1], "id")) {
        printf("0x%x\n", pix);
        return 0;
    }

    xcb_get_geometry_reply_t *g = xcb_get_geometry_reply(c, xcb_get_geometry(c, pix), NULL);
    if (!g)
        return 1;
    int w = g->width, h = g->height;
    free(g);

    if (!strcmp(argv[1], "hash")) {
        // Setters like nitrogen redraw into the same pixmap, so the id alone
        // doesn't change: fingerprint a sparse grid of its pixels instead.
        uint64_t hsh = 1469598103934665603ULL; // FNV-1a
        for (int y = 0; y < h; y += 64) {
            xcb_get_image_reply_t *img = xcb_get_image_reply(
                c, xcb_get_image(c, XCB_IMAGE_FORMAT_Z_PIXMAP, pix, 0, y, w, 1, ~0u), NULL);
            if (!img)
                return 1;
            const unsigned char *d = xcb_get_image_data(img);
            for (int x = 0; x < w; x += 16)
                for (int k = 0; k < 3; k++) {
                    hsh ^= d[4 * x + k];
                    hsh *= 1099511628211ULL;
                }
            free(img);
        }
        printf("%016llx\n", (unsigned long long)hsh);
        return 0;
    }

    if (strcmp(argv[1], "dump") || argc < 3)
        return 2;
    int step = 8;

    FILE *f = fopen(argv[2], "wb");
    if (!f)
        return 1;
    fprintf(f, "P6\n%d %d\n255\n", w / step, h / step);

    // one row at a time keeps each request small; keep every 8th pixel
    unsigned char *line = malloc(3 * (w / step));
    for (int y = 0; y + step <= h; y += step) {
        xcb_get_image_reply_t *img = xcb_get_image_reply(
            c, xcb_get_image(c, XCB_IMAGE_FORMAT_Z_PIXMAP, pix, 0, y, w, 1, ~0u), NULL);
        if (!img) {
            fclose(f);
            return 1;
        }
        const unsigned char *d = xcb_get_image_data(img); // depth 24: B G R X
        for (int x = 0; x < w / step; x++) {
            const unsigned char *px = d + 4 * (x * step);
            line[3 * x + 0] = px[2];
            line[3 * x + 1] = px[1];
            line[3 * x + 2] = px[0];
        }
        fwrite(line, 3, w / step, f);
        free(img);
    }
    free(line);
    return fclose(f) ? 1 : 0;
}
