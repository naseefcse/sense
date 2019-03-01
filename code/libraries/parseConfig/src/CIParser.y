/*
** File: ciParser.y
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
#define YYPARSER /* distinguishes Yacc output from other code files */

#include <stdio.h>
#include "CIParseRtns.h"
#include "CImisc.h"

int	yyparse();
extern char	*yytext;

#define YYSTYPE	MyUnion

%}

/* Punctuation */
%token LBrace RBrace Comma

/* Basic Types */
%token Integer Float
%token True False
%token AnError

/* Key words */
%token Node Id Location Neighbors Events

/* Possible Event types */
%token Up Down


%% /* Grammar for SSR configuration input file */

configuration	: nodeList	{}

nodeList	: node nodeList	{}
		| node		{}

node		: Node { initNode(); } LBrace nodeStmtList RBrace { saveNode(); }

nodeStmtList	: nodeStmt Comma nodeStmtList	{}
		| nodeStmt	{}

nodeStmt	: idStmt	{}
		| locationStmt	{}
		| neighborsStmt	{}
		| eventsStmt	{}

idStmt		: Id anInteger { setId( $2.intVal); }

locationStmt	: Location LBrace aFloat Comma aFloat RBrace { setLocation( $3.floatVal, $5.floatVal); }

neighborsStmt	: nS1 listOfIntegers RBrace { endNeighborList(); }
		| nS1 RBrace { endNeighborList(); }

nS1		: Neighbors { startNeighborList(); } LBrace

eventsStmt	: eS1 listOfEvents RBrace { endEventList(); }
		| eS1 RBrace { endEventList(); }

eS1		: Events { startEventList(); } LBrace

listOfEvents	: anEvent { gotEvent( $1.eventVal.eTime, $1.eventVal.eType); } Comma listOfEvents
		| anEvent { gotEvent( $1.eventVal.eTime, $1.eventVal.eType); } 

anEvent		: Up aFloat { $$.eventVal.eType = NodeUp; $$.eventVal.eTime = $2.floatVal; }
		| Down aFloat { $$.eventVal.eType = NodeDown; $$.eventVal.eTime = $2.floatVal; }

listOfIntegers	: anInteger	{ gotInteger( $1.intVal); } Comma listOfIntegers
		| anInteger	{ gotInteger( $1.intVal); }

anInteger	: Integer	{ $$.intVal = atoi( yytext); }

aFloat		: Float		{ $$.floatVal = atof( yytext); }
		| Integer	{ $$.floatVal = atof( yytext); }

%%

void parse()
{
  yyparse();
  return;
}

