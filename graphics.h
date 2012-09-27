#ifndef _INCLUDED_GRAPHICS_H
#define _INCLUDED_GRAPHICS_H 1

#define FIGURE_INCLUDEGRAPHICS 1
#define FIGURE_EPSFFILE        2
#define FIGURE_EPSFBOX         3
#define FIGURE_BOXEDEPSF       4
#define FIGURE_PSFIG           5

typedef enum {BITMAP, EPS} conversion_t;
 
void PrepareDisplayedBitmap(char *the_type);
void FinishDisplayedBitmap(void);
void WriteLatexAsBitmapOrEPS(char *pre, char *eq, char *post, conversion_t convertTo);
void PutLatexFile(const char *tex_file_stem, double scale, const char *pre, conversion_t convertTo, int hinline);
void CmdGraphics(int code);
void CmdPicture(int code);
void CmdPsPicture(int code);
void CmdPsGraph(int code);
void CmdMusic(int code);
void CmdPsset(int code);
void CmdNewPsStyle(int code);
void CmdGraphicsPath(int code);

typedef void PutFileFnc(char *, double, double, double, double);

typedef struct {
    char       *extension;
    PutFileFnc *encoder;
} GraphConvertElement;
#endif
