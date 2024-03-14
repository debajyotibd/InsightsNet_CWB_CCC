/*
 *  IMS Open Corpus Workbench (CWB)
 *  Copyright (C) 1993-2006 by IMS, University of Stuttgart
 *  Copyright (C) 2007-     by the respective contributers (see file AUTHORS)
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2, or (at your option) any later
 *  version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 *  Public License for more details (in the file "COPYING", or available via
 *  WWW at http://www.gnu.org/copyleft/gpl.html).
 */
/*
 *  Windows/Unicode-compatibility extensions to CWB in this file
 *  Copyright (C) 2010      by ANR Textométrie, ENS de Lyon
 */

/**
 * @file
 *
 * This file contains the Optimised Regular Expression object, which wraps around  PCRE.
 */


#include <glib.h>
#include <stdint.h>

/* include external regular expression library */
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

#include "globals.h"
#include "regtab.h"


/**
 * Maximum number of grains of optimisation.
 *
 * There's no point in scanning for too many grains, but some regexps used to be bloody inefficient.
 */
#define MAX_GRAINS 12

/**
 * Underlying structure for opaqCL_Regex object.
 */
struct _cl_regex {
  pcre2_code_8 *needle;              /**< buffer for the actual regex object (PCRE2) */
  pcre2_match_data_8 *match_data;    /**< buffer where the match datga is placed (CL normally has no use for this). */
  CorpusCharset charset;             /**< the character set in use for this regex */
  int icase;                         /**< whether IGNORE_CASE flag was set for this regex (needs special processing) */
  int idiac;                         /**< whether IGNORE_DIAC flag was set for this regex */
  char *haystack_buf;                /**< a buffer of size CL_MAX_LINE_LENGTH used for accent folding by cl_regex_match(),
                                          allocated only if IGNORE_DIAC was specified */
  char *haystack_casefold;           /**< additional, larger (2 * CL_MAX_LINE_LENGTH) buffer for a case-folded version,
                                          allocated only if optimizer is active and IGNORE_CASE was specified */
  /* Note: these buffers are for the string being tested, NOT for the regular expression.
   * They are allocated once here to avoid frequent small allocation and deallocations in cl_regex_match(). */

  /* data from optimiser (see global variables in regopt.c for comments) */
  int grains;                        /**< number of grains (0 = not optimised). @see cl_regopt_grains */
  int grain_len;                     /**< minimum grain length @see cl_regopt_grain_len */
  char *grain[MAX_GRAINS];           /**< actual grain pointers @see cl_regopt_grain */
  int anchor_start;                  /**< bool, is this start-anchorted? @see cl_regopt_anchor_start */
  int anchor_end;                    /**< bool, is this end-anchored? @see cl_regopt_anchor_end */
  int jumptable[256];                /**< the jumptable (maps unsigned character value to jump length) @see cl_regopt_jumptable @see make_jump_table */
};


/**
 * @file
 *
 * The CL_Regex object, and the CL Regular Expression Optimiser.
 *
 * This is the CL front-end to POSIX regular expressions with CL semantics
 * (most notably: CL regexes always match the entire string and NOT
 * substrings.)
 *
 * Note that the optimiser is handled automatically by the CL_Regex object.
 *
 * All variables / functions containing "regopt" are internal to this
 * module and are not exported in the CL API.
 *
 * Optimisation is done by means of "grains". The grain array in a CL_Regex
 * object is a list of short strings. Any string which will match the
 * regex must contain at least one of these. Thus, the grains
 * provide a quick way of filtering out strings that definitely WON'T
 * match, and avoiding a time-wasting call to the POSIX regex
 * matching function.
 *
 * While a regex is being optimised, the grains are stored in non-exported
 * global variables in this module. Subsequently they are transferred to
 * members of the CL_regex object with which they are associated.
 * The use of global variables and a fixed-size buffer for
 * grains is partly due to historical reasons,
 * but it does also serve to reduce memory allocation overhead.
 */

/* optimiser variables */

char *cl_regopt_grain[MAX_GRAINS]; /**< list of 'grains' (any matching string must contain one of these); pointers into the actual data block public_grain_data  */
int cl_regopt_grain_len;           /**< length of shortest grain (in characters) */
int cl_regopt_grains;              /**< number of grains found so far */
int cl_regopt_anchor_start;        /**< Boolean: whether grains are anchored at beginning of string */
int cl_regopt_anchor_end;          /**< Boolean: whether grains are anchored at end of string */

/** whether the regular expression is in UTF-8 encoding (set before calling regopt_analyse()) */
int cl_regopt_utf8;


/**
 * Intermediate buffer for grains.
 *
 * When a regex is parsed, grains for each segment are written to this intermediate buffer;
 * if the new set of grains is better than the current one, it is copied to the cl_regopt_ variables.
 */
char *grain_buffer[MAX_GRAINS];
/** the length of each grain in this intermediate buffer (in characters). @see grain_buffer */
int grain_buffer_len[MAX_GRAINS];
/** The number of grains currently in the intermediate buffer. @see grain_buffer */
int grain_buffer_grains = 0;

/** Data storage for grain strings addressed by cl_regopt_grain. @see local_grain_data */
char public_grain_data[CL_MAX_LINE_LENGTH]; /* input regexp shouldn't be longer than CL_MAX_LINE_LENGTH, so all grains must fit;
                                             * RE() operator can create GIANT regexen, so cl_new_regex() only runs the optimiser
                                             * iff the length of the regex is less than CL_MAX_LINE_LENGTH. */

/** A buffer for grain strings. @see public_grain_data */
char local_grain_data[CL_MAX_LINE_LENGTH];

/**
 * A counter of how many times the "grain" system has allowed us to avoid
 * calling the regex engine.
 *
 * @see cl_regopt_count_get
 */
int cl_regopt_successes = 0;


/*
 * interface functions (ie "public methods" of CL_Regex)
 */

/**
 * The error message from (PCRE) regex compilation are placed in this buffer
 * if cl_new_regex() fails.
 *
 * This global variable is part of the CL_Regex object's API.
 */
char cl_regex_error[CL_MAX_LINE_LENGTH];


/*
 * static prototypes
 */
static void regopt_data_copy_to_regex_object(CL_Regex rx);
static int regopt_analyse(char *regex);


/**
 * Create a new CL_regex object (ie a regular expression buffer).
 *
 * This function compiles the regular expression according to the specified flags
 * (IGNORE_CASE and/or IGNORE_DIAC and/or REQUIRE_NFC) and for the specified
 * character encoding. The regex is automatically anchored to the start and end of
 * the string (i.e. wrapped in ^(?:...)$).
 *
 * The regular expression engine used is PCRE. However, the regex is optimized by
 * scanning it for literal strings ("grains") that must be contained in any
 * match; the grains can be used as a fast pre-filter (using Boyer-Moore search
 * for the grains).
 *
 * The optimizer only understands a subset of PCRE syntax:
 *  - literal characters (alphanumeric, safe punctuation, escaped punctuation)
 *  - numeric character codes (\\x and \\o)
 *  - escape sequences for character classes and Unicode properties
 *  - all repetition operators
 *  - simple alternatives (...|...|...)
 *  - nested capturing (...) and non-capturing (?:...) groups
 * Any regexp that contains other syntactic elements such as
 *  - character sets [...]
 *  - named groups, look-ahead and look-behind patterns, etc.
 *  - backreferences
 *  - modifiers such as (?i)
 * cannot be parsed and optimized. Note that even if a regexp is parsed by
 * the optimizer, it might not be able to extract all grains (because grain
 * recognition uses an even more restrictive syntax).
 *
 * The optimizer is always disabled with IGNORE_CASE if either PCRE JIT is
 * available or the charset is UTF-8. Testing has showed that in these cases
 * the overhead from case-folding each input string outweighs the benefits
 * of the optimizer.
 *
 * @param regex    String containing the regular expression
 * @param flags    IGNORE_CASE, or IGNORE_DIAC, or both, or 0.
 * @param charset  The character set of the regex.
 * @return         The new CL_Regex object, or NULL in case of error.
 */
CL_Regex
cl_new_regex(char *regex, int flags, CorpusCharset charset)
{
  /* allocate buffers dynamically to support very long regexps (from RE() operator) */
  char *delatexed_regex;
  char *preprocessed_regex;
  char *anchored_regex;

  CL_Regex rx;
  int optimised, l;

  uint32_t options_for_pcre = 0; /* stores options to pass to pcre2_compile. */
  int errcode_for_pcre = 0;
  pcre2_compile_context_8 *ccontext_for_pcre = NULL; /* used to pass in a character table if need be. */
  PCRE2_SIZE erroffset_for_pcre = 0;
  uint32_t is_pcre_jit_available; /* in some cases, PCRE + JIT may be faster without the optimizer */


  /* allocate temporary strings */
  l = strlen(regex);
  delatexed_regex = (char *) cl_malloc(l + 1);

  /* allocate and initialise CL_Regex object */
  rx = (CL_Regex) cl_malloc(sizeof(struct _cl_regex));

  rx->haystack_buf = NULL;
  rx->haystack_casefold = NULL;
  rx->charset = charset;
  rx->icase = (flags & IGNORE_CASE); /* handled separately in CWB 3.4.10+ */
  rx->idiac = (flags & IGNORE_DIAC);
  rx->grains = 0; /* indicates no optimisation -> other optimizer-related fields are invalid */

  /* pre-process regular expression (translate latex escapes, normalize, fold accents if required) */
  cl_string_latex2iso(regex, delatexed_regex, l);
  /* only fold accents at this stage */
  preprocessed_regex = cl_string_canonical(delatexed_regex, charset, rx->idiac | REQUIRE_NFC, CL_STRING_CANONICAL_STRDUP);
  cl_free(delatexed_regex);

  /* add start and end anchors to improve performance of regex matcher for expressions such as ".*ung" */
  anchored_regex = (char *) cl_malloc(strlen(preprocessed_regex) + 7);
  sprintf(anchored_regex, "^(?:%s)$", preprocessed_regex);

  /* compile regular expression with PCRE library function */
  options_for_pcre = PCRE2_UCP; /* use Unicode properties for \w, \d, etc. */
  if (rx->icase)
    options_for_pcre |= PCRE2_CASELESS; /* case folding is left to the PCRE matcher */
  if (charset == utf8) {
    if (cl_debug)
      fprintf(stderr, "CL: enabling PCRE's UTF8 mode for regex %s\n", anchored_regex);
    /* note we assume all strings have been checked upon input (i.e. indexing or by the parser) */
    options_for_pcre |= PCRE2_UTF|PCRE2_NO_UTF_CHECK;
  }
  else if (charset != ascii)
  {
    /* prior theorem about ascii/utf8 EXPLODED!! so pick the appropriate PCRE data table for character properties. */
    /* (might it be more efficient to have a ccontext for each charset pre-compiled on init? this is probably OK.) */
    ccontext_for_pcre = pcre2_compile_context_create_8(NULL);
    pcre2_set_character_tables_8(ccontext_for_pcre, tables_for_pcre[charset]);
  }

  rx->needle = pcre2_compile_8((PCRE2_SPTR8)anchored_regex, PCRE2_ZERO_TERMINATED, options_for_pcre, &errcode_for_pcre, &erroffset_for_pcre, ccontext_for_pcre);
  if (NULL == rx->needle) {
      pcre2_get_error_message_8(errcode_for_pcre, (PCRE2_UCHAR *)cl_regex_error, CL_MAX_LINE_LENGTH);
    fprintf(stderr, "CL: Regex Compile Error @ string offset %ld: %s\n", erroffset_for_pcre, cl_regex_error);
    cl_free(rx);
    cl_free(preprocessed_regex);
    cl_free(anchored_regex);
    cl_errno = CDA_EBADREGEX;
    return NULL;
  }
  else if (cl_debug)
    fprintf(stderr, "CL: Regex compiled successfully using PCRE library\n");
  pcre2_compile_context_free_8(ccontext_for_pcre);

  /* we set up the match data now. It lives to the end of this CL_Regex. */
  if (!(rx->match_data = pcre2_match_data_create_from_pattern_8(rx->needle, NULL))) {
    /* error output akin to cl_malloc's. */
    fprintf(stderr, "CL: Out of memory while creating PCRE2 Match Data (killed).\n");
    printf("\n");       /* for CQP's child mode */
    exit(1);
  }

  /* a spot of code to handle the unexpected case of JIT not having been compiled into PCRE. */
#ifdef PCRE2_CONFIG_JIT
  pcre2_config_8(PCRE2_CONFIG_JIT, &is_pcre_jit_available);
#else
  is_pcre_jit_available = 0;
#endif
  if (cl_debug)
    fprintf(stderr, "CL: PCRE's JIT compiler is %s.\n", (is_pcre_jit_available ? "available" : "unavailable"));

  /* now, run JIT */
  if (is_pcre_jit_available) {
    int jit_err;
    if (0 > (jit_err = pcre2_jit_compile_8(rx->needle, 0)))
      if (cl_debug)
        fprintf(stderr, "CL: PCRE's JIT compiler reported error # %d.\n", jit_err);
    /* we assume the interpreted version will continue to work if JIT did not work. */
  }

  /* attempt to optimise regular expression */
  cl_regopt_utf8 = (charset == utf8);
  if ((l + 1) <= CL_MAX_LINE_LENGTH)
    optimised = regopt_analyse(preprocessed_regex);
  else
    optimised = 0;   /* IE: avoid optimisation if the regex is longer than the grain buffer. */

  /* decide whether it makes sense to use the optimizer:
   *  - testing showed that it is usually faster to rely directly on PCRE's caseless matching than
   *    casefold each input string for a Boyer-Moore search
   *  - this is always the case if PCRE has JIT capability
   *  - without JIT, it is still usually better to avoid expensive UTF-8 case-folding
   * NB: accent-folding for %d cannot be avoided, so it makes no sense to disable the optimizer there
   * NB: cannot debug the optimizer with %c any more (because regopt_data_copy_to_regex_object will not be called)
   */
  if (optimised && cl_debug) {
    int i;
    fprintf(stderr, "CL: Found grain set with %d items(s)", cl_regopt_grains);
    for (i = 0; i < cl_regopt_grains; i++) {
      fprintf(stderr, " [%s]", cl_regopt_grain[i]);
    }
    fprintf(stderr, "\n");
  }

  if (rx->icase && (charset == utf8 || is_pcre_jit_available)) {
    if (optimised && cl_debug) {
      fprintf(stderr, "\nCL: but optimization disabled for case-insensitive search\n");
      optimised = 0;
    }
  }

  if (optimised) {
    /* copy optimiser data to CL_Regex object and construct jump table for Boyer-Moore search */
    regopt_data_copy_to_regex_object(rx); /* will also casefold grains if rx->icase is set */
  }

  if (rx->idiac)
    /* allocate string buffer for accent folding in cl_regex_match() */
    rx->haystack_buf = (char *) cl_malloc(CL_MAX_LINE_LENGTH); /* this is for the string being matched, not the regex! */
  if (rx->icase && optimised)
    /* allocate second buffer for case-folded version (only needed for optimizer) */
    rx->haystack_casefold = (char *) cl_malloc(2 * CL_MAX_LINE_LENGTH);

  cl_free(preprocessed_regex);
  cl_free(anchored_regex);
  cl_errno = CDA_OK;
  return rx;
}

/**
 * Finds the level of optimisation of a CL_Regex.
 *
 * This function returns the approximate level of optimisation,
 * computed from the ratio of grain length to number of grains
 * (0 = no grains, ergo not optimised at all).
 *
 * @param rx  The CL_Regex to check.
 * @return    0 if rx is not optimised; otherwise an integer
 *            indicating optimisation level.
 */
int
cl_regex_optimised(CL_Regex rx)
{
  if (rx->grains == 0)
    return 0; /* not optimised */
  else {
    int level = (3 * rx->grain_len) / rx->grains;
    return ((level >= 1) ? level + 1 : 1);
  }
}

/**
 * Matches a regular expression against a string.
 *
 * The pre-compiled regular expression contained in the CL_Regex is compared to the string.
 * This regex automatically uses the case/accent folding flags and character encoding
 * that were specified when the CL_Regex constructor was called.
 *
 * If the subject string is a UTF-8 string from an external sources, the caller can request
 * enforcement of the subject to canonical NFC form by setting the third argument to true.
 *
 * @see   cl_new_regex
 * @param rx              The regular expression to match.
 * @param str             The subject (the string to compare the regex to).
 * @param normalize_utf8  Boolean: if a UTF-8 string from an external source is passed as subject,
 *                        set to this parameter to true, and the function will make sure that
 *                        the comparison is based on the canonical NFC form. For known-NFC
 *                        strings, this parameter should be false. If the regex is not UTF-8,
 *                        this parameter is ignored.
 * @return                Boolean: true if the regex matched, otherwise false.
 */
int
cl_regex_match(CL_Regex rx, char *str, int normalize_utf8)
{
  char *haystack_pcre, *haystack; /* possibly case/accent folded versions of str for PCRE regexp and optimizer, respectively */
  int optimised = (rx->grains > 0);
  int i, di, k, max_i, len, jump;
  int grain_match, result;
//  int ovector[30]; /* memory for pcre to use for back-references in pattern matches */
  int do_nfc = (normalize_utf8 && (rx->charset == utf8)) ? REQUIRE_NFC : 0; /* whether we need to normalize the input to NFC */

  if (rx->idiac || do_nfc) { /* perform accent folding on input string if necessary */
    haystack_pcre = rx->haystack_buf;
    cl_strcpy(haystack_pcre, str);
    cl_string_canonical(haystack_pcre, rx->charset, rx->idiac | do_nfc, CL_MAX_LINE_LENGTH);
  }
  else
    haystack_pcre = str;
  len = strlen(haystack_pcre);

  /* Prior to 3.5.0, the optimiser was dependent on cl_optimize
   * (so most users didn't get the optimised version).
   */
  if (optimised) {
    if (rx->icase) {
      haystack = rx->haystack_casefold;
      cl_strcpy(haystack, haystack_pcre);
      cl_string_canonical(haystack, rx->charset, rx->icase, 2 * CL_MAX_LINE_LENGTH);
    }
    else
      haystack = haystack_pcre;
    /* this 'optimised' matcher may look fairly complicated, but it's still way ahead of POSIX regexen */
    /* string offset where first character of each grain would be */
    grain_match = 0;
    max_i = len - rx->grain_len; /* stop trying to match when i > max_i */
    if (rx->anchor_end)
      i = (max_i >= 0) ? max_i : 0; /* if anchored at end, align grains with end of string */
    else
      i = 0;

    while (i <= max_i) {
      jump = rx->jumptable[(unsigned char) haystack[i + rx->grain_len - 1]];
      if (jump > 0)
        i += jump; /* Boyer-Moore search */
      else {
        /* for each grain */
        for (k = 0; k < rx->grains; k++) {
          di = 0;
          while ((di < rx->grain_len) && (rx->grain[k][di] == haystack[i + di]))
            di++;
          if (di >= rx->grain_len) {
            grain_match = 1;
            break; /* we have found a grain match and can quit the loop */
          }
        }
        i++;
      }
      if (rx->anchor_start)
        break; /* if anchored at start, only the first iteration can match */
    }
  } /* endif optimised */
  else
    /* if the regex is not optimised, always behave as if a grain was matched */
    grain_match = 1;

  /* if there was a grain-match, we call pcre2_match, which might match or might not find a match in the end;
   * but if there wasn't a grain-match, we know that PCRE won't match; so we don't bother calling it. */

  if (!grain_match) {
    cl_regopt_successes++;
    result = PCRE2_ERROR_NOMATCH;  /* the return code from PCRE when there is, um, no match */
  }
#if 1
  /* you can set to 0 for debug purposes: if so, it always calls PCRE regardless of whether grains matched. */
  /* this allows the code in the #if 1 below to check whether or not grains are behaving as they should. */
  else {
#else
  if (1) {
#endif

    result = pcre2_match(rx->needle, (PCRE2_SPTR8)haystack_pcre, len, 0, PCRE2_NO_UTF_CHECK, rx->match_data, NULL);

    if (0 == result)
      fprintf(stderr, "CL: Regex Match Error: insufficient ovector memory (see `man pcre2_match` for explanation)\n");
    else if (PCRE2_ERROR_NOMATCH > result && cl_debug)
      /* note, "no match" is a PCRE "error", but all actual errors are lower numbers */
      fprintf(stderr, "CL: Regex Match Error no. %d (see `man pcre2api` for error codes)\n", result);
  }


  /* debugging code used before version 2.2.b94, modified to pcre return values & re-enabled in 3.2.b3 */
  /* check for critical error: optimiser didn't accept candidate, but regex matched */
  if ((result > 0) && !grain_match)
    fprintf(stderr, "CL ERROR: regex optimiser did not accept '%s' although it should have!\n", str);

  return (result > 0); /* return true if regular expression matched */
}

/**
 * Deletes a CL_Regex object, and frees all resources associated with
 * the pre-compiled regex.
 *
 * @param rx  The CL_Regex to delete. Null-safe.
 */
void
cl_delete_regex(CL_Regex rx)
{
  /* DON'T use cl_free() for PCRE opaque objects, just in case; use PCRE built-in
   * pcre_free(). Note this will probably just be set to = free(). But it might not.
   * We can let PCRE worry about that. That does mean, however, we should test the
   * pointers for non-nullity before calling pcre_free. Normally we would also set the
   * pointers to NULL after freeing the target. However, in this case, we know the
   * structure they belong to will be freed by the end of the function, so no worries.
   */
  int i;

  /* sanity check for NULL pointer */
  if (!rx)
    return;

  if (rx->needle)
    pcre2_code_free_8(rx->needle);         /* free PCRE regex buffer */
  if (rx->match_data)
    pcre2_match_data_free_8(rx->match_data); /* and match data buffer */
  cl_free(rx->haystack_buf);       /* free string buffers if they were allocated */
  cl_free(rx->haystack_casefold);
  for (i = 0; i < rx->grains; i++)
    cl_free(rx->grain[i]);         /* free grain strings if regex was optimised */

  cl_free(rx);
}

/*
 * ================================
 * helper functions (for optimiser)
 * (non-exported in the public API)
 * ================================
 */

/**
 * Is the given character a 'safe' character which will only match itself in a regex?
 *
 * What counts as safe: A to Z, a to z, 0 to 9, minus, quote marks, percent,
 * ampersand, slash, excl mark, colon, semi colon, character, underscore, tilde,
 * any values above 0x7f (ISO 8859 extension or UTF-8 non-ASCII character).
 *
 * What counts as not safe therefore includes: brackets, braces, square brackets;
 * questionmark, plus, and star; circumflex and dollar sign; dot; hash; backslash, etc.
 * (But, in UTF8, Unicode PUNC area equivalents of these characters will be safe.)
 *
 * A safe character can never be the start of a meta element (even if it might appear
 * as part of one), so it's safe to include in a literal grain.
 *
 * @param c  The character (cast to unsigned for the comparison).
 * @return   True for non-special characters; false for special characters.
 */
static int
is_safe_char(unsigned char c)
{
  /* note: this function is UTF8-safe because byte values above 0x7f
   * (forming UTF-8 multi-byte sequences) are always allowed */
  if (
      (c >= 'A' && c <= 'Z') ||
      (c >= 'a' && c <= 'z') ||
      (c >= '0' && c <= '9') ||
      (c >= 128) /* && (c <= 255); omitted to avoid compiler warnings */
  ) {
    return 1;
  }
  else {
    switch (c) {
    case '!':
    case '"':
    case '#':
    case '%':
    case '&':
    case '\'':
    case ',':
    case '-':
    case '/':
    case ':':
    case ';':
    case '<':
    case '=':
    case '>':
    case '@':
    case '_':
    case '`':
    case '~':
      return 1;
    default:
      return 0;
    }
  }
}

/**
 * Is the given character an ASCII alphanumeric?
 *
 * ASCII alphanumeric characters comprise A-Z, a-z and 0-9; they are the only
 * characters that form special escape sequences in PCRE regular expressions.
 *
 * @param c  The character (cast to unsigned for the comparison).
 * @return   True if ASCII alphanumeric; false otherwise.
 */
static int
is_ascii_alnum(unsigned char c)
{
  return (
      (c >= 'A' && c <= 'Z') ||
      (c >= 'a' && c <= 'z') ||
      (c >= '0' && c <= '9')
      );
}

/**
 * Is the given character ASCII punctuation?
 *
 * ASCII punctuation symbols are the only characters that may need to be protected
 * by a \ in regular expressions. They cannot form special escape sequences.
 *
 * @param c  The character (cast to unsigned for the comparison).
 * @return   True if ASCII alphanumeric; false otherwise.
 */
static int
is_ascii_punct(unsigned char c)
{
  switch (c) {
  case '!':
  case '"':
  case '#':
  case '$':
  case '%':
  case '&':
  case '(':
  case '\'':
  case ')':
  case '*':
  case '+':
  case ',':
  case '-':
  case '.':
  case '/':
  case ':':
  case ';':
  case '<':
  case '=':
  case '>':
  case '?':
  case '@':
  case '[':
  case '\\':
  case ']':
  case '^':
  case '_':
  case '`':
  case '{':
  case '|':
  case '}':
  case '~':
    return 1;
  default:
    return 0;
  }
}

/**
 * Is the given character a valid hexadecimal digit?
 *
 * @param c  The character (cast to unsigned for the comparison).
 * @return   True if valid hexidecimal digit; false otherwise.
 */
static int
is_hexadecimal(unsigned char c)
{
  return (
      (c >= 'A' && c <= 'F') ||
      (c >= 'a' && c <= 'f') ||
      (c >= '0' && c <= '9')
      );
}


/**
 * Read in an escape sequence for a character or class - part of the CL Regex Optimiser.
 *
 * This function reads one of the following escape sequences:
 *
 *     \x##, \x{###} ... hexadecimal character code
 *     \o{###}       ... octal character code
 *     \w, \W, \d, \D, \s, \S  ...  generic character types
 *     \p#, \p{###}  ... Unicode properties
 *     \P#, \P{###}  ... negated Unicode properties
 *     \X            ... Unicode extended grapheme cluster
 *
 */
static char *
read_escape_seq(char *mark)
{
  char *point = mark;

  /* have we actually got an escape sequence starting at this point? */
  if (*point != '\\')
    return mark;
  else
    point++;

  switch (*point) {
  case 'x':
    /* hexadecimal escape seq */
    point++;
    if (*point == '{') {
      point++;
      while (is_hexadecimal(*point))
        point++;
      if (*point == '}')
        return point + 1;
    }
    else if (is_hexadecimal(point[0]) && is_hexadecimal(point[1]))
      return point + 2;
    return mark; /* not recognised */

  case 'o':
    /* octal escape seq */
    point++;
    if (*point == 'o') {
      point++;
      while (*point >= '0' && *point <= '7')
        point++;
      if (*point == '}')
        return point + 1;
    }
    return mark; /* not recognised */

  case 'w':
  case 'W':
  case 'd':
  case 'D':
  case 's':
  case 'S':
  case 'X':
    /* assorted seqs which are definitely just the slash plus one char */
    return point + 1;

  case 'p':
  case 'P':
    /* unicode propery escape seq might be longer than 1 char */
    point++;
    if (*point >= 'A' && *point <= 'Z')
      return point + 1;
    else if (*point == '{') {
      point++;
      while (is_ascii_alnum(*point) || *point == '_' || *point == '&')
        point++;
      if (*point == '}')
        return point + 1;
    }
    return mark; /* not recognised */

  default:
    /* any other type of slash-plus something, incl. \\ \N \C \R \b \B \G \A \Z ... etc. */
    return mark; /* not recognised */
  }
}

/**
 * Reads in an element matching some character - part of the CL Regex Optimiser.
 *
 * This function reads in an element known to match some character.
 * The following elements are currently recognized:
 *
 *  - a matchall (.)
 *  - a safe literal character or escaped ASCII punctuation
 *  - an escape sequence for a simple character class (\\w, \\d, etc.)
 *  - an escape sequence for a Unicode property
 *  - a hexadecimal (\\x) or octal (\\o) character code
 *  - a simple character set such as [a-z], [A-Z] and [0-9]
 *
 * The precise syntax of character sets is rather messy, so we will not
 * make an attempt to recognize more complex sets.
 *
 * @param mark  Pointer to location in the regex string from
 *              which to read.
 * @return      Pointer to the first character after the character
 *              (class) it has read in (or the original "mark"
 *              pointer if nothing suitable was found).
 */
static char *
read_matchall(char *mark)
{
  char *point = mark;
  char *p2;

  if (*mark == '.')
    /* read the matchall dot */
    return mark + 1;

  else if (is_safe_char(*mark))
    /* a literal character */
    return cl_regopt_utf8 ? g_utf8_next_char(mark) : mark + 1;

  else if (*mark == '\\')
    /* a (possible) supported escape sequence, if it can be read */
    return read_escape_seq(mark);

  else if (*mark == '[') {
    /* a character set */
    point = mark + 1;
    if (*point == '^')
      point++; /* negated character set */
    while (1) {
      /* keep scrolling through safe characters or dashes ... */
      if (is_safe_char(*point) || *point == '-')
        point++;
      /* and hop over escape sequences (if possible; if not we have reached ] or something unexpected, so break) */
      else {
        p2 = read_escape_seq(point);
        if (p2 > point)
          point = p2;
        else
          break;
      }
    }
    /* did we find a complete char class? */
    if (*point == ']' && point > mark + 1)
      return point + 1; /* note that we exclude [] because of ambiguity with []..] */
  }

  return mark; /* no element has been recognized */
}

/**
 * Reads in a repetition operator - part of the CL Regex Optimiser.
 *
 * This function reads in any repetition operator allowed in PCRE syntax:
 *
 *  - * (Kleene star), ?, +
 *  - {n}, {n,m}, {,m}, {n,}
 *  - optionally followed by a non-greedy (?) or possessive (+) modifier
 *
 * It returns a pointer to the first character after the repetition modifier
 * it has found. If none is found, this pointer will be equal to mark.
 * If *one_or_more is not null, it is set to True if the quantifier
 * operator mandates at least one instance of the thing modified
 * (i.e. it does not make the preceding element optional).
 *
 * @param mark         Pointer to location in the regex string from
 *                     which to read.
 * @param one_or_more  Optional pointer to integer, which will be set
 *                     to True if repetition is not optional.
 *                     Unchanged if no quantifier found.
 * @return             Pointer to the first character after the star
 *                     or other modifier it has read in (or the original
 *                     "mark" pointer if a repetition modifier was not
 *                     read).
 */
static char *
read_kleene(char *mark, int *one_or_more)
{
  int plus = 0; /* True: repetition is non-optional, i.e. 1 or more. False: repetition is zero-or-more (or unfound) */
  int ok = 1;   /* whether a valid quantifier has been recognized; we begin assuming one will be */

  char *point = mark;

  /* check for the different characters that might possibly be the start of a quantifier */
  if (*point == '?' || *point == '*') {
    plus = 0; /* zero-or-more */
    point++;
  }

  else if (*point == '+') {
    plus = 1; /* one-or-more */
    point++;
  }

  else if (*point == '{') {
    point++;
    plus = (*point >= '1' && *point <= '9'); /* anything other than 0 here ==>  one-or-more */
    while ((*point >= '0' && *point <= '9') || (*point == ','))
      point++;
    if (*point != '}')
      ok = 0;
    else
      point++;
  }

  else
    ok = 0; /* no quantifier found */

  /* did we find anything ? */
  if (ok) {
    if (*point == '?' || *point == '+')
      point++; /* lazy or possessive quantifier */
    if (one_or_more)
      *one_or_more = plus;
    return point;
  }
  else
    return mark;
}

/**
 * Reads in a wildcard - part of the CL Regex Optimiser.
 *
 * This function reads in a wildcard segment consisting of an
 * element matching some character (read_matchall) OR a capturing
 * or non-capturing group, followed by an optional quantifier (read_kleene).
 * It returns a pointer to the first character after the wildcard segment.
 *
 * Groups are parsed recursively and must consist of one or more
 * alternatives containing only valid wildcard segments.
 *
 * @param mark  Pointer to location in the regex string from which to read.
 * @return      Pointer to the first character after the
 *              wildcard segment (or the original "mark" pointer
 *              if a wildcard segment was not found).
 */
static char *
read_wildcard(char *mark)
{
  char *point, *p2;

  /* check for a single character or dot or [class] possibly with repetition after it */
  point = read_matchall(mark);
  if (point > mark)
    return read_kleene(point, NULL);

  /* check for a (group) possibly with repetition after it */
  else if (*point == '(') {
    point++;

    /* check for start-of-group flags, but only accept ?: */
    if (*point == '?') {
      if (point[1] == ':')
        point += 2;  /* non-capturing group */
      else
        return mark; /* other special groups are not supported */
    }

    /* body of group must consist only of wildcard segments and disjunction operators */
    while (1)
      if ((p2 = read_wildcard(point)) > point)
        point = p2;
      else if (*point == '|')
        point++;
      else
        break;

    /* did we get to the end of a valid group? If so, check for a following quantifier before returning */
    if (*point == ')')
      return read_kleene(point + 1, NULL);
  }

  return mark; /* no wildcard segment found */
}

/**
 * Reads in a literal grain from a regex - part of the CL Regex Optimiser.
 *
 * A grain is a string of safe symbols: alphanumeric, safe punctuation and
 * escaped punctuation; numeric character codes (\\x, \\u) are not supported.
 * The last symbol might be followed by a repetition operator. It is only
 * included in the grain if the repetition count is at least one.
 *
 * This function finds the longest grain it can starting at the point
 * in the regex indicated by mark.  If *grain is not NULL, the grain data
 * are unescaped and copied to the specified buffer.  Note that the buffer
 * will be mangled even if no grain is matched; it is guaranteed to contain
 * a NUL-terminated string, though.  If *len is not null, the length of the
 * grain (in characters) is stored there.  If *align_end is not null, it
 * will be set to 0 if the last literal is followed by a quantifier (which
 * means grain cannot be anchored at end); it is left unaltered otherwise.
 *
 * @param mark        Pointer to location in the regex string from
 *                    which to read.
 * @param grain_data  Optional pointer to a buffer into which the grain data
 *                    will be copied. Guaranteed to contain a NUL-terminated
 *                    string even if no grain is found.
 * @param len         Pointer to integer in which length of grain in characters
 *                    will be stored.
 * @param align_end   Pointer to integer which will be set to 0 if last character
 *                    is followed by quantifier (so grain can't be anchored at end)
 * @return            Pointer to the first character after the grain
 *                    it has read in (or the original "mark" pointer
 *                    if no grain is found).
 */
static char *
read_grain(char *mark, char *grain_data, int *len, int *align_end)
{
  char *point = mark;
  char *grain_point = grain_data;
  char *grain_last_char = grain_data; /* pointer to start of last symbol in grain buffer */
  char *end, *q;

  int glen = 0; /* length of grain in characters */
  int one_or_more;

  /* read sequence of safe literal characters (OR escaped punctuation) */
  while ( is_safe_char(*point)  ||  (*point == '\\' && is_ascii_punct(point[1])) ) {
    if (*point == '\\') {
      /* copy escaped character to grain */
      if (grain_data) {
        grain_last_char = grain_point;
        *grain_point++ = point[1];
      }
      glen++;
      point += 2;
    }
    else {
      /* copy complete character to grain */
      end = cl_regopt_utf8 ? g_utf8_next_char(point) : point + 1;
      if (grain_data) {
        grain_last_char = grain_point;
        for (q = point; q < end; q++)
          *grain_point++ = *q;
      }
      glen++;
      point = end;
    }
  }

  /* did we find any grain at all? */
  if (glen > 0) {
    /* check for quantifier on last symbol of the grain */
    end = read_kleene(point, &one_or_more);
    if (end > point) {
      /* remove last symbol from grain_data if made optional by quantifier */
      if (!one_or_more) {
        glen--;
        if (grain_data)
          grain_point = grain_last_char;
      }
      if (align_end)
        *align_end = 0; /* grain cannot be anchored at end (even if one_or_more) */
    }
    point = end;
  }

  if (grain_data)
    *grain_point = '\0'; /* NUL-terminated extracted grain */
  if (len)
    *len = glen;

  if (glen > 0)
    return point;
  else
    return mark;
}

/**
 * Finds grains in a simple disjunction group - part of the CL Regex Optimiser.
 *
 * This function parses a simple parenthesized disjunction within a regular expression
 * and attempts to extract one grain from each alternative. Grains are written to the
 * local grain buffer. If a complete grain set has been found, the functions returns
 * a pointer to the first character after the disjunction. Otherwise it returns mark
 * and the caller can try to accept the group with read_matchall.
 *
 * For simplicity, only the first grain in each alternative is considered. This makes
 * it easier to check start/end alignment of the grains. Note that the local grain
 * buffer is always mangled, even if the function is unsuccessful.
 *
 * The first argument, mark, must point to the '(' at the beginning of the
 * disjunction group (unless no_paren is set).
 *
 * The booleans align_start and align_end are set to true if the grains from
 * *all* alternatives are anchored at the start or end of the disjunction
 * group, respectively.
 *
 * This is a non-exported function.
 *
 * @param mark         Pointer to the disjunction group (see also function
 *                     description).
 * @param align_start  See function description.
 * @param align_end    See function description.
 * @param no_paren     Bool: if true, attempt to read a top-level disjunction
 *                     without parentheses, which must extend to the end of the string.
 * @return             A pointer to first character after the part of the string
 *                     that was checked iff the parse succeeded, the original pointer in
 *                     the mark argument otherwise.
 *
 */
static char *
read_disjunction(char *mark, int *align_start, int *align_end, int no_paren)
{
  char *point, *p2, *buf;
  int grain;

  point = mark;
  if (no_paren) {
    if (*point == '(')
      return mark; /* bare disjunction must NOT be parenthesized */
  }
  else {
    if (*point == '(') {
      point++;
      if (*point == '?') {
        point++;
        /* don't accept special (?...) elements, except for simple non-capturing (?:...) */
        if (*point == ':')
          point++;
        else
          return mark; /* failed to parse disjunction */
      }
    }
    else
      return mark; /* disjunction group must be parenthesized */
  }

  buf = local_grain_data;
  grain_buffer_grains = 0;
  grain = 0;

  /* Note: if we can extend the disjunction parser further to allow parentheses around the
   * initial segment of an alternative, then regexen created by the matches operator will
   * also be optimised. */

  *align_start = *align_end = 1;

  /* loop over alternatives in disjunction, using first grain found in each */
  while (1) {
    /* attempt to read a grain */
    p2 = read_grain(point, buf, &(grain_buffer_len[grain]), align_end);

    /* if no grain could be read, try reading a wildcard. */
    while (p2 == point) {
      p2 = read_wildcard(point); /* try skipping a wildcard element */
      if (p2 > point) {
        point = p2;       /* advance point and look for grain again */
        *align_start = 0; /* grain in this alternative can't be aligned at start */
      }
      else
        return mark;      /* no grain found in this alternative -> unsuccessful */
      p2 = read_grain(point, buf, &(grain_buffer_len[grain]), align_end);
    }

    grain_buffer[grain] = buf; /* store grain in local grain buffer */
    buf += strlen(buf) + 1;
    grain++;

    if (grain >= MAX_GRAINS)
      return mark; /* too many alternatives, can't extract full grain set */

    point = p2;
    // TODO BUG??: should there be a check for point pointing to 0 before attempting the next loop????

    while (*point != '|' && *point != ')') {
      p2 = read_wildcard(point); /* try skipping data up to next | or ) */
      if (p2 > point) {
        point = p2;
        *align_end = 0;          /* grain in this alternative can't be aligned at end */
      }
      else
        break;
    }

    if (*point == '|')
      point++;                   /* continue with next alternative */
    else
      break;                     /* abort scanning */
  }
  /* end of while (1) loop over alternatives in disjunction */

  if ( (!no_paren && *point == ')') || (no_paren && *point == '\0') ) {
    /* we've successfully read the entire disjunction */
    grain_buffer_grains = grain;
    if (grain >= 1)
// BUG - in old version, we always skip an extra char for the )   but if no_paren, we're already at the end of the string.
// (this caused the caller to have a point AFTER the terminating nul!)
//      return point + 1;          /* success */
      return point + (no_paren ? 0 : 1);          /* success */
  }

  return mark;  /* unsuccessful */
}

/**
 * Updates the public grain buffer -- part of the CL Regex Optimiser.
 *
 * This function copies the local grains to the public buffer, if they
 * are better than the set of grains currently there. The decision is
 * made with a heuristic based on the character length of the shortest
 * grain and the number of different grains.
 *
 * A non-exported function.
 *
 * @param at_start  Boolean: if True, all grains are anchored on the left
 * @param at_end    Boolean: if True, all grains are anchored on the right
 *
 */
static void
update_grain_buffer(int at_start, int at_end)
{
  char *buf = public_grain_data;
  int i, len;

  /* N is a convenience variable */
  int N = grain_buffer_grains;

  if (N > 0) {
    /* len is set to the length of the shortest grain in the intermediate buffer */
    len = grain_buffer_len[0];
    for (i = 1; i < N; i++) {
      if (grain_buffer_len[i] < len)
        len = grain_buffer_len[i];
    }

    if (len >= 2) {
      /* don't bother assessing the grain set unless the minimum grain length in the set is 2 or more.
         a jump length of 1 is no good to anyone. If we DO have a min length of at least 2,
         we make a heuristic decision whether the new set of grains is better than the current one,
         based on grain length and the number of grains:

         If the new grains are a lot longer, they win.
         If the new grains are slightly longer, they win unless there are 3 times more of them.
         If the new grains are the same length, they win if there are fewer of them (by any amount).
         If the new grains are slightly shorter, they only win if there are 3 times fewer of them.
       */
      if (    (len >  (cl_regopt_grain_len + 1))
          || ((len == (cl_regopt_grain_len + 1)) && ( N      <= (3 * cl_regopt_grains) ))
          || ((len ==  cl_regopt_grain_len     ) && ( N      <       cl_regopt_grains  ))
          || ((len == (cl_regopt_grain_len - 1)) && ((3 * N) <       cl_regopt_grains  ))
      ) {
        /* the new set of grains is better, copy them to the output buffer */
        for (i = 0; i < N; i++) {
          strcpy(buf, grain_buffer[i]);  /* we are physically copying from local_grain_data to public_grain_data */
          cl_regopt_grain[i] = buf;      /* and putting pointers to the new public_grain_data content in the main array */
          buf += strlen(buf) + 1;
        }
        cl_regopt_grains = N;
        cl_regopt_grain_len = len;
        cl_regopt_anchor_start = at_start;
        cl_regopt_anchor_end = at_end;
      }
    }
  }
}

/**
 * Print out a jump table.
 */
static void
print_jump_table(int *jumptable)
{
#if 0
  int j, k;
#endif
  unsigned int ch;

  /* Print out the entire jumptable. FORMAT FOR EACH CELL:
   *                              | x y |
   * where x is the jump distance, and y is the character (as a char
   * in the main ascii range, as a hex number for the rest of the range). */

#if 0
  fprintf(stderr, "CL: cl_regopt_jumptable for Boyer-Moore search is\n");
  for (k = 0; k < 256; k += 16) {
    fprintf(stderr, "CL: ");
    for (j = 0, ch = k ; j < 15 ; j++, ch++)
      // TODO BUG??? shouldn't the condition in the above line be j <= 15 or j < 16? since k increases by incremenets of 16. Empirically, the 0x?f member of each row isn't printed.
      fprintf(stderr, (ch >= 32 && ch < 127) ? "|%2d %c  " : "|%2d %02X ", jumptable[ch], ch);
    fprintf(stderr, "\n");
  }
#else

  // simplified single-loop version of the above,,,,
  fprintf(stderr, "CL: cl_regopt_jumptable for Boyer-Moore search is");
  for (ch = 0 ; ch < 256 ; ch++) {
    if (0 == ch % 16)
      fprintf(stderr, "\nCL: ");
    fprintf(stderr, (ch >= 32 && ch < 127) ? "|%2d %c  " : "|%2d %02X ", jumptable[ch], ch);
  }
  fprintf(stderr, "\n");
#endif
}

/**
 * Computes a jump table for Boyer-Moore searches -- part of the CL Regex Optimiser.
 *
 * Unlike the textbook version, this jumptable includes the last character of each grain
 * (so we don't have to start the string comparison loops every time).
 *
 * SOME DETAIL: The jump table maps each possible char (0 to 255) to the number of bytes that may
 * be skipped in a subject string when it is encountered.
 *
 * EG if min grain length is 2, chars not in any grain will map to 2 (because that point in the
 * string can't be an endpoint, and neither can the following position, since that char can't be in first or second position).
 *
 * But chars possibly at the end of a grain will map to 0, because the current position *could*
 * be the end of a grain, and therefore we can't skip over the next position.
 *
 * A non-exported function.
 */
static void
make_jump_table(CL_Regex rx)
{
  int jdw;  /* jump distance - working */
  int jump; /* shortest jump distance found so far */
  int g_ix; /* index into grain array */

  unsigned int ch;      /* char we're currently calculating the jump for */
  unsigned char *grain; /* we want an unsigned char to compare with unsigned int ch */

  /* clear the jump table -- i.e. set jump distance to 0 for all chars. */
  for (ch = 0; ch < 256; ch++)
    rx->jumptable[ch] = 0;

  if (rx->grains > 0) {
    /* compute smallest jump distance for each char (0 -> matches last byte of one or more grains) */
    for (ch = 0; ch < 256; ch++) {
      /* begin by assuming that we can jump by the minimum-grain-length for the full set of grains.
       * then, try to disprove that by looking for that character in each grain. */
      jump = rx->grain_len;

      /* for each grain... */
      for (g_ix = 0; g_ix < rx->grains; g_ix++) {
        /* get a pointer to last byte in grain */
        grain = (unsigned char *) rx->grain[g_ix] + rx->grain_len - 1;
        // TODO rx->grain_len is the min len for the whole set of grains,
        // so the comment on the line above "pointer to last byte in grain"
        // isn't true - it *might* be a pointer to the middle of a grain...
        // is it the comment that is correct, or the code?

        /* and go through grain backwards byte by byte, until we find the character. */
        for (jdw = 0; jdw < rx->grain_len; jdw++, grain--) {
          if (*grain == ch) {
            /* j is the min jump distance yielded by this grain. If it is shorter than
             * the shortest jump found so far, set it as the new shortest. */
            if (jdw < jump)
              jump = jdw;
            break; /* can't find shorter jump distance for this grain */
          }
        }
        /* grain check completed; go on to the next grain */
      }

      /* store jump, which is the shortest jump found for any grain -
       * or, still rx->grain_len, if nothing shorter was found */
      rx->jumptable[ch] = jump;
    }

    if (cl_debug)
      print_jump_table(rx->jumptable);
  }
  /* if no grains have been found, don't do anything (apart from clearing the jump table) */
}

static void
print_regopt_data(CL_Regex rx)
{
  int i;
  fprintf(stderr, "CL: Regex optimised, %d grain(s) of length %d\n",
      rx->grains, rx->grain_len);
  fprintf(stderr, "CL:     grain set is");
  for (i = 0; i < rx->grains; i++) {
    fprintf(stderr, " [%s]", rx->grain[i]);
  }
  if (rx->anchor_start)
    fprintf(stderr, " (anchored at beginning of string)");
  if (rx->anchor_end)
    fprintf(stderr, " (anchored at end of string)");
  fprintf(stderr, "\n");

}

/**
 * Copy grain set from internal global variables to CL_Regex object -- part of the CL Regex Optimiser.
 *
 * This function carries out four important tasks:
 *  - It copies the grain set found by cl_regopt_analyse from the internal global buffer
 *    to member variables of the specified CL_Regex object.
 *  - It casefolds the grains if required (i.e. if rx->icase is True).
 *  - It cuts grains to equal byte length, preserving start/end anchoring if possible.
 *    Note that our implementation of Boyer-Moore search requires equally sized grains.
 *  - It calls make_jump_table to generate a lookup table for the Boyer-Moore search.
 *
 * This is a  non-exported function.
 *
 * @param rx    a pointer to an initialized CL_Regex object
 */
static void
regopt_data_copy_to_regex_object(CL_Regex rx)
{
  char *grain;
  int i, l, len, cut_prefix;

  rx->grains = cl_regopt_grains;

  if (rx->icase) {
    /* casefold strings if required (in case this reduces byte length) */
    grain = local_grain_data; /* we can use this static buffer for the casefolded grains */
    for (i = 0; i < rx->grains; i++) {
      strcpy(grain, cl_regopt_grain[i]);
      cl_string_canonical(grain, rx->charset, IGNORE_CASE, CL_MAX_LINE_LENGTH);
      cl_regopt_grain[i] = grain; /* pointers into static buffer, so we can just move them */
      grain += strlen(grain) + 1;
    }
  }

  /* determine common byte length of grains */
  len = CL_MAX_LINE_LENGTH;
  for (i = 0; i < rx->grains; i++) {
    l = strlen(cl_regopt_grain[i]);
    if (l < len)
      len = l;
  }

  /* determine whether to remove prefix or suffix of grains to reduce to common length */
  cut_prefix = (cl_regopt_anchor_end && !cl_regopt_anchor_start);

  /* now allocate copy of each grain (reduced to specified number of bytes) */
  for (i = 0; i < rx->grains; i++) {
    grain = cl_regopt_grain[i];
    l = strlen(grain);
    if (l > len) {
      /* need to shorten */
      if (cut_prefix) {
        rx->grain[i] = cl_strdup(grain + l - len);
        cl_regopt_anchor_start = 0; /* no longer anchored left */
      }
      else {
        rx->grain[i] = cl_strdup(grain);
        rx->grain[i][len] = '\0';
        cl_regopt_anchor_end = 0;  /* no longer anchored right */
      }
    }
    else {
      /* grain already has the desired length */
      rx->grain[i] = cl_strdup(grain);
    }
  }

  rx->grain_len = len;
  rx->anchor_start = cl_regopt_anchor_start;
  rx->anchor_end = cl_regopt_anchor_end;

  if (cl_debug)
    print_regopt_data(rx);

  /* compute jump table for Boyer-Moore search */
  if (rx->grains > 0)
    make_jump_table(rx);

  if (cl_debug)
    fprintf(stderr, "CL: using %d grain(s) for optimised regex matching\n", rx->grains);
}

/**
 * Analyses a regular expression and tries to find the best set of grains.
 *
 * Part of the regex optimiser. For a given regular expression, this function will
 * try to extract a set of grains from regular expression {regex_string}. These
 * grains are then used by the CL regex matcher and cl_regex2id() as a pre-filter
 * for faster regular expression search.
 *
 * The function only recognizes relatively simple regular expressions without
 * recursive nesting, which roughly correspond to wildcard searches similar to
 * SQL's LIKE operator. Searching for a fixed prefix, suffix or infix (or a
 * small set of alternatives) will see the most noticeable performance improvement.
 * Any more complex expression is passed directly to the standard PCRE evaluation.
 * See cl_new_regex() for an overview of the supported subsets of PCRE syntax.
 *
 * If successful, this function returns True and stores the grains
 * in the optimiser's global variables, from which they then must be
 * copied to a CL_Regex object with regopt_data_copy_to_regex_object(). This step
 * will also casefold grains if necessary and prepare the Boyer-Moore jump tables.
 *
 * Usage: optimised = cl_regopt_analyse(regex_string);
 *
 * This is a non-exported function.
 *
 * @param regex  String containing the regex to optimise.
 * @return       Boolean: true = ok, false = couldn't optimise regex.
 */
static int
regopt_analyse(char *regex)
{
  /* mark  = where we are up to in the regex.
   * point = potential point at the end of a grain. */
  char *point, *mark;

  int ok, at_start, at_end, one_or_more, align_start, align_end;

  mark = regex;
  if (cl_debug)
    fprintf(stderr, "CL: cl_regopt_analyse('%s')\n", regex);

  /* clear the public grain data */
  cl_regopt_grains = 0;
  cl_regopt_grain_len = 0;
  cl_regopt_anchor_start = cl_regopt_anchor_end = 0;

  /**
   * The code below parses a regular expression using the supported PCRE subset
   * and attempts to extract a literal string ("grain") that must occur in every
   * match of the regular expression. In a disjunction group, each alternative
   * must contain a grain in order to be used for optimized matching; we refer to
   * this as a complete grain set.
   *
   * The algorithm scans the regular expression from left to right,
   * looking for the following supported elements in the specified order:
   *
   *  1. a literal grain (read_grain)
   *  2. a parenthesized disjunction containing grains (read_disjunction)
   *  3. a segment matching some, possibly empty substring (read_wildcard),
   *     including recursively nested capturing or non-capturing groups
   *
   * See the respective functions for further information on these elements.
   * Grains are collected by read_grain and read_disjunction in a local buffer.
   * If a complete grain set has been found, it replaces the previous global
   * grain set if it is considered to be more effective based on the length
   * and number of grains.
   *
   * As a special case, the optimizer first attempts to match the entire
   * regexp as a simple disjunction (read_disjunction) without parentheses.
   */

  /* uniquely, at start of string check for a bare disjunction at top level (not parenthesized) */
  point = read_disjunction(mark, &align_start, &align_end, 1);
  if (point > mark) {
// debug note: delete anon
//if (cl_debug)printf("Found a BARE disjunction from \"%s\", starts at [%x] with %x, ends at [%x] with %x and is start-anch=%d, end-anch=%d\n", mark, mark, *mark, point, *point, align_start, align_end);
    update_grain_buffer(align_start, align_end);
    /* if we found one, then we're done */
    return (cl_regopt_grains > 0) ? 1 : 0;
  }

  ok = 1;
  while (ok) {
    /* accept whatever we're got so far, if we're at end of the regex */
    if (*mark == '\0')
      return ok = (cl_regopt_grains > 0) ? 1 : 0;

    at_start = (mark == regex);

    /* check for a grain at the current mark */
    point = read_grain(mark, local_grain_data, &(grain_buffer_len[0]), &align_end);
    if (point > mark) {
      /* grain found : store it by copying a pointer into the intermediate buffer */
      grain_buffer[0] = local_grain_data;
      grain_buffer_grains = 1;
      mark = point;
      /* update public grain set */
      at_end = (*mark == '\0');
      update_grain_buffer(at_start, at_end);
    }
    else {
      /* check for a simple parenthesized disjunction */
      point = read_disjunction(mark, &align_start, &align_end, 0);
      if (point > mark) {
        mark = point;
        at_end = (*mark == '\0'); /* can only be anchored if there's no quantifier */
        /* can't accept grain set if there's a quantifier that makes the group optional */
        point = read_kleene(mark, &one_or_more);
        if (point == mark)
          one_or_more = 1; /* no quantifier found, so not optional */
        else
          mark = point;
        if (one_or_more) {
          /* update public grain set */
          at_start = (at_start && align_start); /* check that grains within disjunction are aligned, too */
          at_end = (at_end && align_end);
          update_grain_buffer(at_start, at_end);
        }
      }
      else {
        point = read_wildcard(mark);
        if (point > mark) { /* found segment matching some (possibly empty) substring -> skip */
          mark = point;
        }
        else {
          ok = 0; /* no recognised segment starting at mark */
        }
      }
    }
  } /* end of while (ok) */

  /* couldn't analyse regexp -> no optimisation */
  return 0;
}


/* two monitoring functions */

/**
 * Reset the "success counter" for optimised regexes.
 */
void
cl_regopt_count_reset(void)
{
  cl_regopt_successes = 0;
}

/**
 * Get a reading from the "success counter" for optimised regexes.
 *
 * The counter is incremented by 1 every time the "grain" system
 * is used successfully to avoid calling PCRE. That is, it is
 * incremented every time a string is scrutinised and found to
 * contain none of the grains.
 *
 * Usage:
 *
 * cl_regopt_count_reset();
 *
 * for (i = 0, hits = 0; i < n; i++)
 *   if (cl_regex_match(rx, haystacks[i]))
 *     hits++;
 *
 * fprintf(stderr,
 *         "Found %d matches; avoided regex matching %d times out of %d trials",
 *         hits, cl_regopt_count_get(), n );
 *
 * @see cl_regopt_count_reset
 * @return an integer indicating the number of times a regular expression
 *         has been matched using the regopt system of "grains", rather
 *         than by calling an external regex library.
 */
int
cl_regopt_count_get(void)
{
  return cl_regopt_successes;
}
