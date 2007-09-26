
/* equations.c - Translate TeX equations

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
#include <ctype.h>
#include "main.h"
#include "convert.h"
#include "commands.h"
#include "stack.h"
#include "fonts.h"
#include "cfg.h"
#include "ignore.h"
#include "parser.h"
#include "equations.h"
#include "counters.h"
#include "funct1.h"
#include "lengths.h"
#include "utils.h"
#include "graphics.h"
#include "xrefs.h"
#include "chars.h"
#include "preamble.h"

int g_equation_column = 1;

int script_shift(void)
{
    return CurrentFontSize() / 3.0;
}

int script_size(void)
{
    return CurrentFontSize() / 1.2;
}

void CmdNonumber(int code)

/******************************************************************************
 purpose   : Handles \nonumber to suppress numbering in equations
 ******************************************************************************/
{
    if (g_processing_eqnarray || !g_processing_tabular)
        g_suppress_equation_number = TRUE;
}

static char *SlurpDollarEquation(void)

/******************************************************************************
 purpose   : reads an equation delimited by $...$
 			 this routine is needed to handle $ x \mbox{if $x$} $ 
 ******************************************************************************/
{
    int brace = 0;
    int slash = 0;
    int i;
    char *s, *t, *u;

    s = malloc(1024 * sizeof(char));
    t = s;

    for (i = 0; i < 1024; i++) {
        *t = getTexChar();
        if (*t == '\\')
            slash++;
        else if (*t == '{' && even(slash) &&
          !(i > 5 && strncmp(t - 5, "\\left{", 6) == 0) && !(i > 6 && strncmp(t - 6, "\\right{", 7) == 0))
            brace++;
        else if (*t == '}' && even(slash) &&
          !(i > 5 && !strncmp(t - 5, "\\left}", 6)) && !(i > 6 && !strncmp(t - 6, "\\right}", 7)))
            brace--;
        else if (*t == '$' && even(slash) && brace == 0) {
            break;
        } else
            slash = 0;
        t++;
    }
    *t = '\0';

    u = strdup(s);              /* return much smaller string */
    free(s);                    /* release the big string */
    return u;
}

static void SlurpEquation(int code, char **pre, char **eq, char **post)
{
    int true_code = code & ~ON;

    switch (true_code) {

        case EQN_MATH:
            diagnostics(4, "SlurpEquation() ... \\begin{math}");
            *pre = strdup("\\begin{math}");
            *post = strdup("\\end{math}");
            *eq = getTexUntil(*post, 0);
            break;

        case EQN_DOLLAR:
            diagnostics(4, "SlurpEquation() ... $");
            *pre = strdup("$");
            *post = strdup("$");
            *eq = SlurpDollarEquation();
            break;

        case EQN_RND_OPEN:
            diagnostics(4, "SlurpEquation() ... \\(");
            *pre = strdup("\\(");
            *post = strdup("\\)");
            *eq = getTexUntil(*post, 0);
            break;

        case EQN_DISPLAYMATH:
            diagnostics(4, "SlurpEquation -- displaymath");
            *pre = strdup("\\begin{displaymath}");
            *post = strdup("\\end{displaymath}");
            *eq = getTexUntil(*post, 0);
            break;

        case EQN_EQUATION_STAR:
            diagnostics(4, "SlurpEquation() -- equation*");
            *pre = strdup("\\begin{equation*}");
            *post = strdup("\\end{equation*}");
            *eq = getTexUntil(*post, 0);
            break;

        case EQN_DOLLAR_DOLLAR:
            diagnostics(4, "SlurpEquation() -- $$");
            *pre = strdup("$$");
            *post = strdup("$$");
            *eq = getTexUntil(*post, 0);
            break;

        case EQN_BRACKET_OPEN:
            diagnostics(4, "SlurpEquation()-- \\[");
            *pre = strdup("\\[");
            *post = strdup("\\]");
            *eq = getTexUntil(*post, 0);
            break;

        case EQN_EQUATION:
            diagnostics(4, "SlurpEquation() -- equation");
            *pre = strdup("\\begin{equation}");
            *post = strdup("\\end{equation}");
            *eq = getTexUntil(*post, 0);
            break;

        case EQN_ARRAY_STAR:
            diagnostics(4, "Entering CmdDisplayMath -- eqnarray* ");
            *pre = strdup("\\begin{eqnarray*}");
            *post = strdup("\\end{eqnarray*}");
            *eq = getTexUntil(*post, 0);
            break;

        case EQN_ARRAY:
            diagnostics(4, "SlurpEquation() --- eqnarray");
            *pre = strdup("\\begin{eqnarray}");
            *post = strdup("\\end{eqnarray}");
            *eq = getTexUntil(*post, 0);
            break;

        case EQN_ALIGN_STAR:
            diagnostics(4, "Entering CmdDisplayMath -- align* ");
            *pre = strdup("\\begin{align*}");
            *post = strdup("\\end{align*}");
            *eq = getTexUntil(*post, 0);
            break;

        case EQN_ALIGN:
            diagnostics(4, "SlurpEquation() --- align");
            *pre = strdup("\\begin{align}");
            *post = strdup("\\end{align}");
            *eq = getTexUntil(*post, 0);
            break;
    }
}

static int EquationNeedsFields(char *eq)

/******************************************************************************
 purpose   : Determine if equation needs EQ field for RTF conversion
 ******************************************************************************/
{
    if (strstr(eq, "\\frac"))
        return 1;
    if (strstr(eq, "\\sum"))
        return 1;
    if (strstr(eq, "\\int"))
        return 1;
    if (strstr(eq, "\\iint"))
        return 1;
    if (strstr(eq, "\\iiint"))
        return 1;
    if (strstr(eq, "\\prod"))
        return 1;
    if (strstr(eq, "\\begin{array}"))
        return 1;
    if (strstr(eq, "\\left"))
        return 1;
    if (strstr(eq, "\\right"))
        return 1;
    if (strstr(eq, "\\root"))
        return 1;
    if (strstr(eq, "\\sqrt"))
        return 1;
    if (strstr(eq, "\\over"))
        return 1;
    if (strstr(eq, "\\stackrel"))
        return 1;
    if (strstr(eq, "_") || strstr(eq, "^"))
        return 1;
    if (strstr(eq, "\\dfrac"))
        return 1;
    if (strstr(eq, "\\lim"))
        return 1;
    if (strstr(eq, "\\liminf"))
        return 1;
    if (strstr(eq, "\\limsup"))
        return 1;
    if (strstr(eq, "\\overline"))
        return 1;
    return 0;
}

static void WriteEquationAsRawLatex(char *pre, char *eq, char *post)

/******************************************************************************
 purpose   : Writes equation to RTF file as text of COMMENT field
 ******************************************************************************/
{
    fprintRTF("<<:");
    while (*pre)
        putRtfCharEscaped(*pre++);
    while (*eq)
        putRtfCharEscaped(*eq++);
    while (*post)
        putRtfCharEscaped(*post++);
    fprintRTF(":>>");
}

static void WriteEquationAsComment(char *pre, char *eq, char *post)

/******************************************************************************
 purpose   : Writes equation to RTF file as text of COMMENT field
 ******************************************************************************/
{
    fprintRTF("{\\field{\\*\\fldinst{ COMMENTS \" ");
    while (*pre)
        putRtfCharEscaped(*pre++);
    while (*eq)
        putRtfCharEscaped(*eq++);
    while (*post)
        putRtfCharEscaped(*post++);
    fprintRTF("\" }{ }}{\\fldrslt }}");
}

static char *SaveEquationAsFile(char *pre, char *eq_with_spaces, char *post)
{
    FILE *f;
    char name[15];
    char *tmp_dir, *fullname, *texname, *eq;
    static int file_number = 0;

    if (!pre || !eq_with_spaces || !post)
        return NULL;

    eq = strdup_noendblanks(eq_with_spaces);

/* create needed file names */
    file_number++;
    tmp_dir = getTmpPath();
    snprintf(name, 15, "l2r_%04d", file_number);
    fullname = strdup_together(tmp_dir, name);
    texname = strdup_together(fullname, ".tex");

    diagnostics(4, "SaveEquationAsFile =%s", texname);

    f = fopen(texname, "w");
    while (eq && (*eq == '\n' || *eq == ' '))
        eq++;                   /* skip whitespace */
    if (f) {
        fprintf(f, "%s", g_preamble);
        fprintf(f, "\\thispagestyle{empty}\n");
        fprintf(f, "\\begin{document}\n");
        fprintf(f, "\\setcounter{equation}{%d}\n", getCounter("equation"));
        if ((strcmp(pre, "$") == 0) || (strcmp(pre, "\\begin{math}") == 0) || (strcmp(pre, "\\(") == 0)) {
            fprintf(f, "%%INLINE_DOT_ON_BASELINE\n");
            fprintf(f, "%s\n.\\quad %s\n%s", pre, eq, post);
        } else if (strstr(pre, "equation"))
            fprintf(f, "$$%s$$", eq);
        else
            fprintf(f, "%s\n%s\n%s", pre, eq, post);
        fprintf(f, "\n\\end{document}");
        fclose(f);
    } else {
        free(fullname);
        fullname = NULL;
    }

    free(eq);
    free(tmp_dir);
    free(texname);
    return (fullname);
}


void WriteLatexAsBitmap(char *pre, char *eq, char *post)

/******************************************************************************
 purpose   : Convert LaTeX to Bitmap and write to RTF file
 ******************************************************************************/
{
    char *p, *name;
    double scale;

    diagnostics(4, "Entering WriteEquationAsBitmap");

    if (eq == NULL)
        return;

    scale = g_png_equation_scale;
    if (strstr(pre, "music") || strstr(pre, "figure") 
                             || strstr(pre, "picture")
                             || strstr(pre, "longtable") )
        scale = g_png_figure_scale;

/* suppress bitmap equation numbers in eqnarrays with zero or one \label{}'s*/
    if (strcmp(pre, "\\begin{eqnarray}") == 0) {
        p = strstr(eq, "\\label");
        if (p != NULL && strlen(p) > 6) /* found one ... is there a second? */
            p = strstr(p + 6, "\\label");
        if (p == NULL)
            name = SaveEquationAsFile("\\begin{eqnarray*}", eq, "\\end{eqnarray*}");
        else
            name = SaveEquationAsFile(pre, eq, post);

    } else if (strcmp(pre, "\\begin{align}") == 0) {
        p = strstr(eq, "\\label");
        if (p != NULL && strlen(p) > 6) /* found one ... is there a second? */
            p = strstr(p + 6, "\\label");
        if (p == NULL)
            name = SaveEquationAsFile("\\begin{align*}", eq, "\\end{align*}");
        else
            name = SaveEquationAsFile(pre, eq, post);
    } else

        name = SaveEquationAsFile(pre, eq, post);

    PutLatexFile(name, 0, 0, scale, pre);
}

static void PrepareRtfEquation(int code, int EQ_Needed)
{
    int width, a, b, c;

    width = getLength("textwidth");
    a = (int) (0.45 * width);
    b = (int) (0.50 * width);
    c = (int) (0.55 * width);

    switch (code) {

        case EQN_MATH:
            diagnostics(4, "PrepareRtfEquation ... \\begin{math}");
            SetTexMode(MODE_MATH);
            break;

        case EQN_DOLLAR:
            diagnostics(4, "PrepareRtfEquation ... $");
            fprintRTF("{");
            SetTexMode(MODE_MATH);
            break;

        case EQN_ENSUREMATH:
            diagnostics(4, "PrepareRtfEquation ... \ensuremath{}");
            fprintRTF("{");
            SetTexMode(MODE_MATH);
            break;

        case EQN_RND_OPEN:
            diagnostics(4, "PrepareRtfEquation ... \\(");
            fprintRTF("{");
            SetTexMode(MODE_MATH);
            break;

        case EQN_DOLLAR_DOLLAR:
            diagnostics(4, "PrepareRtfEquation -- $$");
            CmdEndParagraph(0);
            SetTexMode(MODE_DISPLAYMATH);
            g_show_equation_number = FALSE;
            fprintRTF("{\\pard\\tqc\\tx%d\\tab ", b);
            break;

        case EQN_BRACKET_OPEN:
        case EQN_DISPLAYMATH:
            if (code == EQN_DISPLAYMATH)
                diagnostics(4, "PrepareRtfEquation -- displaymath");
            else
                diagnostics(4, "PrepareRtfEquation -- \\[");
            g_show_equation_number = FALSE;
            fprintRTF("\\par\\par\n\\pard");
            fprintRTF("\\tqc\\tx%d", b);
            fprintRTF("\\tab ");
            SetTexMode(MODE_DISPLAYMATH);
            break;

        case EQN_EQUATION_STAR:
            diagnostics(4, "PrepareRtfEquation -- equation*");
            g_show_equation_number = FALSE;
            fprintRTF("\\par\\par\n\\pard");
            fprintRTF("\\tqc\\tx%d", b);
            fprintRTF("\\tab ");
            SetTexMode(MODE_DISPLAYMATH);
            break;

        case EQN_EQUATION:
            diagnostics(4, "PrepareRtfEquation -- equation");
            g_equation_column = 5;  /* avoid adding \tabs when finishing */
            g_show_equation_number = TRUE;
            g_suppress_equation_number = FALSE;
            fprintRTF("\\par\\par\n\\pard");
            fprintRTF("\\tqc\\tx%d\\tqr\\tx%d", b, width);
            fprintRTF("\\tab ");
            SetTexMode(MODE_DISPLAYMATH);
            break;

        case EQN_ARRAY_STAR:
            diagnostics(4, "PrepareRtfEquation -- eqnarray* ");
            g_show_equation_number = FALSE;
            g_processing_eqnarray = TRUE;
            g_processing_tabular = TRUE;
            g_suppress_equation_number = FALSE;
            g_equation_column = 1;
            fprintRTF("\\par\\par\n\\pard");
            fprintRTF("\\tqr\\tx%d\\tqc\\tx%d\\tql\\tx%d", a, b, c);
            fprintRTF("\\tab ");
            SetTexMode(MODE_DISPLAYMATH);
            break;

        case EQN_ARRAY:
            diagnostics(4, "PrepareRtfEquation --- eqnarray");
            g_show_equation_number = TRUE;
            g_processing_eqnarray = TRUE;
            g_processing_tabular = TRUE;
            g_equation_column = 1;
            fprintRTF("\\par\\par\n\\pard");
            fprintRTF("\\tqr\\tx%d\\tqc\\tx%d\\tql\\tx%d\\tqr\\tx%d", a, b, c, width);
            fprintRTF("\\tab ");
            SetTexMode(MODE_DISPLAYMATH);
            break;

        case EQN_ALIGN_STAR:
            diagnostics(4, "PrepareRtfEquation -- align* ");
            g_show_equation_number = FALSE;
            g_processing_eqnarray = TRUE;
            g_processing_tabular = TRUE;
            g_equation_column = 1;
            fprintRTF("\\par\\par\n\\pard");
            fprintRTF("\\tqr\\tx%d\\tql\\tx%d", a, b);
            fprintRTF("\\tab ");
            SetTexMode(MODE_DISPLAYMATH);
            break;

        case EQN_ALIGN:
            diagnostics(4, "PrepareRtfEquation --- align");
            g_show_equation_number = TRUE;
            g_processing_eqnarray = TRUE;
            g_processing_tabular = TRUE;
            g_suppress_equation_number = FALSE;
            g_equation_column = 1;
            fprintRTF("\\par\\par\n\\pard");
            fprintRTF("\\tqr\\tx%d\\tql\\tx%d\\tqr\\tx%d", a, b, width);
            fprintRTF("\\tab ");
            SetTexMode(MODE_DISPLAYMATH);
            break;
            
        default:
            diagnostics(ERROR, "calling PrepareRtfEquation with OFF code");
            break;
    }

    if (g_fields_use_EQ && EQ_Needed && g_processing_fields == 0) {
        fprintRTF("{\\field{\\*\\fldinst{ EQ ");
        g_processing_fields++;
    }

}

static char *CreateEquationLabel(void)
{
	char *number = malloc(30);
	int n;
	
	n = getCounter("equation");
	
	if (g_document_type == FORMAT_REPORT ||
	    g_document_type == FORMAT_BOOK )
		snprintf(number, 29, "%d.%d", getCounter("chapter"), getCounter("equation"));
	else
		snprintf(number, 29, "%d", getCounter("equation"));
		
	return number;
}

	


static void FinishRtfEquation(int code, int EQ_Needed)
{
    if (g_fields_use_EQ && EQ_Needed && g_processing_fields == 1) {
        fprintRTF("}}{\\fldrslt }}");
        g_processing_fields--;
    }

    switch (code) {

        case EQN_MATH:
            diagnostics(4, "FinishRtfEquation -- \\end{math}");
            CmdIndent(INDENT_INHIBIT);
            SetTexMode(MODE_HORIZONTAL);
            break;

        case EQN_DOLLAR:
            diagnostics(4, "FinishRtfEquation -- $");
            fprintRTF("}");
            SetTexMode(MODE_HORIZONTAL);
            break;

        case EQN_ENSUREMATH:
            diagnostics(4, "FinishRtfEquation -- \ensuremath{}");
            fprintRTF("}");
            SetTexMode(MODE_HORIZONTAL);
            break;

        case EQN_RND_OPEN:
            diagnostics(4, "FinishRtfEquation -- \\)");
            fprintRTF("}");
            SetTexMode(MODE_HORIZONTAL);
            break;

        case EQN_DOLLAR_DOLLAR:
            diagnostics(4, "FinishRtfEquation -- $$");
            CmdEndParagraph(0);
            CmdIndent(INDENT_INHIBIT);
            fprintRTF("}");
            break;

        case EQN_BRACKET_OPEN:
        case EQN_DISPLAYMATH:
            if (code == EQN_DISPLAYMATH)
                diagnostics(4, "FinishRtfEquation -- displaymath");
            else
                diagnostics(4, "FinishRtfEquation -- \\]");
            CmdEndParagraph(0);
            CmdIndent(INDENT_INHIBIT);
            break;

        case EQN_EQUATION_STAR:
            diagnostics(4, "FinishRtfEquation -- equation*");
            CmdEndParagraph(0);
            CmdIndent(INDENT_INHIBIT);
            break;

        case EQN_ARRAY_STAR:
            diagnostics(4, "FinishRtfEquation -- eqnarray* ");
            CmdEndParagraph(0);
            CmdIndent(INDENT_INHIBIT);
            g_processing_eqnarray = FALSE;
            g_processing_tabular = FALSE;
            break;

        case EQN_ALIGN_STAR:
            diagnostics(4, "FinishRtfEquation -- align* ");
            CmdEndParagraph(0);
            CmdIndent(INDENT_INHIBIT);
            g_processing_eqnarray = FALSE;
            g_processing_tabular = FALSE;
            break;

        case EQN_EQUATION:
        case EQN_ARRAY:
        case EQN_ALIGN:
            diagnostics(4, "FinishRtfEquation --- equation or eqnarray or align");
            if (g_show_equation_number && !g_suppress_equation_number) {
                char *number;

                incrementCounter("equation");
                for (; g_equation_column < 3; g_equation_column++)
                    fprintRTF("\\tab ");
                fprintRTF("\\tab{\\b0 (");
                number = CreateEquationLabel();
                InsertBookmark(g_equation_label, number);
                free(number);
                if (g_equation_label) {
                    free(g_equation_label);
                    g_equation_label = NULL;
                }
                fprintRTF(")}");
            }
            g_processing_eqnarray = FALSE;
            g_processing_tabular = FALSE;
            CmdEndParagraph(0);
            CmdIndent(INDENT_INHIBIT);
            break;

        default:
            diagnostics(ERROR, "calling FinishRtfEquation with OFF code");
            break;
    }
}

static char *scanback(char *s, char *t)

/******************************************************************************
 purpose   : Find '{' that starts a fraction designated by \over 
 			 Consider \int_0 { \{a_{x+y} + b \} \over a_{x+y} }
 	                  ^		 ^                  ^
 	                  s    result               t
 ******************************************************************************/
{
    int braces = 1;

    if (!s || !t || t < s)
        return NULL;

    while (braces && s < t) {
        if (*t == '{' && !(*(t - 1) == '\\') && /* avoid \{ */
          !(s + 5 <= t && !strncmp(t - 5, "\\left", 5)) &&  /* avoid \left{ */
          !(s + 6 <= t && !strncmp(t - 6, "\\right", 6))    /* avoid \right{ */
          )
            braces--;

        if (*t == '}' && !(*(t - 1) == '\\') && /* avoid \} */
          !(s + 5 <= t && !strncmp(t - 5, "\\left", 5)) &&  /* avoid \left} */
          !(s + 6 <= t && !strncmp(t - 6, "\\right", 6))    /* avoid \right} */
          )
            braces++;

        if (braces)
            t--;
    }
    return t;
}

static char *scanahead(char *s)

/******************************************************************************
 purpose   : Find '}' that ends a fraction designated by \over 
 			 Consider \int_0 { \{a_{x+y} + b \} \over a_{x+y} }
 	                                            ^             ^ 
 	                                            s             t
 ******************************************************************************/
{
    char *t;
    int braces = 1;
    int slashes = 0;

    if (!s)
        return NULL;
    t = s;

    while (braces && t && *t != '\0') {

        if (even(slashes)) {
            if (*t == '}' && !(s + 5 <= t && !strncmp(t - 5, "\\left", 5)) &&   /* avoid \left} */
              !(s + 6 <= t && !strncmp(t - 6, "\\right", 6))    /* avoid \right} */
              )
                braces--;

            if (*t == '{' && !(s + 5 <= t && !strncmp(t - 5, "\\left", 5)) &&   /* avoid \left{ */
              !(s + 6 <= t && !strncmp(t - 6, "\\right", 6))    /* avoid \right{ */
              )
                braces++;
        }

        if (*t == '\\')
            slashes++;
        else
            slashes = 0;
        if (braces)
            t++;
    }

    return t;
}

static void ConvertOverToFrac(char **equation)

/******************************************************************************
 purpose   : Convert {A \over B} to \frac{A}{B} 
 ******************************************************************************/
{
    char cNext, *eq, *mid, *first, *last, *s, *p, *t;

    eq = *equation;
    p = eq;
    diagnostics(4, "ConvertOverToFrac before <%s>", p);

    while ((mid = strstr(p, "\\over")) != NULL) {
        diagnostics(5, "Matched at <%s>", mid);
        cNext = *(mid + 5);
        diagnostics(5, "Next char is <%c>", cNext);
        if (!isalpha((int) cNext)) {

            first = scanback(eq, mid);
            diagnostics(6, "first = <%s>", first);
            last = scanahead(mid);
            diagnostics(6, "last = <%s>", last);

            strncpy(mid, "  }{ ", 5);
            diagnostics(6, "mid = <%s>", mid);
            s = (char *) malloc(strlen(eq) + sizeof("\\frac{}") + 1);
            t = s;

            strncpy(t, eq, (size_t) (first - eq));  /* everything up to {A\over B} */
            t += first - eq;

            strncpy(t, "\\frac", 5);    /* insert new \frac */
            t += 5;
            if (*first != '{') {
                *t = '{';
                t++;
            }
            /* add { if missing */
            strncpy(t, first, (size_t) (last - first)); /* copy A}{B */
            t += last - first;

            if (*last != '}') {
                *t = '}';
                t++;
            }
            /* add } if missing */
            strcpy(t, last);    /* everything after {A\over B} */
            free(eq);
            eq = s;
            p = eq;
        } else
            p = mid + 5;
        diagnostics(6, "ConvertOverToFrac current <%s>", eq);
    }
    *equation = eq;
    diagnostics(4, "ConvertOverToFrac after <%s>", eq);
}

static void WriteEquationAsRTF(int code, char **eq)

/******************************************************************************
 purpose   : Translate equation to RTF 
 ******************************************************************************/
{
    int EQ_Needed;

    EQ_Needed = EquationNeedsFields(*eq);

    PrepareRtfEquation(code, EQ_Needed);
    ConvertOverToFrac(eq);
    fprintRTF("{");
    ConvertString(*eq);
    fprintRTF("}");
    FinishRtfEquation(code, EQ_Needed);
}

void CmdEquation(int code)

/******************************************************************************
 purpose   : Handle everything associated with equations
 ******************************************************************************/
{
    char *pre=NULL, *eq, *post=NULL;
    int inline_equation, number, true_code;

    true_code = code & ~ON;

    if (!(code & ON || code==EQN_ENSUREMATH))
        return;

    if (code==EQN_ENSUREMATH)
    	eq = getBraceParam();
    else
    	SlurpEquation(code, &pre, &eq, &post);

    diagnostics(4, "Entering CmdEquation --------%x\n<%s>\n<%s>\n<%s>", code, pre, eq, post);

    inline_equation = (true_code == EQN_MATH) || 
                      (true_code == EQN_DOLLAR) || 
                      (true_code == EQN_RND_OPEN) ||
                      (true_code == EQN_ENSUREMATH);

    number = getCounter("equation");

    if (g_equation_comment)
        WriteEquationAsComment(pre, eq, post);

    if (g_equation_raw_latex)
        WriteEquationAsRawLatex(pre, eq, post);

    diagnostics(4, "inline=%d  inline_bitmap=%d", inline_equation, g_equation_inline_bitmap);
    diagnostics(4, "inline=%d display_bitmap=%d", inline_equation, g_equation_display_bitmap);
    diagnostics(4, "inline=%d  inline_rtf   =%d", inline_equation, g_equation_inline_rtf);
    diagnostics(4, "inline=%d display_rtf   =%d", inline_equation, g_equation_display_rtf);

    if ((inline_equation && g_equation_inline_bitmap) || (!inline_equation && g_equation_display_bitmap)) {
        if (true_code == EQN_ARRAY) {
            char *s, *t;

            s = eq;
            diagnostics(4, "eqnarray whole = <%s>", s);
            do {
                t = strstr(s, "\\\\");
                if (t)
                    *t = '\0';
                diagnostics(4, "eqnarray piece = <%s>", s);
                if (strstr(s, "\\nonumber"))
                    g_suppress_equation_number = TRUE;
                else
                    g_suppress_equation_number = FALSE;

                PrepareRtfEquation(true_code, FALSE);
                WriteLatexAsBitmap("\\begin{eqnarray*}", s, "\\end{eqnarray*}");
                FinishRtfEquation(true_code, FALSE);
                if (t)
                    s = t + 2;
            } while (t);
            
        } else  if (true_code == EQN_ALIGN) {
            char *s, *t;

            s = eq;
            diagnostics(4, "align whole = <%s>", s);
            do {
                t = strstr(s, "\\\\");
                if (t)
                    *t = '\0';
                diagnostics(4, "array piece = <%s>", s);
                if (strstr(s, "\\notag"))
                    g_suppress_equation_number = TRUE;
                else
                    g_suppress_equation_number = FALSE;

                PrepareRtfEquation(true_code, FALSE);
                WriteLatexAsBitmap("\\begin{align*}", s, "\\end{align*}");
                FinishRtfEquation(true_code, FALSE);
                if (t)
                    s = t + 2;
            } while (t);
            
        } else {
            PrepareRtfEquation(true_code, FALSE);
            WriteLatexAsBitmap(pre, eq, post);
            FinishRtfEquation(true_code, FALSE);
        }
    }

    if ((inline_equation && g_equation_inline_rtf) || (!inline_equation && g_equation_display_rtf)) {
        setCounter("equation", number);
        WriteEquationAsRTF(true_code, &eq);
    }

/* balance \begin{xxx} with \end{xxx} call */
    if (true_code == EQN_MATH     || true_code == EQN_DISPLAYMATH   ||
        true_code == EQN_EQUATION || true_code == EQN_EQUATION_STAR ||
        true_code == EQN_ARRAY    || true_code == EQN_ARRAY_STAR    ||
        true_code == EQN_ALIGN    || true_code == EQN_ALIGN_STAR)
        ConvertString(post);

    free(pre);
    free(eq);
    free(post);

}

/******************************************************************************
 purpose   : Handle \ensuremath
 ******************************************************************************/
void CmdEnsuremath(int code)
{
	int mode = GetTexMode();

    diagnostics(2, "Entering CmdEnsuremath");
	if (mode == MODE_MATH || mode == MODE_DISPLAYMATH) {
		char *eq = getBraceParam();
        diagnostics(2, "already in math mode <%s>", eq);
		ConvertString(eq);
		free(eq);
	} else {
        diagnostics(2, "need to start new equation");
		CmdEquation(EQN_ENSUREMATH);
	}
}


void CmdRoot(int code)

/******************************************************************************
 purpose: converts \sqrt{x} or \root[\alpha]{x+y}
******************************************************************************/
{
    char *root = NULL;
    char *power = NULL;

    power = getBracketParam();
    root = getBraceParam();

    if (g_fields_use_EQ) {
        fprintRTF(" \\\\R(");
        if (power && strlen(power) > 0)
            ConvertString(power);
        fprintRTF("%c", g_field_separator);
        ConvertString(root);
        fprintRTF(")");
    } else {
        if (power && strlen(power) > 0) {
            fprintRTF("{\\up%d\\fs%d ", script_shift(), script_size());
            ConvertString(power);
            fprintRTF("}");
        }
        CmdSymbolChar(0xd6);
        fprintRTF("(");
        ConvertString(root);
        fprintRTF(")");
    }

    if (power)
        free(power);
    if (root)
        free(root);
}

void CmdFraction(int code)

/******************************************************************************
 purpose: converts \frac{x}{y}
******************************************************************************/
{
    char *denominator, *numerator, *nptr, *dptr;

    numerator = getBraceParam();
    nptr = strdup_noendblanks(numerator);
    skipSpaces();
    denominator = getBraceParam();
    dptr = strdup_noendblanks(denominator);

    free(numerator);
    free(denominator);
    diagnostics(4, "CmdFraction -- numerator   = <%s>", nptr);
    diagnostics(4, "CmdFraction -- denominator = <%s>", dptr);

    if (g_fields_use_EQ) {
        fprintRTF(" \\\\F(");
        ConvertString(nptr);
        fprintRTF("%c", g_field_separator);
        ConvertString(dptr);
        fprintRTF(")");
    } else {
        fprintRTF(" ");
        ConvertString(nptr);
        fprintRTF("/");
        ConvertString(dptr);
        fprintRTF(" ");
    }

    free(nptr);
    free(dptr);
}

void CmdArrows(int code)

/******************************************************************************
 converts: amssymb \leftrightarrows and \rightleftarrows
 ******************************************************************************/
{
    int size = (int) (CurrentFontSize() / 4.5);

    fprintRTF(" \\\\o ({\\up%d ", size);

    switch (code) {
        case LEFT_RIGHT:
            ConvertString("\\leftarrow");
            fprintRTF("}%c{\\dn%d ", g_field_separator, size);
            ConvertString("\\rightarrow");
            break;

        case RIGHT_LEFT:
            ConvertString("\\rightarrow");
            fprintRTF("}%c{\\dn%d ", g_field_separator, size);
            ConvertString("\\leftarrow");
            break;

        case RIGHT_RIGHT:
            ConvertString("\\rightarrow");
            fprintRTF("}%c{\\dn%d ", g_field_separator, size);
            ConvertString("\\rightarrow");
            break;

        case LEFT_LEFT:
            ConvertString("\\leftarrow");
            fprintRTF("}%c{\\dn%d ", g_field_separator, size);
            ConvertString("\\leftarrow");
            break;

        case LONG_LEFTRIGHT:
            ConvertString("\\longleftarrow");
            fprintRTF("}%c{\\dn%d ", g_field_separator, size);
            ConvertString("\\longrightarrow");
            break;

        case LONG_RIGHTLEFT:
            ConvertString("\\longrightarrow");
            fprintRTF("}%c{\\dn%d ", g_field_separator, size);
            ConvertString("\\longleftarrow");
            break;

        case LONG_LEFT:
            ConvertString("\\leftarrow");
        	CmdSymbolChar(0xbe);  /* extension character */
            break;

        case LONG_RIGHT:
        	CmdSymbolChar(0xbe);  /* extension character */
            ConvertString("\\rightarrow");
            break;
    }
    fprintRTF("}) ");
}

void CmdLim(int code)

/******************************************************************************
 purpose: handles \lim
parameter: 0=\lim, 1=\limsup, 2=\liminf
 ******************************************************************************/
{
    char cThis, *s, *lower_limit = NULL;

    cThis = getNonBlank();
    if (cThis == '_')
        lower_limit = getBraceParam();
    else
        ungetTexChar(cThis);

    if (code == 0)
        s = strdup("lim");
    else if (code == 1)
        s = strdup("lim sup");
    else
        s = strdup("lim inf");

    if (g_fields_use_EQ) {
        if (lower_limit)
            fprintRTF("\\\\a\\\\ac(");

        fprintRTF("%s", s);

        if (lower_limit) {
            fprintRTF("%c", g_field_separator);
            ConvertString(lower_limit);
            fprintRTF(")");
        }

    } else {
        fprintRTF("%s ", s);
        if (lower_limit) {
            fprintRTF("{\\dn%d\\fs%d ", script_shift(), script_size());
            ConvertString(lower_limit);
            fprintRTF("}");
        }

    }

    free(s);
}

void CmdIntegral(int code)

/******************************************************************************
 purpose: converts integral symbol and the "exponent" and "subscript" fields
parameter: type of operand
 ******************************************************************************/
{
    char *upper_limit = NULL;
    char *lower_limit = NULL;
    char cThis;
    int no_limits = FALSE;
    int limits = FALSE;

    /* is there an exponent/subscript ? */
    cThis = getNonBlank();

    if (cThis == '\\') {        /* accept \nolimits and \limits */
        char *command;

        ungetTexChar(cThis);
        command = getSimpleCommand();
        if (strcmp(command, "\\nolimits") == 0) {
            no_limits = TRUE;
            diagnostics(WARNING, "\\nolimits found");
            free(command);
        } else if (strcmp(command, "\\limits") == 0) {
            limits = TRUE;
            diagnostics(WARNING, "\\limits found");
            free(command);
        } else {
            diagnostics(WARNING, "pushing <%s>",command);
            PushSource(NULL, command);
        }    
        
        cThis = getNonBlank();
    }

    if (cThis == '_')
        lower_limit = getBraceParam();
    else if (cThis == '^')
        upper_limit = getBraceParam();
    else
        ungetTexChar(cThis);

    if (upper_limit || lower_limit) {
        cThis = getNonBlank();
        if (cThis == '_')
            lower_limit = getBraceParam();
        else if (cThis == '^')
            upper_limit = getBraceParam();
        else
            ungetTexChar(cThis);
    }

    if (g_fields_use_EQ) {

        fprintRTF(" \\\\i ");
        switch (code) {
            case 4:
                if (limits)
                    fprintRTF("( %c %c )\\\\I", g_field_separator, g_field_separator);
                else
                    fprintRTF("\\\\in( %c %c )\\\\I", g_field_separator, g_field_separator);
                /* \iiint --- fall through */
            case 3:
                if (limits)
                    fprintRTF("( %c %c )\\\\I", g_field_separator, g_field_separator);
                else
                    fprintRTF("\\\\in( %c %c )\\\\I", g_field_separator, g_field_separator);
                /* \iint --- fall through */
            case 0:
                if (limits)
                    fprintRTF("(", g_field_separator, g_field_separator);
                else
                    fprintRTF("\\\\in(", g_field_separator, g_field_separator);
                break;
            case 1:
                fprintRTF("\\\\su(");
                break;
            case 2:
                fprintRTF("\\\\pr(");
                break;
            default:
                diagnostics(ERROR, "Illegal code to CmdIntegral");
        }

        if (lower_limit)
            ConvertString(lower_limit);
        fprintRTF("%c", g_field_separator);
        if (upper_limit)
            ConvertString(upper_limit);
        fprintRTF("%c )", g_field_separator);

    } else {

        switch (code) {
            case 0:
                CmdSymbolChar(0xf2);
                break;
            case 1:
                CmdSymbolChar(0xe5);
                break;
            case 2:
                CmdSymbolChar(0xd5);
                break;
            case 3:  /* \iint  */
                CmdSymbolChar(0xf2);
                CmdSymbolChar(0xf2);
                break;
            case 4:  /* \iiint  */
                CmdSymbolChar(0xf2);
                CmdSymbolChar(0xf2);
                CmdSymbolChar(0xf2);
                break;
                
            default:
                diagnostics(ERROR, "Illegal code to CmdIntegral");
        }

        if (lower_limit) {
            fprintRTF("{\\dn%d\\fs%d ", script_shift(), script_size());
            ConvertString(lower_limit);
            fprintRTF("}");
        }
        if (upper_limit) {
            fprintRTF("{\\up%d\\fs%d ", script_shift(), script_size());
            ConvertString(upper_limit);
            fprintRTF("}");
        }
    }

    if (lower_limit)
        free(lower_limit);
    if (upper_limit)
        free(upper_limit);
}

void SubSupWorker(bool big)

/******************************************************************************
 purpose   : Stack a superscript and a subscript together  
 ******************************************************************************/
{
    int vertical_shift;
    char cThis;
    char *upper_limit = NULL;
    char *lower_limit = NULL;

    diagnostics(4, "SubSupWorker() ... big=%d",big);
    for (;;) {
        cThis = getNonBlank();
        if (cThis == '_') {
            if (lower_limit)
                diagnostics(WARNING, "Double subscript");
            lower_limit = getBraceParam();
        } else if (cThis == '^') {
            if (upper_limit)
                diagnostics(WARNING, "Double superscript");
            upper_limit = getBraceParam();
        } else {
            ungetTexChar(cThis);
            break;
        }
    }

    diagnostics(4, "...subscript  ='%s'",lower_limit ? lower_limit : "");
    diagnostics(4, "...superscript='%s'",upper_limit ? upper_limit : "");

    if (big)
        vertical_shift = CurrentFontSize() / 1.4;
    else
        vertical_shift = CurrentFontSize() / 4;

    if (upper_limit && lower_limit) {
        fprintRTF("\\\\s\\\\up({\\fs%d ", script_size());
        ConvertString(upper_limit);
        if (big)
            fprintRTF("%c %c", g_field_separator, g_field_separator);
        else
            fprintRTF("%c", g_field_separator);
        ConvertString(lower_limit);
        fprintRTF("})");

    } else if (lower_limit) {
        fprintRTF("\\\\s\\\\do%d({\\fs%d ", vertical_shift, script_size());
        ConvertString(lower_limit);
        fprintRTF("})");

    } else if (upper_limit) {
        fprintRTF("\\\\s\\\\up%d({\\fs%d ", vertical_shift, script_size());
        ConvertString(upper_limit);
        fprintRTF("})");
    }

    if (lower_limit)
        free(lower_limit);
    if (upper_limit)
        free(upper_limit);
}

void CmdSuperscript(int code)

/******************************************************************************
 purpose   : Handles superscripts ^\alpha, ^a, ^{a}       code=0
                                  \textsuperscript{a}     code=1
 ******************************************************************************/
{
    char *s = NULL;

    diagnostics(4, "CmdSuperscript() ... ");

    if (code == 0 && g_fields_use_EQ) {
        ungetTexChar('^');
        SubSupWorker(FALSE);
        return;
    }

    if ((s = getBraceParam())) {
        fprintRTF("{\\up%d\\fs%d ", script_shift(), script_size());
        ConvertString(s);
        fprintRTF("}");
        free(s);
    }
}

void CmdSubscript(int code)

/******************************************************************************
 purpose   : Handles subscripts _\alpha, _a, _{a},           code=0
                                \textsubscript{script}       code=1
                                \lower4emA                   code=2
 ******************************************************************************/
{
    char *s = NULL;
    int size;
    

    if (code == 0) {
        diagnostics(4, "CmdSubscript() code==0, equation ");
    	if (g_fields_use_EQ) {
			ungetTexChar('_');
			SubSupWorker(FALSE);
       }
    }

    if (code == 1) {
        diagnostics(4, "CmdSubscript() code==1, \\textsubscript ");
        s=getBraceParam();
        fprintRTF("{\\dn%d\\fs%d ", script_shift(), script_size());
        ConvertString(s);
        fprintRTF("}");
        free(s);
    }

    if (code == 2) {
        diagnostics(4, "CmdSubscript() code==2, \\lower ");
		size = getDimension();  /* size is in half-points */
        fprintRTF("{\\dn%d ", size);
        s=getBraceParam();
        if (strcmp(s,"\\hbox")==0)
        	CmdBox(BOX_HBOX);
        else
        	ConvertString(s);
        fprintRTF("}");
        free(s);
    }

}

/* slight extension of getSimpleCommand to allow \{ and \| */
static void getDelimOrCommand(char *delim, char **s)
{
    *delim = '\0';
    
    ungetTexChar('\\');
    *s = getSimpleCommand();
    
/* handle special cases \{ \} and \| */
    if (strlen(*s) == 1) {
        free(*s);
        *s = malloc(3 * sizeof(char));
        (*s)[0] = '\\';
        (*s)[1] = getTexChar();
        (*s)[2] = '\0';
    } 

/* commands have a simple delimiter should just use the delimiter */
    if (*s) {
        if (strcmp(*s,"\\{")      ==0 || strcmp(*s,"\\lbrace")==0)
            *delim = '{';
        if (strcmp(*s,"\\}")      ==0 || strcmp(*s,"\\rbrace")==0)
            *delim = '}';
        if (strcmp(*s,"\\bracevert")==0)
            *delim = '|';
        if (strcmp(*s,"\\langle") ==0)
            *delim = '<';
        if (strcmp(*s,"\\rangle") ==0) {
            *delim ='>';
        }
        
        if (*delim) {
            free (*s);
            *s = NULL;
        }
    }
}

/******************************************************************************
 purpose   : Handles \left \right
 			 to properly handle \left. or \right. would require prescanning the
 			 entire equation.  
 ******************************************************************************/
void CmdLeftRight(int code)
{
    char ldelim, rdelim;
    char *contents;
    char *lcommand = NULL;
    char *rcommand = NULL;

    diagnostics(4, "CmdLeftRight() ... ");
    ldelim = getNonSpace();
    
    if (ldelim=='\0') {
        diagnostics(1,"right here");
        PopSource();
        ldelim = getNonSpace();
     }

    if (ldelim == '\\') getDelimOrCommand(&ldelim, &lcommand);

    contents = getLeftRightParam();
    rdelim = getNonSpace();

    if (rdelim == '\\') getDelimOrCommand(&rdelim, &rcommand);
    
    if (code == 1)
        diagnostics(ERROR, "\\right without opening \\left");

    diagnostics(4, "CmdLeftRight() ... \\left <%c> \\right <%c>", ldelim, rdelim);
      
    if (g_fields_use_EQ) {

        fprintRTF(" \\\\b ");

        /* these first four cases () {} [] <> are most common and work best without \lc and \rc */
        if (ldelim == '(' && rdelim == ')') {
            fprintRTF("(");
            goto finish;
        }

        if (ldelim == '{' && rdelim == '}') {
            fprintRTF("\\\\bc\\\\\\{ (");
            goto finish;
        }

        if (ldelim == '[' && rdelim == ']') {
            fprintRTF("\\\\bc\\\\[ (");
            goto finish;
        }

        if (ldelim == '<' && rdelim == '>') {
            fprintRTF("\\\\bc\\\\< (");
            goto finish;
        }

        /* insert left delimiter if it is not a '.' */
        if (ldelim == '{' || ldelim == '}' || ldelim == '(' || ldelim == ')')
            fprintRTF("\\\\lc\\\\\\%c", ldelim);
        else if (ldelim && ldelim != '.')
            fprintRTF("\\\\lc\\\\%c", ldelim);
        else if (lcommand) {
            fprintRTF("\\\\lc\\\\");
            ConvertString(lcommand);
        }

        /* insert right delimiter if it is not a '.' */
        if (rdelim == '{' || rdelim == '}' || rdelim == '(' || rdelim == ')')
            fprintRTF("\\\\rc\\\\\\%c", rdelim);
        else if (rdelim && rdelim != '.')
            fprintRTF("\\\\rc\\\\%c", rdelim);
        else if (rcommand) {
             fprintRTF("\\\\rc\\\\");
             ConvertString(rcommand);
        }
        
        /* start bracketed equation */
        fprintRTF(" (");

      finish:
        ConvertString(contents);
        fprintRTF(")");
        SubSupWorker(TRUE);     /* move super or subscripts a lot */

    } else {                    /* g_fields_use_EQ */

        putRtfCharEscaped(ldelim);
        ConvertString(contents);
        putRtfCharEscaped(rdelim);
    }

    if (lcommand) free(lcommand);
    if (rcommand) free(rcommand);
}

void CmdArray(int code)

/******************************************************************************
 purpose   : Handles \begin{array}[c]{ccc} ... \end{array}
 ******************************************************************************/
{
    char *v_align, *col_align, *s;
    int n = 0;

    if (code & ON) {
        v_align = getBracketParam();
        col_align = getBraceParam();
        diagnostics(4, "CmdArray() ... \\begin{array}[%s]{%s}", v_align ? v_align : "", col_align);
        if (v_align)
            free(v_align);

        s = col_align;
        while (*s) {
            if (*s == 'c' || *s == 'l' || *s == 'r')
                n++;
            s++;
        }
        free(col_align);

        fprintRTF(" \\\\a \\\\ac \\\\co%d (", n);
        g_processing_arrays++;

    } else {
        fprintRTF(")");
        diagnostics(4, "CmdArray() ... \\end{array}");
        g_processing_arrays--;
    }
}

void CmdMatrix(int code)

/******************************************************************************
 purpose   : Does not handle plain tex \matrix command, but does not
             produce improper RTF either.
 ******************************************************************************/
{
    char *contents;

    fprintRTF(" matrix not implemented ");
    contents = getBraceParam();
    free(contents);
}

void CmdStackrel(int code)

/******************************************************************************
 purpose   : Handles \stackrel{a}{=}
 ******************************************************************************/
{
    char *numer, *denom;
    int size;

    size = CurrentFontSize() / 1.2;
    numer = getBraceParam();
    denom = getBraceParam();
    diagnostics(4, "CmdStackrel() ... \\stackrel{%s}{%s}", numer, denom);

    if (g_fields_use_EQ) {
        fprintRTF(" \\\\a ({\\fs%d ", size);
        ConvertString(numer);
        fprintRTF("}%c", g_field_separator);
        ConvertString(denom);
        fprintRTF(") ");

    } else {
        diagnostics(WARNING, "sorry stackrel requires fields");
        fprintRTF("{");
        ConvertString(numer);
        fprintRTF(" ");
        ConvertString(denom);
        fprintRTF("}");
    }

    free(numer);
    free(denom);
}

/******************************************************************************
 purpose   : Handles \overline{a}
 ******************************************************************************/
void CmdOverLine(int code)
{
    char *argument;

    argument = getBraceParam();
    diagnostics(4, "CmdOverLine() ... \\overline{%s}", argument);

    if (g_fields_use_EQ) {
        fprintRTF(" \\\\x\\\\to( ");
        ConvertString(argument);
        fprintRTF(") ");

    } else {
        diagnostics(WARNING, "sorry overline requires fields");
        fprintRTF("{");
        ConvertString(argument);
        fprintRTF("}");
    }

    free(argument);
}