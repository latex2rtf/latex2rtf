/*
 * "optind.c". This file is used for compatibility with some programs (like
 * latex2rtf) that expect optarg and optind to be declared sowewhere.
 */

#ifdef HAS_NO_GETOPT
char           *optarg = 0;
int             optind = 1;
#else
int AVOID_EMPTY_OPTIND_SOURCE;
#endif
/* end of optind.c */
