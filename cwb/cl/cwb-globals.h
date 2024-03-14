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

/**
 * @file
 *
 * This file contains variables, constants etc. that are defined inside the CL,
 * but are not part of the public interface in the cl.h - but they really belong to
 * all of CWB, not just the CL, and the utilities and CQP need them too.
 */

#ifndef _cl_cwb_globals_h_
#define _cl_cwb_globals_h_

/* so we can test whether log2 already exists in standard math library or needs to be defined by us */
#include <math.h>

/* The CWB_VERSION macro should be defined by the build environment.
 * If it isn't already defined, this definition ensures compilation of the CL,
 * and any programs that use it, won't fail (e.g. if you're test-compiling
 * a single file that contains CWB_VERSION).
 */
#ifndef CWB_VERSION
/**
 * The current version of CWB.
 */
#  define CWB_VERSION " x.y.z "
#endif


/**
 * String used to identify the default attribute.
 *
 * It is "word".
 *
 * Don't change this or we'll all end up in hell !!! I MEAN IT !!!!
 */
#define CWB_DEFAULT_ATT_NAME "word"

/**
 * Default string used as value of p-attributes when a value is missing, ie if a tab-delimited field is empty.
 */
#define CWB_PA_UNDEF_VALUE "__UNDEF__"


/* default registry settings */
#ifndef CWB_REGISTRY_DEFAULT_PATH
#  ifndef _WIN64
     /** The default path assumed for the location of the corpus registry. This is almost always overriden by the make system. */
#    define CWB_REGISTRY_DEFAULT_PATH  "/usr/local/share/cwb/registry"
#  else
#    define CWB_REGISTRY_DEFAULT_PATH  (cl_variable_registry)
/* this extern is here so that it is available anywhere the CWB_REGISTRY_DEFAULT_PATH macro is. */
extern char cl_variable_registry[];
#  endif
#endif


#ifndef CWB_REGISTRY_ENVVAR
/** The environment variable from which the value of the registry will be taken. */
#  define CWB_REGISTRY_ENVVAR        "CORPUS_REGISTRY"
#endif

#ifndef CWB_USE_7Z_ENVVAR
/** The environment variable which controls use of 7z over gzip/bzip2/xz/7z. */
#  define CWB_USE_7Z_ENVVAR          "CWB_USE_7Z"
#endif

#ifndef CWB_COMPRESSOR_PATH_ENVVAR
/** The environment variable which identifies the folder to look in for gzip/bzip2/xz/7z. */
#  define CWB_COMPRESSOR_PATH_ENVVAR "CWB_COMPRESSOR_PATH"
#endif




/* default filename of an info file */
#ifndef _WIN64
#  define CWB_INFOFILE_DEFAULT_NAME ".info"
#else
  /* since ANYTHING can be specified manually in the reg file,
   * we might as well make the default filename one that Windows
   * will actually allow you to create! */
#  define CWB_INFOFILE_DEFAULT_NAME "corpus-info.txt"
  /* only used in cwb-encode, so here isn't really the place for it, but
   * for now let's keep it with other OS-path-control macros */
#endif


/** magic number for subcorpus (incl. query) file format: ORIGINAL version */
#define CWB_SUBCORPMAGIC_ORIG 36193928
/* the sum of the original programmers' birthdays: 15081963 (Max) + 21111965 (Oli) */

/** magic number for subcorpus (incl. query) file format: NEW version (== orig + 1) */
#define CWB_SUBCORPMAGIC_NEW 36193929
/* new format -- Mon Jul 31 17:19:27 1995 (oli) */




/*
 * Macros for path-handling: different between Unix and Windows.
 *
 * They are used across CWB, so are defined here.
 * Lack of prefix should not be a problem because CQP/utils *know* about these macros
 * and won't use their names for anything else.
 */
#ifndef _WIN64
    /* Unix */
/** character used to separate different paths in a string variable */
#  define PATH_SEPARATOR ':'
/** character used to delimit subdirectories in a path */
#  define SUBDIR_SEPARATOR '/'
/** character from SUBDIR_SEPARATOR as a string for compile-time concatenation */
#  define SUBDIR_SEP_STRING "/"
/** name of directory for temporary files (as string, absolute path) */
#  define TEMPDIR_PATH "/tmp"
#else
    /* Windows */
#  define PATH_SEPARATOR ';'
#  define SUBDIR_SEPARATOR '\\'
#  define SUBDIR_SEP_STRING "\\"
#  define TEMPDIR_PATH "." /* A CQP user may not have access to C:\Temp, which is where they SHOULD go */
#endif
/*
 * NOTE:
 * Glib has G_DIR_SEPARATOR and G_SEARCHPATH_SEPARATOR, which might be better than the above.
 */



/**
 * size in bytes of string buffers capable of holding absolute paths
 * of temporary filenames; needs to be big enough for TEMPDIR_PATH plus
 * the result of a call to tempnam() plus the length of a process ID, at least.
 */
#define TEMP_FILENAME_BUFSIZE 128

/* this is also Win32 compatibility... extra flag for open() */
/* so that (x | O_BINARY) always == x under POSIX */
#ifndef O_BINARY
#  ifdef _O_BINARY
#    define O_BINARY _O_BINARY
#  else
#    define O_BINARY 0
#  endif
#endif

#ifndef _WIN64
  /* for use with [fs]printf(), all decimal or floating-point conversions, as follows:
   * "%" COMMA_SEP_THOUSANDS_CONVSPEC "d" (or equivalent) */
#  define COMMA_SEP_THOUSANDS_CONVSPEC "'"
#else
#  define COMMA_SEP_THOUSANDS_CONVSPEC ""
  /* this feature only supported on POSIX 2008+ compliant printf(), which mingw-64 printf() isn't
   * -- probably because actual Windows printf() isn't. In the Windows API you are supposed to use
   * some horror called GetNumberFormat(), which, sorry, but no. */
#endif


/*
 * making sure we have:
 *     log2()
 *     bool, true, false
 *     the zu format specifier for size_t, or a fallback to lu
 */
#if __STDC_VERSION__ >= 199901L

/* C99 has log2() already, but many pre-C99 Cs don't */
#  include <stdbool.h>
#  define SIZET_SPEC "%zu"

#else

#  ifndef log2
#    define log2(x) (log(x)/log(2.0))
#  endif

#  ifdef __bool_true_false_are_defined
    /* I guess we have them after all! */
#  else
#    ifndef bool
#      define bool  int
#    endif
#    ifndef true
#      define true  1
#    endif
#    ifndef false
#      define false 0
#    endif
#  endif

#  define SIZE_SPEC "%lu"

#endif





/* Path to the compression binaries is external within CL, but not outside it. */
extern char cl_compressor_path[CL_MAX_FILENAME_LENGTH];


#endif

