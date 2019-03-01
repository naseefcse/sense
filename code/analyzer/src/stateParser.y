/*
** File: stateParser.y
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
#include "parseRtns.h"

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

/* Key words - Simulation as a whole */
%token Common NumNodes BackOff Continuous RouteRepair HopCounts Senders
%token SlotWidth TransitionTime
/* types of back off */
%token SSR SHR Incorrect

/* Key words - Cost Table */
%token CostTable MaxCounter CurrentHC PendingHC UpdateCtr CostTableState
/* possible states */
%token Steady Initial Changing

/* Key words - Node */
%token Node Location Net Addr SeqNumber ForwardDelay RXThresh
%token AckWindow MaxResend TimeToLive AdditionalHop TotalDelay TotalSamples
%token TotalHop SentPackets RecvPackets RecvUniPackets RecvDataPackets
%token SentSubopt CanceledPkts CanceledSubopt
%token Neighbors


%% /* Grammar for SSR status snapshot file */

snapshot	: common cmnCostTable senders nodeList	{}

common		: Common { initSimInfo(); } LBrace cmnStmtList RBrace	{}

cmnStmtList	: cmnStmt Comma cmnStmtList		{}
		| cmnStmt		{}

cmnStmt		: numNodesStmt		{}
		| backOffStmt		{}
		| continuousStmt	{}
		| routeRepairStmt	{}
		| hopCountsStmt		{}
		| slotWidthStmt		{}
		| transitionTimeStmt	{}

numNodesStmt	: NumNodes anInteger	{ setNumNodes( $2.intVal); }

backOffStmt	: BackOff backOffType	{ setBackOff( $2.backOff); }

backOffType	: SSR			{ $$.backOff = BOT_SSR; }
		| SHR			{ $$.backOff = BOT_SHR; }
		| Incorrect		{ $$.backOff = BOT_Incorrect; }

continuousStmt	: Continuous boolean	{ setContinuous( $2.boolVal); }

routeRepairStmt	: RouteRepair boolean	{ setRouteRepair( $2.boolVal); }

hopCountsStmt	: HopCounts anInteger	{ setHopCounts( $2.intVal); }

slotWidthStmt	: SlotWidth aFloat	{ setSlotWidth( $2.intVal); }

transitionTimeStmt : TransitionTime aFloat { setTransitionTime( $2.floatVal); }

cmnCostTable	: CostTable LBrace ctCmnStmtList RBrace		{}

ctCmnStmtList	: ctStmt Comma ctCmnStmtList		{}
		| ctStmt		{}

ctStmt		: MaxCounter anInteger	{ setMaxCounter( $2.intVal); }

senders		: Senders { startSenders(); } LBrace listOfIntegers RBrace { endSenders(); };

nodeList	: nodeInfo nodeList	{}
		| nodeInfo		{}

nodeInfo	: Node { initNode(); } LBrace nodeStmtList RBrace { saveNode(); }

nodeStmtList	: nodeStmt nodeStmtList	{}
		| nodeStmt		{}

nodeStmt	: locationStmt		{}
		| netStmt		{}
		| neighborsStmt		{}
		| costTableStmt		{}

locationStmt	: Location LBrace aFloat { setLocX( $3.floatVal); } Comma aFloat { setLocY( $6.floatVal); } RBrace

netStmt		: Net LBrace netStmtList RBrace	{}

netStmtList	: netStmt Comma netStmtList	{}
		| netStmt		{}

netStmt		: myEtherAddrStmt	{}
		| seqNumberStmt		{}
		| forwardDelayStmt	{}
		| rxThreshStmt		{}
		| ackWindowStmt		{}
		| maxResendStmt		{}
		| ttlStmt		{}
		| addHopStmt		{}
		| totalDelayStmt	{}
		| totalSamplesStmt	{}
		| totalHopStmt		{}
		| sentPktsStmt		{}
		| recvPktsStmt		{}
		| recvUniPktsStmt	{}
		| recvDataPktsStmt	{}
		| sentSuboptStmt	{}
		| canceledPktsStmt	{}
		| canceledSuboptStmt	{}


myEtherAddrStmt	: Addr anInteger	{ setAddr( $2.intVal); }

seqNumberStmt	: SeqNumber anInteger	{ setSeqNumber( $2.intVal); }

forwardDelayStmt : ForwardDelay aFloat	{ setForwardDelay( $2.floatVal); }

rxThreshStmt	: RXThresh aFloat	{ setRXThresh( $2.floatVal); }

ackWindowStmt	: AckWindow aFloat	{ setAckWindow( $2.floatVal); }

maxResendStmt	: MaxResend anInteger	{ setMaxResend( $2.intVal); }

ttlStmt		: TimeToLive anInteger	{ setTimeToLive( $2.intVal); }

addHopStmt	: AdditionalHop anInteger { setAdditionalHop( $2.intVal); }

totalDelayStmt	: TotalDelay aFloat	{ setTotalDelay( $2.floatVal);}

totalSamplesStmt : TotalSamples anInteger { setTotalSamples( $2.intVal);}

totalHopStmt	: TotalHop anInteger	{ setTotalHop( $2.intVal); }

sentPktsStmt	: SentPackets anInteger	{ setSentPkts( $2.intVal); }

canceledPktsStmt : CanceledPkts anInteger	{ setCanceledPkts( $2.intVal); }

recvPktsStmt	: RecvPackets anInteger	{ setRecvPkts( $2.intVal); }

recvUniPktsStmt	: RecvUniPackets anInteger { setRecvUniPkts( $2.intVal); }

recvDataPktsStmt : RecvDataPackets anInteger { setRecvDataPkts( $2.intVal);}

sentSuboptStmt	: SentSubopt anInteger	{ setSentSubopt( $2.intVal); }

canceledSuboptStmt : CanceledSubopt anInteger	{ setCanceledSubopt( $2.intVal); }

costTableStmt	: CostTable LBrace ctStmtList RBrace		{}
		| CostTable LBrace RBrace		{}

ctStmtList	: ctStmt ctStmtList	{}
		| ctStmt		{}

ctStmt		: anInteger { initCTEntry( $1.intVal); } LBrace ctFieldList RBrace { saveCTEntry(); }

ctFieldList	: ctField Comma ctFieldList	{}
		| ctField		{}

ctField		: currentHCStmt		{}
		| ctStateStmt		{}
		| pendingHCStmt		{}
		| updateCtrStmt		{}

currentHCStmt	: CurrentHC anInteger	{ setCTCurrentHC( $2.intVal); }

ctStateStmt	: CostTableState ctState	{ setCTState( $2.state); }

ctState		: Initial	{ $$.state = CT_Initial; }
		| Steady	{ $$.state = CT_Steady; }
		| Changing	{ $$.state = CT_Changing; }

pendingHCStmt	: PendingHC anInteger	{ setCTPendingHC( $2.intVal); }

updateCtrStmt	: UpdateCtr anInteger	{ setCTUpdateCtr( $2.intVal); }

neighborsStmt	: nS1 listOfIntegers RBrace { endNeighborList(); }
		| nS1 RBrace { endNeighborList(); }

nS1		: Neighbors { startNeighborList(); } LBrace

listOfIntegers	: anInteger	{ gotInteger( $1.intVal); } Comma listOfIntegers
		| anInteger	{ gotInteger( $1.intVal); }

anInteger	: Integer	{ $$.intVal = atoi( yytext); }

aFloat		: Float		{ $$.floatVal = atof( yytext); }

boolean		: True		{ $$.boolVal = 1; }
		| False		{ $$.boolVal = 0; }

%%

void parse()
{
  yyparse();
  return;
}

