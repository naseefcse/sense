/*
** File: CIScanner.l
**
** Copyright 2006 Mark Lisee and Rensselaer Polytechnic Institute. All
** worldwide rights reserved.  A license to use, copy, modify and distribute
** this software for non-commercial research purposes only is hereby granted,
** provided that this copyright notice and accompanying disclaimer is not
** modified or removed from the software.
**
** DISCLAIMER: The software is distributed "AS IS" without any express or
** implied warranty, including but not limited to, any implied warranties of
** merchantability or fitness for a particular purpose or any warranty of
** non-infringement of any current or pending patent rights. The authors of the
** software make no representations about the suitability of this software for
** any particular purpose. The entire risk as to the quality and performance of
** the software is with the user. Should the software prove defective, the user
** assumes the cost of all necessary servicing, repair or correction. In
** particular, neither Rensselaer Polytechnic Institute, nor the authors of
** the software are liable for any indirect, special, consequential, or
** incidental damages related to the software, to the maximum extent the law
** permits.
*/

%{
#include "CIParser.h"
#include "CImisc.h"

%}

digit       [0-9]
integer     {digit}+
float       -?({digit}+(\.{digit}*)?|\.{digit}+)
letter      [a-zA-Z]
/* The \r allows the parsing of DOS files */
newline     [\n\r]
whitespace  [ \t]+
/* comment     # */

%%

	/* the main groupings of commands */
"node"		{ return Node; }
"id"		{ return Id; }
"location"	{ return Location; }
"neighbors"	{ return Neighbors; }
"events"	{ return Events; }
"up"		{ return Up; }
"down"		{ return Down; }

	/* Punctuation */
"{"		{ return LBrace; }
"}"		{ return RBrace; }
","		{ return Comma; }

"true"		{ return True; }
"false"		{ return False; }
{integer}       { return Integer; }
{float}		{ return Float; }
{newline}       { /* ignore newlines */ }
{whitespace}    { /* skip whitespace */ }
"#".*		{ /* skip comment */ }
.		{ return AnError;}
<<EOF>>		{ yyterminate(); }

%%

/* end file stateScanner.l */
