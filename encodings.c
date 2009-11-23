
/* encodings.c - Translate high bit chars into RTF using codepage 1252

Copyright (C) 1995-2002 The Free Software Foundation

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

This file is available from http://sourceforge.net/projects/latex2rtf/
 
Authors:
    1995-1997 Ralf Schlatterbeck
    1998-2000 Georg Lehner
    2001-2002 Scott Prahl
*/

#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "fonts.h"
#include "funct1.h"
#include "encodings.h"
#include "encoding_tables.h"
#include "chars.h"

static void put_breve_char(char c)
{
    int num = RtfFontNumber("MT Extra");
    int upsize = CurrentFontSize() / 2;

    fprintRTF("{\\field{\\*\\fldinst  EQ \\\\O(%c", c);
    fprintRTF("%c\\\\S(\\up%d\\f%d \\\\())}", g_field_separator, upsize, num);
    fprintRTF("{\\fldrslt }}");
}

/*
static void put_acute_char(char c)
{
	fprintRTF("{\\field{\\*\\fldinst  EQ \\\\O(%c",c);
	fprintRTF("%c\\\\S(\\'b4))}", g_field_separator);
	fprintRTF("{\\fldrslt }}");
}
*/

static void put_cedilla_char(char c)
{
    int down = CurrentFontSize() / 4;

    fprintRTF("{\\field{\\*\\fldinst  EQ \\\\O(%c", c);
    fprintRTF("%c\\\\S(\\dn%d\\'b8))}", g_field_separator, down);
    fprintRTF("{\\fldrslt }}");
}

static void put_ring_char(char c)
{
    fprintRTF("{\\field{\\*\\fldinst  EQ \\\\O(%c", c);
    fprintRTF("%c\\\\S(\\'b0))}", g_field_separator);
    fprintRTF("{\\fldrslt }}");
}

static void put_macron_char(char c, int unicode)
{
    fprintRTF("\\u%d?",unicode);
	if (0) {
		fprintRTF("{\\field{\\*\\fldinst  EQ \\\\O(%c", c);
		fprintRTF("%c\\\\S(\\'ee))}", g_field_separator);
		fprintRTF("{\\fldrslt }}");
    }
}

static void put_tilde_char(char c)
{
    int num = RtfFontNumber("MT Extra");

    fprintRTF("{\\field{\\*\\fldinst  EQ \\\\O(%c", c);
    fprintRTF("%c\\\\S(\\f%d\\'25))}", g_field_separator, num);
    fprintRTF("{\\fldrslt }}");
}

static void put_dot_char(char c)
{
    int num = RtfFontNumber("MT Extra");

    fprintRTF("{\\field{\\*\\fldinst  EQ \\\\O(%c", c);
    fprintRTF("%c\\\\S(\\f%d\\'26))}", g_field_separator, num);
    fprintRTF("{\\fldrslt }}");
}

static int hex_to_int(char *s)
{
	int i,n,num;
	if (s==NULL) return 0;
	n = (int) strlen(s);
	num=0;
	
	for (i=0; i<n; i++) {
		if (s[i]>='A')
			num = (int) (s[i]-'A'+10) + num * 16;
		else
			num = (int) (s[i]-'0') + num * 16;
	}
	return num;
	
}

static void applemac_enc(int apple_index)
{
    char *s;

    s = applemac_2_ansi[apple_index];
    if (s[0] != '0') {
        fprintRTF("\\'%s", s);
        return;
    }
    s = applemac_2_sym[apple_index];
    if (s[0] != '0') {
        CmdSymbolChar(hex_to_int(s));
        return;
    }
    if (0xDE) {                 /* U+64257 LATIN SMALL LIGATURE FI */
        fprintRTF("fi");
        return;
    }
    if (apple_index + 128 == 0xDF) {  /* U+64258 LATIN SMALL LIGATURE FL */
        fprintRTF("fl");
        return;
    }
    if (apple_index + 128 == 0xF0) {  /* U+65535 unknown */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (apple_index + 128 == 0xF5) {  /* U+305 LATIN SMALL LETTER DOTLESS I */
        fprintRTF("i");
        return;
    }
    if (apple_index + 128 == 0xF9) {  /* U+728 BREVE */
        int num = RtfFontNumber("MT Extra");

        fprintRTF("{\\f%d\\'28}", num);
        return;
    }
    if (apple_index + 128 == 0xFA) {  /* U+729 DOT ABOVE */
        put_dot_char(' ');
        return;
    }
    if (apple_index + 128 == 0xFB) {  /* U+730 RING ABOVE */
        put_ring_char(' ');
        return;
    }
    if (apple_index + 128 == 0xFD) {  /* U+733 DOUBLE ACUTE ACCENT */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (apple_index + 128 == 0xFE) {  /* U+731 OGONEK */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (apple_index + 128 == 0xFF) {  /* U+711 CARON */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
}

static void cp437_enc(int cp437_index)
{
    char *s;

    s = cp437_2_ansi[cp437_index];
    if (s[0] != '0') {
        fprintRTF("\\'%s", s);
        return;
    }
    s = cp437_2_sym[cp437_index];
    if (s[0] != '0') {
        CmdSymbolChar(hex_to_int(s));
        return;
    }
    if (cp437_index + 128 == 0x9E) {  /* U+8359 PESETA SIGN */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (cp437_index + 128 == 0xA9) {  /* U+8976 REVERSED NOT SIGN */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (cp437_index + 128 == 0xB0) {  /* U+9617 LIGHT SHADE */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (cp437_index + 128 == 0xB1) {  /* U+9618 MEDIUM SHADE */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (cp437_index + 128 == 0xB2) {  /* U+9619 DARK SHADE */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (cp437_index + 128 == 0xDB) {  /* U+9608 FULL BLOCK */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (cp437_index + 128 == 0xDC) {  /* U+9604 LOWER HALF BLOCK */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (cp437_index + 128 == 0xDD) {  /* U+9612 LEFT HALF BLOCK */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (cp437_index + 128 == 0xDE) {  /* U+9616 RIGHT HALF BLOCK */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (cp437_index + 128 == 0xDF) {  /* U+9600 UPPER HALF BLOCK */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (cp437_index + 128 == 0xF8) {  /* U+8728 RING OPERATOR */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (cp437_index + 128 == 0xFA) {  /* U+8729 BULLET OPERATOR */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (cp437_index + 128 == 0xFC) {  /* U+8319 SUPERSCRIPT LATIN SMALL LETTER N */
        fprintRTF("{\\up6 n}");
        return;
    }
    if (cp437_index + 128 == 0xFE) {  /* U+9632 BLACK SQUARE */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
}

static void cp850_enc(int cp850_index)
{
    char *s;

    s = cp850_2_ansi[cp850_index];
    if (s[0] != '0') {
        fprintRTF("\\'%s", s);
        return;
    }
    s = cp850_2_sym[cp850_index];
    if (s[0] != '0') {
        CmdSymbolChar(hex_to_int(s));
        return;
    }
    if (cp850_index + 128 == 0xB0) {  /* U+9617 LIGHT SHADE */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (cp850_index + 128 == 0xB1) {  /* U+9618 MEDIUM SHADE */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (cp850_index + 128 == 0xB2) {  /* U+9619 DARK SHADE */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (cp850_index + 128 == 0xD5) {  /* U+305 LATIN SMALL LETTER DOTLESS I */
        fprintRTF("i");
        return;
    }
    if (cp850_index + 128 == 0xDB) {  /* U+9608 FULL BLOCK */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (cp850_index + 128 == 0xDC) {  /* U+9604 LOWER HALF BLOCK */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (cp850_index + 128 == 0xDF) {  /* U+9600 UPPER HALF BLOCK */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (cp850_index + 128 == 0xF7) {  /* U+731 OGONEK */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (cp850_index + 128 == 0xFA) {  /* U+729 DOT ABOVE */
        put_dot_char(' ');
        return;
    }
    if (cp850_index + 128 == 0xFE) {  /* U+9632 BLACK SQUARE */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
}

static void cp852_enc(int cp852_index)
{
    char *s;

    s = cp852_2_1250[cp852_index];
    if (s[0] != '0') {
        int n = CurrentLatin2FontFamily();

        if (n >= 0)
            fprintRTF("{\\f%d\\'%s}", n, s);
        else                    /* already using Latin2 Font */
            fprintRTF("\\'%s", s);
        return;
    }

    s = cp852_2_ansi[cp852_index];
    if (s[0] != '0') {
        int n = CurrentLatin1FontFamily();

        if (n >= 0)
            fprintRTF("{\\f%d\\'%s}", n, s);
        else                    /* already using Latin1 Font */
            fprintRTF("\\'%s", s);
        return;
    }

    s = cp852_2_sym[cp852_index];
    if (s[0] != '0') {
        CmdSymbolChar(hex_to_int(s));
        return;
    }
}

static void cp865_enc(int cp865_index)
{
    char *s;

    s = cp865_2_ansi[cp865_index];
    if (s[0] != '0') {
        fprintRTF("\\'%s", s);
        return;
    }
    s = cp865_2_sym[cp865_index];
    if (s[0] != '0') {
        CmdSymbolChar(hex_to_int(s));
        return;
    }
    if (cp865_index + 128 == 0x9E) {  /* U+8359 PESETA SIGN */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (cp865_index + 128 == 0xA9) {  /* U+8976 REVERSED NOT SIGN */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (cp865_index + 128 == 0xB0) {  /* U+9617 LIGHT SHADE */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (cp865_index + 128 == 0xB1) {  /* U+9618 MEDIUM SHADE */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (cp865_index + 128 == 0xB2) {  /* U+9619 DARK SHADE */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (cp865_index + 128 == 0xDB) {  /* U+9608 FULL BLOCK */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (cp865_index + 128 == 0xDC) {  /* U+9604 LOWER HALF BLOCK */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (cp865_index + 128 == 0xDD) {  /* U+9612 LEFT HALF BLOCK */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (cp865_index + 128 == 0xDE) {  /* U+9616 RIGHT HALF BLOCK */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (cp865_index + 128 == 0xDF) {  /* U+9600 UPPER HALF BLOCK */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (cp865_index + 128 == 0xF8) {  /* U+8728 RING OPERATOR */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (cp865_index + 128 == 0xFA) {  /* U+8729 BULLET OPERATOR */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (cp865_index + 128 == 0xFC) {  /* U+8319 SUPERSCRIPT LATIN SMALL LETTER N */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (cp865_index + 128 == 0xFE) {  /* U+9632 BLACK SQUARE */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
}

static void cp1250_enc(int cp1250_index)
{
    char *s;

    s = cp1250_2_1250[cp1250_index];
    if (s[0] != '0') {
        int n = CurrentLatin2FontFamily();

        if (n >= 0)
            fprintRTF("{\\f%d\\'%s}", n, s);
        else                    /* already using Latin2 Font */
            fprintRTF("\\'%s", s);
        return;
    }

}

static void cp1252_enc(int cp1252_index)
{
    char *s;

    s = cp1252_2_ansi[cp1252_index];
    if (s[0] != '0') {
        fprintRTF("\\'%s", s);
        return;
    }
    s = cp1252_2_sym[cp1252_index];
    if (s[0] != '0') {
        CmdSymbolChar(hex_to_int(s));
        return;
    }
    if (cp1252_index + 128 == 0x80) {  /* U+65535 unknown */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (cp1252_index + 128 == 0x81) {  /* U+65535 unknown */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (cp1252_index + 128 == 0x8D) {  /* U+65535 unknown */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (cp1252_index + 128 == 0x8E) {  /* U+65535 unknown */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (cp1252_index + 128 == 0x8F) {  /* U+65535 unknown */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (cp1252_index + 128 == 0x90) {  /* U+65535 unknown */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (cp1252_index + 128 == 0x9D) {  /* U+65535 unknown */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (cp1252_index + 128 == 0x9E) {  /* U+65535 unknown */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
}

static void decmulti_enc(int dec_index)
{
    char *s;

    s = decmulti_2_ansi[dec_index];
    if (s[0] != '0') {
        fprintRTF("\\'%s", s);
        return;
    }
    s = decmulti_2_sym[dec_index];
    if (s[0] != '0') {
        CmdSymbolChar(hex_to_int(s));
        return;
    }
    if (dec_index + 128 == 0xA0) {  /* U+65535 unknown */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (dec_index + 128 == 0xA4) {  /* U+65535 unknown */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (dec_index + 128 == 0xA6) {  /* U+65535 unknown */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (dec_index + 128 == 0xAC) {  /* U+65535 unknown */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (dec_index + 128 == 0xAD) {  /* U+65535 unknown */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (dec_index + 128 == 0xAE) {  /* U+65535 unknown */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (dec_index + 128 == 0xAF) {  /* U+65535 unknown */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (dec_index + 128 == 0xB4) {  /* U+65535 unknown */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (dec_index + 128 == 0xB8) {  /* U+65535 unknown */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (dec_index + 128 == 0xBE) {  /* U+65535 unknown */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (dec_index + 128 == 0xD0) {  /* U+65535 unknown */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (dec_index + 128 == 0xDE) {  /* U+65535 unknown */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (dec_index + 128 == 0xF0) {  /* U+65535 unknown */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (dec_index + 128 == 0xFE) {  /* U+65535 unknown */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (dec_index + 128 == 0xFF) {  /* U+65535 unknown */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
}

static void latin1_enc(int latin1_index)
{
    char *s;

    s = latin1_2_ansi[latin1_index];
    if (s[0] != '0') {
        fprintRTF("\\'%s", s);
        return;
    }
    s = latin1_2_sym[latin1_index];
    if (s[0] != '0') {
        CmdSymbolChar(hex_to_int(s));
        return;
    }
}

static void latin2_enc(int latin2_index)
{
    char *s;

    s = latin2_2_1250[latin2_index];
    if (s[0] != '0') {
        int n = CurrentLatin2FontFamily();

        if (n >= 0)
            fprintRTF("{\\f%d\\'%s}", n, s);
        else                    /* already using Latin2 Font */
            fprintRTF("\\'%s", s);
        return;
    }

    s = latin2_2_ansi[latin2_index];
    if (s[0] != '0') {
        int n = CurrentLatin1FontFamily();

        if (n >= 0)
            fprintRTF("{\\f%d\\'%s}", n, s);
        else                    /* already using Latin1 Font */
            fprintRTF("\\'%s", s);
        return;
    }

    s = latin2_2_sym[latin2_index];
    if (s[0] != '0') {
        CmdSymbolChar(hex_to_int(s));
        return;
    }
}

static void latin3_enc(int latin3_index)
{
    char *s;

    s = latin3_2_ansi[latin3_index];
    if (s[0] != '0') {
        fprintRTF("\\'%s", s);
        return;
    }
    s = latin3_2_sym[latin3_index];
    if (s[0] != '0') {
        CmdSymbolChar(hex_to_int(s));
        return;
    }
    if (latin3_index + 128 == 0xA1) {  /* U+294 LATIN CAPITAL LETTER H WITH STROKE */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin3_index + 128 == 0xA2) {  /* U+728 BREVE */
        int num = RtfFontNumber("MT Extra");

        fprintRTF("{\\f%d\\'28}", num);
        return;
    }
    if (latin3_index + 128 == 0xA5) {  /* U+65535 unknown */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin3_index + 128 == 0xA6) {  /* U+292 LATIN CAPITAL LETTER H WITH CIRCUMFLEX */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin3_index + 128 == 0xA9) {  /* U+304 LATIN CAPITAL LETTER I WITH DOT ABOVE */
        put_dot_char('I');
        return;
    }
    if (latin3_index + 128 == 0xAA) {  /* U+350 LATIN CAPITAL LETTER S WITH CEDILLA */
        put_cedilla_char('S');
        return;
    }
    if (latin3_index + 128 == 0xAB) {  /* U+286 LATIN CAPITAL LETTER G WITH BREVE */
        put_breve_char('G');
        return;
    }
    if (latin3_index + 128 == 0xAC) {  /* U+308 LATIN CAPITAL LETTER J WITH CIRCUMFLEX */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin3_index + 128 == 0xAE) {  /* U+65535 unknown */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin3_index + 128 == 0xAF) {  /* U+379 LATIN CAPITAL LETTER Z WITH DOT ABOVE */
        put_dot_char('Z');
        return;
    }
    if (latin3_index + 128 == 0xB1) {  /* U+295 LATIN SMALL LETTER H WITH STROKE */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin3_index + 128 == 0xB6) {  /* U+293 LATIN SMALL LETTER H WITH CIRCUMFLEX */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin3_index + 128 == 0xB9) {  /* U+305 LATIN SMALL LETTER DOTLESS I */
        fprintRTF("i");
        return;
    }
    if (latin3_index + 128 == 0xBA) {  /* U+351 LATIN SMALL LETTER S WITH CEDILLA */
        put_cedilla_char('s');
        return;
    }
    if (latin3_index + 128 == 0xBB) {  /* U+287 LATIN SMALL LETTER G WITH BREVE */
        put_breve_char('g');
        return;
    }
    if (latin3_index + 128 == 0xBC) {  /* U+309 LATIN SMALL LETTER J WITH CIRCUMFLEX */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin3_index + 128 == 0xBE) {  /* U+65535 unknown */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin3_index + 128 == 0xBF) {  /* U+380 LATIN SMALL LETTER Z WITH DOT ABOVE */
        put_dot_char('z');
        return;
    }
    if (latin3_index + 128 == 0xC3) {  /* U+65535 unknown */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin3_index + 128 == 0xC5) {  /* U+266 LATIN CAPITAL LETTER C WITH DOT ABOVE */
        put_dot_char('C');
        return;
    }
    if (latin3_index + 128 == 0xC6) {  /* U+264 LATIN CAPITAL LETTER C WITH CIRCUMFLEX */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin3_index + 128 == 0xD0) {  /* U+65535 unknown */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin3_index + 128 == 0xD5) {  /* U+288 LATIN CAPITAL LETTER G WITH DOT ABOVE */
        put_dot_char('G');
        return;
    }
    if (latin3_index + 128 == 0xD8) {  /* U+284 LATIN CAPITAL LETTER G WITH CIRCUMFLEX */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin3_index + 128 == 0xDD) {  /* U+364 LATIN CAPITAL LETTER U WITH BREVE */
        put_breve_char('U');
        return;
    }
    if (latin3_index + 128 == 0xDE) {  /* U+348 LATIN CAPITAL LETTER S WITH CIRCUMFLEX */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin3_index + 128 == 0xE3) {  /* U+65535 unknown */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin3_index + 128 == 0xE5) {  /* U+267 LATIN SMALL LETTER C WITH DOT ABOVE */
        put_dot_char('c');
        return;
    }
    if (latin3_index + 128 == 0xE6) {  /* U+265 LATIN SMALL LETTER C WITH CIRCUMFLEX */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin3_index + 128 == 0xF0) {  /* U+65535 unknown */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin3_index + 128 == 0xF5) {  /* U+289 LATIN SMALL LETTER G WITH DOT ABOVE */
        put_dot_char('g');
        return;
    }
    if (latin3_index + 128 == 0xF8) {  /* U+285 LATIN SMALL LETTER G WITH CIRCUMFLEX */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin3_index + 128 == 0xFD) {  /* U+365 LATIN SMALL LETTER U WITH BREVE */
        put_breve_char('u');
        return;
    }
    if (latin3_index + 128 == 0xFE) {  /* U+349 LATIN SMALL LETTER S WITH CIRCUMFLEX */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin3_index + 128 == 0xFF) {  /* U+729 DOT ABOVE */
        put_dot_char(' ');
        return;
    }
}

static void latin4_enc(int latin4_index)
{
    char *s;

    s = latin4_2_ansi[latin4_index];
    if (s[0] != '0') {
        fprintRTF("\\'%s", s);
        return;
    }
    s = latin4_2_sym[latin4_index];
    if (s[0] != '0') {
        CmdSymbolChar(hex_to_int(s));
        return;
    }
    if (latin4_index + 128 == 0xA1) {  /* U+260 LATIN CAPITAL LETTER A WITH OGONEK */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin4_index + 128 == 0xA2) {  /* U+312 LATIN SMALL LETTER KRA */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin4_index + 128 == 0xA3) {  /* U+342 LATIN CAPITAL LETTER R WITH CEDILLA */
        put_cedilla_char('R');
        return;
    }
    if (latin4_index + 128 == 0xA5) {  /* U+296 LATIN CAPITAL LETTER I WITH TILDE */
        put_tilde_char('I');
        return;
    }
    if (latin4_index + 128 == 0xA6) {  /* U+315 LATIN CAPITAL LETTER L WITH CEDILLA */
        put_cedilla_char('L');
        return;
    }
    if (latin4_index + 128 == 0xAA) {  /* U+274 LATIN CAPITAL LETTER E WITH MACRON */
        put_macron_char('E',274);
        return;
    }
    if (latin4_index + 128 == 0xAB) {  /* U+290 LATIN CAPITAL LETTER G WITH CEDILLA */
        put_cedilla_char('G');
        return;
    }
    if (latin4_index + 128 == 0xAC) {  /* U+358 LATIN CAPITAL LETTER T WITH STROKE */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin4_index + 128 == 0xAE) {  /* U+381 LATIN CAPITAL LETTER Z WITH CARON */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin4_index + 128 == 0xB1) {  /* U+261 LATIN SMALL LETTER A WITH OGONEK */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin4_index + 128 == 0xB2) {  /* U+731 OGONEK */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin4_index + 128 == 0xB3) {  /* U+343 LATIN SMALL LETTER R WITH CEDILLA */
        put_cedilla_char('r');
        return;
    }
    if (latin4_index + 128 == 0xB5) {  /* U+297 LATIN SMALL LETTER I WITH TILDE */
        put_tilde_char('i');
        return;
    }
    if (latin4_index + 128 == 0xB6) {  /* U+316 LATIN SMALL LETTER L WITH CEDILLA */
        put_cedilla_char('l');
        return;
    }
    if (latin4_index + 128 == 0xB7) {  /* U+711 CARON */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin4_index + 128 == 0xBA) {  /* U+275 LATIN SMALL LETTER E WITH MACRON */
        put_macron_char('e',275);
        return;
    }
    if (latin4_index + 128 == 0xBB) {  /* U+291 LATIN SMALL LETTER G WITH CEDILLA */
        put_cedilla_char('g');
        return;
    }
    if (latin4_index + 128 == 0xBC) {  /* U+359 LATIN SMALL LETTER T WITH STROKE */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin4_index + 128 == 0xBD) {  /* U+330 LATIN CAPITAL LETTER ENG */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin4_index + 128 == 0xBE) {  /* U+382 LATIN SMALL LETTER Z WITH CARON */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin4_index + 128 == 0xBF) {  /* U+331 LATIN SMALL LETTER ENG */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin4_index + 128 == 0xC0) {  /* U+256 LATIN CAPITAL LETTER A WITH MACRON */
        put_macron_char('A',256);
        return;
    }
    if (latin4_index + 128 == 0xC7) {  /* U+302 LATIN CAPITAL LETTER I WITH OGONEK */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin4_index + 128 == 0xC8) {  /* U+268 LATIN CAPITAL LETTER C WITH CARON */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin4_index + 128 == 0xCA) {  /* U+280 LATIN CAPITAL LETTER E WITH OGONEK */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin4_index + 128 == 0xCC) {  /* U+278 LATIN CAPITAL LETTER E WITH DOT ABOVE */
        put_dot_char('E');
        return;
    }
    if (latin4_index + 128 == 0xCF) {  /* U+298 LATIN CAPITAL LETTER I WITH MACRON */
        put_macron_char('I',298);
        return;
    }
    if (latin4_index + 128 == 0xD0) {  /* U+272 LATIN CAPITAL LETTER D WITH STROKE */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin4_index + 128 == 0xD1) {  /* U+325 LATIN CAPITAL LETTER N WITH CEDILLA */
        put_cedilla_char('N');
        return;
    }
    if (latin4_index + 128 == 0xD2) {  /* U+332 LATIN CAPITAL LETTER O WITH MACRON */
        put_macron_char('O',332);
        return;
    }
    if (latin4_index + 128 == 0xD3) {  /* U+310 LATIN CAPITAL LETTER K WITH CEDILLA */
        put_cedilla_char('K');
        return;
    }
    if (latin4_index + 128 == 0xD9) {  /* U+370 LATIN CAPITAL LETTER U WITH OGONEK */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin4_index + 128 == 0xDD) {  /* U+360 LATIN CAPITAL LETTER U WITH TILDE */
        put_tilde_char('U');
        return;
    }
    if (latin4_index + 128 == 0xDE) {  /* U+362 LATIN CAPITAL LETTER U WITH MACRON */
        put_macron_char('U',362);
        return;
    }
    if (latin4_index + 128 == 0xE0) {  /* U+257 LATIN SMALL LETTER A WITH MACRON */
        put_macron_char('a',256);
        return;
    }
    if (latin4_index + 128 == 0xE7) {  /* U+303 LATIN SMALL LETTER I WITH OGONEK */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin4_index + 128 == 0xE8) {  /* U+269 LATIN SMALL LETTER C WITH CARON */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin4_index + 128 == 0xEA) {  /* U+281 LATIN SMALL LETTER E WITH OGONEK */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin4_index + 128 == 0xEC) {  /* U+279 LATIN SMALL LETTER E WITH DOT ABOVE */
        put_dot_char('e');
        return;
    }
    if (latin4_index + 128 == 0xEF) {  /* U+299 LATIN SMALL LETTER I WITH MACRON */
        put_macron_char('i',299);
        return;
    }
    if (latin4_index + 128 == 0xF0) {  /* U+273 LATIN SMALL LETTER D WITH STROKE */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin4_index + 128 == 0xF1) {  /* U+326 LATIN SMALL LETTER N WITH CEDILLA */
        put_cedilla_char('n');
        return;
    }
    if (latin4_index + 128 == 0xF2) {  /* U+333 LATIN SMALL LETTER O WITH MACRON */
        put_macron_char('o',333);
        return;
    }
    if (latin4_index + 128 == 0xF3) {  /* U+311 LATIN SMALL LETTER K WITH CEDILLA */
        put_cedilla_char('k');
        return;
    }
    if (latin4_index + 128 == 0xF9) {  /* U+371 LATIN SMALL LETTER U WITH OGONEK */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin4_index + 128 == 0xFD) {  /* U+361 LATIN SMALL LETTER U WITH TILDE */
        put_tilde_char('u');
        return;
    }
    if (latin4_index + 128 == 0xFE) {  /* U+363 LATIN SMALL LETTER U WITH MACRON */
        put_macron_char('u',363);
        return;
    }
    if (latin4_index + 128 == 0xFF) {  /* U+729 DOT ABOVE */
        put_dot_char(' ');
        return;
    }
}

static void latin5_enc(int latin5_index)
{
    char *s;

    s = latin5_2_ansi[latin5_index];
    if (s[0] != '0') {
        fprintRTF("\\'%s", s);
        return;
    }
    s = latin5_2_sym[latin5_index];
    if (s[0] != '0') {
        CmdSymbolChar(hex_to_int(s));
        return;
    }
    if (latin5_index + 128 == 0xD0) {  /* U+286 LATIN CAPITAL LETTER G WITH BREVE */
        put_breve_char('G');
        return;
    }
    if (latin5_index + 128 == 0xDD) {  /* U+304 LATIN CAPITAL LETTER I WITH DOT ABOVE */
        put_dot_char('I');
        return;
    }
    if (latin5_index + 128 == 0xDE) {  /* U+350 LATIN CAPITAL LETTER S WITH CEDILLA */
        put_cedilla_char('S');
        return;
    }
    if (latin5_index + 128 == 0xEA) {  /* U+281 LATIN SMALL LETTER E WITH OGONEK */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin5_index + 128 == 0xEC) {  /* U+279 LATIN SMALL LETTER E WITH DOT ABOVE */
        put_dot_char('e');
        return;
    }
    if (latin5_index + 128 == 0xEF) {  /* U+299 LATIN SMALL LETTER I WITH MACRON */
        put_macron_char('i',299);
        return;
    }
    if (latin5_index + 128 == 0xF0) {  /* U+287 LATIN SMALL LETTER G WITH BREVE */
        put_breve_char('g');
        return;
    }
    if (latin5_index + 128 == 0xFD) {  /* U+305 LATIN SMALL LETTER DOTLESS I */
        fprintRTF("i");
        return;
    }
    if (latin5_index + 128 == 0xFE) {  /* U+351 LATIN SMALL LETTER S WITH CEDILLA */
        put_cedilla_char('s');
        return;
    }
}

static void latin9_enc(int latin9_index)
{
    char *s;

    s = latin9_2_ansi[latin9_index];
    if (s[0] != '0') {
        fprintRTF("\\'%s", s);
        return;
    }
    s = latin9_2_sym[latin9_index];
    if (s[0] != '0') {
        CmdSymbolChar(hex_to_int(s));
        return;
    }
    if (latin9_index + 128 == 0xA4) {  /* U+8364 EURO SIGN*/
        fprintRTF("\\'80");
        return;
    }
    if (latin9_index + 128 == 0xB4) {  /* U+381 LATIN CAPITAL LETTER Z WITH CARON */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (latin9_index + 128 == 0xB8) {  /* U+382 LATIN SMALL LETTER Z WITH CARON */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
}

static void next_enc(int NeXt_index)
{
    char *s;

    s = next_2_ansi[NeXt_index];
    if (s[0] != '0') {
        fprintRTF("\\'%s", s);
        return;
    }
    s = next_2_sym[NeXt_index];
    if (s[0] != '0') {
        CmdSymbolChar(hex_to_int(s));
        return;
    }
    if (NeXt_index + 128 == 0xAE) {  /* U+64257 LATIN SMALL LIGATURE FI */
        fprintRTF("fi");
        return;
    }
    if (NeXt_index + 128 == 0xAF) {  /* U+64258 LATIN SMALL LIGATURE FL */
        fprintRTF("fl");
        return;
    }
    if (NeXt_index + 128 == 0xC1) {  /* U+715 MODIFIER LETTER GRAVE ACCENT */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (NeXt_index + 128 == 0xC6) {  /* U+728 BREVE */
        int num = RtfFontNumber("MT Extra");

        fprintRTF("{\\f%d\\'28}", num);
        return;
    }
    if (NeXt_index + 128 == 0xC7) {  /* U+729 DOT ABOVE */
        put_dot_char(' ');
        return;
    }
    if (NeXt_index + 128 == 0xCA) {  /* U+730 RING ABOVE */
        put_ring_char(' ');
        return;
    }
    if (NeXt_index + 128 == 0xCD) {  /* U+733 DOUBLE ACUTE ACCENT */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (NeXt_index + 128 == 0xCE) {  /* U+731 OGONEK */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (NeXt_index + 128 == 0xCF) {  /* U+711 CARON */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (NeXt_index + 128 == 0xE8) {  /* U+321 LATIN CAPITAL LETTER L WITH STROKE */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (NeXt_index + 128 == 0xF5) {  /* U+305 LATIN SMALL LETTER DOTLESS I */
        fprintRTF("i");
        return;
    }
    if (NeXt_index + 128 == 0xF8) {  /* U+322 LATIN SMALL LETTER L WITH STROKE */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (NeXt_index + 128 == 0xFE) {  /* U+65535 unknown */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
    if (NeXt_index + 128 == 0xFF) {  /* U+65535 unknown */

/*		fprintRTF("TRANSLATION");*/
        return;
    }
}

static void cp1251_enc(int cp1251_index)
{
    char *s;

    s = cp1251_2_cp1251[cp1251_index];
    CmdCyrillicStrChar(s);
}

static void cp855_enc(int cp855_index)
{
    char *s;

    s = cp855_2_cp1251[cp855_index];
    CmdCyrillicStrChar(s);
}

static void cp866_enc(int cp866_index)
{
    char *s;

    s = cp866_2_cp1251[cp866_index];
    CmdCyrillicStrChar(s);
}

static void koi8r_enc(int koi8r_index)
{
    char *s;

    s = koi8r_2_cp1251[koi8r_index];
    CmdCyrillicStrChar(s);
}

static void koi8u_enc(int koi8u_index)
{
    char *s;

    s = koi8u_2_cp1251[koi8u_index];
    CmdCyrillicStrChar(s);
}

static void maccyr_enc(int maccyr_index)
{
    char *s;

    s = maccyr_2_cp1251[maccyr_index];
    CmdCyrillicStrChar(s);
}

static void macukr_enc(int macukr_index)
{
    char *s;

    s = macukr_2_cp1251[macukr_index];
    CmdCyrillicStrChar(s);
}

void WriteEightBitChar(unsigned char cThis, FILE *f)
{
    int eightbit_index;

	if ( cThis <= 127) {
		fputc(cThis, f);
		return;
	}

	eightbit_index = cThis - 128;
    diagnostics(6, "WriteEightBitChar char=%d index=%d encoding=%s", (unsigned int) cThis, eightbit_index, g_charset_encoding_name);

    if (strcmp(g_charset_encoding_name, "raw") == 0)
        fprintRTF("\\'%2X", (unsigned char) cThis);
    else if (strcmp(g_charset_encoding_name, "applemac") == 0)
        applemac_enc(eightbit_index);
    else if (strcmp(g_charset_encoding_name, "cp437") == 0)
        cp437_enc(eightbit_index);
    else if (strcmp(g_charset_encoding_name, "cp850") == 0)
        cp850_enc(eightbit_index);
    else if (strcmp(g_charset_encoding_name, "cp852") == 0)
        cp852_enc(eightbit_index);
    else if (strcmp(g_charset_encoding_name, "cp865") == 0)
        cp865_enc(eightbit_index);
    else if (strcmp(g_charset_encoding_name, "437") == 0)
        cp437_enc(eightbit_index);
    else if (strcmp(g_charset_encoding_name, "850") == 0)
        cp850_enc(eightbit_index);
    else if (strcmp(g_charset_encoding_name, "852") == 0)
        cp852_enc(eightbit_index);
    else if (strcmp(g_charset_encoding_name, "865") == 0)
        cp865_enc(eightbit_index);
    else if (strcmp(g_charset_encoding_name, "decmulti") == 0)
        decmulti_enc(eightbit_index);
    else if (strcmp(g_charset_encoding_name, "cp1250") == 0)
        cp1250_enc(eightbit_index);
    else if (strcmp(g_charset_encoding_name, "cp1252") == 0)
        cp1252_enc(eightbit_index);
    else if (strcmp(g_charset_encoding_name, "1250") == 0)
        cp1250_enc(eightbit_index);
    else if (strcmp(g_charset_encoding_name, "1252") == 0)
        cp1252_enc(eightbit_index);
    else if (strcmp(g_charset_encoding_name, "latin1") == 0)
        latin1_enc(eightbit_index);
    else if (strcmp(g_charset_encoding_name, "latin2") == 0)
        latin2_enc(eightbit_index);
    else if (strcmp(g_charset_encoding_name, "latin3") == 0)
        latin3_enc(eightbit_index);
    else if (strcmp(g_charset_encoding_name, "latin4") == 0)
        latin4_enc(eightbit_index);
    else if (strcmp(g_charset_encoding_name, "latin5") == 0)
        latin5_enc(eightbit_index);
    else if (strcmp(g_charset_encoding_name, "latin9") == 0)
        latin9_enc(eightbit_index);
    else if (strcmp(g_charset_encoding_name, "next") == 0)
        next_enc(eightbit_index);
    else if (strcmp(g_charset_encoding_name, "cp1251") == 0)
        cp1251_enc(eightbit_index);
    else if (strcmp(g_charset_encoding_name, "cp855") == 0)
        cp855_enc(eightbit_index);
    else if (strcmp(g_charset_encoding_name, "cp866") == 0)
        cp866_enc(eightbit_index);
    else if (strcmp(g_charset_encoding_name, "1251") == 0)
        cp1251_enc(eightbit_index);
    else if (strcmp(g_charset_encoding_name, "855") == 0)
        cp855_enc(eightbit_index);
    else if (strcmp(g_charset_encoding_name, "866") == 0)
        cp866_enc(eightbit_index);
    else if (strcmp(g_charset_encoding_name, "koi8-r") == 0)
        koi8r_enc(eightbit_index);
    else if (strcmp(g_charset_encoding_name, "koi8-u") == 0)
        koi8u_enc(eightbit_index);
    else if (strcmp(g_charset_encoding_name, "maccyr") == 0)
        maccyr_enc(eightbit_index);
    else if (strcmp(g_charset_encoding_name, "macukr") == 0)
        macukr_enc(eightbit_index);

}
