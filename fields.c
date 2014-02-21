/* fields.c - handle MS Word fields

Copyright (C) 2008 The Free Software Foundation

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

This file is available from http://sourceforge.net/projects/latex2rtf/
 
Authors:
    2008 Scott Prahl
*/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "main.h"
#include "fields.h"

#define FIELD_MAX_DEPTH 20

int g_equation_field_EQ_in_effect = 0;

int g_fields_use_EQ       = 1;
int g_fields_use_REF      = 1;
int g_fields_use_SYMBOL   = 1;
int g_fields_use_PAGE     = 1;
int g_fields_use_PAGE_REF = 1;
int g_fields_use_COMMENT  = 1;

int g_fields_allowed      = 1;

char g_field_separator  = ',';

int  g_field_depth = -1;
int  g_field[FIELD_MAX_DEPTH];

/* OK there are a bunch of possible states

g_fields_allowed   :   should fields be used at all
g_fields_use_EQ    :   is the EQ type of field allowed
g_fields_use_REF   :   is the REF type of field allowed
g_fields_use_SYMBOL:   is the SYMBOL type of field allowed

g_field_depth      :   0 if not processing field
g_field_type       :   


EQ fields should never be nested
*/

int EQ_field_active(void)
{
    int i;
    for (i=0; i<=g_field_depth; i++) {
        if (g_field[i] == FIELD_EQ) 
            return 1;
    }
    return 0;
}
void startField(int type)
{        
    if (!g_fields_allowed) return;
    
    switch (type) {
        case FIELD_EQ:
            diagnostics(4,"starting EQ field");
            diagnostics(6, "EQ fields allowed = %d",g_fields_use_EQ);
            if (!g_fields_use_EQ) return;
            if (EQ_field_active())
                diagnostics(1,"nested EQ fields ???");

            fprintRTF("{\\field{\\*\\fldinst{ EQ ");
            break;
            
        case FIELD_REF:
            diagnostics(4,"starting REF field");
            if (!g_fields_use_REF) return;
            break;
            
        case FIELD_SYMBOL:
            diagnostics(4,"starting SYMBOL field");
            if (!g_fields_use_SYMBOL) return;
            break;
            
        case FIELD_PAGE:
            diagnostics(4,"starting PAGE field");
            if (!g_fields_use_PAGE) return;
            break;
            
        case FIELD_PAGE_REF:
            diagnostics(4,"starting PAGE_REF field");
            if (!g_fields_use_PAGE_REF) return;
            break;
            
        case FIELD_COMMENT:
            diagnostics(4,"starting COMMENT field");
            if (!g_fields_use_COMMENT) return;
            fprintRTF("{\\field{\\*\\fldinst{ COMMENTS \" ");
            break;
    }
    
    g_field_depth++;
    g_field[g_field_depth] = type;
    if (g_field_depth >= FIELD_MAX_DEPTH)
        diagnostics(0, "Nesting of fields is excessive!");
        
}

void endCurrentField(void)
{
    diagnostics(4, "end Field");
    if (!g_fields_allowed) return;
    if (g_field_depth < 0) {
        diagnostics(1, "oops, looks like fields are too shallow!");
        return;
    }
    
    switch (g_field[g_field_depth]) {
        case FIELD_EQ:
            if (!g_fields_use_EQ) return;
            fprintRTF("}}{\\fldrslt }}\n");
            break;
            
        case FIELD_REF:
            if (!g_fields_use_REF) return;
            break;
            
        case FIELD_SYMBOL:
            if (!g_fields_use_SYMBOL) return;
            break;
            
        case FIELD_PAGE:
            if (!g_fields_use_PAGE) return;
            break;
            
        case FIELD_PAGE_REF:
            if (!g_fields_use_PAGE_REF) return;
            break;
            
        case FIELD_COMMENT:
            if (!g_fields_use_COMMENT) return;
            fprintRTF("\" }}{\\fldrslt }}\n");
            break;
    }
    
    g_field_depth--;
}

void endAllFields(void)
{
    if (!g_fields_allowed) return;
    while (g_field_depth >= 0) 
        endCurrentField();
}

void fprintfRTF_field_separator(void)
{
    fprintRTF("%c", g_field_separator);
}

void set_field_separator(char c)
{
    g_field_separator = c;
}

void set_fields_use_REF(int i)
{
    g_fields_use_REF = i;
}

int fields_use_REF(void)
{
    return g_fields_use_REF;
}

void set_fields_use_EQ(int i)
{
    g_fields_use_EQ = i;
}

int fields_use_EQ(void)
{
    return g_fields_use_EQ;
}

int processing_fields(void)
{
    if (g_field_depth >= 0)
        return 1;
    else
        return 0;
}

