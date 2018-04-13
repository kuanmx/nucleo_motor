/*
 * Copyright (c) 2014-2015 ARM Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "common_functions.h"
#include "ip6string.h"

static uint16_t hex(const char *p);

/**
 * Convert numeric IPv6 address string to a binary.
 * IPv4 tunnelling addresses are not covered.
 * \param ip6addr IPv6 address in string format.
 * \param len Length of ipv6 string.
 * \param dest buffer for address. MUST be 16 bytes.
 */
void stoip6(const char *ip6addr, size_t len, void *dest)
{
    uint8_t *addr;
    const char *p, *q;
    int_fast8_t field_no, coloncolon = -1;

    addr = dest;

    if (len > 39) { // Too long, not possible. We do not support IPv4-mapped IPv6 addresses
        return;
    }

    // First go forward the string, until end, noting :: position if any
    for (field_no = 0, p = ip6addr; (len > (size_t)(p - ip6addr)) && *p && field_no < 8; p = q + 1) {
        q = p;
        // Seek for ':' or end
        while (*q && (*q != ':')) {
            q++;
        }
        //Convert and write this part, (high-endian AKA network byte order)
        addr = common_write_16_bit(hex(p), addr);
        field_no++;
        //Check if we reached "::"
        if ((len > (size_t)(q - ip6addr)) && *q && (q[0] == ':') && (q[1] == ':')) {
            coloncolon = field_no;
            q++;
        }
    }

    if (coloncolon != -1) {
        /* Insert zeros in the appropriate place */
        uint_fast8_t head_size = 2 * coloncolon;
        uint_fast8_t inserted_size = 2 * (8 - field_no);
        uint_fast8_t tail_size = 16 - head_size - inserted_size;
        addr = dest;
        memmove(addr + head_size + inserted_size, addr + head_size, tail_size);
        memset(addr + head_size, 0, inserted_size);
    } else if (field_no != 8) {
        /* Should really report an error if we didn't get 8 fields */
        memset(addr, 0, 16 - field_no * 2);
    }
}
unsigned char  sipv6_prefixlength(const char *ip6addr)
{
    char *ptr = strchr(ip6addr, '/');
    if (ptr) {
        return (unsigned char)strtoul(ptr + 1, 0, 10);
    }
    return 0;
}
static uint16_t hex(const char *p)
{
    uint16_t val = 0;

    for (;;) {
        char c = *p++;
        if ((c >= '0') && (c <= '9')) {
            val = (val << 4) | (c - '0');
        } else if ((c >= 'A') && (c <= 'F')) {
            val = (val << 4) | (10 + (c - 'A'));
        } else if ((c >= 'a') && (c <= 'f')) {
            val = (val << 4) | (10 + (c - 'a'));
        } else {
            break; // Non hex character
        }
    }
    return val;
}
