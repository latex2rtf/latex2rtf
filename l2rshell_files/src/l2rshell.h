#define UNICODE
#include <windows.h>
#include <commctrl.h>   // tab control
#include <shlobj.h>     // SHBrowseForFolder
#include <stdio.h>
#include <math.h>

#define ID_TAB                              990
#define ID_RUN                              991
#define ID_HELP                             992
#define ID_EXIT                             993

#define ID_TEXFILE_TEXT                     1000
#define ID_TEXFILENAME                      1001
#define ID_TEXFILE_BUTTON                   1002

#define ID_RTFFILE_TEXT                     1010
#define ID_RTFDEF                           1011
#define ID_RTFFILENAME                      1012
#define ID_RTFFILE_BUTTON                   1013

#define ID_AUXFILE_TEXT                     1020
#define ID_AUXDEF                           1021
#define ID_AUXFILENAME                      1022
#define ID_AUXFILE_BUTTON                   1023

#define ID_TMPDIR_TEXT                      1025
#define ID_TMPDEF                           1026
#define ID_TMPDIRNAME                       1027
#define ID_TMPDIR_BUTTON                    1028

#define ID_BBLFILE_TEXT                     1030
#define ID_BBLDEF                           1031
#define ID_BBLFILENAME                      1032
#define ID_BBLFILE_BUTTON                   1033

#define ID_EQUATIONS_GROUP                  1040
#define ID_EQUATIONS_TXT1                   1041
#define ID_DISPLAYED_TO_RTF                 1042
#define ID_DISPLAYED_TO_BITMAP              1043
#define ID_EQUATIONS_TXT2                   1044
#define ID_INLINE_TO_RTF                    1045
#define ID_INLINE_TO_BITMAP                 1046
#define ID_TABLES_TXT                       1047
#define ID_TABLES_TO_RTF                    1048
#define ID_TABLES_TO_BITMAP                 1049
#define ID_EQUATIONS_TXT3                   1050
#define ID_EQTEXTTOTEXT                     1051
#define ID_EQTEXTTOCOMM                     1052
#define ID_DISPLAYED_TO_EPS                 1053
#define ID_INLINE_TO_EPS                    1054

#define ID_BITMAPS_GROUP                    1060
#define ID_BITMAPS_TXT1                     1061
#define ID_BITMAPS_TXT2                     1062
#define ID_BITMAPS_TXT3                     1063
#define ID_BITMAPS_TXT4                     1064
#define ID_BITMAPS_TXT5                     1065
#define ID_BMPRES                           1066
#define ID_EQSCALE                          1067
#define ID_FIGSCALE                         1068
#define ID_BMPRES_UPDOWN                    1069
#define ID_EQSCALE_UPDOWN                   1070
#define ID_FIGSCALE_UPDOWN                  1071
#define ID_BMPFORFIG                        1072
#define ID_FIGSASFILENAMES                  1073

#define ID_RTF_GROUP                        1080
#define ID_PARANTHESES                      1081
#define ID_SEMICOLONS                       1082
#define ID_USEEQFIELDS                      1083
#define ID_USEREFFIELDS                     1084
#define ID_RTF_TXT1                         1085
#define ID_RTF_TXT2                         1086
#define ID_RTF_TXT3                         1087
#define ID_ADDEXTRA                         1088

#define ID_LANG_GROUP                       1090
#define ID_LANG                             1091
#define ID_CODEPAGE                         1092
#define ID_LANG_TXT1                        1093
#define ID_LANG_TXT2                        1094

#define ID_DEBUG_GROUP                      1100
#define ID_DEBUG_TXT                        1101
#define ID_DEBUGLEVEL                       1102
#define ID_WARNINGSINRTF                    1103
#define ID_DEBUGTOFILE                      1104
#define ID_DEBUGFILENAME                    1105

#define ID_PATHS_GROUP                      1110
#define ID_CONFIG_TEXT                      1111
#define ID_CONFIG                           1112
#define ID_CONFIG_BUTTON                    1113
#define ID_CFGDEF                           1114
#define ID_LATEX_TEXT                       1115
#define ID_LATEX                            1116
#define ID_LATEX_BUTTON                     1117
#define ID_LATEXDEF                         1118
#define ID_IMAGICK_TEXT                     1119
#define ID_IMAGICK                          1120
#define ID_IMAGICK_BUTTON                   1121
#define ID_IMAGICKDEF                       1122
#define ID_GS_TEXT                          1123
#define ID_GS                               1124
#define ID_GS_BUTTON                        1125
#define ID_GSDEF                            1126

#define ID_ABOUT_GROUP                      1140
#define ID_ABOUT_TEXT01                     1141
#define ID_ABOUT_TEXT02                     1142
#define ID_ABOUT_TEXT03                     1143
#define ID_ABOUT_TEXT04                     1144
#define ID_ABOUT_TEXT05                     1145
#define ID_ABOUT_TEXT06                     1146
#define ID_ABOUT_TEXT07                     1147
#define ID_ABOUT_TEXT08                     1148
#define ID_ABOUT_TEXT09                     1149
#define ID_ABOUT_TEXT10                     1150
#define ID_ABOUT_TEXT11                     1151
#define ID_HOMEPAGE_BUTTON                  1152
#define ID_CANHELP_BUTTON                   1153
#define ID_GPL_BUTTON                       1154

#define ID_CHANGELANG_GROUP                 1160
#define ID_SHELLLANG                        1161

