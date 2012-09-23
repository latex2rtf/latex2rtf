#ifndef _MAIN_H_INCLUDED
#define _MAIN_H_INCLUDED 1

#if defined(UNIX)
#define ENVSEP ':'
#define PATHSEP '/'
#define HAS_STRDUP
#endif

#if defined(MSDOS) || defined(OS2)
#define ENVSEP ';'
#define PATHSEP '\\'
#endif 

#if defined(VMS)
#define ENVSEP ','
#define PATHSEP ''
#endif

#ifdef HAS_STRDUP
#else
#define strdup my_strdup
#endif

#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#endif

#define ERROR 0
#define WARNING 1

#define MAXCOMMANDLEN 100

/* available values for alignment */
#define LEFT      'l'
#define RIGHT     'r'
#define CENTERED  'c'
#define JUSTIFIED 'j'

#define PATHMAX 255

/*** error constants ***/
#include <assert.h>
#include <stdio.h>
#include <stdint.h>

/*** handy boolean type definition ***/
#ifndef TRUE
typedef enum { FALSE = 0,
              TRUE } boolean;
#endif

/*** interpret comment lines that follow the '%' with this string ***/
extern const char  * InterpretCommentString;

void            diagnostics(int level, char *format,...);

extern          char *g_aux_name;
extern          char *g_toc_name;
extern          char *g_lof_name;
extern          char *g_lot_name;
extern          char *g_bbl_name;
extern          char *g_home_dir;
extern          char *progname;         /* name of the executable file */

extern int      GermanMode;
extern int      FrenchMode;
extern int      RussianMode;
extern int      CzechMode;
extern int      pagenumbering;
extern int      headings;

extern int      g_verbosity_level;
extern int      RecursionLevel;

/* table  & tabbing variables */
extern long     pos_begin_kill;
extern int      g_tab_counter;
extern int      g_equation_column;

extern int      twocolumn;
extern int      titlepage;

extern int      g_processing_equation;
extern int      g_processing_preamble;
extern int      g_processing_figure;
extern int      g_processing_table;
extern int      g_processing_tabbing;
extern int      g_processing_tabular;
extern int      g_processing_eqnarray;
extern int      g_processing_arrays;
extern uint16_t g_dots_per_inch;

extern int      g_document_type;
extern int      g_document_bibstyle;

extern int      g_equation_number;
extern int      g_escape_parens;
extern int      g_show_equation_number;
extern int      g_enumerate_depth;
extern int      g_suppress_equation_number;
extern int      g_aux_file_missing;
extern int      g_bbl_file_missing;
extern int      g_graphics_package;
extern int      g_amsmath_package;

extern char     *g_figure_label;
extern char     *g_table_label;
extern char     *g_equation_label;
extern char     *g_section_label;
extern char     *g_config_path;
extern char     *g_script_dir;
extern char     g_field_separator;
extern char     *g_preamble;

extern double   g_png_equation_scale; 
extern double   g_png_figure_scale;
extern int      g_latex_figures;
extern int      g_endfloat_figures;
extern int      g_endfloat_tables;
extern int      g_endfloat_markers;

extern int      g_equation_inline_rtf;
extern int      g_equation_display_rtf;
extern int      g_equation_inline_bitmap;
extern int      g_equation_display_bitmap;
extern int      g_equation_comment;
extern int      g_equation_raw_latex;
extern int      g_equation_inline_eps;
extern int      g_equation_display_eps;
extern int      g_equation_mtef;

extern int      g_figure_include_direct;
extern int      g_figure_include_converted;
extern int      g_figure_comment_direct;
extern int      g_figure_comment_converted;

extern int      g_tabular_display_rtf;
extern int      g_tabular_display_bitmap;

extern int      g_little_endian;
extern int      g_tableofcontents;

void fprintRTF(char *format, ...);
void putRtfCharEscaped(char cThis);
void putRtfStrEscaped(const char * string);
char *getTmpPath(void);
char *my_strdup(const char *str);
FILE *my_fopen(char *path, char *mode);

void debug_malloc(void);

#endif
