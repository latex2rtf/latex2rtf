#define FOOTNOTE            1
#define FOOTNOTE_TEXT       2
#define FOOTNOTE_THANKS     3

#define LABEL_LABEL			1
#define LABEL_HYPERREF		2
#define LABEL_REF			3
#define LABEL_HYPERCITE  	4
#define LABEL_CITE			5
#define LABEL_HYPERPAGEREF	6
#define LABEL_PAGEREF		7

void    CmdFootNote(int code);
void    CmdLabel(int code);
void 	CmdNoCite(int code);
void 	CmdBibliography(int code);
void 	CmdThebibliography(int code);
void 	CmdBibitem(int code);
void 	CmdNewblock(int code);
void	CmdIndex(int code);
void	CmdPrintIndex(int code);
