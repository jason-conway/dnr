/**
 * @file utf8.c
 * @author Jason Conway (jpc@jasonconway.dev)
 * @brief Minimal UTF-8 handling for `console`
 * @version 0.9.2
 * @date 2022-05-29
 *
 * @copyright Copyright (c) 2022 - 2024 Jason Conway. All rights reserved.
 *
 */

#include "console.h"

static bool search_table(uint32_t c, const codepoint_range_t *table, size_t len)
{
    // Check if within range
    if (c < table[0].start || c > table[len - 1].end) {
        return false;
    }

    // Binary search through the table
    for (size_t bounds[2] = { len - 1, 0 }; bounds[0] >= bounds[1];) {
        size_t avg = (bounds[1] + bounds[0]) / 2;
        if (c > table[avg].end) {
            bounds[1] = avg + 1;
        }
        else if (c < table[avg].start) {
            bounds[0] = avg - 1;
        }
        else {
            return true;
        }
    }
    return false;
}

static size_t cp_rendered_width(uint32_t c)
{
    static const codepoint_range_t ucd_wide[] = {
        { 0x01100, 0x0115f }, { 0x0231a, 0x0231b }, { 0x02329, 0x0232a }, { 0x023e9, 0x023ec },
        { 0x023f0, 0x023f0 }, { 0x023f3, 0x023f3 }, { 0x025fd, 0x025fe }, { 0x02614, 0x02615 },
        { 0x02648, 0x02653 }, { 0x0267f, 0x0267f }, { 0x02693, 0x02693 }, { 0x026a1, 0x026a1 },
        { 0x026aa, 0x026ab }, { 0x026bd, 0x026be }, { 0x026c4, 0x026c5 }, { 0x026ce, 0x026ce },
        { 0x026d4, 0x026d4 }, { 0x026ea, 0x026ea }, { 0x026f2, 0x026f3 }, { 0x026f5, 0x026f5 },
        { 0x026fa, 0x026fa }, { 0x026fd, 0x026fd }, { 0x02705, 0x02705 }, { 0x0270a, 0x0270b },
        { 0x02728, 0x02728 }, { 0x0274c, 0x0274c }, { 0x0274e, 0x0274e }, { 0x02753, 0x02755 },
        { 0x02757, 0x02757 }, { 0x02795, 0x02797 }, { 0x027b0, 0x027b0 }, { 0x027bf, 0x027bf },
        { 0x02b1b, 0x02b1c }, { 0x02b50, 0x02b50 }, { 0x02b55, 0x02b55 }, { 0x02e80, 0x02e99 },
        { 0x02e9b, 0x02ef3 }, { 0x02f00, 0x02fd5 }, { 0x02ff0, 0x02ffb }, { 0x03000, 0x0303e },
        { 0x03041, 0x03096 }, { 0x03099, 0x030ff }, { 0x03105, 0x0312f }, { 0x03131, 0x0318e },
        { 0x03190, 0x031e3 }, { 0x031f0, 0x0321e }, { 0x03220, 0x03247 }, { 0x03250, 0x04dbf },
        { 0x04e00, 0x0a48c }, { 0x0a490, 0x0a4c6 }, { 0x0a960, 0x0a97c }, { 0x0ac00, 0x0d7a3 },
        { 0x0f900, 0x0faff }, { 0x0fe10, 0x0fe19 }, { 0x0fe30, 0x0fe52 }, { 0x0fe54, 0x0fe66 },
        { 0x0fe68, 0x0fe6b }, { 0x0ff01, 0x0ff60 }, { 0x0ffe0, 0x0ffe6 }, { 0x16fe0, 0x16fe4 },
        { 0x16ff0, 0x16ff1 }, { 0x17000, 0x187f7 }, { 0x18800, 0x18cd5 }, { 0x18d00, 0x18d08 },
        { 0x1aff0, 0x1aff3 }, { 0x1aff5, 0x1affb }, { 0x1affd, 0x1affe }, { 0x1b000, 0x1b122 },
        { 0x1b132, 0x1b132 }, { 0x1b150, 0x1b152 }, { 0x1b155, 0x1b155 }, { 0x1b164, 0x1b167 },
        { 0x1b170, 0x1b2fb }, { 0x1f004, 0x1f004 }, { 0x1f0cf, 0x1f0cf }, { 0x1f18e, 0x1f18e },
        { 0x1f191, 0x1f19a }, { 0x1f200, 0x1f202 }, { 0x1f210, 0x1f23b }, { 0x1f240, 0x1f248 },
        { 0x1f250, 0x1f251 }, { 0x1f260, 0x1f265 }, { 0x1f300, 0x1f320 }, { 0x1f32d, 0x1f335 },
        { 0x1f337, 0x1f37c }, { 0x1f37e, 0x1f393 }, { 0x1f3a0, 0x1f3ca }, { 0x1f3cf, 0x1f3d3 },
        { 0x1f3e0, 0x1f3f0 }, { 0x1f3f4, 0x1f3f4 }, { 0x1f3f8, 0x1f43e }, { 0x1f440, 0x1f440 },
        { 0x1f442, 0x1f4fc }, { 0x1f4ff, 0x1f53d }, { 0x1f54b, 0x1f54e }, { 0x1f550, 0x1f567 },
        { 0x1f57a, 0x1f57a }, { 0x1f595, 0x1f596 }, { 0x1f5a4, 0x1f5a4 }, { 0x1f5fb, 0x1f64f },
        { 0x1f680, 0x1f6c5 }, { 0x1f6cc, 0x1f6cc }, { 0x1f6d0, 0x1f6d2 }, { 0x1f6d5, 0x1f6d7 },
        { 0x1f6dc, 0x1f6df }, { 0x1f6eb, 0x1f6ec }, { 0x1f6f4, 0x1f6fc }, { 0x1f7e0, 0x1f7eb },
        { 0x1f7f0, 0x1f7f0 }, { 0x1f90c, 0x1f93a }, { 0x1f93c, 0x1f945 }, { 0x1f947, 0x1f9ff },
        { 0x1fa70, 0x1fa7c }, { 0x1fa80, 0x1fa88 }, { 0x1fa90, 0x1fabd }, { 0x1fabf, 0x1fac5 },
        { 0x1face, 0x1fadb }, { 0x1fae0, 0x1fae8 }, { 0x1faf0, 0x1faf8 }, { 0x20000, 0x2fffd },
        { 0x30000, 0x3fffd }
    };

    static const codepoint_range_t ucd_ambiguous[] = {
        { 0x000a1, 0x000a1 }, { 0x000a4, 0x000a4 }, { 0x000a7, 0x000a8 }, { 0x000aa, 0x000aa },
        { 0x000ad, 0x000ae }, { 0x000b0, 0x000b4 }, { 0x000b6, 0x000ba }, { 0x000bc, 0x000bf },
        { 0x000c6, 0x000c6 }, { 0x000d0, 0x000d0 }, { 0x000d7, 0x000d8 }, { 0x000de, 0x000e1 },
        { 0x000e6, 0x000e6 }, { 0x000e8, 0x000ea }, { 0x000ec, 0x000ed }, { 0x000f0, 0x000f0 },
        { 0x000f2, 0x000f3 }, { 0x000f7, 0x000fa }, { 0x000fc, 0x000fc }, { 0x000fe, 0x000fe },
        { 0x00101, 0x00101 }, { 0x00111, 0x00111 }, { 0x00113, 0x00113 }, { 0x0011b, 0x0011b },
        { 0x00126, 0x00127 }, { 0x0012b, 0x0012b }, { 0x00131, 0x00133 }, { 0x00138, 0x00138 },
        { 0x0013f, 0x00142 }, { 0x00144, 0x00144 }, { 0x00148, 0x0014b }, { 0x0014d, 0x0014d },
        { 0x00152, 0x00153 }, { 0x00166, 0x00167 }, { 0x0016b, 0x0016b }, { 0x001ce, 0x001ce },
        { 0x001d0, 0x001d0 }, { 0x001d2, 0x001d2 }, { 0x001d4, 0x001d4 }, { 0x001d6, 0x001d6 },
        { 0x001d8, 0x001d8 }, { 0x001da, 0x001da }, { 0x001dc, 0x001dc }, { 0x00251, 0x00251 },
        { 0x00261, 0x00261 }, { 0x002c4, 0x002c4 }, { 0x002c7, 0x002c7 }, { 0x002c9, 0x002cb },
        { 0x002cd, 0x002cd }, { 0x002d0, 0x002d0 }, { 0x002d8, 0x002db }, { 0x002dd, 0x002dd },
        { 0x002df, 0x002df }, { 0x00300, 0x0036f }, { 0x00391, 0x003a1 }, { 0x003a3, 0x003a9 },
        { 0x003b1, 0x003c1 }, { 0x003c3, 0x003c9 }, { 0x00401, 0x00401 }, { 0x00410, 0x0044f },
        { 0x00451, 0x00451 }, { 0x02010, 0x02010 }, { 0x02013, 0x02016 }, { 0x02018, 0x02019 },
        { 0x0201c, 0x0201d }, { 0x02020, 0x02022 }, { 0x02024, 0x02027 }, { 0x02030, 0x02030 },
        { 0x02032, 0x02033 }, { 0x02035, 0x02035 }, { 0x0203b, 0x0203b }, { 0x0203e, 0x0203e },
        { 0x02074, 0x02074 }, { 0x0207f, 0x0207f }, { 0x02081, 0x02084 }, { 0x020ac, 0x020ac },
        { 0x02103, 0x02103 }, { 0x02105, 0x02105 }, { 0x02109, 0x02109 }, { 0x02113, 0x02113 },
        { 0x02116, 0x02116 }, { 0x02121, 0x02122 }, { 0x02126, 0x02126 }, { 0x0212b, 0x0212b },
        { 0x02153, 0x02154 }, { 0x0215b, 0x0215e }, { 0x02160, 0x0216b }, { 0x02170, 0x02179 },
        { 0x02189, 0x02189 }, { 0x02190, 0x02199 }, { 0x021b8, 0x021b9 }, { 0x021d2, 0x021d2 },
        { 0x021d4, 0x021d4 }, { 0x021e7, 0x021e7 }, { 0x02200, 0x02200 }, { 0x02202, 0x02203 },
        { 0x02207, 0x02208 }, { 0x0220b, 0x0220b }, { 0x0220f, 0x0220f }, { 0x02211, 0x02211 },
        { 0x02215, 0x02215 }, { 0x0221a, 0x0221a }, { 0x0221d, 0x02220 }, { 0x02223, 0x02223 },
        { 0x02225, 0x02225 }, { 0x02227, 0x0222c }, { 0x0222e, 0x0222e }, { 0x02234, 0x02237 },
        { 0x0223c, 0x0223d }, { 0x02248, 0x02248 }, { 0x0224c, 0x0224c }, { 0x02252, 0x02252 },
        { 0x02260, 0x02261 }, { 0x02264, 0x02267 }, { 0x0226a, 0x0226b }, { 0x0226e, 0x0226f },
        { 0x02282, 0x02283 }, { 0x02286, 0x02287 }, { 0x02295, 0x02295 }, { 0x02299, 0x02299 },
        { 0x022a5, 0x022a5 }, { 0x022bf, 0x022bf }, { 0x02312, 0x02312 }, { 0x02460, 0x024e9 },
        { 0x024eb, 0x0254b }, { 0x02550, 0x02573 }, { 0x02580, 0x0258f }, { 0x02592, 0x02595 },
        { 0x025a0, 0x025a1 }, { 0x025a3, 0x025a9 }, { 0x025b2, 0x025b3 }, { 0x025b6, 0x025b7 },
        { 0x025bc, 0x025bd }, { 0x025c0, 0x025c1 }, { 0x025c6, 0x025c8 }, { 0x025cb, 0x025cb },
        { 0x025ce, 0x025d1 }, { 0x025e2, 0x025e5 }, { 0x025ef, 0x025ef }, { 0x02605, 0x02606 },
        { 0x02609, 0x02609 }, { 0x0260e, 0x0260f }, { 0x0261c, 0x0261c }, { 0x0261e, 0x0261e },
        { 0x02640, 0x02640 }, { 0x02642, 0x02642 }, { 0x02660, 0x02661 }, { 0x02663, 0x02665 },
        { 0x02667, 0x0266a }, { 0x0266c, 0x0266d }, { 0x0266f, 0x0266f }, { 0x0269e, 0x0269f },
        { 0x026bf, 0x026bf }, { 0x026c6, 0x026cd }, { 0x026cf, 0x026d3 }, { 0x026d5, 0x026e1 },
        { 0x026e3, 0x026e3 }, { 0x026e8, 0x026e9 }, { 0x026eb, 0x026f1 }, { 0x026f4, 0x026f4 },
        { 0x026f6, 0x026f9 }, { 0x026fb, 0x026fc }, { 0x026fe, 0x026ff }, { 0x0273d, 0x0273d },
        { 0x02776, 0x0277f }, { 0x02b56, 0x02b59 }, { 0x03248, 0x0324f }, { 0x0e000, 0x0f8ff },
        { 0x0fe00, 0x0fe0f }, { 0x0fffd, 0x0fffd }, { 0x1f100, 0x1f10a }, { 0x1f110, 0x1f12d },
        { 0x1f130, 0x1f169 }, { 0x1f170, 0x1f18d }, { 0x1f18f, 0x1f190 }, { 0x1f19b, 0x1f1ac },
        { 0xe0100, 0xe01ef }, { 0xf0000, 0xffffd }, { 0x100000, 0x10fffd }
    };

    const size_t wide_len = sizeof(ucd_wide) / sizeof(*ucd_wide);
    const size_t ambiguous_len = sizeof(ucd_ambiguous) / sizeof(*ucd_ambiguous);

    // Just ASCII
    if (c >= 0x20 && c <= 0x7e) {
        return 1;
    }
    // Wide characters
    if (search_table(c, ucd_wide, wide_len)) {
        return 2;
    }
    // Ambiguous characters
    if (search_table(c, ucd_ambiguous, ambiguous_len)) {
        return 2;
    }

    return 1;
}

static uint32_t utf8_to_utf32(const uint8_t *c, size_t cp_size)
{
    switch (cp_size) {
        case 0:
            return 0;
        case 1:
            return *c;
        case 2:
            return ((c[0] & 0x1f) << 0x06) |
                   ((c[1] & 0x3f) << 0x00);
        case 3:
            return ((c[0] & 0x0f) << 0x0c) |
                   ((c[1] & 0x3f) << 0x06) |
                   ((c[2] & 0x3f) << 0x00);
        case 4:
            return ((c[0] & 0x07) << 0x12) |
                   ((c[1] & 0x3f) << 0x0c) |
                   ((c[2] & 0x3f) << 0x06) |
                   ((c[3] & 0x3f) << 0x00);
    }
    return 0;
}

size_t codepoint_width(const char *str, size_t len)
{
    const uint8_t *s = (const uint8_t *)str;
    const uint32_t cp = utf8_to_utf32(s, len);
    return cp_rendered_width(cp);
}

char *prev_codepoint(const char *str, size_t *cp_size, size_t *cp_len)
{
    const char *s = str;
    size_t cp_bytes = 0;

    do {
        cp_bytes++;
        s--;
    } while ((s[0] & 0x80) && ((s[0] & 0xc0) == 0x80));

    if (cp_size) {
        *cp_size = cp_bytes;
    }
    if (cp_len) {
        *cp_len = codepoint_width(s, cp_bytes);
    }

    return (char *)s;
}

char *next_codepoint(const char *str, size_t *cp_size, size_t *cp_len)
{
    size_t cp_bytes = 0;
    if ((str[0] & 0xf8) == 0xf0) { // 11110xxx
        cp_bytes = 4;
    }
    else if ((str[0] & 0xf0) == 0xe0) { // 1110xxxx
        cp_bytes = 3;
    }
    else if ((str[0] & 0xe0) == 0xc0) { // 110xxxxx
        cp_bytes = 2;
    }
    else {
        cp_bytes = 1;
    }

    if (cp_size) {
        *cp_size = cp_bytes;
    }
    if (cp_len) {
        *cp_len = codepoint_width(str, cp_bytes);
    }

    return (char *)(str + cp_bytes);
}

// Length of rendered unicode string
static size_t utf8_strnlen(const char *str, size_t len)
{
    const char *s = str;
    size_t length = 0;

    while ((size_t)(str - s) < len && str[0]) {
        if ((str[0] & 0xf8) == 0xf0) { // 11110xxx
            str += 4;
        }
        else if ((str[0] & 0xf0) == 0xe0) { // 1110xxxx
            str += 3;
        }
        else if ((str[0] & 0xe0) == 0xc0) { // 110xxxxx
            str += 2;
        }
        else {
            str += 1;
        }

        length++;
    }

    return ((size_t)(str - s) > len) ? length - 1 : length;
}

/**
 * @brief utf8_strnlen() capable of handling strings containing escape sequences
 *
 * @param[in] str Input string
 * @return Number of "cells" required to render `str`
 */
size_t utf8_rendered_length(const char *str)
{
    // Rendered length will be <= the number of bytes in `str`
    char *stripped = xcalloc(strlen(str));

    // Create a copy of the original, but with escape sequences stripped away
    size_t len = 0;
    for (size_t i = 0; str[i]; i++) {
        if (str[i] == ESC) {
            for (; str[i] != 'm'; i++) {
            };
            i++; // Increment past 'm' before copying to `stripped`
        }
        if (!str[i]) {
            break;
        }
        stripped[len++] = str[i];
    }

    len = utf8_strnlen(stripped, len);
    xfree(stripped);
    return len;
}
