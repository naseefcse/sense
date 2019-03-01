/*
** File: stateScanner.l
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
/* mwl #include <string.h>	*/
#include "stateParser.h"
#include "misc.h"

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
"common"	{ return Common; }
"NumNodes"	{ return NumNodes; }
"backOff"	{ return BackOff; }
"continuous"	{ return Continuous; }
"routeRepair"	{ return RouteRepair; }
"hopCounts"	{ return HopCounts; }
"senders"	{ return Senders; }
"slotWidth"	{ return SlotWidth; }
"transitionTime"	{ return TransitionTime; }
"SSR"		{ return SSR; }
"SHR"		{ return SHR; }
"Incorrect"	{ return Incorrect; }
"costTable"	{ return CostTable; }
"maxCounter"	{ return MaxCounter; }
"currentHC"	{ return CurrentHC; }
"pendingHC"	{ return PendingHC; }
"updateCtr"	{ return UpdateCtr; }
"state"		{ return CostTableState; }
"Steady"	{ return Steady; }
"Initial"	{ return Initial; }
"Changing"	{ return Changing; }
"node"		{ return Node; }
"location"	{ return Location; }
"net"		{ return Net; }
"addr"		{ return Addr; }
"seqNumber"	{ return SeqNumber; }
"ForwardDelay"	{ return ForwardDelay; }
"RXThresh"	{ return RXThresh; }
"AckWindow"	{ return AckWindow; }
"MaxResend"	{ return MaxResend; }
"TimeToLive"	{ return TimeToLive; }
"AdditionalHop"	{ return AdditionalHop; }
"TotalDelay"	{ return TotalDelay; }
"TotalSamples"	{ return TotalSamples; }
"TotalHop"	{ return TotalHop; }
"SentPackets"	{ return SentPackets; }
"RecvPackets"	{ return RecvPackets; }
"RecvUniPackets"	{ return RecvUniPackets; }
"RecvDataPackets"	{ return RecvDataPackets; }
"SentSuboptimal"	{ return SentSubopt;}
"CanceledPackets"	{ return CanceledPkts;}
"CanceledSuboptimal"	{ return CanceledSubopt;}
"neighbors"	{ return Neighbors; }
 

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
