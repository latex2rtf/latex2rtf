#define FIGURE_INCLUDEGRAPHICS 1
#define FIGURE_EPSFFILE         2
#define FIGURE_EPSFBOX         3
#define FIGURE_BOXEDEPSF       4
#define FIGURE_PSFIG           5

void PrepareDisplayedBitmap(char *the_type);
void FinishDisplayedBitmap(void);
void WriteLatexAsBitmap(char *pre, char *eq, char *post);
void PutLatexFile(const char *tex_file_stem, double scale, const char *pre);
void CmdGraphics(int code);
void CmdPicture(int code);
void CmdPsPicture(int code);
void CmdPsGraph(int code);
void CmdMusic(int code);
void CmdPsset(int code);
void CmdNewPsStyle(int code);
