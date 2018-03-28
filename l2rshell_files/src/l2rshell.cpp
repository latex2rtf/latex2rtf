    //////////////////////////////////////////////////////
    //                                                  //
    //      l2rshell (GUI for LaTeX2RTF converter)      //
    //                 v.0.6.9 for win32                //
    //    author: Mikhail Polianski (mnpol@mail.ru)     //
    //                                                  //
    //////////////////////////////////////////////////////
    //     -> version number is also in line 1502       //
//
// libcomctl32 library has to be included into compiler options
#include "l2rshell.h"
#include "dlg.h"

// l2rshell variables
HINSTANCE hInstance;

// functions
WORD* CreateDlg();
LRESULT CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
void SetTab(HWND, int);           //set/change the current tab control item
void changeext(WCHAR*, WCHAR*);   //example: changeext(L"file.tex", L".rtf") will convert "file.tex" into "file.rtf"
void getdir(WCHAR*);              //example: getdir(L"C:\\directory\\file.tex") --''-- "C:\\directory"
void getfilename(WCHAR*);         //example: getfilename(L"C:\\directory\\file.tex") --''-- "file.tex"
void ChangeTheLanguage(HWND);     //changes the user interface language
void ConvertToSomething(HWND);    //checks if equations and table converted at least to something. If not require convertion to RTF
void ResetLangCombo(HWND);        //resets the "language" combobox contents
void ResetCodepageCombo(HWND);    //resets the "codepage" combobox contents
void ResetShellLangCombo(HWND);   //resets the Shell language combobox contents
void Run(HWND);
bool OpenFile(HWND, WCHAR*);
bool FindLatex(WCHAR*);           //Find path to 'latex.exe'
bool FindImageMagick(WCHAR*);     //Find path to 'convert.exe'
bool FindGhostscript(WCHAR*);     //Find path to 'gsdll32.dll'
bool FileDialog(HWND, WCHAR*, WCHAR*, DWORD, WCHAR*, bool); // Call Open File or Save File dialog
bool PathDialog(HWND, WCHAR*);                              // Call Get Path dialog
WCHAR* GetInitString(WCHAR*, WCHAR*, WCHAR*);               // Reads a string from initialization file
int GetInitInt(WCHAR*, int);                                // Reads a number from initialization file
WCHAR* GetLngString(HWND, WCHAR*, WCHAR*, WCHAR*);          // Reads a string from current language file
WCHAR* GetVersionString(WCHAR*);                            // Reads a version information from 'version.ini'
void SaveUserData(HWND);

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInstance,
                LPSTR lpszCmdLine, int nCmdShow)
{
    hInstance=hInst;
    char str_ascii[MAX_PATH];
    WCHAR str[MAX_PATH];
    HKEY hkey;
    DWORD dwrd, dwType;
    WIN32_FIND_DATA findfiledata;
    int j=0;
    
    j=0;
    for(int i=0; i<strlen(lpszCmdLine); i++)
        if(lpszCmdLine[i] != '"')
        {
            str_ascii[j] = lpszCmdLine[i];
            j++;
        }
    str_ascii[j]=0;

    MultiByteToWideChar(CP_ACP, 0, str_ascii, -1, str, MAX_PATH);
    
    changeext(str, L".tex");
    CharLower(str);
    if(FindFirstFile(str, &findfiledata)!=INVALID_HANDLE_VALUE) // if '.tex' file specified in command line
    {
        int i = 0;
        RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\latex2rtf", 0, NULL,
            REG_OPTION_NON_VOLATILE	, KEY_WRITE, NULL, &hkey, &dwrd);
        RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\latex2rtf", 0, KEY_WRITE, &hkey);
        RegSetValueEx(hkey, L"texfilename", 0, REG_SZ, (BYTE*)&str, 2*lstrlen(str)+1);
        RegSetValueEx(hkey, L"tabctrl_item", 0, REG_DWORD, (BYTE*)&i, sizeof(i));
        RegCloseKey(hkey);
    }
    
    RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\latex2rtf", 0, KEY_READ, &hkey);
    dwrd = MAX_PATH;
    if(RegQueryValueEx(hkey, L"", NULL, &dwType, (LPBYTE)str, &dwrd)==ERROR_SUCCESS)
        SetCurrentDirectory(str);;
    RegCloseKey(hkey);
    
    InitCommonControls(); // !!!!! Tab control support (libcomctl32 library has to be included in compiler options)
    CreateDlg();
    return 0;
}

WORD* CreateDlg()
{
    WORD *p, *pdlgtemplate;
    pdlgtemplate=p=(PWORD)LocalAlloc(LPTR, 8000);
    // dialog template data
    DWORD lStyle=DS_3DLOOK|DS_CENTER|WS_MINIMIZEBOX|WS_VISIBLE|WS_CAPTION|WS_SYSMENU|DS_SHELLFONT;
    DlgTemplate(p,lStyle,103,0,0,340,277,L"LaTeX2RTF"); //103 - total number of dialog items
    // group boxes
    lStyle=WS_CHILD|BS_GROUPBOX;
    DlgItemTemplate(p,lStyle,15,85,320,71,ID_EQUATIONS_GROUP,L"button", NULL);
    DlgItemTemplate(p,lStyle,15,186,280,57,ID_BITMAPS_GROUP,L"button", NULL);
    DlgItemTemplate(p,lStyle,15,97,280,73,ID_RTF_GROUP,L"button", NULL);
    DlgItemTemplate(p,lStyle,15,174,135,57,ID_LANG_GROUP,L"button", NULL);
    DlgItemTemplate(p,lStyle,160,174,135,72,ID_DEBUG_GROUP,L"button", NULL);
    DlgItemTemplate(p,lStyle,15,25,280,37,ID_CHANGELANG_GROUP,L"button", NULL);
    DlgItemTemplate(p,lStyle,15,72,280,171,ID_PATHS_GROUP,L"button", NULL);
    DlgItemTemplate(p,lStyle,15,25,280,218,ID_ABOUT_GROUP,L"button", NULL);
    // static text
    lStyle=WS_CHILD;
    DlgItemTemplate(p,lStyle,15,25,280,12,ID_TEXFILE_TEXT,L"static", NULL);
    DlgItemTemplate(p,lStyle,15,55,70,12,ID_RTFFILE_TEXT,L"static", NULL);
    DlgItemTemplate(p,lStyle,25,97,160,12,ID_EQUATIONS_TXT1,L"static", NULL);
    DlgItemTemplate(p,lStyle,25,112,160,12,ID_EQUATIONS_TXT2,L"static", NULL);
    DlgItemTemplate(p,lStyle,25,127,160,12,ID_EQUATIONS_TXT3,L"static", NULL);
    DlgItemTemplate(p,lStyle,25,142,160,12,ID_TABLES_TXT,L"static", NULL);
    DlgItemTemplate(p,lStyle,25,198,60,12,ID_BITMAPS_TXT1,L"static", NULL);
    DlgItemTemplate(p,lStyle,134,198,100,12,ID_BITMAPS_TXT2,L"static", NULL);
    DlgItemTemplate(p,lStyle,25,213,60,12,ID_BITMAPS_TXT3,L"static", NULL);
    DlgItemTemplate(p,lStyle,134,213,60,12,ID_BITMAPS_TXT4,L"static", NULL);
    DlgItemTemplate(p,lStyle,229,213,60,12,ID_BITMAPS_TXT5,L"static", NULL);
    DlgItemTemplate(p,lStyle,15,20,160,12,ID_BBLFILE_TEXT,L"static", NULL);
    DlgItemTemplate(p,lStyle,15,45,70,12,ID_AUXFILE_TEXT,L"static", NULL);
    DlgItemTemplate(p,lStyle,15,70,70,12,ID_TMPDIR_TEXT,L"static", NULL);
    DlgItemTemplate(p,lStyle,25,155,40,12,ID_RTF_TXT1,L"static", NULL);
    DlgItemTemplate(p,lStyle,100,154,170,12,ID_RTF_TXT2,L"static", NULL);
    DlgItemTemplate(p,lStyle,25,108,170,12,ID_RTF_TXT3,L"static", NULL);
    DlgItemTemplate(p,lStyle,25,188,50,12,ID_LANG_TXT1,L"static", NULL);
    DlgItemTemplate(p,lStyle,25,211,50,12,ID_LANG_TXT2,L"static", NULL);
    DlgItemTemplate(p,lStyle,170,188,74,12,ID_DEBUG_TXT,L"static", NULL);
    DlgItemTemplate(p,lStyle,25,85,240,12,ID_CONFIG_TEXT,L"static", NULL);
    DlgItemTemplate(p,lStyle,25,125,240,12,ID_LATEX_TEXT,L"static", L"LaTeX (latex.exe)");
    DlgItemTemplate(p,lStyle,25,165,240,12,ID_IMAGICK_TEXT,L"static", L"ImageMagick (convert.exe)");
    DlgItemTemplate(p,lStyle,25,205,240,12,ID_GS_TEXT,L"static", L"Ghostscript (gsdll32.dll):");
    DlgItemTemplate(p,lStyle,30,85,30,12,ID_ABOUT_TEXT02,L"static", NULL);
    DlgItemTemplate(p,lStyle,80,85,150,12,ID_ABOUT_TEXT03,L"static",L"Wilfried Hennings  (project administrator)");
    DlgItemTemplate(p,lStyle,80,100,75,12,ID_ABOUT_TEXT04,L"static",L"Fernando Dorner");
    DlgItemTemplate(p,lStyle,170,100,75,12,ID_ABOUT_TEXT05,L"static",L"Andreas Granzer");
    DlgItemTemplate(p,lStyle,80,115,75,12,ID_ABOUT_TEXT06,L"static",L"Ralf Schlatterbeck");
    DlgItemTemplate(p,lStyle,170,115,75,12,ID_ABOUT_TEXT07,L"static",L"Friedrich Polzer");
    DlgItemTemplate(p,lStyle,80,130,75,12,ID_ABOUT_TEXT08,L"static",L"Gerhard Trisko");
    DlgItemTemplate(p,lStyle,170,130,75,12,ID_ABOUT_TEXT09,L"static",L"Georg Lehner");
    DlgItemTemplate(p,lStyle,80,145,75,12,ID_ABOUT_TEXT10,L"static",L"Scott Prahl");
    DlgItemTemplate(p,lStyle,170,145,75,12,ID_ABOUT_TEXT11,L"static",L"Mikhail Polyanskiy");
    lStyle=WS_CHILD|ES_CENTER;
    DlgItemTemplate(p,lStyle,30,50,250,12,ID_ABOUT_TEXT01,L"static",NULL);
    // edit controls
    lStyle=WS_CHILD|ES_AUTOHSCROLL|WS_BORDER;
    DlgItemTemplate(p,lStyle,15,35,260,12,ID_TEXFILENAME,L"edit", NULL);
    DlgItemTemplate(p,lStyle,15,65,260,12,ID_RTFFILENAME,L"edit", NULL);
    DlgItemTemplate(p,lStyle,15,30,260,12,ID_BBLFILENAME,L"edit", NULL);
    DlgItemTemplate(p,lStyle,15,55,260,12,ID_AUXFILENAME,L"edit", NULL);
    DlgItemTemplate(p,lStyle,15,80,260,12,ID_TMPDIRNAME,L"edit", NULL);  
    DlgItemTemplate(p,lStyle,181,213,90,12,ID_DEBUGFILENAME,L"edit", NULL);
    DlgItemTemplate(p,lStyle,25,98,240,12,ID_CONFIG,L"edit", NULL);
    DlgItemTemplate(p,lStyle,25,138,240,12,ID_LATEX,L"edit", NULL);
    DlgItemTemplate(p,lStyle,25,178,240,12,ID_IMAGICK,L"edit", NULL);
    DlgItemTemplate(p,lStyle,25,218,240,12,ID_GS,L"edit", NULL);
    DlgItemTemplate(p,lStyle,95,211,35,12,ID_EQSCALE,L"edit", NULL);
    DlgItemTemplate(p,lStyle,190,211,35,12,ID_FIGSCALE,L"edit", NULL);
    lStyle=WS_CHILD|ES_NUMBER|WS_BORDER;
    DlgItemTemplate(p,lStyle,95,196,35,12,ID_BMPRES,L"edit", NULL);
    // pushbuttons
    lStyle=WS_CHILD|WS_TABSTOP|WS_VISIBLE|BS_PUSHBUTTON;
    DlgItemTemplate(p,lStyle,5,259,60,15,ID_RUN,L"button", NULL);     // L"Run!" button
    DlgItemTemplate(p,lStyle,175,259,60,15,ID_HELP,L"button", NULL);  // L"Help" button
    DlgItemTemplate(p,lStyle,245,259,60,15,ID_EXIT,L"button", NULL);  // L"Exit" button
    lStyle=WS_CHILD|BS_PUSHBUTTON;
    DlgItemTemplate(p,lStyle,285,35,12,12,ID_TEXFILE_BUTTON,L"button", L"...");
    DlgItemTemplate(p,lStyle,285,65,12,12,ID_RTFFILE_BUTTON,L"button", L"...");
    DlgItemTemplate(p,lStyle,285,30,12,12,ID_BBLFILE_BUTTON,L"button", L"...");
    DlgItemTemplate(p,lStyle,285,55,12,12,ID_AUXFILE_BUTTON,L"button",L"...");
    DlgItemTemplate(p,lStyle,285,80,12,12,ID_TMPDIR_BUTTON,L"button",L"...");
    DlgItemTemplate(p,lStyle,275,98,12,12,ID_CONFIG_BUTTON,L"button",L"...");
    DlgItemTemplate(p,lStyle,275,138,12,12,ID_LATEX_BUTTON,L"button",L"...");
    DlgItemTemplate(p,lStyle,275,178,12,12,ID_IMAGICK_BUTTON,L"button",L"...");
    DlgItemTemplate(p,lStyle,275,218,12,12,ID_GS_BUTTON,L"button",L"...");
    DlgItemTemplate(p,lStyle,30,190,120,14,ID_HOMEPAGE_BUTTON,L"button",NULL);
    DlgItemTemplate(p,lStyle,160,190,120,14,ID_CANHELP_BUTTON,L"button",NULL);
    DlgItemTemplate(p,lStyle,30,215,250,14,ID_GPL_BUTTON,L"button",NULL);
    // checkboxes
    lStyle=WS_CHILD|BS_AUTOCHECKBOX;
    DlgItemTemplate(p,lStyle,85,55,210,8,ID_RTFDEF,L"button", NULL);
    DlgItemTemplate(p,lStyle,160,95,40,12,ID_DISPLAYED_TO_RTF,L"button", L"RTF");
    DlgItemTemplate(p,lStyle,220,95,40,12,ID_DISPLAYED_TO_BITMAP,L"button", L"bitmap");
    DlgItemTemplate(p,lStyle,280,95,40,12,ID_DISPLAYED_TO_EPS,L"button", L"EPS");
    DlgItemTemplate(p,lStyle,160,110,40,12,ID_INLINE_TO_RTF,L"button", L"RTF");
    DlgItemTemplate(p,lStyle,220,110,40,12,ID_INLINE_TO_BITMAP,L"button", L"bitmap");
    DlgItemTemplate(p,lStyle,280,110,40,12,ID_INLINE_TO_EPS,L"button", L"EPS");
    DlgItemTemplate(p,lStyle,160,125,55,12,ID_EQTEXTTOTEXT,L"button", NULL);
    DlgItemTemplate(p,lStyle,220,125,70,12,ID_EQTEXTTOCOMM,L"button", NULL);
    DlgItemTemplate(p,lStyle,160,140,40,12,ID_TABLES_TO_RTF,L"button", L"RTF");
    DlgItemTemplate(p,lStyle,220,140,40,12,ID_TABLES_TO_BITMAP,L"button", L"bitmap");
    DlgItemTemplate(p,lStyle,25,227,265,12,ID_BMPFORFIG,L"button", NULL);
    DlgItemTemplate(p,lStyle,25,165,265,12,ID_FIGSASFILENAMES,L"button", NULL);
    DlgItemTemplate(p,lStyle,85,20,210,8,ID_BBLDEF,L"button", NULL);
    DlgItemTemplate(p,lStyle,85,45,210,8,ID_AUXDEF,L"button",NULL);
    DlgItemTemplate(p,lStyle,85,70,210,8,ID_TMPDEF,L"button",NULL); 
    DlgItemTemplate(p,lStyle,25,122,265,12,ID_PARANTHESES,L"button", NULL);
    DlgItemTemplate(p,lStyle,25,138,265,12,ID_SEMICOLONS,L"button", NULL);
    DlgItemTemplate(p,lStyle,125,106,60,12,ID_USEEQFIELDS,L"button", NULL);
    DlgItemTemplate(p,lStyle,200,106,90,12,ID_USEREFFIELDS,L"button", NULL);
    DlgItemTemplate(p,lStyle,170,201,120,12,ID_DEBUGTOFILE,L"button", NULL);
    DlgItemTemplate(p,lStyle,170,230,120,12,ID_WARNINGSINRTF,L"button", NULL);
    DlgItemTemplate(p,lStyle,165,85,120,8,ID_CFGDEF,L"button", NULL);
    DlgItemTemplate(p,lStyle,165,125,120,8,ID_LATEXDEF,L"button", NULL);
    DlgItemTemplate(p,lStyle,165,165,120,8,ID_IMAGICKDEF,L"button", NULL);
    DlgItemTemplate(p,lStyle,165,205,120,8,ID_GSDEF,L"button", NULL);
    // lists
    lStyle=WS_CHILD|CBS_DROPDOWNLIST;
    DlgItemTemplate(p,lStyle,70,153,23,120,ID_ADDEXTRA,L"combobox", NULL);
    DlgItemTemplate(p,lStyle,255,186,23,96,ID_DEBUGLEVEL,L"combobox", NULL);
    lStyle=WS_CHILD|CBS_DROPDOWNLIST|WS_VSCROLL;
    DlgItemTemplate(p,lStyle,70,186,75,86,ID_LANG,L"combobox", NULL);
    DlgItemTemplate(p,lStyle,70,209,75,86,ID_CODEPAGE,L"combobox", NULL);
    lStyle=WS_CHILD|CBS_DROPDOWNLIST|WS_VSCROLL;
    DlgItemTemplate(p,lStyle,100,40,110,240,ID_SHELLLANG,L"combobox", NULL);
    // tab control
    lStyle=WS_CHILD|WS_TABSTOP|WS_VISIBLE;
    DlgItemTemplate(p,lStyle,5, 5, 390, 250,ID_TAB,(LPCTSTR)WC_TABCONTROL, NULL);
    // create dialog
    DialogBoxIndirect(hInstance, (LPDLGTEMPLATE)pdlgtemplate, NULL, (DLGPROC)DlgProc);
    LocalFree(LocalHandle(pdlgtemplate));
    return pdlgtemplate;
}

LRESULT CALLBACK DlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    WCHAR str[MAX_PATH];
    int i;
    float fl;
    void* hIcon;
    
    switch (msg)
    {
        case WM_INITDIALOG:   
            // Load icons
            GetModuleFileName(NULL, str, MAX_PATH);
            getdir(str);
            lstrcat(str, L"\\l2rshell_files\\l2rshell.ico");
            hIcon = LoadImage(NULL, str, IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
            if(hIcon) SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
            hIcon = LoadImage(NULL, str, IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
            if(hIcon) SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
            // Initialize controls
            SendMessage(GetDlgItem(hwnd, ID_TEXFILENAME), EM_LIMITTEXT, MAX_PATH-1, 0L);
            SendMessage(GetDlgItem(hwnd, ID_RTFFILENAME), EM_LIMITTEXT, MAX_PATH-1, 0L);
            SendMessage(GetDlgItem(hwnd, ID_BBLFILENAME), EM_LIMITTEXT, MAX_PATH-1, 0L);
            SendMessage(GetDlgItem(hwnd, ID_AUXFILENAME), EM_LIMITTEXT, MAX_PATH-1, 0L);
            SendMessage(GetDlgItem(hwnd, ID_TMPDIRNAME), EM_LIMITTEXT, MAX_PATH-1, 0L);
            SendMessage(GetDlgItem(hwnd, ID_DEBUGFILENAME), EM_LIMITTEXT, MAX_PATH-1, 0L);
            SendMessage(GetDlgItem(hwnd, ID_CONFIG), EM_LIMITTEXT, MAX_PATH-1, 0L);
            SendMessage(GetDlgItem(hwnd, ID_LATEX), EM_LIMITTEXT, MAX_PATH-1, 0L);
            SendMessage(GetDlgItem(hwnd, ID_IMAGICK), EM_LIMITTEXT, MAX_PATH-1, 0L);
            SendMessage(GetDlgItem(hwnd, ID_GS), EM_LIMITTEXT, MAX_PATH-1, 0L);
            SendMessage(GetDlgItem(hwnd, ID_BMPRES), EM_LIMITTEXT, 3, 0L);
            SendMessage(GetDlgItem(hwnd, ID_EQSCALE), EM_LIMITTEXT, 5, 0L);
            SendMessage(GetDlgItem(hwnd, ID_FIGSCALE), EM_LIMITTEXT, 5, 0L);
            // TEX filename
            SetDlgItemText(hwnd, ID_TEXFILENAME, GetInitString(L"texfilename", L"", str));
            // RTF filename and "use default" checkbox
            SetDlgItemText(hwnd, ID_RTFFILENAME, GetInitString(L"rtffilename", L"", str));
            i = GetInitInt(L"rtfdef", 1);
            SendMessage(GetDlgItem(hwnd, ID_RTFDEF), BM_SETCHECK, i, 0L);
            SendMessage(hwnd, WM_COMMAND, ID_RTFDEF, i);
            // BBL filename and "use default" checkbox
            SetDlgItemText(hwnd, ID_BBLFILENAME, GetInitString(L"bblfilename", L"", str));
            i = GetInitInt(L"bbldef", 1);
            SendMessage(GetDlgItem(hwnd, ID_BBLDEF), BM_SETCHECK, i, 0L);
            SendMessage(hwnd, WM_COMMAND, ID_BBLDEF, i);
            // AUX filename and "use default" checkbox
            SetDlgItemText(hwnd, ID_AUXFILENAME, GetInitString(L"auxfilename", L"", str));
            i = GetInitInt(L"auxdef", 1);
            SendMessage(GetDlgItem(hwnd, ID_AUXDEF), BM_SETCHECK, i, 0L);
            SendMessage(hwnd, WM_COMMAND, ID_AUXDEF, i);
            // TMP dirname and "use default" checkbox
            SetDlgItemText(hwnd, ID_TMPDIRNAME, GetInitString(L"tmpdirname", L"", str));
            i = GetInitInt(L"tmpdef", 1);
            SendMessage(GetDlgItem(hwnd, ID_TMPDEF), BM_SETCHECK, i, 0L);
            SendMessage(hwnd, WM_COMMAND, ID_TMPDEF, i);
            // DEBUG filename
            SetDlgItemText(hwnd, ID_DEBUGFILENAME, GetInitString(L"debugfilename", L"latex2rtf.log", str));
            // Language combo: must be initialized before Path to configuration directory
            SendMessage(GetDlgItem(hwnd, ID_LANG), CB_ADDSTRING, 0, (LPARAM)GetInitString(L"lang", L"-", str));
            SendMessage(GetDlgItem(hwnd, ID_LANG), CB_SETCURSEL, 0, 0);
            // Path to configuration directory and "use default" checkbox
            SetDlgItemText(hwnd, ID_CONFIG, GetInitString(L"config", L"", str));
            i = GetInitInt(L"cfgdef", 1);
            SendMessage(GetDlgItem(hwnd, ID_CFGDEF), BM_SETCHECK, i, 0L);
            SendMessage(hwnd, WM_COMMAND, ID_CFGDEF, i);
            // Path to LaTeX and "use default" checkbox
            SetDlgItemText(hwnd, ID_LATEX, GetInitString(L"latex", L"", str));
            i = GetInitInt(L"latexdef", 1);
            SendMessage(GetDlgItem(hwnd, ID_LATEXDEF), BM_SETCHECK, i, 0L);
            SendMessage(hwnd, WM_COMMAND, ID_LATEXDEF, i);
            // Path to Imagemagick and "use default" checkbox
            SetDlgItemText(hwnd, ID_IMAGICK, GetInitString(L"imagick", L"", str));
            i = GetInitInt(L"imagickdef", 1);
            SendMessage(GetDlgItem(hwnd, ID_IMAGICKDEF), BM_SETCHECK, i, 0L);
            SendMessage(hwnd, WM_COMMAND, ID_IMAGICKDEF, i);
            // Path to Ghostscript and "use default" checkbox
            SetDlgItemText(hwnd, ID_GS, GetInitString(L"gs", L"", str));
            i = GetInitInt(L"gsdef", 1);
            SendMessage(GetDlgItem(hwnd, ID_GSDEF), BM_SETCHECK, i, 0L);
            SendMessage(hwnd, WM_COMMAND, ID_GSDEF, i);
            // Version string from 'version.ini'
            SetDlgItemText(hwnd, ID_ABOUT_TEXT01, GetVersionString(str));
            // Checkboxes (except "use default")
            SendMessage(GetDlgItem(hwnd, ID_DISPLAYED_TO_RTF), BM_SETCHECK, GetInitInt(L"displayed_to_rtf", 1), 0L);
            SendMessage(GetDlgItem(hwnd, ID_DISPLAYED_TO_BITMAP), BM_SETCHECK, GetInitInt(L"displayed_to_bitmap", 0), 0L);
            SendMessage(GetDlgItem(hwnd, ID_DISPLAYED_TO_EPS), BM_SETCHECK, GetInitInt(L"displayed_to_eps", 0), 0L);
            SendMessage(GetDlgItem(hwnd, ID_INLINE_TO_RTF), BM_SETCHECK, GetInitInt(L"inline_to_rtf", 1), 0L);
            SendMessage(GetDlgItem(hwnd, ID_INLINE_TO_BITMAP), BM_SETCHECK, GetInitInt(L"inline_to_bitmap", 0), 0L);
            SendMessage(GetDlgItem(hwnd, ID_INLINE_TO_EPS), BM_SETCHECK, GetInitInt(L"inline_to_eps", 0), 0L);
            SendMessage(GetDlgItem(hwnd, ID_EQTEXTTOTEXT), BM_SETCHECK, GetInitInt(L"eqtexttotext", 0), 0L);
            SendMessage(GetDlgItem(hwnd, ID_EQTEXTTOCOMM), BM_SETCHECK, GetInitInt(L"eqtexttocomm", 0), 0L);
            SendMessage(GetDlgItem(hwnd, ID_TABLES_TO_RTF), BM_SETCHECK, GetInitInt(L"tables_to_rtf", 1), 0L);
            SendMessage(GetDlgItem(hwnd, ID_TABLES_TO_BITMAP), BM_SETCHECK, GetInitInt(L"tables_to_bitmap", 0), 0L);
            SendMessage(GetDlgItem(hwnd, ID_BMPFORFIG), BM_SETCHECK, GetInitInt(L"bmpforfig", 0), 0L);
            SendMessage(GetDlgItem(hwnd, ID_FIGSASFILENAMES), BM_SETCHECK, GetInitInt(L"figsasfilenames", 0), 0L);
            SendMessage(GetDlgItem(hwnd, ID_PARANTHESES), BM_SETCHECK, GetInitInt(L"parantheses", 0), 0L);
            SendMessage(GetDlgItem(hwnd, ID_SEMICOLONS), BM_SETCHECK, GetInitInt(L"semicolons", 0), 0L);
            SendMessage(GetDlgItem(hwnd, ID_USEEQFIELDS), BM_SETCHECK, GetInitInt(L"useeqfields", 1), 0L);
            SendMessage(GetDlgItem(hwnd, ID_USEREFFIELDS), BM_SETCHECK, GetInitInt(L"usereffields", 1), 0L);
            SendMessage(GetDlgItem(hwnd, ID_DEBUGTOFILE), BM_SETCHECK, GetInitInt(L"debugtofile", 0), 0L);
            SendMessage(GetDlgItem(hwnd, ID_DEBUGFILENAME), EM_SETREADONLY, 1-SendMessage(GetDlgItem(hwnd, ID_DEBUGTOFILE), BM_GETCHECK, 0, 0L), 0L);
            SendMessage(GetDlgItem(hwnd, ID_WARNINGSINRTF), BM_SETCHECK, GetInitInt(L"warningsinrtf", 0), 0L);
            
            ConvertToSomething(hwnd);
            
            // Windows position
            SetWindowPos(hwnd, NULL, GetInitInt(L"xpos", 50), GetInitInt(L"ypos", 50), 0, 0, SWP_NOSIZE|SWP_NOZORDER);
            // "Add ... extra '}'" combobox
            for(i=0; i<=9; i++)
            {
                wsprintf(str, L"%d", i);
                SendMessage(GetDlgItem(hwnd, ID_ADDEXTRA), CB_ADDSTRING, 0, (LPARAM)str);
                if(i<=7)
                    SendMessage(GetDlgItem(hwnd, ID_DEBUGLEVEL), CB_ADDSTRING, 0, (LPARAM)str);
            }
            SendMessage(GetDlgItem(hwnd, ID_ADDEXTRA), CB_SETCURSEL, GetInitInt(L"addextra", 0), 0L);
            // "Debugging level" combobox
            SendMessage(GetDlgItem(hwnd, ID_DEBUGLEVEL), CB_SETCURSEL, GetInitInt(L"debuglevel", 1), 0L);
            // "Codepage" combobox
            SendMessage(GetDlgItem(hwnd, ID_CODEPAGE), CB_ADDSTRING, 0, (LPARAM)GetInitString(L"codepage", L"-", str));
            SendMessage(GetDlgItem(hwnd, ID_CODEPAGE), CB_SETCURSEL, 0, 0);
            ResetCodepageCombo(hwnd);
            // "Change the language" combobox
            SendMessage(GetDlgItem(hwnd, ID_SHELLLANG), CB_ADDSTRING, 0, (LPARAM)GetInitString(L"shell_lang", L"-", str));
            SendMessage(GetDlgItem(hwnd, ID_SHELLLANG), CB_SETCURSEL, 0, 0);
            // "Resolution": edit box and up-down control
            CreateUpDownControl(WS_CHILD|WS_VISIBLE|UDS_ALIGNRIGHT|UDS_SETBUDDYINT,
            0,0,0,0,hwnd,ID_BMPRES_UPDOWN,NULL,GetDlgItem(hwnd, ID_BMPRES),600,25,GetInitInt(L"bmpres", 300));
            // "Equation scale": edit box and up-down control
            CreateUpDownControl(WS_CHILD|WS_VISIBLE|UDS_ALIGNRIGHT,
            0,0,0,0,hwnd,ID_EQSCALE_UPDOWN,NULL,GetDlgItem(hwnd, ID_EQSCALE),1000,10,122);
            SetDlgItemText(hwnd, ID_EQSCALE, GetInitString(L"eqscale", L"1.00", str));
            // "Figure scale": edit box and up-down control
            CreateUpDownControl(WS_CHILD|WS_VISIBLE|UDS_ALIGNRIGHT,
            0,0,0,0,hwnd,ID_FIGSCALE_UPDOWN,NULL,GetDlgItem(hwnd, ID_FIGSCALE),1000,10,135);
            SetDlgItemText(hwnd, ID_FIGSCALE, GetInitString(L"figscale", L"1.00", str));
            
            // Initialize tabs
            TC_ITEM item;
            for(i=0; i<=3; i++)
                TabCtrl_InsertItem(GetDlgItem(hwnd, ID_TAB), i, &item);
            SetTab(hwnd, GetInitInt(L"tabctrl_item", 0));       
            // Set the GUI language
            ResetShellLangCombo(hwnd);
            ChangeTheLanguage(hwnd);
        break;
        
        case WM_NOTIFY:
            // Display selected tab
            if(LOWORD(wParam)==ID_TAB)
            {
                if(((NMHDR FAR *)lParam)->code == TCN_SELCHANGE)
                    SetTab(hwnd, TabCtrl_GetCurSel(GetDlgItem(hwnd, ID_TAB)));
            }
            // Changing equations scale using UpDown control
             if(LOWORD(wParam)==ID_EQSCALE_UPDOWN)
            {
                GetDlgItemText(hwnd, ID_EQSCALE, str, 5);
                swscanf(str, L"%f", &fl);
                i = (int)floor(fl*100+0.5) + ((NM_UPDOWN FAR *)lParam)->iDelta;
                if(i<10) i=10;
                if(i>1000) i=1000;
                swprintf(str, L"%.2f", i/100.0);
                SetDlgItemText(hwnd, ID_EQSCALE, str);
            }
            // Changing figures scale using UpDown control
             if(LOWORD(wParam)==ID_FIGSCALE_UPDOWN)
            {
                GetDlgItemText(hwnd, ID_FIGSCALE, str, 5);
                swscanf(str, L"%f", &fl);
                i = (int)floor(fl*100+0.5) + ((NM_UPDOWN FAR *)lParam)->iDelta;
                if(i<10) i=10;
                if(i>1000) i=1000;
                swprintf(str, L"%.2f", i/100.0);
                SetDlgItemText(hwnd, ID_FIGSCALE, str);
            }
        break;
        
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case ID_RUN:                    
                    Run(hwnd);
                break;
                
                case ID_TEXFILENAME:
                    if(HIWORD(wParam)==EN_CHANGE)
                    {
                        SendMessage(GetDlgItem(hwnd, ID_TEXFILENAME), WM_GETTEXT, MAX_PATH, (LPARAM)str);
                        if(SendMessage(GetDlgItem(hwnd, ID_RTFDEF), BM_GETCHECK, 0, 0L))
                        {
                            getfilename(str);
                            changeext(str, L".rtf");
                            SetDlgItemText(hwnd, ID_RTFFILENAME, str);
                        }
                        if(SendMessage(GetDlgItem(hwnd, ID_BBLDEF), BM_GETCHECK, 0, 0L))
                        {
                            getfilename(str);
                            changeext(str, L".bbl");
                            SetDlgItemText(hwnd, ID_BBLFILENAME, str);
                        }
                        if(SendMessage(GetDlgItem(hwnd, ID_AUXDEF), BM_GETCHECK, 0, 0L))
                        {
                            getfilename(str);
                            changeext(str, L".aux");
                            SetDlgItemText(hwnd, ID_AUXFILENAME, str);
                        }
                    }
                break;
                
                case ID_RTFDEF:
                    SendMessage(GetDlgItem(hwnd, ID_RTFFILENAME), EM_SETREADONLY, SendMessage(GetDlgItem(hwnd, ID_RTFDEF), BM_GETCHECK, 0, 0L), 0L);
                    if(SendMessage(GetDlgItem(hwnd, ID_RTFDEF), BM_GETCHECK, 0, 0L))
                    {
                        SendMessage(GetDlgItem(hwnd, ID_TEXFILENAME), WM_GETTEXT, MAX_PATH, (LPARAM)str);
                        getfilename(str);
                        changeext(str, L".rtf");
                        SetDlgItemText(hwnd, ID_RTFFILENAME, str);
                    }
                break;
                
                case ID_BBLDEF:
                    SendMessage(GetDlgItem(hwnd, ID_BBLFILENAME), EM_SETREADONLY, SendMessage(GetDlgItem(hwnd, ID_BBLDEF), BM_GETCHECK, 0, 0L), 0L);
                    if(SendMessage(GetDlgItem(hwnd, ID_BBLDEF), BM_GETCHECK, 0, 0L))
                    {
                        SendMessage(GetDlgItem(hwnd, ID_TEXFILENAME), WM_GETTEXT, MAX_PATH, (LPARAM)str);
                        getfilename(str);
                        changeext(str, L".bbl");
                        SetDlgItemText(hwnd, ID_BBLFILENAME, str);
                    }
                break;
                
                case ID_AUXDEF:
                    SendMessage(GetDlgItem(hwnd, ID_AUXFILENAME), EM_SETREADONLY, SendMessage(GetDlgItem(hwnd, ID_AUXDEF), BM_GETCHECK, 0, 0L), 0L);
                    if(SendMessage(GetDlgItem(hwnd, ID_AUXDEF), BM_GETCHECK, 0, 0L))
                    {
                        SendMessage(GetDlgItem(hwnd, ID_TEXFILENAME), WM_GETTEXT, MAX_PATH, (LPARAM)str);
                        getfilename(str);
                        changeext(str, L".aux");
                        SetDlgItemText(hwnd, ID_AUXFILENAME, str);
                    }
                break;
                
                case ID_TMPDEF:
                    SendMessage(GetDlgItem(hwnd, ID_TMPDIRNAME), EM_SETREADONLY, SendMessage(GetDlgItem(hwnd, ID_TMPDEF), BM_GETCHECK, 0, 0L), 0L);
                    if(SendMessage(GetDlgItem(hwnd, ID_TMPDEF), BM_GETCHECK, 0, 0L))
                    {
                        lstrcpy(str, L"");
                        SetDlgItemText(hwnd, ID_TMPDIRNAME, str);
                    }
                break;

                case ID_DEBUGTOFILE:
                    SendMessage(GetDlgItem(hwnd, ID_DEBUGFILENAME), EM_SETREADONLY, 1-SendMessage(GetDlgItem(hwnd, ID_DEBUGTOFILE), BM_GETCHECK, 0, 0L), 0L);
                break;

                case ID_DEBUGFILENAME:
                    if(HIWORD(wParam)==EN_CHANGE)
                    {
                        SendMessage(GetDlgItem(hwnd, ID_DEBUGFILENAME), WM_GETTEXT, MAX_PATH, (LPARAM)str);
                    }
                break;
                
                case ID_CFGDEF:
                    SendMessage(GetDlgItem(hwnd, ID_CONFIG), EM_SETREADONLY, SendMessage(GetDlgItem(hwnd, ID_CFGDEF), BM_GETCHECK, 0, 0L), 0L);
                    if(SendMessage(GetDlgItem(hwnd, ID_CFGDEF), BM_GETCHECK, 0, 0L))
                    {
                        GetModuleFileName(NULL, str, MAX_PATH);
                        getdir(str);
                        lstrcat(str, L"\\cfg");
                        SetDlgItemText(hwnd, ID_CONFIG, str);
                    }
                break;
                
                case ID_LATEXDEF:
                    SendMessage(GetDlgItem(hwnd, ID_LATEX), EM_SETREADONLY, SendMessage(GetDlgItem(hwnd, ID_LATEXDEF), BM_GETCHECK, 0, 0L), 0L);
                    if(SendMessage(GetDlgItem(hwnd, ID_LATEXDEF), BM_GETCHECK, 0, 0L))
                    {
                        if(!FindLatex(str))
                            GetLngString(hwnd, L"310", L"- not installed (?) -", str);
                        SetDlgItemText(hwnd, ID_LATEX, str);
                    }
                break;
                
                case ID_IMAGICKDEF:
                    SendMessage(GetDlgItem(hwnd, ID_IMAGICK), EM_SETREADONLY, SendMessage(GetDlgItem(hwnd, ID_IMAGICKDEF), BM_GETCHECK, 0, 0L), 0L);
                    if(SendMessage(GetDlgItem(hwnd, ID_IMAGICKDEF), BM_GETCHECK, 0, 0L))
                    {
                        if(!FindImageMagick(str))
                           GetLngString(hwnd, L"310", L"- not installed (?) -", str);
                        SetDlgItemText(hwnd, ID_IMAGICK, str);
                    }
                break;
                
                case ID_GSDEF:
                    SendMessage(GetDlgItem(hwnd, ID_GS), EM_SETREADONLY, SendMessage(GetDlgItem(hwnd, ID_GSDEF), BM_GETCHECK, 0, 0L), 0L);
                    if(SendMessage(GetDlgItem(hwnd, ID_GSDEF), BM_GETCHECK, 0, 0L))
                    {
                        if(!FindGhostscript(str))
                           GetLngString(hwnd, L"310", L"- not installed (?) -", str);
                        SetDlgItemText(hwnd, ID_GS, str);
                    }
                break;
                
                case ID_TEXFILE_BUTTON:
                    SendMessage(GetDlgItem(hwnd, ID_TEXFILENAME), WM_GETTEXT, MAX_PATH, (LPARAM)str);
                    if(FileDialog(hwnd, L"LaTeX (*.tex)\0*.tex\0All Files (*.*)\0*.*\0\0", str, OFN_FILEMUSTEXIST, L"tex", 1))
                        SetDlgItemText(hwnd, ID_TEXFILENAME, str);
                break;
                
                case ID_RTFFILE_BUTTON:
                    SendMessage(GetDlgItem(hwnd, ID_RTFFILENAME), WM_GETTEXT, MAX_PATH, (LPARAM)str);
                    if(FileDialog(hwnd, L"RTF (*.rtf)\0*.rtf\0All Files (*.*)\0*.*\0\0", str, OFN_HIDEREADONLY|OFN_NOCHANGEDIR, L"rtf", 0))
                    {
                        getfilename(str);
                        SetDlgItemText(hwnd, ID_RTFFILENAME, str);
                        SendMessage(GetDlgItem(hwnd, ID_RTFFILENAME), EM_SETREADONLY, 0, 0L);
                        SendMessage(GetDlgItem(hwnd, ID_RTFDEF), BM_SETCHECK, 0, 0L);
                    }
                break;
                
                case ID_BBLFILE_BUTTON:
                    SendMessage(GetDlgItem(hwnd, ID_BBLFILENAME), WM_GETTEXT, MAX_PATH, (LPARAM)str);
                    if(FileDialog(hwnd, L"BBL (*.bbl)\0*.bbl\0All Files (*.*)\0*.*\0\0", str, OFN_FILEMUSTEXIST|OFN_NOCHANGEDIR, L"bbl", 1))
                    {
                        getfilename(str);
                        SetDlgItemText(hwnd, ID_BBLFILENAME, str);
                        SendMessage(GetDlgItem(hwnd, ID_BBLFILENAME), EM_SETREADONLY, 0, 0L);
                        SendMessage(GetDlgItem(hwnd, ID_BBLDEF), BM_SETCHECK, 0, 0L);
                    }
                break;
                
                case ID_AUXFILE_BUTTON:
                    SendMessage(GetDlgItem(hwnd, ID_AUXFILENAME), WM_GETTEXT, MAX_PATH, (LPARAM)str);
                    if(FileDialog(hwnd, L"AUX (*.aux)\0*.aux\0All Files (*.*)\0*.*\0\0", str, OFN_FILEMUSTEXIST|OFN_NOCHANGEDIR, L"aux", 1))
                    {
                        getfilename(str);
                        SetDlgItemText(hwnd, ID_AUXFILENAME, str);
                        SendMessage(GetDlgItem(hwnd, ID_AUXFILENAME), EM_SETREADONLY, 0, 0L);
                        SendMessage(GetDlgItem(hwnd, ID_AUXDEF), BM_SETCHECK, 0, 0L);
                    }
                break;
                
                case ID_TMPDIR_BUTTON:
                    SendMessage(GetDlgItem(hwnd, ID_TMPDIRNAME), WM_GETTEXT, MAX_PATH, (LPARAM)str);
                    if(PathDialog(hwnd, str))
                    {
                        SetDlgItemText(hwnd, ID_TMPDIRNAME, str);
                        SendMessage(GetDlgItem(hwnd, ID_TMPDIRNAME), EM_SETREADONLY, 0, 0L);
                        SendMessage(GetDlgItem(hwnd, ID_TMPDEF), BM_SETCHECK, 0, 0L);
                    }
                break;
                
                case ID_CONFIG_BUTTON:
                    if(PathDialog(hwnd, str))
                    {
                        SetDlgItemText(hwnd, ID_CONFIG, str);
                        SendMessage(GetDlgItem(hwnd, ID_CONFIG), EM_SETREADONLY, 0, 0L);
                        SendMessage(GetDlgItem(hwnd, ID_CFGDEF), BM_SETCHECK, 0, 0L);
                    }
                break;
                
                case ID_LATEX_BUTTON:
                    SendMessage(GetDlgItem(hwnd, ID_LATEX), WM_GETTEXT, MAX_PATH, (LPARAM)str);
                    if(FileDialog(hwnd, L"LaTeX (latex.exe)\0latex.exe\0All Files (*.*)\0*.*\0\0", str, OFN_FILEMUSTEXIST, L"exe", 1))
                    {
                        SetDlgItemText(hwnd, ID_LATEX, str);
                        SendMessage(GetDlgItem(hwnd, ID_LATEX), EM_SETREADONLY, 0, 0L);
                        SendMessage(GetDlgItem(hwnd, ID_LATEXDEF), BM_SETCHECK, 0, 0L);
                    }
                break;
                
                case ID_IMAGICK_BUTTON:
                    SendMessage(GetDlgItem(hwnd, ID_IMAGICK), WM_GETTEXT, MAX_PATH, (LPARAM)str);
                    if(FileDialog(hwnd, L"ImageMagick (convert.exe)\0convert.exe\0All Files (*.*)\0*.*\0\0", str, OFN_FILEMUSTEXIST, L"exe", 1))
                    {
                        SetDlgItemText(hwnd, ID_IMAGICK, str);
                        SendMessage(GetDlgItem(hwnd, ID_IMAGICK), EM_SETREADONLY, 0, 0L);
                        SendMessage(GetDlgItem(hwnd, ID_IMAGICKDEF), BM_SETCHECK, 0, 0L);
                    }
                break;
                
                case ID_GS_BUTTON:
                    SendMessage(GetDlgItem(hwnd, ID_GS), WM_GETTEXT, MAX_PATH, (LPARAM)str);
                    if(FileDialog(hwnd, L"GhostScript (gsdll32.dll)\0gsdll32.dll\0All Files (*.*)\0*.*\0\0", str, OFN_FILEMUSTEXIST, L"dll", 1))
                    {
                        SetDlgItemText(hwnd, ID_GS, str);
                        SendMessage(GetDlgItem(hwnd, ID_GS), EM_SETREADONLY, 0, 0L);
                        SendMessage(GetDlgItem(hwnd, ID_GSDEF), BM_SETCHECK, 0, 0L);
                    }
                break;
                
                case ID_EQSCALE:
                    if(HIWORD(wParam) == EN_KILLFOCUS)
                    {
                        GetDlgItemText(hwnd, ID_EQSCALE, str, 5);
                        swscanf(str, L"%f", &fl);
                        i = (int)floor(fl*100+0.5);
                        if(i<10) i=10;
                        if(i>1000) i=1000;
                        swprintf(str, L"%.2f", i/100.0);
                        SetDlgItemText(hwnd, ID_EQSCALE, str);
                    }
                break;
                
                case ID_FIGSCALE:
                    if(HIWORD(wParam) == EN_KILLFOCUS)
                    {
                        GetDlgItemText(hwnd, ID_FIGSCALE, str, 5);
                        swscanf(str, L"%f", &fl);
                        i = (int)floor(fl*100+0.5);
                        if(i<10) i=10;
                        if(i>1000) i=1000;
                        swprintf(str, L"%.2f", i/100.0);
                        SetDlgItemText(hwnd, ID_FIGSCALE, str);
                    }
                break;
                
                case ID_BMPRES:
                    if(HIWORD(wParam) == EN_KILLFOCUS) // Set resolution value within the range of up-down control (25~600)
                    {
                        i=SendMessage(GetDlgItem(hwnd,ID_BMPRES_UPDOWN), UDM_GETPOS, 0, 0);
                        SendMessage(GetDlgItem(hwnd,ID_BMPRES_UPDOWN), UDM_SETPOS, 0, (LPARAM)i);
                    }
                break;
                
                case ID_SHELLLANG:
                    if(HIWORD(wParam)==CBN_SELCHANGE)
                    {
                        ResetShellLangCombo(hwnd);
                        ChangeTheLanguage(hwnd);
                    }
                break;

                case ID_CONFIG:
                    if(HIWORD(wParam)==EN_CHANGE)
                        ResetLangCombo(hwnd);
                break;
                
                case ID_HELP:
                    OpenFile(hwnd, GetLngString(hwnd, L"helpfile", L"\\doc\\latex2rtf.html", str));
                break;
                
                case ID_HOMEPAGE_BUTTON:
                    OpenFile(hwnd, GetLngString(hwnd, L"homepage_url", L"\\l2rshell_files\\web\\LaTeX2RTF-Homepage.url", str));
                break;
                
                case ID_CANHELP_BUTTON :
                    OpenFile(hwnd, GetLngString(hwnd, L"canhelp", L"\\l2rshell_files\\You can help.txt", str));
                break;
                
                case ID_GPL_BUTTON:
                    OpenFile(hwnd, GetLngString(hwnd, L"gpl_url", L"\\l2rshell_files\\web\\GPL2_en.url", str));
                break;
                
                case ID_EXIT:
                    SendMessage(hwnd, WM_CLOSE, 0, 0);
                break;
            }
        break;
        
        case WM_CLOSE:
            SaveUserData(hwnd);
            EndDialog(hwnd, 0);
        break;
    }
    return 0;
}

void Run(HWND hwnd)
{
    void lstrcatS(HWND, WCHAR*, int, WCHAR*);
	int i;
	int MAX7PATH;
    WCHAR str[MAX_PATH];
    WCHAR strfl[MAX_PATH];
    WCHAR out_str[7*MAX_PATH];
    WIN32_FIND_DATA findfiledata;
    MAX7PATH = 7 * MAX_PATH;
    
    SendMessage(GetDlgItem(hwnd, ID_TEXFILENAME), WM_GETTEXT, MAX_PATH, (LPARAM)str);
    if(FindFirstFile(str, &findfiledata)!=INVALID_HANDLE_VALUE)
    {
        lstrcpy(out_str, L"/C PATH=");
        GetModuleFileName(NULL, str, MAX_PATH);
        getdir(str);
        lstrcatS(hwnd, out_str, MAX7PATH, str);
        lstrcatS(hwnd, out_str, MAX7PATH, L";");
        
        SendMessage(GetDlgItem(hwnd, ID_IMAGICK), WM_GETTEXT, MAX_PATH, (LPARAM)str);
        getdir(str);
        lstrcatS(hwnd, out_str, MAX7PATH, str);
        lstrcatS(hwnd, out_str, MAX7PATH, L";");
        
        SendMessage(GetDlgItem(hwnd, ID_GS), WM_GETTEXT, MAX_PATH, (LPARAM)str);
        getdir(str);
        lstrcatS(hwnd, out_str, MAX7PATH, str);
        lstrcatS(hwnd, out_str, MAX7PATH, L";");
        
        SendMessage(GetDlgItem(hwnd, ID_GS), WM_GETTEXT, MAX_PATH, (LPARAM)str);
        getdir(str);
        getdir(str);
        lstrcatS(hwnd, out_str, MAX7PATH, str);
        lstrcatS(hwnd, out_str, MAX7PATH, L"\\lib;");
        
        SendMessage(GetDlgItem(hwnd, ID_LATEX), WM_GETTEXT, MAX_PATH, (LPARAM)str);
        getdir(str);
        lstrcatS(hwnd, out_str, MAX7PATH, str);
        lstrcatS(hwnd, out_str, MAX7PATH, L";");
        
        GetSystemDirectory(str, MAX_PATH);
        lstrcatS(hwnd, out_str, MAX7PATH, str);
        lstrcatS(hwnd, out_str, MAX7PATH, L";");
        
        GetWindowsDirectory(str, MAX_PATH);
        lstrcatS(hwnd, out_str, MAX7PATH, str);

        lstrcat(out_str, L" & set RTFPATH=");
        SendMessage(GetDlgItem(hwnd, ID_CONFIG), WM_GETTEXT, MAX_PATH, (LPARAM)str);
        lstrcatS(hwnd, out_str, MAX7PATH, str);
        lstrcatS(hwnd, out_str, MAX7PATH, L";");

        lstrcat(out_str, L" & pushd ");
        SendMessage(GetDlgItem(hwnd, ID_TEXFILENAME), WM_GETTEXT, MAX_PATH, (LPARAM)str);
        getdir(str);
        lstrcatS(hwnd, out_str, MAX7PATH, str);
        
        if(!SendMessage(GetDlgItem(hwnd, ID_TMPDEF), BM_GETCHECK, 0, 0L))
        {
            SendMessage(GetDlgItem(hwnd, ID_TMPDIRNAME), WM_GETTEXT, MAX_PATH, (LPARAM)str);
            lstrcatS(hwnd, out_str, MAX7PATH, L" & mkdir ");        
            lstrcatS(hwnd, out_str, MAX7PATH, str);
        }
        
        lstrcatS(hwnd, out_str, MAX7PATH, L" & latex2rt");
        
        if(SendMessage(GetDlgItem(hwnd, ID_CODEPAGE), CB_GETCURSEL, 0, 0L))
        {
            lstrcatS(hwnd, out_str, MAX7PATH, L" -C ");
            SendMessage(GetDlgItem(hwnd, ID_CODEPAGE), CB_GETLBTEXT, SendMessage(GetDlgItem(hwnd, ID_CODEPAGE), CB_GETCURSEL, 0, 0L), (LPARAM)str);
            lstrcatS(hwnd, out_str, MAX7PATH, str);
        }
            
        if(SendMessage(GetDlgItem(hwnd, ID_DEBUGLEVEL), CB_GETCURSEL, 0, 0L)!=1)
        {
            wsprintf(str, L" -d%d", SendMessage(GetDlgItem(hwnd, ID_DEBUGLEVEL), CB_GETCURSEL, 0, 0L));
            lstrcatS(hwnd, out_str, MAX7PATH, str);
        }
        
        SendMessage(GetDlgItem(hwnd, ID_BMPRES), WM_GETTEXT, 4, (LPARAM)str);
        if(lstrcmp(str, L"300"))
        {
            lstrcatS(hwnd, out_str, MAX7PATH, L" -D");
            lstrcatS(hwnd, out_str, MAX7PATH, str);
        }
        
        SendMessage(GetDlgItem(hwnd, ID_EQSCALE), WM_GETTEXT, 6, (LPARAM)str);
        if(lstrcmp(str, L"1.00") && lstrcmp(str, L"1,00"))
        {
            lstrcatS(hwnd, out_str, MAX7PATH, L" -se");
            lstrcatS(hwnd, out_str, MAX7PATH, str);
        }
        
        SendMessage(GetDlgItem(hwnd, ID_FIGSCALE), WM_GETTEXT, 6, (LPARAM)str);
        if(lstrcmp(str, L"1.00") && lstrcmp(str, L"1,00"))
        {
            lstrcatS(hwnd, out_str, MAX7PATH, L" -sf");
            lstrcatS(hwnd, out_str, MAX7PATH, str);
        }
        
        if(SendMessage(GetDlgItem(hwnd, ID_BMPFORFIG), BM_GETCHECK, 0, 0L))
            lstrcatS(hwnd, out_str, MAX7PATH, L" -F");
            
        if(SendMessage(GetDlgItem(hwnd, ID_FIGSASFILENAMES), BM_GETCHECK, 0, 0L))
            lstrcatS(hwnd, out_str, MAX7PATH, L" -E12");

        i=SendMessage(GetDlgItem(hwnd, ID_USEEQFIELDS), BM_GETCHECK, 0, 0L)+2*SendMessage(GetDlgItem(hwnd, ID_USEREFFIELDS), BM_GETCHECK, 0, 0L);
        if(i!= 3)
        {
            wsprintf(str, L" -f%d", i);
            lstrcatS(hwnd, out_str, MAX7PATH, str);
        }
        
        if(SendMessage(GetDlgItem(hwnd, ID_LANG), CB_GETCURSEL, 0, 0L))
        {
            lstrcatS(hwnd, out_str, MAX7PATH, L" -i ");
            SendMessage(GetDlgItem(hwnd, ID_LANG), CB_GETLBTEXT , SendMessage(GetDlgItem(hwnd, ID_LANG), CB_GETCURSEL, 0, 0L), (LPARAM)str);
            lstrcatS(hwnd, out_str, MAX7PATH, str);
        }
        
        ConvertToSomething(hwnd);
        
        i = SendMessage(GetDlgItem(hwnd, ID_DISPLAYED_TO_RTF), BM_GETCHECK, 0, 0L)+ 
            SendMessage(GetDlgItem(hwnd, ID_DISPLAYED_TO_BITMAP), BM_GETCHECK, 0, 0L)*4 +
            SendMessage(GetDlgItem(hwnd, ID_INLINE_TO_RTF), BM_GETCHECK, 0, 0L)*2 +
            SendMessage(GetDlgItem(hwnd, ID_INLINE_TO_BITMAP), BM_GETCHECK, 0, 0L)*8 +
            SendMessage(GetDlgItem(hwnd, ID_EQTEXTTOCOMM), BM_GETCHECK, 0, 0L)*16 +
            SendMessage(GetDlgItem(hwnd, ID_EQTEXTTOTEXT), BM_GETCHECK, 0, 0L)*32 +
            SendMessage(GetDlgItem(hwnd, ID_DISPLAYED_TO_EPS), BM_GETCHECK, 0, 0L)*64 +
            SendMessage(GetDlgItem(hwnd, ID_INLINE_TO_EPS), BM_GETCHECK, 0, 0L)*128;
        if(i!=3)
        {
            wsprintf(str, L" -M%d", i);
            lstrcatS(hwnd, out_str, MAX7PATH, str);
        }
        
        i = SendMessage(GetDlgItem(hwnd, ID_TABLES_TO_RTF), BM_GETCHECK, 0, 0L)+ 
            SendMessage(GetDlgItem(hwnd, ID_TABLES_TO_BITMAP), BM_GETCHECK, 0, 0L)*2;
        if(i!=1)
        {
            wsprintf(str, L" -t%d", i);
            lstrcatS(hwnd, out_str, MAX7PATH, str);
        }
        
        if(SendMessage(GetDlgItem(hwnd, ID_PARANTHESES), BM_GETCHECK, 0, 0L))
            lstrcatS(hwnd, out_str, MAX7PATH, L" -p");
            
        if(SendMessage(GetDlgItem(hwnd, ID_SEMICOLONS), BM_GETCHECK, 0, 0L))
            lstrcatS(hwnd, out_str, MAX7PATH, L" -S");
            
        if(SendMessage(GetDlgItem(hwnd, ID_WARNINGSINRTF), BM_GETCHECK, 0, 0L))
            lstrcatS(hwnd, out_str, MAX7PATH, L" -W");
            
        if(SendMessage(GetDlgItem(hwnd, ID_ADDEXTRA), CB_GETCURSEL, 0, 0L))
        {
            wsprintf(str, L" -Z%d", SendMessage(GetDlgItem(hwnd, ID_ADDEXTRA), CB_GETCURSEL, 0, 0L));
            lstrcatS(hwnd, out_str, MAX7PATH, str);
        }
        
        if(!SendMessage(GetDlgItem(hwnd, ID_TMPDEF), BM_GETCHECK, 0, 0L))
        {
            lstrcatS(hwnd, out_str, MAX7PATH, L" -T ");
            SendMessage(GetDlgItem(hwnd, ID_TMPDIRNAME), WM_GETTEXT, MAX_PATH, (LPARAM)str);
            lstrcatS(hwnd, out_str, MAX7PATH, str);
        }
            
        if(!SendMessage(GetDlgItem(hwnd, ID_AUXDEF), BM_GETCHECK, 0, 0L))
        {
            lstrcatS(hwnd, out_str, MAX7PATH, L" -a ");
            SendMessage(GetDlgItem(hwnd, ID_AUXFILENAME), WM_GETTEXT, MAX_PATH, (LPARAM)str);
            lstrcatS(hwnd, out_str, MAX7PATH, str);
        }
            
        if(!SendMessage(GetDlgItem(hwnd, ID_BBLDEF), BM_GETCHECK, 0, 0L))
        {
            lstrcatS(hwnd, out_str, MAX7PATH, L" -b ");
            SendMessage(GetDlgItem(hwnd, ID_BBLFILENAME), WM_GETTEXT, MAX_PATH, (LPARAM)str);
            lstrcatS(hwnd, out_str, MAX7PATH, str);
        }
        
        if(!SendMessage(GetDlgItem(hwnd, ID_RTFDEF), BM_GETCHECK, 0, 0L))
        {
            lstrcatS(hwnd, out_str, MAX7PATH, L" -o ");
            SendMessage(GetDlgItem(hwnd, ID_RTFFILENAME), WM_GETTEXT, MAX_PATH, (LPARAM)str);
            lstrcatS(hwnd, out_str, MAX7PATH, str);
        }
        
        lstrcatS(hwnd, out_str, MAX7PATH, L" ");
        SendMessage(GetDlgItem(hwnd, ID_TEXFILENAME), WM_GETTEXT, MAX_PATH, (LPARAM)str);
        getfilename(str);
        lstrcatS(hwnd, out_str, MAX7PATH, str);
        
        if(SendMessage(GetDlgItem(hwnd, ID_DEBUGTOFILE), BM_GETCHECK, 0, 0L))
        {
            wsprintf(str, L" 2>");
            lstrcatS(hwnd, out_str, MAX7PATH, str);
            SendMessage(GetDlgItem(hwnd, ID_DEBUGFILENAME), WM_GETTEXT, MAX_PATH, (LPARAM)str);
            lstrcatS(hwnd, out_str, MAX7PATH, str);
            lstrcpy(strfl,L"The debugging output will be written to file ");
            lstrcatS(hwnd, strfl, MAX7PATH, str);
           // MessageBox(hwnd, strfl, L"LaTeX2RTF", MB_OK);
        }
        
        lstrcatS(hwnd, out_str, MAX7PATH, L" & popd & pause");

        GetWindowsDirectory(str, MAX_PATH); // Default directory for CMD.exe (to avoid error message)

        //  if(MessageBox(hwnd, out_str, L"l2rshell: command line", MB_OKCANCEL) == IDOK) // for Beta version only
            ShellExecute(hwnd, NULL, L"cmd", out_str, str, SW_SHOWNORMAL);     
        
    }
    
    else
    {
        MessageBox(hwnd, GetLngString(hwnd, L"error_01", L"Specify a correct path to '.tex' file!", str), L"LaTeX2RTF", MB_OK|MB_ICONERROR);
        SetTab(hwnd, 0);
    }
}

bool OpenFile(HWND hwnd, WCHAR* str)
{
    WCHAR path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);
    getdir(path);
    lstrcat(path, str);
    if(ShellExecute(hwnd, L"open", path, NULL, NULL, SW_SHOWNORMAL) <= (HINSTANCE)32)
        return 0;
    return 1;
}

void SetTab(HWND hwnd, int tabctrl_item)
{
    int show_hide;
    TabCtrl_SetCurSel(GetDlgItem(hwnd, ID_TAB), tabctrl_item);
    // "General" tab
    if(tabctrl_item==0) show_hide=SW_SHOW; else show_hide=SW_HIDE;
    ShowWindow(GetDlgItem(hwnd, ID_TEXFILE_TEXT), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_TEXFILENAME), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_TEXFILE_BUTTON), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_RTFFILE_TEXT), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_RTFDEF), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_RTFFILENAME), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_RTFFILE_BUTTON), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_EQUATIONS_GROUP), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_EQUATIONS_TXT1), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_EQUATIONS_TXT2), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_EQUATIONS_TXT3), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_DISPLAYED_TO_RTF), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_DISPLAYED_TO_BITMAP), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_DISPLAYED_TO_EPS), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_INLINE_TO_RTF), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_INLINE_TO_BITMAP), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_INLINE_TO_EPS), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_TABLES_TXT), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_TABLES_TO_RTF), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_TABLES_TO_BITMAP), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_EQTEXTTOTEXT), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_EQTEXTTOCOMM), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_BITMAPS_GROUP), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_BMPFORFIG), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_FIGSASFILENAMES), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_BITMAPS_TXT1), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_BITMAPS_TXT2), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_BITMAPS_TXT3), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_BITMAPS_TXT4), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_BITMAPS_TXT5), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_BMPRES), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_EQSCALE), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_FIGSCALE), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_BMPRES_UPDOWN), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_EQSCALE_UPDOWN), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_FIGSCALE_UPDOWN), show_hide);
    // "Advanced" tab
    if(tabctrl_item==1) show_hide=SW_SHOW; else show_hide=SW_HIDE;
    ShowWindow(GetDlgItem(hwnd, ID_AUXFILE_TEXT), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_AUXDEF), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_AUXFILENAME), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_AUXFILE_BUTTON), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_BBLFILE_TEXT), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_BBLDEF), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_BBLFILENAME), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_BBLFILE_BUTTON), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_TMPDIR_TEXT), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_TMPDEF), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_TMPDIRNAME), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_TMPDIR_BUTTON), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_RTF_GROUP), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_PARANTHESES), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_SEMICOLONS), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_USEEQFIELDS), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_USEREFFIELDS), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_RTF_TXT1), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_RTF_TXT2), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_RTF_TXT3), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_ADDEXTRA), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_LANG_GROUP), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_LANG), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_CODEPAGE), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_LANG_TXT1), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_LANG_TXT2), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_DEBUG_GROUP), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_DEBUG_TXT), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_DEBUGLEVEL), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_DEBUGTOFILE), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_DEBUGFILENAME), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_WARNINGSINRTF), show_hide);
    // "Environment" tab
    if(tabctrl_item==2) show_hide=SW_SHOW; else show_hide=SW_HIDE;
    ShowWindow(GetDlgItem(hwnd, ID_CHANGELANG_GROUP), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_SHELLLANG), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_PATHS_GROUP), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_CONFIG_TEXT), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_CONFIG), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_CONFIG_BUTTON), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_CFGDEF), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_LATEX_TEXT), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_LATEX), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_LATEX_BUTTON), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_LATEXDEF), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_IMAGICK_TEXT), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_IMAGICK), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_IMAGICK_BUTTON), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_IMAGICKDEF), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_GS_TEXT), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_GS), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_GS_BUTTON), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_GSDEF), show_hide);
    // "About" tab
    if(tabctrl_item==3) show_hide=SW_SHOW; else show_hide=SW_HIDE;
    ShowWindow(GetDlgItem(hwnd, ID_ABOUT_GROUP), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_ABOUT_TEXT01), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_ABOUT_TEXT02), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_ABOUT_TEXT03), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_ABOUT_TEXT04), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_ABOUT_TEXT05), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_ABOUT_TEXT06), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_ABOUT_TEXT07), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_ABOUT_TEXT08), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_ABOUT_TEXT09), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_ABOUT_TEXT10), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_ABOUT_TEXT11), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_HOMEPAGE_BUTTON), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_CANHELP_BUTTON ), show_hide);
    ShowWindow(GetDlgItem(hwnd, ID_GPL_BUTTON), show_hide);
}

void setstr(WCHAR* path, WCHAR* newext)
{
    lstrcat(path, newext);
}

void changeext(WCHAR* path, WCHAR* newext)
{
    int i;
    bool flag=0;
    for(i=lstrlen(path)-1; i>=0; i--)
        {if(path[i]=='.')
            {path[i]=0;
            lstrcat(path, newext);
            flag=1;
            break;}}
    if(flag==0) lstrcat(path, newext);
}

void getdir(WCHAR* path)
{
    bool flag=0;
    for(int i=lstrlen(path)-1; i>=0; i--)
        {if(path[i]=='\\')
            {path[i]=0;
            flag=1;
            break;}}
    if(flag==0) path[0]=0;
}

void getfilename(WCHAR* path)
{
    int i;
    WCHAR str[MAX_PATH];
    for(i=lstrlen(path)-1; i>=0; i--)
        if(path[i]=='\\')
            break;
    for(int j=0; j<=lstrlen(path)-i-1; j++)
        str[j]=path[i+j+1];
    lstrcpy(path, str);
}

void ChangeTheLanguage(HWND hwnd)
{
    WCHAR str[MAX_PATH];
    int i;
    TC_ITEM item;

    SendMessage(GetDlgItem(hwnd, ID_RUN), WM_SETTEXT, 0, (LPARAM)GetLngString(hwnd, L"0", L"Run!", str));
    SendMessage(GetDlgItem(hwnd, ID_HELP), WM_SETTEXT, 0, (LPARAM)GetLngString(hwnd, L"1", L"Help", str));
    SendMessage(GetDlgItem(hwnd, ID_EXIT), WM_SETTEXT, 0, (LPARAM)GetLngString(hwnd, L"2", L"Exit", str));
    
    item.mask = TCIF_TEXT;
    item.pszText = GetLngString(hwnd, L"11", L"General", str);
    TabCtrl_SetItem(GetDlgItem(hwnd, ID_TAB), 0, &item);
    item.pszText = GetLngString(hwnd, L"12", L"Advanced", str);
    TabCtrl_SetItem(GetDlgItem(hwnd, ID_TAB), 1, &item);
    item.pszText = GetLngString(hwnd, L"13", L"Environment", str);
    TabCtrl_SetItem(GetDlgItem(hwnd, ID_TAB), 2, &item);
    item.pszText = GetLngString(hwnd, L"14", L"About", str);
    TabCtrl_SetItem(GetDlgItem(hwnd, ID_TAB), 3, &item);

    SendMessage(GetDlgItem(hwnd, ID_TEXFILE_TEXT), WM_SETTEXT, 0, (LPARAM)GetLngString(hwnd, L"101", L"Input file (LaTeX):", str));
    SendMessage(GetDlgItem(hwnd, ID_RTFFILE_TEXT), WM_SETTEXT, 0, (LPARAM)GetLngString(hwnd, L"102", L"RTF file:", str));
    GetLngString(hwnd, L"103", L"use default", str);
    SendMessage(GetDlgItem(hwnd, ID_RTFDEF), WM_SETTEXT, 0, (LPARAM)str);
    SendMessage(GetDlgItem(hwnd, ID_BBLDEF), WM_SETTEXT, 0, (LPARAM)str);
    SendMessage(GetDlgItem(hwnd, ID_AUXDEF), WM_SETTEXT, 0, (LPARAM)str);
    SendMessage(GetDlgItem(hwnd, ID_TMPDEF), WM_SETTEXT, 0, (LPARAM)str);
    SendMessage(GetDlgItem(hwnd, ID_CFGDEF), WM_SETTEXT, 0, (LPARAM)str);
    SendMessage(GetDlgItem(hwnd, ID_LATEXDEF), WM_SETTEXT, 0, (LPARAM)str);
    SendMessage(GetDlgItem(hwnd, ID_IMAGICKDEF), WM_SETTEXT, 0, (LPARAM)str);
    SendMessage(GetDlgItem(hwnd, ID_GSDEF), WM_SETTEXT, 0, (LPARAM)str);
    SendMessage(GetDlgItem(hwnd, ID_EQUATIONS_GROUP), WM_SETTEXT, 0, (LPARAM)GetLngString(hwnd, L"104", L"Equations and tables", str));
    SendMessage(GetDlgItem(hwnd, ID_EQUATIONS_TXT1), WM_SETTEXT, 0, (LPARAM)GetLngString(hwnd, L"105", L"Convert displayed equations to:", str));
    SendMessage(GetDlgItem(hwnd, ID_EQUATIONS_TXT2), WM_SETTEXT, 0, (LPARAM)GetLngString(hwnd, L"106", L"Convert inline equations to:", str));
    SendMessage(GetDlgItem(hwnd, ID_TABLES_TXT), WM_SETTEXT, 0, (LPARAM)GetLngString(hwnd, L"1065", L"Convert tables to:", str));
    SendMessage(GetDlgItem(hwnd, ID_EQUATIONS_TXT3), WM_SETTEXT, 0, (LPARAM)GetLngString(hwnd, L"1067", L"Insert original equation text as:", str));
    SendMessage(GetDlgItem(hwnd, ID_EQTEXTTOTEXT), WM_SETTEXT, 0, (LPARAM)GetLngString(hwnd, L"107", L"plain text", str));
    SendMessage(GetDlgItem(hwnd, ID_EQTEXTTOCOMM), WM_SETTEXT, 0, (LPARAM)GetLngString(hwnd, L"108", L"Word comment", str));
    SendMessage(GetDlgItem(hwnd, ID_BITMAPS_GROUP), WM_SETTEXT, 0, (LPARAM)GetLngString(hwnd, L"109", L"Bitmaps", str));
    SendMessage(GetDlgItem(hwnd, ID_BMPFORFIG), WM_SETTEXT, 0, (LPARAM)GetLngString(hwnd, L"110", L"Convert all figures to bitmaps", str));
    SendMessage(GetDlgItem(hwnd, ID_BITMAPS_TXT1), WM_SETTEXT, 0, (LPARAM)GetLngString(hwnd, L"111", L"Resolution:", str));
    SendMessage(GetDlgItem(hwnd, ID_BITMAPS_TXT2), WM_SETTEXT, 0, (LPARAM)GetLngString(hwnd, L"112", L"DPI", str));
    SendMessage(GetDlgItem(hwnd, ID_BITMAPS_TXT3), WM_SETTEXT, 0, (LPARAM)GetLngString(hwnd, L"113", L"Scale:", str));
    SendMessage(GetDlgItem(hwnd, ID_BITMAPS_TXT4), WM_SETTEXT, 0, (LPARAM)GetLngString(hwnd, L"114", L"(equations)", str));
    SendMessage(GetDlgItem(hwnd, ID_BITMAPS_TXT5), WM_SETTEXT, 0, (LPARAM)GetLngString(hwnd, L"115", L"(bitmaps)", str));
    SendMessage(GetDlgItem(hwnd, ID_FIGSASFILENAMES), WM_SETTEXT, 0, (LPARAM)GetLngString(hwnd, L"116", L"Insert all figures as filenames", str));
    
    SendMessage(GetDlgItem(hwnd, ID_TMPDIR_TEXT), WM_SETTEXT, 0, (LPARAM)GetLngString(hwnd, L"200", L"TMP dir:", str));
    SendMessage(GetDlgItem(hwnd, ID_BBLFILE_TEXT), WM_SETTEXT, 0, (LPARAM)GetLngString(hwnd, L"201", L"BBL file:", str));
    SendMessage(GetDlgItem(hwnd, ID_AUXFILE_TEXT), WM_SETTEXT, 0, (LPARAM)GetLngString(hwnd, L"202", L"AUX file:", str));
    SendMessage(GetDlgItem(hwnd, ID_RTF_GROUP), WM_SETTEXT, 0, (LPARAM)(LPARAM)GetLngString(hwnd, L"203", L"RTF", str));
    SendMessage(GetDlgItem(hwnd, ID_PARANTHESES), WM_SETTEXT, 0, (LPARAM)(LPARAM)GetLngString(hwnd, L"204", L"Use ' \\( ' and ' \\) ' instead of ' ( ' and ' ) ' in EQ fields", str));
    SendMessage(GetDlgItem(hwnd, ID_SEMICOLONS), WM_SETTEXT, 0, (LPARAM)(LPARAM)GetLngString(hwnd, L"205", L"Use ' ; ' instead of ' , ' to separate arguments in RTF fields", str));
    SendMessage(GetDlgItem(hwnd, ID_USEEQFIELDS), WM_SETTEXT, 0, (LPARAM)(LPARAM)GetLngString(hwnd, L"221", L"equations", str));
    SendMessage(GetDlgItem(hwnd, ID_USEREFFIELDS), WM_SETTEXT, 0, (LPARAM)(LPARAM)GetLngString(hwnd, L"222", L"\\ref and \\cite", str));
    SendMessage(GetDlgItem(hwnd, ID_RTF_TXT1), WM_SETTEXT, 0, (LPARAM)(LPARAM)GetLngString(hwnd, L"206", L"Add", str));
    SendMessage(GetDlgItem(hwnd, ID_RTF_TXT2), WM_SETTEXT, 0, (LPARAM)(LPARAM)GetLngString(hwnd, L"207", L"extra '}' to the end of file", str));
    SendMessage(GetDlgItem(hwnd, ID_RTF_TXT3), WM_SETTEXT, 0, (LPARAM)(LPARAM)GetLngString(hwnd, L"220", L"Use fields for:", str));
    SendMessage(GetDlgItem(hwnd, ID_LANG_GROUP), WM_SETTEXT, 0, (LPARAM)(LPARAM)GetLngString(hwnd, L"208", L"Language", str));
    SendMessage(GetDlgItem(hwnd, ID_LANG_TXT1), WM_SETTEXT, 0, (LPARAM)(LPARAM)GetLngString(hwnd, L"209", L"Language:", str));
    SendMessage(GetDlgItem(hwnd, ID_LANG_TXT2), WM_SETTEXT, 0, (LPARAM)(LPARAM)GetLngString(hwnd, L"210", L"Codepage:", str));
    SendMessage(GetDlgItem(hwnd, ID_DEBUG_GROUP), WM_SETTEXT, 0, (LPARAM)(LPARAM)GetLngString(hwnd, L"211", L"Debugging", str));
    SendMessage(GetDlgItem(hwnd, ID_DEBUG_TXT), WM_SETTEXT, 0, (LPARAM)(LPARAM)GetLngString(hwnd, L"212", L"Debugging level:", str));
    SendMessage(GetDlgItem(hwnd, ID_DEBUGTOFILE), WM_SETTEXT, 0, (LPARAM)(LPARAM)GetLngString(hwnd, L"214", L"Write debug output to file", str));
    SendMessage(GetDlgItem(hwnd, ID_WARNINGSINRTF), WM_SETTEXT, 0, (LPARAM)(LPARAM)GetLngString(hwnd, L"213", L"Include warnings in the RTF", str));
    
    i =SendMessage(GetDlgItem(hwnd, ID_LANG), CB_GETCURSEL, 0, 0);
    SendMessage(GetDlgItem(hwnd, ID_LANG), CB_INSERTSTRING, 0, (LPARAM)GetLngString(hwnd, L"250", L"-not specify-", str));
    SendMessage(GetDlgItem(hwnd, ID_LANG), CB_DELETESTRING, 1, 0);
    SendMessage(GetDlgItem(hwnd, ID_LANG), CB_SETCURSEL, i, 0);
    
    i = SendMessage(GetDlgItem(hwnd, ID_CODEPAGE), CB_GETCURSEL, 0, 0);
    SendMessage(GetDlgItem(hwnd, ID_CODEPAGE), CB_INSERTSTRING, 0, (LPARAM)GetLngString(hwnd, L"250", L"-not specify-", str));
    SendMessage(GetDlgItem(hwnd, ID_CODEPAGE), CB_DELETESTRING, 1, 0);
    SendMessage(GetDlgItem(hwnd, ID_CODEPAGE), CB_SETCURSEL, i, 0);

    SendMessage(GetDlgItem(hwnd, ID_CHANGELANG_GROUP), WM_SETTEXT, 0, (LPARAM)GetLngString(hwnd, L"301", L"Change the language", str));
    SendMessage(GetDlgItem(hwnd, ID_PATHS_GROUP), WM_SETTEXT, 0, (LPARAM)GetLngString(hwnd, L"13", L"Environment", str));
    SendMessage(GetDlgItem(hwnd, ID_CONFIG_TEXT), WM_SETTEXT, 0, (LPARAM)GetLngString(hwnd, L"303", L"Configuration directory", str));
    
    if(SendMessage(GetDlgItem(hwnd, ID_LATEXDEF), BM_GETCHECK, 0, 0L) && !FindLatex(str))
        SetDlgItemText(hwnd, ID_LATEX, GetLngString(hwnd, L"310", L"- not installed (?) -", str));
        
    if(SendMessage(GetDlgItem(hwnd, ID_IMAGICKDEF), BM_GETCHECK, 0, 0L) && !FindImageMagick(str))
        SetDlgItemText(hwnd, ID_IMAGICK, GetLngString(hwnd, L"310", L"- not installed (?) -", str));
    
    if(SendMessage(GetDlgItem(hwnd, ID_GSDEF), BM_GETCHECK, 0, 0L) && !FindGhostscript(str))
        SetDlgItemText(hwnd, ID_GS, GetLngString(hwnd, L"310", L"- not installed (?) -", str));

    SendMessage(GetDlgItem(hwnd, ID_ABOUT_TEXT02), WM_SETTEXT, 0, (LPARAM)GetLngString(hwnd, L"401", L"Authors:", str));
    SendMessage(GetDlgItem(hwnd, ID_HOMEPAGE_BUTTON), WM_SETTEXT, 0, (LPARAM)GetLngString(hwnd, L"402", L"Homepage", str));
    SendMessage(GetDlgItem(hwnd, ID_CANHELP_BUTTON), WM_SETTEXT, 0, (LPARAM)GetLngString(hwnd, L"403", L"You can help...", str));
    SendMessage(GetDlgItem(hwnd, ID_GPL_BUTTON), WM_SETTEXT, 0, (LPARAM)GetLngString(hwnd, L"404", L"Distributed under the GNU General Public License (GPL)", str));

    RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
}

void ResetLangCombo(HWND hwnd)
{
    WCHAR str[MAX_PATH];
    WCHAR curcombotext[50];
    HANDLE hdl;
    WIN32_FIND_DATA findfiledata;
    
    SendMessage(GetDlgItem(hwnd, ID_LANG), WM_GETTEXT, 50, (LPARAM)curcombotext);
    SendMessage(GetDlgItem(hwnd, ID_LANG), CB_RESETCONTENT, 0, 0);
    
    SendMessage(GetDlgItem(hwnd, ID_LANG), CB_ADDSTRING, 0, (LPARAM)GetLngString(hwnd, L"250", L"-not specify-", str));
    
    SendMessage(GetDlgItem(hwnd, ID_CONFIG), WM_GETTEXT, MAX_PATH, (LPARAM)str);
    lstrcat(str, L"\\*.cfg");
    if((hdl=FindFirstFile(str, &findfiledata))!=INVALID_HANDLE_VALUE)
    {
        do
        {
            lstrcpy(str, findfiledata.cFileName);
            changeext(str, L"");
            if(lstrcmp(str, L"fonts")&& lstrcmp(str, L"fonts_pc") && lstrcmp(str, L"direct") && lstrcmp(str, L"ignore") && lstrcmp(str, L"style"))
                SendMessage(GetDlgItem(hwnd, ID_LANG), CB_ADDSTRING, 0, (LPARAM)str);
        }
        while(FindNextFile(hdl, &findfiledata));
    }
    FindClose(hdl);
    
    if(SendMessage(GetDlgItem(hwnd, ID_LANG), CB_SELECTSTRING, (WPARAM)-1, (LPARAM)curcombotext)==CB_ERR)
        SendMessage(GetDlgItem(hwnd, ID_LANG), CB_SETCURSEL, 0, 0);
}

void ResetCodepageCombo(HWND hwnd)
{
    WCHAR str[MAX_PATH];
    WCHAR path_to_encodings[MAX_PATH];
    WCHAR number[5];
    WCHAR curcombotext[50];
    
    GetModuleFileName(NULL, path_to_encodings, MAX_PATH);
    getdir(path_to_encodings);
    lstrcat(path_to_encodings, L"\\l2rshell_files\\encodings.ini");
    
    SendMessage(GetDlgItem(hwnd, ID_CODEPAGE), WM_GETTEXT, 50, (LPARAM)curcombotext);
    SendMessage(GetDlgItem(hwnd, ID_CODEPAGE), CB_RESETCONTENT, 0, 0);
    
    SendMessage(GetDlgItem(hwnd, ID_CODEPAGE), CB_ADDSTRING, 0, (LPARAM)GetLngString(hwnd, L"250", L"-not specify-", str));
    
    for(int i=0; i<1000; i++)
    {
        wsprintf(number, L"%d", i);
        if(!GetPrivateProfileString(L"encodings", number, NULL, str, 100, path_to_encodings))
            break;
        SendMessage(GetDlgItem(hwnd, ID_CODEPAGE), CB_ADDSTRING, 0, (LPARAM)str);
    }
    if(SendMessage(GetDlgItem(hwnd, ID_CODEPAGE), CB_SELECTSTRING, (WPARAM)-1, (LPARAM)curcombotext)==CB_ERR)
        SendMessage(GetDlgItem(hwnd, ID_CODEPAGE), CB_SETCURSEL, 0, 0);
}

void ResetShellLangCombo(HWND hwnd)
{
    WCHAR curfile[MAX_PATH];
    WCHAR str[MAX_PATH];
    WCHAR curcombotext[50];
    HANDLE hdl;
    WIN32_FIND_DATA findfiledata;
    bool isEnglishExist;
    
    SendMessage(GetDlgItem(hwnd, ID_SHELLLANG), WM_GETTEXT, 50, (LPARAM)curcombotext);
    SendMessage(GetDlgItem(hwnd, ID_SHELLLANG), CB_RESETCONTENT, 0, 0);
    isEnglishExist=0;
    
    GetModuleFileName(NULL, str, MAX_PATH);
    getdir(str);
    lstrcat(str, L"\\l2rshell_files\\lang\\*.lng");
    
    if((hdl=FindFirstFile(str, &findfiledata))!=INVALID_HANDLE_VALUE)
    {
        do
        {
            GetModuleFileName(NULL, curfile, MAX_PATH);
            getdir(curfile);
            lstrcat(curfile, L"\\l2rshell_files\\lang\\");
            lstrcat(curfile, findfiledata.cFileName);
            GetPrivateProfileString(L"language", L"language", findfiledata.cFileName, str, 50, curfile);
            SendMessage(GetDlgItem(hwnd, ID_SHELLLANG), CB_ADDSTRING, 0, (LPARAM)str);
            if(!lstrcmp(L"English", str))
                isEnglishExist=1;
        }
        while(FindNextFile(hdl, &findfiledata));
    }
    FindClose(hdl);
    
    if(!isEnglishExist)
        SendMessage(GetDlgItem(hwnd, ID_SHELLLANG), CB_ADDSTRING, 0, (LPARAM)L"English");
    
    if(SendMessage(GetDlgItem(hwnd, ID_SHELLLANG), CB_SELECTSTRING, (WPARAM)-1, (LPARAM)curcombotext)==CB_ERR)
        SendMessage(GetDlgItem(hwnd, ID_SHELLLANG), CB_SELECTSTRING, (WPARAM)-1, (LPARAM)L"English");
}

bool FindLatex(WCHAR* str)
{
    HKEY hkey1, hkey2;
    DWORD dwrd=MAX_PATH;
    DWORD dwType=MAX_PATH;
    bool isinstalled = 0;
    int i;
    WCHAR str1[MAX_PATH], str2[MAX_PATH];
    
    // If path specified in the PATH environment variable
    if(FindExecutable(L"latex", NULL, str)>(HINSTANCE)32)
    {
        isinstalled = 1;
        return isinstalled;
    }
    // Check for the path in the registry (MikTeX v<=2.4)
    RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\MiK\\MiKTeX\\CurrentVersion\\MiKTeX", 0, KEY_READ, &hkey1); 
    if(RegQueryValueEx(hkey1, L"Install Root", NULL, &dwType, (LPBYTE)str, &dwrd)==ERROR_SUCCESS)
    {
        lstrcat(str, L"\\miktex\\bin\\latex.exe");
        isinstalled = 1;
    }
    RegCloseKey(hkey1);
    if(isinstalled) return isinstalled;
    
    // Check for the path in the registry (MikTeX v>=2.5, global install)
    RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\MiKTeX.org\\MiKTeX", 0, KEY_READ, &hkey1);
    i=0;
    while(RegEnumKey(hkey1, i, str1, dwrd) == ERROR_SUCCESS)
    {
        lstrcpy(str2, L"Software\\MiKTeX.org\\MiKTeX\\");
        lstrcat(str2, str1);
        lstrcat(str2, L"\\Core");
        RegOpenKeyEx(HKEY_LOCAL_MACHINE, str2, 0, KEY_READ, &hkey2);
        if(RegQueryValueEx(hkey2, L"Install", NULL, &dwType, (LPBYTE)str2, &dwrd)==ERROR_SUCCESS)
        {
            isinstalled = 1;
            lstrcpy(str, str2);
            lstrcat(str, L"\\miktex\\bin\\latex.exe");
        }
        RegCloseKey(hkey2);
        i++;
    }
    RegCloseKey(hkey1);
    if(isinstalled) return isinstalled;

    // Check for the path in the registry (MikTeX v>=2.5, user install)
    RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\MiKTeX.org\\MiKTeX", 0, KEY_READ, &hkey1);
    i=0;
    while(RegEnumKey(hkey1, i, str1, dwrd) == ERROR_SUCCESS)
    {
        lstrcpy(str2, L"Software\\MiKTeX.org\\MiKTeX\\");
        lstrcat(str2, str1);
        lstrcat(str2, L"\\Core");
        RegOpenKeyEx(HKEY_CURRENT_USER, str2, 0, KEY_READ, &hkey2);
        if(RegQueryValueEx(hkey2, L"UserInstall", NULL, &dwType, (LPBYTE)str2, &dwrd)==ERROR_SUCCESS)
        {
            isinstalled = 1;
            lstrcpy(str, str2);
            lstrcat(str, L"\\miktex\\bin\\latex.exe");
        }
        RegCloseKey(hkey2);
        i++;
    }
    RegCloseKey(hkey1);
        
    return isinstalled;
}

bool FindImageMagick(WCHAR* str)
{
    HKEY hkey;
    DWORD dwrd = MAX_PATH;
    DWORD dwType=MAX_PATH;   
    
    RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\ImageMagick\\Current", 0, KEY_READ, &hkey);
    if(RegQueryValueEx(hkey, L"BinPath", NULL, &dwType, (LPBYTE)str, &dwrd)==ERROR_SUCCESS)
    {
        lstrcat(str, L"\\convert.exe");
        RegCloseKey(hkey);
        return 1;
    }
    RegCloseKey(hkey);
                        
    return 0;
}

bool FindGhostscript(WCHAR* str)
{
    WCHAR str1[MAX_PATH], str2[MAX_PATH];
    HKEY hkey1, hkey2;
    WIN32_FIND_DATA findfiledata;
    int i;
    bool isinstalled = 0;
    DWORD  dwrd = MAX_PATH;
    DWORD  dwType = MAX_PATH;   
    
    RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\AFPL Ghostscript", 0, KEY_READ, &hkey1);
    i=0;
    while(RegEnumKey(hkey1, i, str1, dwrd) == ERROR_SUCCESS)
    {
        lstrcpy(str2, L"Software\\AFPL Ghostscript\\");
        lstrcat(str2, str1);
        RegOpenKeyEx(HKEY_LOCAL_MACHINE, str2, 0, KEY_READ, &hkey2);
        if(RegQueryValueEx(hkey2, L"GS_DLL", NULL, &dwType, (LPBYTE)str2, &dwrd)==ERROR_SUCCESS)
            if(FindFirstFile(str2, &findfiledata)!=INVALID_HANDLE_VALUE)
            {
                isinstalled = 1;
                lstrcpy(str, str2);
            }
        RegCloseKey(hkey2);  
        i++;
    }
    RegCloseKey(hkey1);
    
          
    RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\GPL Ghostscript", 0, KEY_READ, &hkey1);
    i=0;
    while(RegEnumKey(hkey1, i, str1, dwrd) == ERROR_SUCCESS)
    {
        lstrcpy(str2, L"Software\\GPL Ghostscript\\");
        lstrcat(str2, str1);
        RegOpenKeyEx(HKEY_LOCAL_MACHINE, str2, 0, KEY_READ, &hkey2);
        if(RegQueryValueEx(hkey2, L"GS_DLL", NULL, &dwType, (LPBYTE)str2, &dwrd)==ERROR_SUCCESS)
            if(FindFirstFile(str2, &findfiledata)!=INVALID_HANDLE_VALUE)
            {
                isinstalled = 1;
                lstrcpy(str, str2);
            }
        RegCloseKey(hkey2);  
        i++;
    }
    RegCloseKey(hkey1);
    
    return isinstalled;
}


bool FileDialog(HWND hwnd, WCHAR* filter, WCHAR* path, DWORD flags, WCHAR* defext, bool opensave) //Open: opensave=1. Save: opensave=0
{
    WCHAR initdir[MAX_PATH];
    
    lstrcpy(initdir, path);
    
    OPENFILENAME openfilename;
    
    ZeroMemory(&openfilename, sizeof(openfilename));

    openfilename.lStructSize = sizeof(openfilename);
    openfilename.hwndOwner = hwnd;
    openfilename.lpstrFilter = filter;
    openfilename.lpstrFile = path;
    openfilename.nMaxFile = MAX_PATH;
    openfilename.lpstrInitialDir = initdir;
    openfilename.Flags = flags;
    openfilename.lpstrDefExt = defext;
    
    if(opensave)    // Open file dialog
    {
        if(GetOpenFileName(&openfilename))
            return 1;
    }
    else            // Save file dialog
    {
        if(GetSaveFileName(&openfilename))
            return 1;
    }
    return 0;
}

bool PathDialog(HWND hwnd, WCHAR* path)
{
    BROWSEINFO bi;
    
    bi.hwndOwner = hwnd;
    bi.pidlRoot = NULL;
    bi.pszDisplayName = NULL;
    bi.lpszTitle = NULL;
    bi.ulFlags = 0;
    bi.lpfn = NULL;
    bi.lParam = 0;

    if(SHGetPathFromIDList(SHBrowseForFolder(&bi), path))
        return 1;
    else return 0;
}

WCHAR* GetInitString(WCHAR* id, WCHAR* def, WCHAR* str)   //returned value is 'str'
{
    WCHAR inifile[MAX_PATH];
    
    lstrcpy(str, def);
    
    GetEnvironmentVariable(L"APPDATA", inifile, MAX_PATH);
    lstrcat(inifile, L"\\l2rshell\\l2rshell.ini");
    GetPrivateProfileString(L"l2rshell", id, str, str, MAX_PATH, inifile);
    
    GetModuleFileName(NULL, inifile, MAX_PATH);
    getdir(inifile);
    lstrcat(inifile, L"\\l2rshell_files\\administrator.ini");
    GetPrivateProfileString(L"administrator", id, str, str, MAX_PATH, inifile); 
    
    return str;    
}

int GetInitInt(WCHAR* id, int def)
{
    WCHAR inifile[MAX_PATH];
    
    int i = def;
    
    GetEnvironmentVariable(L"APPDATA", inifile, MAX_PATH);
    lstrcat(inifile, L"\\l2rshell\\l2rshell.ini");
    i=GetPrivateProfileInt(L"l2rshell", id, i, inifile);
    
    GetModuleFileName(NULL, inifile, MAX_PATH);
    getdir(inifile);
    lstrcat(inifile, L"\\l2rshell_files\\administrator.ini");
    i=GetPrivateProfileInt(L"administrator", id, i, inifile);
    
    return i;
}

WCHAR* GetVersionString(WCHAR* str)   //returned value is 'str'
{
    WCHAR inifile[MAX_PATH];
    
    lstrcpy(str, L"LaTeX2RTF  +   l2rshell v.0.6.9");
    
    GetModuleFileName(NULL, inifile, MAX_PATH);
    getdir(inifile);
    lstrcat(inifile, L"\\l2rshell_files\\version.ini");
    GetPrivateProfileString(L"version", L"version", str, str, MAX_PATH, inifile);
    
    return str;
}

void ConvertToSomething(HWND hwnd)
{
    if( !SendMessage(GetDlgItem(hwnd, ID_DISPLAYED_TO_RTF), BM_GETCHECK, 0, 0L) &&
        !SendMessage(GetDlgItem(hwnd, ID_DISPLAYED_TO_BITMAP), BM_GETCHECK, 0, 0L) &&
        !SendMessage(GetDlgItem(hwnd, ID_DISPLAYED_TO_EPS), BM_GETCHECK, 0, 0L) &&
        !SendMessage(GetDlgItem(hwnd, ID_EQTEXTTOTEXT), BM_GETCHECK, 0, 0L) &&
        !SendMessage(GetDlgItem(hwnd, ID_EQTEXTTOCOMM), BM_GETCHECK, 0, 0L) )
    SendMessage(GetDlgItem(hwnd, ID_DISPLAYED_TO_RTF), BM_SETCHECK, 1, 0L);
        
    if( !SendMessage(GetDlgItem(hwnd, ID_INLINE_TO_RTF), BM_GETCHECK, 0, 0L) &&
        !SendMessage(GetDlgItem(hwnd, ID_INLINE_TO_BITMAP), BM_GETCHECK, 0, 0L) &&
        !SendMessage(GetDlgItem(hwnd, ID_INLINE_TO_EPS), BM_GETCHECK, 0, 0L) &&
        !SendMessage(GetDlgItem(hwnd, ID_EQTEXTTOTEXT), BM_GETCHECK, 0, 0L) &&
        !SendMessage(GetDlgItem(hwnd, ID_EQTEXTTOCOMM), BM_GETCHECK, 0, 0L) )
    SendMessage(GetDlgItem(hwnd, ID_INLINE_TO_RTF), BM_SETCHECK, 1, 0L);
    
    if( !SendMessage(GetDlgItem(hwnd, ID_TABLES_TO_RTF), BM_GETCHECK, 0, 0L) &&
        !SendMessage(GetDlgItem(hwnd, ID_TABLES_TO_BITMAP), BM_GETCHECK, 0, 0L) )
    SendMessage(GetDlgItem(hwnd, ID_TABLES_TO_RTF), BM_SETCHECK, 1, 0L);
}

WCHAR* GetLngString(HWND hwnd, WCHAR* id, WCHAR* def, WCHAR* str)    //returned value is 'str'
{
    WCHAR curfile[MAX_PATH];
    WCHAR lngfile[MAX_PATH];
    WCHAR curcombotext[50];
    HANDLE hdl;
    WIN32_FIND_DATA findfiledata;
    
    SendMessage(GetDlgItem(hwnd, ID_SHELLLANG), WM_GETTEXT, 50, (LPARAM)curcombotext);
    lstrcpy(lngfile, L"");
    
    GetModuleFileName(NULL, str, MAX_PATH);
    getdir(str);
    lstrcat(str, L"\\l2rshell_files\\lang\\*.lng");
    
    if((hdl=FindFirstFile(str, &findfiledata))!=INVALID_HANDLE_VALUE)
    {
        do
        {
            GetModuleFileName(NULL, curfile, MAX_PATH);
            getdir(curfile);
            lstrcat(curfile, L"\\l2rshell_files\\lang\\");
            lstrcat(curfile, findfiledata.cFileName);
            GetPrivateProfileString(L"language", L"language", findfiledata.cFileName, str, 50, curfile);
            if(!lstrcmp(curcombotext, str))
            {
                lstrcpy(lngfile, curfile);
                break;
            }
        }
        while(FindNextFile(hdl, &findfiledata));
    }
    FindClose(hdl);
        
    GetPrivateProfileString(L"language", id, def, str, MAX_PATH, lngfile); 
    
    return str;
}

void SaveUserData(HWND hwnd)
{
    WCHAR inifile[MAX_PATH], str[MAX_PATH];
    RECT rect;
    HANDLE hFile;
    DWORD i, j;
    
    // Create 'application data\l2rshell' directory, construct path to 'l2rshell.ini' file
    GetEnvironmentVariable(L"APPDATA", inifile, MAX_PATH);
    CreateDirectory(inifile, NULL); //if 'application data\l2rshell' directory does not exist
    lstrcat(inifile, L"\\l2rshell");
    CreateDirectory(inifile, NULL); //if 'application data\l2rshell' directory does not exist
    lstrcat(inifile, L"\\l2rshell.ini");
    
    // Create an empty Unicode file 'l2rshell.ini'
    hFile = CreateFile(inifile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
    i=0xFEFF;
    WriteFile(hFile, &i, 2, &j,	NULL);
    CloseHandle(hFile);
    
    // Write data to 'l2rshell.ini' file
    // window position
    GetWindowRect(hwnd, &rect);
    wsprintf(str, L"%d", rect.left);
    WritePrivateProfileString(L"l2rshell", L"xpos", str, inifile);
    wsprintf(str, L"%d", rect.top);
    WritePrivateProfileString(L"l2rshell", L"ypos", str, inifile);
    // checkboxes
    wsprintf(str, L"%d", TabCtrl_GetCurSel(GetDlgItem(hwnd, ID_TAB)));
    WritePrivateProfileString(L"l2rshell", L"tabctrl_item", str, inifile);
    wsprintf(str, L"%d", SendMessage(GetDlgItem(hwnd, ID_RTFDEF), BM_GETCHECK, 0, 0L));
    WritePrivateProfileString(L"l2rshell", L"rtfdef", str, inifile);
    wsprintf(str, L"%d", SendMessage(GetDlgItem(hwnd, ID_BBLDEF), BM_GETCHECK, 0, 0L));
    WritePrivateProfileString(L"l2rshell", L"bbldef", str, inifile);
    wsprintf(str, L"%d", SendMessage(GetDlgItem(hwnd, ID_AUXDEF), BM_GETCHECK, 0, 0L));
    WritePrivateProfileString(L"l2rshell", L"auxdef", str, inifile);
    wsprintf(str, L"%d", SendMessage(GetDlgItem(hwnd, ID_TMPDEF), BM_GETCHECK, 0, 0L));
    WritePrivateProfileString(L"l2rshell", L"tmpdef", str, inifile);
    wsprintf(str, L"%d", SendMessage(GetDlgItem(hwnd, ID_CFGDEF), BM_GETCHECK, 0, 0L));
    WritePrivateProfileString(L"l2rshell", L"cfgdef", str, inifile);
    wsprintf(str, L"%d", SendMessage(GetDlgItem(hwnd, ID_LATEXDEF), BM_GETCHECK, 0, 0L));
    WritePrivateProfileString(L"l2rshell", L"latexdef", str, inifile);
    wsprintf(str, L"%d", SendMessage(GetDlgItem(hwnd, ID_IMAGICKDEF), BM_GETCHECK, 0, 0L));
    WritePrivateProfileString(L"l2rshell", L"imagickdef", str, inifile);
    wsprintf(str, L"%d", SendMessage(GetDlgItem(hwnd, ID_GSDEF), BM_GETCHECK, 0, 0L));
    WritePrivateProfileString(L"l2rshell", L"gsdef", str, inifile);
    wsprintf(str, L"%d", SendMessage(GetDlgItem(hwnd, ID_DISPLAYED_TO_RTF), BM_GETCHECK, 0, 0L));
    WritePrivateProfileString(L"l2rshell", L"displayed_to_rtf", str, inifile);
    wsprintf(str, L"%d", SendMessage(GetDlgItem(hwnd, ID_DISPLAYED_TO_BITMAP), BM_GETCHECK, 0, 0L));
    WritePrivateProfileString(L"l2rshell", L"displayed_to_bitmap", str, inifile);
    wsprintf(str, L"%d", SendMessage(GetDlgItem(hwnd, ID_DISPLAYED_TO_EPS), BM_GETCHECK, 0, 0L));
    WritePrivateProfileString(L"l2rshell", L"displayed_to_eps", str, inifile);
    wsprintf(str, L"%d", SendMessage(GetDlgItem(hwnd, ID_INLINE_TO_RTF), BM_GETCHECK, 0, 0L));
    WritePrivateProfileString(L"l2rshell", L"inline_to_rtf", str, inifile);
    wsprintf(str, L"%d", SendMessage(GetDlgItem(hwnd, ID_INLINE_TO_BITMAP), BM_GETCHECK, 0, 0L));
    WritePrivateProfileString(L"l2rshell", L"inline_to_bitmap", str, inifile);
    wsprintf(str, L"%d", SendMessage(GetDlgItem(hwnd, ID_INLINE_TO_EPS), BM_GETCHECK, 0, 0L));
    WritePrivateProfileString(L"l2rshell", L"inline_to_eps", str, inifile);
    wsprintf(str, L"%d", SendMessage(GetDlgItem(hwnd, ID_EQTEXTTOTEXT), BM_GETCHECK, 0, 0L));
    WritePrivateProfileString(L"l2rshell", L"eqtexttotext", str, inifile);
    wsprintf(str, L"%d", SendMessage(GetDlgItem(hwnd, ID_EQTEXTTOCOMM), BM_GETCHECK, 0, 0L));
    WritePrivateProfileString(L"l2rshell", L"eqtexttocomm", str, inifile);
    
    wsprintf(str, L"%d", SendMessage(GetDlgItem(hwnd, ID_TABLES_TO_RTF), BM_GETCHECK, 0, 0L));
    WritePrivateProfileString(L"l2rshell", L"tables_to_rtf", str, inifile);
    wsprintf(str, L"%d", SendMessage(GetDlgItem(hwnd, ID_TABLES_TO_BITMAP), BM_GETCHECK, 0, 0L));
    WritePrivateProfileString(L"l2rshell", L"tables_to_bitmap", str, inifile);
    
    wsprintf(str, L"%d", SendMessage(GetDlgItem(hwnd, ID_BMPFORFIG), BM_GETCHECK, 0, 0L));
    WritePrivateProfileString(L"l2rshell", L"bmpforfig", str, inifile);
    wsprintf(str, L"%d", SendMessage(GetDlgItem(hwnd, ID_FIGSASFILENAMES), BM_GETCHECK, 0, 0L));
    WritePrivateProfileString(L"l2rshell", L"figsasfilenames", str, inifile);
    wsprintf(str, L"%d", SendMessage(GetDlgItem(hwnd, ID_PARANTHESES), BM_GETCHECK, 0, 0L));
    WritePrivateProfileString(L"l2rshell", L"parantheses", str, inifile);
    wsprintf(str, L"%d", SendMessage(GetDlgItem(hwnd, ID_SEMICOLONS), BM_GETCHECK, 0, 0L));
    WritePrivateProfileString(L"l2rshell", L"semicolons", str, inifile);
    wsprintf(str, L"%d", SendMessage(GetDlgItem(hwnd, ID_DEBUGTOFILE), BM_GETCHECK, 0, 0L));
    WritePrivateProfileString(L"l2rshell", L"debugtofile", str, inifile);
    wsprintf(str, L"%d", SendMessage(GetDlgItem(hwnd, ID_WARNINGSINRTF), BM_GETCHECK, 0, 0L));
    WritePrivateProfileString(L"l2rshell", L"warningsinrtf", str, inifile);
    wsprintf(str, L"%d", SendMessage(GetDlgItem(hwnd, ID_USEEQFIELDS), BM_GETCHECK, 0, 0L));
    WritePrivateProfileString(L"l2rshell", L"useeqfields", str, inifile);
    wsprintf(str, L"%d", SendMessage(GetDlgItem(hwnd, ID_USEREFFIELDS), BM_GETCHECK, 0, 0L));
    WritePrivateProfileString(L"l2rshell", L"usereffields", str, inifile);
    // comboboxes
    wsprintf(str, L"%d", SendMessage(GetDlgItem(hwnd, ID_ADDEXTRA), CB_GETCURSEL, 0, 0L));
    WritePrivateProfileString(L"l2rshell", L"addextra", str, inifile);
    wsprintf(str, L"%d", SendMessage(GetDlgItem(hwnd, ID_DEBUGLEVEL), CB_GETCURSEL, 0, 0L));
    WritePrivateProfileString(L"l2rshell", L"debuglevel", str, inifile);
    // strings
    SendMessage(GetDlgItem(hwnd, ID_BMPRES), WM_GETTEXT, 4, (LPARAM)str);
    WritePrivateProfileString(L"l2rshell", L"bmpres", str, inifile);
    SendMessage(GetDlgItem(hwnd, ID_EQSCALE), WM_GETTEXT, 6, (LPARAM)str);
    WritePrivateProfileString(L"l2rshell", L"eqscale", str, inifile);
    SendMessage(GetDlgItem(hwnd, ID_FIGSCALE), WM_GETTEXT, 6, (LPARAM)str);
    WritePrivateProfileString(L"l2rshell", L"figscale", str, inifile);
    SendMessage(GetDlgItem(hwnd, ID_TEXFILENAME), WM_GETTEXT, MAX_PATH, (LPARAM)str);
    WritePrivateProfileString(L"l2rshell", L"texfilename", str, inifile);
    SendMessage(GetDlgItem(hwnd, ID_RTFFILENAME), WM_GETTEXT, MAX_PATH, (LPARAM)str);
    WritePrivateProfileString(L"l2rshell", L"rtffilename", str, inifile);
    SendMessage(GetDlgItem(hwnd, ID_BBLFILENAME), WM_GETTEXT, MAX_PATH, (LPARAM)str);
    WritePrivateProfileString(L"l2rshell", L"bblfilename", str, inifile);
    SendMessage(GetDlgItem(hwnd, ID_AUXFILENAME), WM_GETTEXT, MAX_PATH, (LPARAM)str);
    WritePrivateProfileString(L"l2rshell", L"auxfilename", str, inifile);
    SendMessage(GetDlgItem(hwnd, ID_TMPDIRNAME), WM_GETTEXT, MAX_PATH, (LPARAM)str);
    WritePrivateProfileString(L"l2rshell", L"tmpdirname", str, inifile);
    SendMessage(GetDlgItem(hwnd, ID_DEBUGFILENAME), WM_GETTEXT, MAX_PATH, (LPARAM)str);
    WritePrivateProfileString(L"l2rshell", L"debugfilename", str, inifile);
    SendMessage(GetDlgItem(hwnd, ID_LANG), WM_GETTEXT, 50, (LPARAM)str);
    WritePrivateProfileString(L"l2rshell", L"lang", str, inifile);
    SendMessage(GetDlgItem(hwnd, ID_CODEPAGE), WM_GETTEXT, 50, (LPARAM)str);
    WritePrivateProfileString(L"l2rshell", L"codepage", str, inifile);
    SendMessage(GetDlgItem(hwnd, ID_SHELLLANG), WM_GETTEXT, 50, (LPARAM)str);
    WritePrivateProfileString(L"l2rshell", L"shell_lang", str, inifile);
    SendMessage(GetDlgItem(hwnd, ID_CONFIG), WM_GETTEXT, MAX_PATH, (LPARAM)str);
    WritePrivateProfileString(L"l2rshell", L"config", str, inifile);
    SendMessage(GetDlgItem(hwnd, ID_LATEX), WM_GETTEXT, MAX_PATH, (LPARAM)str);
    WritePrivateProfileString(L"l2rshell", L"latex", str, inifile);
    SendMessage(GetDlgItem(hwnd, ID_IMAGICK), WM_GETTEXT, MAX_PATH, (LPARAM)str);
    WritePrivateProfileString(L"l2rshell", L"imagick", str, inifile);
    SendMessage(GetDlgItem(hwnd, ID_GS), WM_GETTEXT, MAX_PATH, (LPARAM)str);
    WritePrivateProfileString(L"l2rshell", L"gs", str, inifile);
}
void lstrcatS(HWND hwnd, WCHAR* out_str, int len, WCHAR* str)
{
    if ((wcslen(out_str) + wcslen(str)) < len)
      {
      lstrcat(out_str, str);
      }
    else
      {
      MessageBox(hwnd, out_str, L"l2rshell: exceeds maximum length", MB_OKCANCEL);
      }  
}
