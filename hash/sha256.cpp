/*
 * This file is part of the VanitySearch distribution (https://github.com/JeanLucPons/VanitySearch).
 * Copyright (c) 2019 Jean Luc PONS.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string>
#ifdef __AVX2__
#include <immintrin.h>
#endif

#include "sha256.h"

#define BSWAP

/// Internal SHA-256 implementation.
namespace _sha256
{

  static const unsigned char pad[64] = { 0x80 };

#ifndef WIN64
#define _byteswap_ulong __builtin_bswap32
#define _byteswap_uint64 __builtin_bswap64
inline uint32_t _rotr32(uint32_t x, uint8_t r) {
  asm("rorl %1,%0" : "+r" (x) : "c" (r));
  return x;
}
#endif

#define ROR(x,n) _rotr32(x, n)
#define S0(x) (ROR(x,2) ^ ROR(x,13) ^ ROR(x,22))
#define S1(x) (ROR(x,6) ^ ROR(x,11) ^ ROR(x,25))
#define s0(x) (ROR(x,7) ^ ROR(x,18) ^ (x >> 3))
#define s1(x) (ROR(x,17) ^ ROR(x,19) ^ (x >> 10))

#define Maj(x,y,z) ((x&y)^(x&z)^(y&z))
//#define Ch(x,y,z)  ((x&y)^(~x&z))

// The following functions are equivalent to the above
//#define Maj(x,y,z) ((x & y) | (z & (x | y)))
#define Ch(x,y,z) (z ^ (x & (y ^ z)))

// SHA-256 round
#define Round(a, b, c, d, e, f, g, h, k, w) \
    t1 = h + S1(e) + Ch(e,f,g) + k + (w); \
    t2 = S0(a) + Maj(a,b,c); \
    d += t1; \
    h = t1 + t2;

#ifdef BSWAP
#define WRITEBE32(ptr,x) *((uint32_t *)(ptr)) = _byteswap_ulong(x)
#define WRITEBE64(ptr,x) *((uint64_t *)(ptr)) = _byteswap_uint64(x)
#define READBE32(ptr) (uint32_t)_byteswap_ulong(*(uint32_t *)(ptr))
#else
#define WRITEBE32(ptr,x) *(ptr) = x
#define WRITEBE64(ptr,x) *(ptr) = x
#define READBE32(ptr) *(uint32_t *)(ptr)
#endif

  // Initialise state
  void Initialize(uint32_t *s) {

    s[0] = 0x6a09e667ul;
    s[1] = 0xbb67ae85ul;
    s[2] = 0x3c6ef372ul;
    s[3] = 0xa54ff53aul;
    s[4] = 0x510e527ful;
    s[5] = 0x9b05688cul;
    s[6] = 0x1f83d9abul;
    s[7] = 0x5be0cd19ul;

  }


  // Perform SHA-256 transformations, process 64-byte chunks
  void Transform(uint32_t* s, const unsigned char* chunk)
  {
    uint32_t t1;
    uint32_t t2;
    uint32_t a = s[0], b = s[1], c = s[2], d = s[3], e = s[4], f = s[5], g = s[6], h = s[7];
    uint32_t w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10, w11, w12, w13, w14, w15;

    Round(a, b, c, d, e, f, g, h, 0x428a2f98, w0 = READBE32(chunk + 0));
    Round(h, a, b, c, d, e, f, g, 0x71374491, w1 = READBE32(chunk + 4));
    Round(g, h, a, b, c, d, e, f, 0xb5c0fbcf, w2 = READBE32(chunk + 8));
    Round(f, g, h, a, b, c, d, e, 0xe9b5dba5, w3 = READBE32(chunk + 12));
    Round(e, f, g, h, a, b, c, d, 0x3956c25b, w4 = READBE32(chunk + 16));
    Round(d, e, f, g, h, a, b, c, 0x59f111f1, w5 = READBE32(chunk + 20));
    Round(c, d, e, f, g, h, a, b, 0x923f82a4, w6 = READBE32(chunk + 24));
    Round(b, c, d, e, f, g, h, a, 0xab1c5ed5, w7 = READBE32(chunk + 28));
    Round(a, b, c, d, e, f, g, h, 0xd807aa98, w8 = READBE32(chunk + 32));
    Round(h, a, b, c, d, e, f, g, 0x12835b01, w9 = READBE32(chunk + 36));
    Round(g, h, a, b, c, d, e, f, 0x243185be, w10 = READBE32(chunk + 40));
    Round(f, g, h, a, b, c, d, e, 0x550c7dc3, w11 = READBE32(chunk + 44));
    Round(e, f, g, h, a, b, c, d, 0x72be5d74, w12 = READBE32(chunk + 48));
    Round(d, e, f, g, h, a, b, c, 0x80deb1fe, w13 = READBE32(chunk + 52));
    Round(c, d, e, f, g, h, a, b, 0x9bdc06a7, w14 = READBE32(chunk + 56));
    Round(b, c, d, e, f, g, h, a, 0xc19bf174, w15 = READBE32(chunk + 60));

    Round(a, b, c, d, e, f, g, h, 0xe49b69c1, w0 += s1(w14) + w9 + s0(w1));
    Round(h, a, b, c, d, e, f, g, 0xefbe4786, w1 += s1(w15) + w10 + s0(w2));
    Round(g, h, a, b, c, d, e, f, 0x0fc19dc6, w2 += s1(w0) + w11 + s0(w3));
    Round(f, g, h, a, b, c, d, e, 0x240ca1cc, w3 += s1(w1) + w12 + s0(w4));
    Round(e, f, g, h, a, b, c, d, 0x2de92c6f, w4 += s1(w2) + w13 + s0(w5));
    Round(d, e, f, g, h, a, b, c, 0x4a7484aa, w5 += s1(w3) + w14 + s0(w6));
    Round(c, d, e, f, g, h, a, b, 0x5cb0a9dc, w6 += s1(w4) + w15 + s0(w7));
    Round(b, c, d, e, f, g, h, a, 0x76f988da, w7 += s1(w5) + w0 + s0(w8));
    Round(a, b, c, d, e, f, g, h, 0x983e5152, w8 += s1(w6) + w1 + s0(w9));
    Round(h, a, b, c, d, e, f, g, 0xa831c66d, w9 += s1(w7) + w2 + s0(w10));
    Round(g, h, a, b, c, d, e, f, 0xb00327c8, w10 += s1(w8) + w3 + s0(w11));
    Round(f, g, h, a, b, c, d, e, 0xbf597fc7, w11 += s1(w9) + w4 + s0(w12));
    Round(e, f, g, h, a, b, c, d, 0xc6e00bf3, w12 += s1(w10) + w5 + s0(w13));
    Round(d, e, f, g, h, a, b, c, 0xd5a79147, w13 += s1(w11) + w6 + s0(w14));
    Round(c, d, e, f, g, h, a, b, 0x06ca6351, w14 += s1(w12) + w7 + s0(w15));
    Round(b, c, d, e, f, g, h, a, 0x14292967, w15 += s1(w13) + w8 + s0(w0));

    Round(a, b, c, d, e, f, g, h, 0x27b70a85, w0 += s1(w14) + w9 + s0(w1));
    Round(h, a, b, c, d, e, f, g, 0x2e1b2138, w1 += s1(w15) + w10 + s0(w2));
    Round(g, h, a, b, c, d, e, f, 0x4d2c6dfc, w2 += s1(w0) + w11 + s0(w3));
    Round(f, g, h, a, b, c, d, e, 0x53380d13, w3 += s1(w1) + w12 + s0(w4));
    Round(e, f, g, h, a, b, c, d, 0x650a7354, w4 += s1(w2) + w13 + s0(w5));
    Round(d, e, f, g, h, a, b, c, 0x766a0abb, w5 += s1(w3) + w14 + s0(w6));
    Round(c, d, e, f, g, h, a, b, 0x81c2c92e, w6 += s1(w4) + w15 + s0(w7));
    Round(b, c, d, e, f, g, h, a, 0x92722c85, w7 += s1(w5) + w0 + s0(w8));
    Round(a, b, c, d, e, f, g, h, 0xa2bfe8a1, w8 += s1(w6) + w1 + s0(w9));
    Round(h, a, b, c, d, e, f, g, 0xa81a664b, w9 += s1(w7) + w2 + s0(w10));
    Round(g, h, a, b, c, d, e, f, 0xc24b8b70, w10 += s1(w8) + w3 + s0(w11));
    Round(f, g, h, a, b, c, d, e, 0xc76c51a3, w11 += s1(w9) + w4 + s0(w12));
    Round(e, f, g, h, a, b, c, d, 0xd192e819, w12 += s1(w10) + w5 + s0(w13));
    Round(d, e, f, g, h, a, b, c, 0xd6990624, w13 += s1(w11) + w6 + s0(w14));
    Round(c, d, e, f, g, h, a, b, 0xf40e3585, w14 += s1(w12) + w7 + s0(w15));
    Round(b, c, d, e, f, g, h, a, 0x106aa070, w15 += s1(w13) + w8 + s0(w0));

    Round(a, b, c, d, e, f, g, h, 0x19a4c116, w0 += s1(w14) + w9 + s0(w1));
    Round(h, a, b, c, d, e, f, g, 0x1e376c08, w1 += s1(w15) + w10 + s0(w2));
    Round(g, h, a, b, c, d, e, f, 0x2748774c, w2 += s1(w0) + w11 + s0(w3));
    Round(f, g, h, a, b, c, d, e, 0x34b0bcb5, w3 += s1(w1) + w12 + s0(w4));
    Round(e, f, g, h, a, b, c, d, 0x391c0cb3, w4 += s1(w2) + w13 + s0(w5));
    Round(d, e, f, g, h, a, b, c, 0x4ed8aa4a, w5 += s1(w3) + w14 + s0(w6));
    Round(c, d, e, f, g, h, a, b, 0x5b9cca4f, w6 += s1(w4) + w15 + s0(w7));
    Round(b, c, d, e, f, g, h, a, 0x682e6ff3, w7 += s1(w5) + w0 + s0(w8));
    Round(a, b, c, d, e, f, g, h, 0x748f82ee, w8 += s1(w6) + w1 + s0(w9));
    Round(h, a, b, c, d, e, f, g, 0x78a5636f, w9 += s1(w7) + w2 + s0(w10));
    Round(g, h, a, b, c, d, e, f, 0x84c87814, w10 += s1(w8) + w3 + s0(w11));
    Round(f, g, h, a, b, c, d, e, 0x8cc70208, w11 += s1(w9) + w4 + s0(w12));
    Round(e, f, g, h, a, b, c, d, 0x90befffa, w12 += s1(w10) + w5 + s0(w13));
    Round(d, e, f, g, h, a, b, c, 0xa4506ceb, w13 += s1(w11) + w6 + s0(w14));
    Round(c, d, e, f, g, h, a, b, 0xbef9a3f7, w14 + s1(w12) + w7 + s0(w15));
    Round(b, c, d, e, f, g, h, a, 0xc67178f2, w15 + s1(w13) + w8 + s0(w0));

    s[0] += a;
    s[1] += b;
    s[2] += c;
    s[3] += d;
    s[4] += e;
    s[5] += f;
    s[6] += g;
    s[7] += h;

  }

  // Compute SHA256(SHA256(chunk))[0]
  void Transform2(uint32_t* s, const unsigned char* chunk) {

    uint32_t t1;
    uint32_t t2;
    uint32_t w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10, w11, w12, w13, w14, w15;

    uint32_t a = 0x6a09e667ul;
    uint32_t b = 0xbb67ae85ul;
    uint32_t c = 0x3c6ef372ul;
    uint32_t d = 0xa54ff53aul;
    uint32_t e = 0x510e527ful;
    uint32_t f = 0x9b05688cul;
    uint32_t g = 0x1f83d9abul;
    uint32_t h = 0x5be0cd19ul;

    Round(a, b, c, d, e, f, g, h, 0x428a2f98, w0 = READBE32(chunk + 0));
    Round(h, a, b, c, d, e, f, g, 0x71374491, w1 = READBE32(chunk + 4));
    Round(g, h, a, b, c, d, e, f, 0xb5c0fbcf, w2 = READBE32(chunk + 8));
    Round(f, g, h, a, b, c, d, e, 0xe9b5dba5, w3 = READBE32(chunk + 12));
    Round(e, f, g, h, a, b, c, d, 0x3956c25b, w4 = READBE32(chunk + 16));
    Round(d, e, f, g, h, a, b, c, 0x59f111f1, w5 = READBE32(chunk + 20));
    Round(c, d, e, f, g, h, a, b, 0x923f82a4, w6 = READBE32(chunk + 24));
    Round(b, c, d, e, f, g, h, a, 0xab1c5ed5, w7 = READBE32(chunk + 28));
    Round(a, b, c, d, e, f, g, h, 0xd807aa98, w8 = READBE32(chunk + 32));
    Round(h, a, b, c, d, e, f, g, 0x12835b01, w9 = READBE32(chunk + 36));
    Round(g, h, a, b, c, d, e, f, 0x243185be, w10 = READBE32(chunk + 40));
    Round(f, g, h, a, b, c, d, e, 0x550c7dc3, w11 = READBE32(chunk + 44));
    Round(e, f, g, h, a, b, c, d, 0x72be5d74, w12 = READBE32(chunk + 48));
    Round(d, e, f, g, h, a, b, c, 0x80deb1fe, w13 = READBE32(chunk + 52));
    Round(c, d, e, f, g, h, a, b, 0x9bdc06a7, w14 = READBE32(chunk + 56));
    Round(b, c, d, e, f, g, h, a, 0xc19bf174, w15 = READBE32(chunk + 60));

    Round(a, b, c, d, e, f, g, h, 0xe49b69c1, w0 += s1(w14) + w9 + s0(w1));
    Round(h, a, b, c, d, e, f, g, 0xefbe4786, w1 += s1(w15) + w10 + s0(w2));
    Round(g, h, a, b, c, d, e, f, 0x0fc19dc6, w2 += s1(w0) + w11 + s0(w3));
    Round(f, g, h, a, b, c, d, e, 0x240ca1cc, w3 += s1(w1) + w12 + s0(w4));
    Round(e, f, g, h, a, b, c, d, 0x2de92c6f, w4 += s1(w2) + w13 + s0(w5));
    Round(d, e, f, g, h, a, b, c, 0x4a7484aa, w5 += s1(w3) + w14 + s0(w6));
    Round(c, d, e, f, g, h, a, b, 0x5cb0a9dc, w6 += s1(w4) + w15 + s0(w7));
    Round(b, c, d, e, f, g, h, a, 0x76f988da, w7 += s1(w5) + w0 + s0(w8));
    Round(a, b, c, d, e, f, g, h, 0x983e5152, w8 += s1(w6) + w1 + s0(w9));
    Round(h, a, b, c, d, e, f, g, 0xa831c66d, w9 += s1(w7) + w2 + s0(w10));
    Round(g, h, a, b, c, d, e, f, 0xb00327c8, w10 += s1(w8) + w3 + s0(w11));
    Round(f, g, h, a, b, c, d, e, 0xbf597fc7, w11 += s1(w9) + w4 + s0(w12));
    Round(e, f, g, h, a, b, c, d, 0xc6e00bf3, w12 += s1(w10) + w5 + s0(w13));
    Round(d, e, f, g, h, a, b, c, 0xd5a79147, w13 += s1(w11) + w6 + s0(w14));
    Round(c, d, e, f, g, h, a, b, 0x06ca6351, w14 += s1(w12) + w7 + s0(w15));
    Round(b, c, d, e, f, g, h, a, 0x14292967, w15 += s1(w13) + w8 + s0(w0));

    Round(a, b, c, d, e, f, g, h, 0x27b70a85, w0 += s1(w14) + w9 + s0(w1));
    Round(h, a, b, c, d, e, f, g, 0x2e1b2138, w1 += s1(w15) + w10 + s0(w2));
    Round(g, h, a, b, c, d, e, f, 0x4d2c6dfc, w2 += s1(w0) + w11 + s0(w3));
    Round(f, g, h, a, b, c, d, e, 0x53380d13, w3 += s1(w1) + w12 + s0(w4));
    Round(e, f, g, h, a, b, c, d, 0x650a7354, w4 += s1(w2) + w13 + s0(w5));
    Round(d, e, f, g, h, a, b, c, 0x766a0abb, w5 += s1(w3) + w14 + s0(w6));
    Round(c, d, e, f, g, h, a, b, 0x81c2c92e, w6 += s1(w4) + w15 + s0(w7));
    Round(b, c, d, e, f, g, h, a, 0x92722c85, w7 += s1(w5) + w0 + s0(w8));
    Round(a, b, c, d, e, f, g, h, 0xa2bfe8a1, w8 += s1(w6) + w1 + s0(w9));
    Round(h, a, b, c, d, e, f, g, 0xa81a664b, w9 += s1(w7) + w2 + s0(w10));
    Round(g, h, a, b, c, d, e, f, 0xc24b8b70, w10 += s1(w8) + w3 + s0(w11));
    Round(f, g, h, a, b, c, d, e, 0xc76c51a3, w11 += s1(w9) + w4 + s0(w12));
    Round(e, f, g, h, a, b, c, d, 0xd192e819, w12 += s1(w10) + w5 + s0(w13));
    Round(d, e, f, g, h, a, b, c, 0xd6990624, w13 += s1(w11) + w6 + s0(w14));
    Round(c, d, e, f, g, h, a, b, 0xf40e3585, w14 += s1(w12) + w7 + s0(w15));
    Round(b, c, d, e, f, g, h, a, 0x106aa070, w15 += s1(w13) + w8 + s0(w0));

    Round(a, b, c, d, e, f, g, h, 0x19a4c116, w0 += s1(w14) + w9 + s0(w1));
    Round(h, a, b, c, d, e, f, g, 0x1e376c08, w1 += s1(w15) + w10 + s0(w2));
    Round(g, h, a, b, c, d, e, f, 0x2748774c, w2 += s1(w0) + w11 + s0(w3));
    Round(f, g, h, a, b, c, d, e, 0x34b0bcb5, w3 += s1(w1) + w12 + s0(w4));
    Round(e, f, g, h, a, b, c, d, 0x391c0cb3, w4 += s1(w2) + w13 + s0(w5));
    Round(d, e, f, g, h, a, b, c, 0x4ed8aa4a, w5 += s1(w3) + w14 + s0(w6));
    Round(c, d, e, f, g, h, a, b, 0x5b9cca4f, w6 += s1(w4) + w15 + s0(w7));
    Round(b, c, d, e, f, g, h, a, 0x682e6ff3, w7 += s1(w5) + w0 + s0(w8));
    Round(a, b, c, d, e, f, g, h, 0x748f82ee, w8 += s1(w6) + w1 + s0(w9));
    Round(h, a, b, c, d, e, f, g, 0x78a5636f, w9 += s1(w7) + w2 + s0(w10));
    Round(g, h, a, b, c, d, e, f, 0x84c87814, w10 += s1(w8) + w3 + s0(w11));
    Round(f, g, h, a, b, c, d, e, 0x8cc70208, w11 += s1(w9) + w4 + s0(w12));
    Round(e, f, g, h, a, b, c, d, 0x90befffa, w12 += s1(w10) + w5 + s0(w13));
    Round(d, e, f, g, h, a, b, c, 0xa4506ceb, w13 += s1(w11) + w6 + s0(w14));
    Round(c, d, e, f, g, h, a, b, 0xbef9a3f7, w14 + s1(w12) + w7 + s0(w15));
    Round(b, c, d, e, f, g, h, a, 0xc67178f2, w15 + s1(w13) + w8 + s0(w0));

    w0 = 0x6a09e667ul + a;
    w1 = 0xbb67ae85ul + b;
    w2 = 0x3c6ef372ul + c;
    w3 = 0xa54ff53aul + d;
    w4 = 0x510e527ful + e;
    w5 = 0x9b05688cul + f;
    w6 = 0x1f83d9abul + g;
    w7 = 0x5be0cd19ul + h;
    w8 = 0x80000000;
    w9 = 0;
    w10 = 0;
    w11 = 0;
    w12 = 0;
    w13 = 0;
    w14 = 0;
    w15 = 0x100;

    a = 0x6a09e667ul;
    b = 0xbb67ae85ul;
    c = 0x3c6ef372ul;
    d = 0xa54ff53aul;
    e = 0x510e527ful;
    f = 0x9b05688cul;
    g = 0x1f83d9abul;
    h = 0x5be0cd19ul;

    Round(a, b, c, d, e, f, g, h, 0x428a2f98, w0);
    Round(h, a, b, c, d, e, f, g, 0x71374491, w1);
    Round(g, h, a, b, c, d, e, f, 0xb5c0fbcf, w2);
    Round(f, g, h, a, b, c, d, e, 0xe9b5dba5, w3);
    Round(e, f, g, h, a, b, c, d, 0x3956c25b, w4);
    Round(d, e, f, g, h, a, b, c, 0x59f111f1, w5);
    Round(c, d, e, f, g, h, a, b, 0x923f82a4, w6);
    Round(b, c, d, e, f, g, h, a, 0xab1c5ed5, w7);
    Round(a, b, c, d, e, f, g, h, 0xd807aa98, w8);
    Round(h, a, b, c, d, e, f, g, 0x12835b01, w9);
    Round(g, h, a, b, c, d, e, f, 0x243185be, w10);
    Round(f, g, h, a, b, c, d, e, 0x550c7dc3, w11);
    Round(e, f, g, h, a, b, c, d, 0x72be5d74, w12);
    Round(d, e, f, g, h, a, b, c, 0x80deb1fe, w13);
    Round(c, d, e, f, g, h, a, b, 0x9bdc06a7, w14);
    Round(b, c, d, e, f, g, h, a, 0xc19bf174, w15);

    Round(a, b, c, d, e, f, g, h, 0xe49b69c1, w0 += s1(w14) + w9 + s0(w1));
    Round(h, a, b, c, d, e, f, g, 0xefbe4786, w1 += s1(w15) + w10 + s0(w2));
    Round(g, h, a, b, c, d, e, f, 0x0fc19dc6, w2 += s1(w0) + w11 + s0(w3));
    Round(f, g, h, a, b, c, d, e, 0x240ca1cc, w3 += s1(w1) + w12 + s0(w4));
    Round(e, f, g, h, a, b, c, d, 0x2de92c6f, w4 += s1(w2) + w13 + s0(w5));
    Round(d, e, f, g, h, a, b, c, 0x4a7484aa, w5 += s1(w3) + w14 + s0(w6));
    Round(c, d, e, f, g, h, a, b, 0x5cb0a9dc, w6 += s1(w4) + w15 + s0(w7));
    Round(b, c, d, e, f, g, h, a, 0x76f988da, w7 += s1(w5) + w0 + s0(w8));
    Round(a, b, c, d, e, f, g, h, 0x983e5152, w8 += s1(w6) + w1 + s0(w9));
    Round(h, a, b, c, d, e, f, g, 0xa831c66d, w9 += s1(w7) + w2 + s0(w10));
    Round(g, h, a, b, c, d, e, f, 0xb00327c8, w10 += s1(w8) + w3 + s0(w11));
    Round(f, g, h, a, b, c, d, e, 0xbf597fc7, w11 += s1(w9) + w4 + s0(w12));
    Round(e, f, g, h, a, b, c, d, 0xc6e00bf3, w12 += s1(w10) + w5 + s0(w13));
    Round(d, e, f, g, h, a, b, c, 0xd5a79147, w13 += s1(w11) + w6 + s0(w14));
    Round(c, d, e, f, g, h, a, b, 0x06ca6351, w14 += s1(w12) + w7 + s0(w15));
    Round(b, c, d, e, f, g, h, a, 0x14292967, w15 += s1(w13) + w8 + s0(w0));

    Round(a, b, c, d, e, f, g, h, 0x27b70a85, w0 += s1(w14) + w9 + s0(w1));
    Round(h, a, b, c, d, e, f, g, 0x2e1b2138, w1 += s1(w15) + w10 + s0(w2));
    Round(g, h, a, b, c, d, e, f, 0x4d2c6dfc, w2 += s1(w0) + w11 + s0(w3));
    Round(f, g, h, a, b, c, d, e, 0x53380d13, w3 += s1(w1) + w12 + s0(w4));
    Round(e, f, g, h, a, b, c, d, 0x650a7354, w4 += s1(w2) + w13 + s0(w5));
    Round(d, e, f, g, h, a, b, c, 0x766a0abb, w5 += s1(w3) + w14 + s0(w6));
    Round(c, d, e, f, g, h, a, b, 0x81c2c92e, w6 += s1(w4) + w15 + s0(w7));
    Round(b, c, d, e, f, g, h, a, 0x92722c85, w7 += s1(w5) + w0 + s0(w8));
    Round(a, b, c, d, e, f, g, h, 0xa2bfe8a1, w8 += s1(w6) + w1 + s0(w9));
    Round(h, a, b, c, d, e, f, g, 0xa81a664b, w9 += s1(w7) + w2 + s0(w10));
    Round(g, h, a, b, c, d, e, f, 0xc24b8b70, w10 += s1(w8) + w3 + s0(w11));
    Round(f, g, h, a, b, c, d, e, 0xc76c51a3, w11 += s1(w9) + w4 + s0(w12));
    Round(e, f, g, h, a, b, c, d, 0xd192e819, w12 += s1(w10) + w5 + s0(w13));
    Round(d, e, f, g, h, a, b, c, 0xd6990624, w13 += s1(w11) + w6 + s0(w14));
    Round(c, d, e, f, g, h, a, b, 0xf40e3585, w14 += s1(w12) + w7 + s0(w15));
    Round(b, c, d, e, f, g, h, a, 0x106aa070, w15 += s1(w13) + w8 + s0(w0));

    Round(a, b, c, d, e, f, g, h, 0x19a4c116, w0 += s1(w14) + w9 + s0(w1));
    Round(h, a, b, c, d, e, f, g, 0x1e376c08, w1 += s1(w15) + w10 + s0(w2));
    Round(g, h, a, b, c, d, e, f, 0x2748774c, w2 += s1(w0) + w11 + s0(w3));
    Round(f, g, h, a, b, c, d, e, 0x34b0bcb5, w3 += s1(w1) + w12 + s0(w4));
    Round(e, f, g, h, a, b, c, d, 0x391c0cb3, w4 += s1(w2) + w13 + s0(w5));
    Round(d, e, f, g, h, a, b, c, 0x4ed8aa4a, w5 += s1(w3) + w14 + s0(w6));
    Round(c, d, e, f, g, h, a, b, 0x5b9cca4f, w6 += s1(w4) + w15 + s0(w7));
    Round(b, c, d, e, f, g, h, a, 0x682e6ff3, w7 += s1(w5) + w0 + s0(w8));
    Round(a, b, c, d, e, f, g, h, 0x748f82ee, w8 += s1(w6) + w1 + s0(w9));
    Round(h, a, b, c, d, e, f, g, 0x78a5636f, w9 += s1(w7) + w2 + s0(w10));
    Round(g, h, a, b, c, d, e, f, 0x84c87814, w10 += s1(w8) + w3 + s0(w11));
    Round(f, g, h, a, b, c, d, e, 0x8cc70208, w11 += s1(w9) + w4 + s0(w12));
    Round(e, f, g, h, a, b, c, d, 0x90befffa, w12 += s1(w10) + w5 + s0(w13));
    Round(d, e, f, g, h, a, b, c, 0xa4506ceb, w13 += s1(w11) + w6 + s0(w14));
    Round(c, d, e, f, g, h, a, b, 0xbef9a3f7, w14 + s1(w12) + w7 + s0(w15));
    Round(b, c, d, e, f, g, h, a, 0xc67178f2, w15 + s1(w13) + w8 + s0(w0));

    s[0] = 0x6a09e667ul + a;

  }

} // namespace sha256


////// SHA-256

class CSHA256
{
private:
    uint32_t s[8];
    unsigned char buf[64];
    uint64_t bytes;

public:
    static const size_t OUTPUT_SIZE = 32;

    CSHA256();
    void Write(const unsigned char* data, size_t len);
    void Finalize(unsigned char hash[OUTPUT_SIZE]);

};

CSHA256::CSHA256() {
    bytes = 0;
    s[0] = 0x6a09e667ul;
    s[1] = 0xbb67ae85ul;
    s[2] = 0x3c6ef372ul;
    s[3] = 0xa54ff53aul;
    s[4] = 0x510e527ful;
    s[5] = 0x9b05688cul;
    s[6] = 0x1f83d9abul;
    s[7] = 0x5be0cd19ul;
}

void CSHA256::Write(const unsigned char* data, size_t len)
{
  const unsigned char* end = data + len;
  size_t bufsize = bytes % 64;
  if (bufsize && bufsize + len >= 64) {
    // Fill the buffer, and process it.
    memcpy(buf + bufsize, data, 64 - bufsize);
    bytes += 64 - bufsize;
    data += 64 - bufsize;
    _sha256::Transform(s, buf);
    bufsize = 0;
  }
  while (end >= data + 64) {
    // Process full chunks directly from the source.
    _sha256::Transform(s, data);
    bytes += 64;
    data += 64;
  }
  if (end > data) {
    // Fill the buffer with what remains.
    memcpy(buf + bufsize, data, end - data);
    bytes += end - data;
  }
}

void CSHA256::Finalize(unsigned char hash[OUTPUT_SIZE])
{
    unsigned char sizedesc[8];
    WRITEBE64(sizedesc, bytes << 3);
    Write(_sha256::pad, 1 + ((119 - (bytes % 64)) % 64));
    Write(sizedesc, 8);
    WRITEBE32(hash, s[0]);
    WRITEBE32(hash + 4, s[1]);
    WRITEBE32(hash + 8, s[2]);
    WRITEBE32(hash + 12, s[3]);
    WRITEBE32(hash + 16, s[4]);
    WRITEBE32(hash + 20, s[5]);
    WRITEBE32(hash + 24, s[6]);
    WRITEBE32(hash + 28, s[7]);
}

void sha256(unsigned char *input, size_t length, unsigned char *digest) {

	CSHA256 sha;
	sha.Write(input, length);
	sha.Finalize(digest);

}

const uint8_t sizedesc_32[8] = { 0,0,0,0,0,0,1,0 };
const uint8_t sizedesc_33[8] = { 0,0,0,0,0,0,1,8 };
const uint8_t sizedesc_65[8] = { 0,0,0,0,0,0,2,8 };

void sha256_33(unsigned char *input, unsigned char *digest) {

  uint32_t s[8];

  _sha256::Initialize(s);
  memcpy(input + 33, _sha256::pad, 23);
  memcpy(input + 56, sizedesc_33, 8);
  _sha256::Transform(s, input);

  WRITEBE32(digest, s[0]);
  WRITEBE32(digest + 4, s[1]);
  WRITEBE32(digest + 8, s[2]);
  WRITEBE32(digest + 12, s[3]);
  WRITEBE32(digest + 16, s[4]);
  WRITEBE32(digest + 20, s[5]);
  WRITEBE32(digest + 24, s[6]);
  WRITEBE32(digest + 28, s[7]);


}

void sha256_65(unsigned char *input, unsigned char *digest) {

  uint32_t s[8];

  memcpy(input + 65, _sha256::pad, 55);
  memcpy(input + 120, sizedesc_65, 8);

  _sha256::Initialize(s);
  _sha256::Transform(s, input);
  _sha256::Transform(s, input+64);

  WRITEBE32(digest, s[0]);
  WRITEBE32(digest + 4, s[1]);
  WRITEBE32(digest + 8, s[2]);
  WRITEBE32(digest + 12, s[3]);
  WRITEBE32(digest + 16, s[4]);
  WRITEBE32(digest + 20, s[5]);
  WRITEBE32(digest + 24, s[6]);
  WRITEBE32(digest + 28, s[7]);

}

void sha256_checksum(uint8_t *input, int length, uint8_t *checksum) {

  uint32_t s[8];
  uint8_t b[64];
  memcpy(b,input,length);
  memcpy(b + length, _sha256::pad, 56-length);
  WRITEBE64(b + 56, length << 3);
  _sha256::Transform2(s, b);
  WRITEBE32(checksum,s[0]);

}

std::string sha256_hex(unsigned char *digest) {

    char buf[2*32+1];
    buf[2*32] = 0;
    for (int i = 0; i < 32; i++)
        sprintf(buf+i*2,"%02x",digest[i]);
    return std::string(buf);

}

bool sha256_file(const char* file_name, uint8_t* checksum) {
    FILE* file = fopen(file_name, "rb");
    if (file == NULL) {
        printf("Failed to open file: %s\n", file_name);
        return false;
    }
    CSHA256 sha;
    uint8_t buffer[8192]; // Buffer to read file contents
    size_t bytes_read;

	// Read file contents and update SHA256 context
	while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
		sha.Write( buffer, bytes_read);
	}

	// Finalize SHA256 computation
	sha.Finalize(checksum);
	fclose(file);
	return true;
}

void sha256_batch_33(const uint8_t *inputs, size_t n, uint8_t *out) {
    for(size_t i=0; i<n; ++i) {
        sha256_33((unsigned char *)(inputs + i*33), out + i*32);
    }
}

#ifdef __AVX2__

static inline __m256i _ror256(__m256i x, int n) {
    return _mm256_or_si256(_mm256_srli_epi32(x, n),
                           _mm256_slli_epi32(x, 32 - n));
}

static inline __m256i _shr256(__m256i x, int n) {
    return _mm256_srli_epi32(x, n);
}

#define SIGMA0(x) (_mm256_xor_si256(_mm256_xor_si256(_ror256((x), 2), _ror256((x), 13)), _ror256((x), 22)))
#define SIGMA1(x) (_mm256_xor_si256(_mm256_xor_si256(_ror256((x), 6), _ror256((x), 11)), _ror256((x), 25)))
#define sigma0(x) (_mm256_xor_si256(_mm256_xor_si256(_ror256((x), 7), _ror256((x), 18)), _shr256((x), 3)))
#define sigma1(x) (_mm256_xor_si256(_mm256_xor_si256(_ror256((x), 17), _ror256((x), 19)), _shr256((x), 10)))

#define Ch(x,y,z)  _mm256_xor_si256(_mm256_and_si256(x, y), _mm256_andnot_si256(x, z))
#define Maj(x,y,z) _mm256_or_si256(_mm256_and_si256(x, y), _mm256_and_si256(z, _mm256_or_si256(x, y)))

static const uint32_t K256[64] = {
    0x428a2f98ul,0x71374491ul,0xb5c0fbcful,0xe9b5dba5ul,
    0x3956c25bul,0x59f111f1ul,0x923f82a4ul,0xab1c5ed5ul,
    0xd807aa98ul,0x12835b01ul,0x243185beul,0x550c7dc3ul,
    0x72be5d74ul,0x80deb1feul,0x9bdc06a7ul,0xc19bf174ul,
    0xe49b69c1ul,0xefbe4786ul,0x0fc19dc6ul,0x240ca1ccul,
    0x2de92c6ful,0x4a7484aaul,0x5cb0a9dcul,0x76f988daul,
    0x983e5152ul,0xa831c66dul,0xb00327c8ul,0xbf597fc7ul,
    0xc6e00bf3ul,0xd5a79147ul,0x06ca6351ul,0x14292967ul,
    0x27b70a85ul,0x2e1b2138ul,0x4d2c6dfcul,0x53380d13ul,
    0x650a7354ul,0x766a0abbul,0x81c2c92eul,0x92722c85ul,
    0xa2bfe8a1ul,0xa81a664bul,0xc24b8b70ul,0xc76c51a3ul,
    0xd192e819ul,0xd6990624ul,0xf40e3585ul,0x106aa070ul,
    0x19a4c116ul,0x1e376c08ul,0x2748774cul,0x34b0bcb5ul,
    0x391c0cb3ul,0x4ed8aa4aul,0x5b9cca4ful,0x682e6ff3ul,
    0x748f82eeul,0x78a5636ful,0x84c87814ul,0x8cc70208ul,
    0x90befffaul,0xa4506cebul,0xbef9a3f7ul,0xc67178f2ul
};

void sha256_avx2_8(uint32_t *i0, uint32_t *i1, uint32_t *i2, uint32_t *i3,
                   uint32_t *i4, uint32_t *i5, uint32_t *i6, uint32_t *i7,
                   uint8_t *d0, uint8_t *d1, uint8_t *d2, uint8_t *d3,
                   uint8_t *d4, uint8_t *d5, uint8_t *d6, uint8_t *d7) {

    __m256i a = _mm256_set1_epi32(0x6a09e667ul);
    __m256i b = _mm256_set1_epi32(0xbb67ae85ul);
    __m256i c = _mm256_set1_epi32(0x3c6ef372ul);
    __m256i d = _mm256_set1_epi32(0xa54ff53aul);
    __m256i e = _mm256_set1_epi32(0x510e527ful);
    __m256i f = _mm256_set1_epi32(0x9b05688cul);
    __m256i g = _mm256_set1_epi32(0x1f83d9abul);
    __m256i h = _mm256_set1_epi32(0x5be0cd19ul);

    __m256i W[64];

    for (int i = 0; i < 16; ++i) {
        W[i] = _mm256_set_epi32(i7[i], i6[i], i5[i], i4[i], i3[i], i2[i], i1[i], i0[i]);
    }
    for (int i = 16; i < 64; ++i) {
        __m256i s0 = sigma0(W[i-15]);
        __m256i s1 = sigma1(W[i-2]);
        W[i] = _mm256_add_epi32(_mm256_add_epi32(_mm256_add_epi32(W[i-16], s0), W[i-7]), s1);
    }

    for (int i = 0; i < 64; ++i) {
        __m256i T1 = _mm256_add_epi32(h, SIGMA1(e));
        T1 = _mm256_add_epi32(T1, Ch(e, f, g));
        T1 = _mm256_add_epi32(T1, _mm256_set1_epi32(K256[i]));
        T1 = _mm256_add_epi32(T1, W[i]);
        __m256i T2 = _mm256_add_epi32(SIGMA0(a), Maj(a, b, c));

        h = g;
        g = f;
        f = e;
        e = _mm256_add_epi32(d, T1);
        d = c;
        c = b;
        b = a;
        a = _mm256_add_epi32(T1, T2);
    }

    a = _mm256_add_epi32(a, _mm256_set1_epi32(0x6a09e667ul));
    b = _mm256_add_epi32(b, _mm256_set1_epi32(0xbb67ae85ul));
    c = _mm256_add_epi32(c, _mm256_set1_epi32(0x3c6ef372ul));
    d = _mm256_add_epi32(d, _mm256_set1_epi32(0xa54ff53aul));
    e = _mm256_add_epi32(e, _mm256_set1_epi32(0x510e527ful));
    f = _mm256_add_epi32(f, _mm256_set1_epi32(0x9b05688cul));
    g = _mm256_add_epi32(g, _mm256_set1_epi32(0x1f83d9abul));
    h = _mm256_add_epi32(h, _mm256_set1_epi32(0x5be0cd19ul));

    alignas(32) uint32_t outA[8], outB[8], outC[8], outD[8];
    alignas(32) uint32_t outE[8], outF[8], outG[8], outH[8];

    _mm256_store_si256((__m256i*)outA, a);
    _mm256_store_si256((__m256i*)outB, b);
    _mm256_store_si256((__m256i*)outC, c);
    _mm256_store_si256((__m256i*)outD, d);
    _mm256_store_si256((__m256i*)outE, e);
    _mm256_store_si256((__m256i*)outF, f);
    _mm256_store_si256((__m256i*)outG, g);
    _mm256_store_si256((__m256i*)outH, h);

    uint32_t* outs[8] = {outA,outB,outC,outD,outE,outF,outG,outH};
    uint8_t* digests[8] = {d0,d1,d2,d3,d4,d5,d6,d7};

    for (int lane = 0; lane < 8; ++lane) {
        uint8_t* dst = digests[lane];
        ((uint32_t*)dst)[0] = __builtin_bswap32(outA[lane]);
        ((uint32_t*)dst)[1] = __builtin_bswap32(outB[lane]);
        ((uint32_t*)dst)[2] = __builtin_bswap32(outC[lane]);
        ((uint32_t*)dst)[3] = __builtin_bswap32(outD[lane]);
        ((uint32_t*)dst)[4] = __builtin_bswap32(outE[lane]);
        ((uint32_t*)dst)[5] = __builtin_bswap32(outF[lane]);
        ((uint32_t*)dst)[6] = __builtin_bswap32(outG[lane]);
        ((uint32_t*)dst)[7] = __builtin_bswap32(outH[lane]);
    }
}

void sha256_avx2_8(const uint8_t* in[8], uint8_t* out[8]) {
    sha256_avx2_8((uint32_t*)in[0], (uint32_t*)in[1], (uint32_t*)in[2], (uint32_t*)in[3],
                  (uint32_t*)in[4], (uint32_t*)in[5], (uint32_t*)in[6], (uint32_t*)in[7],
                  out[0], out[1], out[2], out[3], out[4], out[5], out[6], out[7]);
}

#endif // __AVX2__
