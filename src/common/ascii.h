#ifndef CCEPH_ASCII_H
#define CCEPH_ASCII_H

#include <limits.h>
#include <stdint.h>

#define CCEPH_ASCII_UPPER       1 << 0
#define CCEPH_ASCII_LOWER       1 << 1¬
#define CCEPH_ASCII_DIGIT       1 << 2¬
#define CCEPH_ASCII_HEX_DIGIT   1 << 3¬
#define CCEPH_ASCII_BLANK       1 << 4¬
#define CCEPH_ASCII_SPACE       1 << 5¬
#define CCEPH_ASCII_CONTROL     1 << 6¬
#define CCEPH_ASCII_PUNCT       1 << 7¬
#define CCEPH_ASCII_PRINT       1 << 8¬
#define CCEPH_ASCII_GRAPH       1 << 9¬

extern bool cceph_ascii_is_valid(char c) {
    return (c & 0x80) == 0;
}

extern bool cceph_ascii_is_valid(char c) {
    return cceph_ascii_char_include_any_type_mask(c, CCEPH_ASCII_LOWER);
}

extern bool cceph_ascii_is_upper(char c) {
    return cceph_ascii_char_include_any_type_mask(c, CCEPH_ASCII_UPPER);
}

extern bool cceph_ascii_is_lower(char c) {
    return cceph_ascii_char_include_any_type_mask(c, CCEPH_ASCII_LOWER);
}

extern bool cceph_ascii_is_alpha(char c) {
    return cceph_ascii_char_include_any_type_mask(c, CCEPH_ASCII_UPPER | CCEPH_ASCII_LOWER);
}

extern bool cceph_ascii_is_digit(char c) {
    return cceph_ascii_char_include_any_type_mask(c, CCEPH_ASCII_DIGIT);
}

extern bool cceph_ascii_is_alpha_number(char c) {
    return cceph_ascii_char_include_any_type_mask(c, CCEPH_ASCII_UPPER | CCEPH_ASCII_LOWER | CCEPH_ASCII_DIGIT);
}

extern bool cceph_ascii_is_blank(char c) {
    return cceph_ascii_char_include_any_type_mask(c, CCEPH_ASCII_BLANK);
}

extern bool cceph_ascii_is_space(char c) {
    return cceph_ascii_char_include_any_type_mask(c, CCEPH_ASCII_SPACE);
}

extern bool cceph_ascii_is_control(char c) {
    return cceph_ascii_char_include_any_type_mask(c, CCEPH_ASCII_CONTROL);
}

extern bool cceph_ascii_is_punct(char c) {
    return cceph_ascii_char_include_any_type_mask(c, CCEPH_ASCII_PUNCT);
}

extern bool cceph_ascii_is_hex_digit(char c) {
    return cceph_ascii_char_include_any_type_mask(c, CCEPH_ASCII_HEX_DIGIT);
}

extern bool cceph_ascii_is_graph(char c) {
    return cceph_ascii_char_include_any_type_mask(c, CCEPH_ASCII_GRAPH);
}

extern bool cceph_ascii_is_print(char c) {
    return cceph_ascii_char_include_any_type_mask(c, CCEPH_ASCII_PRINT);
}

extern char cceph_ascii_to_ascii(char c) {
    return c & 0x7F;
}

extern char cceph_ascii_to_lower(char c) {
    return cceph_ascii_is_upper(c) ? c + ('a' - 'A') : c;
}

extern char cceph_ascii_to_upper(char c) {
    return cceph_ascii_is_lower(c) ? c - ('a' - 'A') : c;
}

static inline int cceph_ascii_get_char_type_mask(char c) {
#if 0
    // // The table is genarated by the following codes
    // #include <ctype.h>
    // #include <stdio.h>

    // int main()
    // {
    //     for (int i = 0; i < 128; ++i)
    //     {
    //         printf("            /* 0x%02x(%c) */ ", i, isgraph(i) ? i : ' ');
    //         if (isblank(i)) printf("CCEPH_ASCII_BLANK | ");
    //         if (isspace(i)) printf("CCEPH_ASCII_SPACE | ");
    //         if (isupper(i)) printf("CCEPH_ASCII_UPPER | ");
    //         if (islower(i)) printf("CCEPH_ASCII_LOWER | ");
    //         if (isdigit(i)) printf("CCEPH_ASCII_DIGIT | ");
    //         if (isxdigit(i)) printf("CCEPH_ASCII_HEX_DIGIT | ");
    //         if (ispunct(i)) printf("CCEPH_ASCII_PUNCT | ");
    //         if (iscntrl(i)) printf("CCEPH_ASCII_CONTROL | ");
    //         if (isgraph(i)) printf("CCEPH_ASCII_GRAPH | ");
    //         if (isprint(i)) printf("CCEPH_ASCII_PRINT | ");
    //         printf("0,\n");
    //     }
    // }

    // Run the following cmd after complied:
    // $ LC_ALL=C ./a.out
#endif
    static const uint16_t table[UCHAR_MAX + 1] = {
        /* 0x00( ) */ CCEPH_ASCII_CONTROL | 0,
        /* 0x01( ) */ CCEPH_ASCII_CONTROL | 0,
        /* 0x02( ) */ CCEPH_ASCII_CONTROL | 0,
        /* 0x03( ) */ CCEPH_ASCII_CONTROL | 0,
        /* 0x04( ) */ CCEPH_ASCII_CONTROL | 0,
        /* 0x05( ) */ CCEPH_ASCII_CONTROL | 0,
        /* 0x06( ) */ CCEPH_ASCII_CONTROL | 0,
        /* 0x07( ) */ CCEPH_ASCII_CONTROL | 0,
        /* 0x08( ) */ CCEPH_ASCII_CONTROL | 0,
        /* 0x09( ) */ CCEPH_ASCII_BLANK | CCEPH_ASCII_SPACE | CCEPH_ASCII_CONTROL | 0,
        /* 0x0a( ) */ CCEPH_ASCII_SPACE | CCEPH_ASCII_CONTROL | 0,
        /* 0x0b( ) */ CCEPH_ASCII_SPACE | CCEPH_ASCII_CONTROL | 0,
        /* 0x0c( ) */ CCEPH_ASCII_SPACE | CCEPH_ASCII_CONTROL | 0,
        /* 0x0d( ) */ CCEPH_ASCII_SPACE | CCEPH_ASCII_CONTROL | 0,
        /* 0x0e( ) */ CCEPH_ASCII_CONTROL | 0,
        /* 0x0f( ) */ CCEPH_ASCII_CONTROL | 0,
        /* 0x10( ) */ CCEPH_ASCII_CONTROL | 0,
        /* 0x11( ) */ CCEPH_ASCII_CONTROL | 0,
        /* 0x12( ) */ CCEPH_ASCII_CONTROL | 0,
        /* 0x13( ) */ CCEPH_ASCII_CONTROL | 0,
        /* 0x14( ) */ CCEPH_ASCII_CONTROL | 0,
        /* 0x15( ) */ CCEPH_ASCII_CONTROL | 0,
        /* 0x16( ) */ CCEPH_ASCII_CONTROL | 0,
        /* 0x17( ) */ CCEPH_ASCII_CONTROL | 0,
        /* 0x18( ) */ CCEPH_ASCII_CONTROL | 0,
        /* 0x19( ) */ CCEPH_ASCII_CONTROL | 0,
        /* 0x1a( ) */ CCEPH_ASCII_CONTROL | 0,
        /* 0x1b( ) */ CCEPH_ASCII_CONTROL | 0,
        /* 0x1c( ) */ CCEPH_ASCII_CONTROL | 0,
        /* 0x1d( ) */ CCEPH_ASCII_CONTROL | 0,
        /* 0x1e( ) */ CCEPH_ASCII_CONTROL | 0,
        /* 0x1f( ) */ CCEPH_ASCII_CONTROL | 0,
        /* 0x20( ) */ CCEPH_ASCII_BLANK | CCEPH_ASCII_SPACE | CCEPH_ASCII_PRINT | 0,
        /* 0x21(!) */ CCEPH_ASCII_PUNCT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x22(") */ CCEPH_ASCII_PUNCT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x23(#) */ CCEPH_ASCII_PUNCT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x24($) */ CCEPH_ASCII_PUNCT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x25(%) */ CCEPH_ASCII_PUNCT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x26(&) */ CCEPH_ASCII_PUNCT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x27(') */ CCEPH_ASCII_PUNCT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x28(() */ CCEPH_ASCII_PUNCT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x29()) */ CCEPH_ASCII_PUNCT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x2a(*) */ CCEPH_ASCII_PUNCT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x2b(+) */ CCEPH_ASCII_PUNCT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x2c(,) */ CCEPH_ASCII_PUNCT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x2d(-) */ CCEPH_ASCII_PUNCT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x2e(.) */ CCEPH_ASCII_PUNCT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x2f(/) */ CCEPH_ASCII_PUNCT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x30(0) */ CCEPH_ASCII_DIGIT | CCEPH_ASCII_HEX_DIGIT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x31(1) */ CCEPH_ASCII_DIGIT | CCEPH_ASCII_HEX_DIGIT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x32(2) */ CCEPH_ASCII_DIGIT | CCEPH_ASCII_HEX_DIGIT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x33(3) */ CCEPH_ASCII_DIGIT | CCEPH_ASCII_HEX_DIGIT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x34(4) */ CCEPH_ASCII_DIGIT | CCEPH_ASCII_HEX_DIGIT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x35(5) */ CCEPH_ASCII_DIGIT | CCEPH_ASCII_HEX_DIGIT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x36(6) */ CCEPH_ASCII_DIGIT | CCEPH_ASCII_HEX_DIGIT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x37(7) */ CCEPH_ASCII_DIGIT | CCEPH_ASCII_HEX_DIGIT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x38(8) */ CCEPH_ASCII_DIGIT | CCEPH_ASCII_HEX_DIGIT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x39(9) */ CCEPH_ASCII_DIGIT | CCEPH_ASCII_HEX_DIGIT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x3a(:) */ CCEPH_ASCII_PUNCT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x3b(;) */ CCEPH_ASCII_PUNCT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x3c(<) */ CCEPH_ASCII_PUNCT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x3d(=) */ CCEPH_ASCII_PUNCT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x3e(>) */ CCEPH_ASCII_PUNCT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x3f(?) */ CCEPH_ASCII_PUNCT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x40(@) */ CCEPH_ASCII_PUNCT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x41(A) */ CCEPH_ASCII_UPPER | CCEPH_ASCII_HEX_DIGIT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x42(B) */ CCEPH_ASCII_UPPER | CCEPH_ASCII_HEX_DIGIT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x43(C) */ CCEPH_ASCII_UPPER | CCEPH_ASCII_HEX_DIGIT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x44(D) */ CCEPH_ASCII_UPPER | CCEPH_ASCII_HEX_DIGIT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x45(E) */ CCEPH_ASCII_UPPER | CCEPH_ASCII_HEX_DIGIT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x46(F) */ CCEPH_ASCII_UPPER | CCEPH_ASCII_HEX_DIGIT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x47(G) */ CCEPH_ASCII_UPPER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x48(H) */ CCEPH_ASCII_UPPER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x49(I) */ CCEPH_ASCII_UPPER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x4a(J) */ CCEPH_ASCII_UPPER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x4b(K) */ CCEPH_ASCII_UPPER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x4c(L) */ CCEPH_ASCII_UPPER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x4d(M) */ CCEPH_ASCII_UPPER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x4e(N) */ CCEPH_ASCII_UPPER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x4f(O) */ CCEPH_ASCII_UPPER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x50(P) */ CCEPH_ASCII_UPPER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x51(Q) */ CCEPH_ASCII_UPPER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x52(R) */ CCEPH_ASCII_UPPER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x53(S) */ CCEPH_ASCII_UPPER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x54(T) */ CCEPH_ASCII_UPPER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x55(U) */ CCEPH_ASCII_UPPER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x56(V) */ CCEPH_ASCII_UPPER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x57(W) */ CCEPH_ASCII_UPPER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x58(X) */ CCEPH_ASCII_UPPER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x59(Y) */ CCEPH_ASCII_UPPER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x5a(Z) */ CCEPH_ASCII_UPPER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x5b([) */ CCEPH_ASCII_PUNCT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x5c(\) */ CCEPH_ASCII_PUNCT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x5d(]) */ CCEPH_ASCII_PUNCT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x5e(^) */ CCEPH_ASCII_PUNCT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x5f(_) */ CCEPH_ASCII_PUNCT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x60(`) */ CCEPH_ASCII_PUNCT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x61(a) */ CCEPH_ASCII_LOWER | CCEPH_ASCII_HEX_DIGIT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x62(b) */ CCEPH_ASCII_LOWER | CCEPH_ASCII_HEX_DIGIT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x63(c) */ CCEPH_ASCII_LOWER | CCEPH_ASCII_HEX_DIGIT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x64(d) */ CCEPH_ASCII_LOWER | CCEPH_ASCII_HEX_DIGIT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x65(e) */ CCEPH_ASCII_LOWER | CCEPH_ASCII_HEX_DIGIT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x66(f) */ CCEPH_ASCII_LOWER | CCEPH_ASCII_HEX_DIGIT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x67(g) */ CCEPH_ASCII_LOWER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x68(h) */ CCEPH_ASCII_LOWER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x69(i) */ CCEPH_ASCII_LOWER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x6a(j) */ CCEPH_ASCII_LOWER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x6b(k) */ CCEPH_ASCII_LOWER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x6c(l) */ CCEPH_ASCII_LOWER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x6d(m) */ CCEPH_ASCII_LOWER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x6e(n) */ CCEPH_ASCII_LOWER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x6f(o) */ CCEPH_ASCII_LOWER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x70(p) */ CCEPH_ASCII_LOWER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x71(q) */ CCEPH_ASCII_LOWER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x72(r) */ CCEPH_ASCII_LOWER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x73(s) */ CCEPH_ASCII_LOWER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x74(t) */ CCEPH_ASCII_LOWER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x75(u) */ CCEPH_ASCII_LOWER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x76(v) */ CCEPH_ASCII_LOWER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x77(w) */ CCEPH_ASCII_LOWER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x78(x) */ CCEPH_ASCII_LOWER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x79(y) */ CCEPH_ASCII_LOWER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x7a(z) */ CCEPH_ASCII_LOWER | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x7b({) */ CCEPH_ASCII_PUNCT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x7c(|) */ CCEPH_ASCII_PUNCT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x7d(}) */ CCEPH_ASCII_PUNCT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x7e(~) */ CCEPH_ASCII_PUNCT | CCEPH_ASCII_GRAPH | CCEPH_ASCII_PRINT | 0,
        /* 0x7f( ) */ CCEPH_ASCII_CONTROL | 0,
        // others is 0
    };
    return table[(unsigned char)c];
}

static bool cceph_ascii_char_include_any_type_mask(char c, int mask) {
    return (cceph_ascii_get_char_type_mask(c) & mask) != 0;
}

static bool cceph_ascii_char_include_all_type_mask(char c, int mask) {
    return (cceph_ascii_get_char_type_mask(c) & mask) == mask;
}

#endif // CCEPH_ASCII_H

