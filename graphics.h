#define FIGURE_INCLUDEGRAPHICS 1
#define FIGURE_EPSFFILE         2
#define FIGURE_EPSFBOX         3
#define FIGURE_BOXEDEPSF       4
#define FIGURE_PSFIG           5

void PutLatexFile(char *s, double height0, double width0, double scale, char *pre);
void CmdGraphics(int code);
void CmdPicture(int code);
void CmdMusic(int code);
