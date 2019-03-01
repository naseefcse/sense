
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     LBrace = 258,
     RBrace = 259,
     Comma = 260,
     Integer = 261,
     Float = 262,
     True = 263,
     False = 264,
     AnError = 265,
     Common = 266,
     NumNodes = 267,
     BackOff = 268,
     Continuous = 269,
     RouteRepair = 270,
     HopCounts = 271,
     Senders = 272,
     SlotWidth = 273,
     TransitionTime = 274,
     SSR = 275,
     SHR = 276,
     Incorrect = 277,
     CostTable = 278,
     MaxCounter = 279,
     CurrentHC = 280,
     PendingHC = 281,
     UpdateCtr = 282,
     CostTableState = 283,
     Steady = 284,
     Initial = 285,
     Changing = 286,
     Node = 287,
     Location = 288,
     Net = 289,
     Addr = 290,
     SeqNumber = 291,
     ForwardDelay = 292,
     RXThresh = 293,
     AckWindow = 294,
     MaxResend = 295,
     TimeToLive = 296,
     AdditionalHop = 297,
     TotalDelay = 298,
     TotalSamples = 299,
     TotalHop = 300,
     SentPackets = 301,
     RecvPackets = 302,
     RecvUniPackets = 303,
     RecvDataPackets = 304,
     SentSubopt = 305,
     CanceledPkts = 306,
     CanceledSubopt = 307,
     Neighbors = 308
   };
#endif
/* Tokens.  */
#define LBrace 258
#define RBrace 259
#define Comma 260
#define Integer 261
#define Float 262
#define True 263
#define False 264
#define AnError 265
#define Common 266
#define NumNodes 267
#define BackOff 268
#define Continuous 269
#define RouteRepair 270
#define HopCounts 271
#define Senders 272
#define SlotWidth 273
#define TransitionTime 274
#define SSR 275
#define SHR 276
#define Incorrect 277
#define CostTable 278
#define MaxCounter 279
#define CurrentHC 280
#define PendingHC 281
#define UpdateCtr 282
#define CostTableState 283
#define Steady 284
#define Initial 285
#define Changing 286
#define Node 287
#define Location 288
#define Net 289
#define Addr 290
#define SeqNumber 291
#define ForwardDelay 292
#define RXThresh 293
#define AckWindow 294
#define MaxResend 295
#define TimeToLive 296
#define AdditionalHop 297
#define TotalDelay 298
#define TotalSamples 299
#define TotalHop 300
#define SentPackets 301
#define RecvPackets 302
#define RecvUniPackets 303
#define RecvDataPackets 304
#define SentSubopt 305
#define CanceledPkts 306
#define CanceledSubopt 307
#define Neighbors 308




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


