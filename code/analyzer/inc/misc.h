/*************************************************************************
 *   @<title> misc.h </title>@
 *
 *   @<!-- Copyright 2006 Mark Lisee, Boleslaw K. Szymanski and Rensselaer
 *   Polytechnic Institute. All worldwide rights reserved.  A license to use,
 *   copy, modify and distribute this software for non-commercial research
 *   purposes only is hereby granted, provided that this copyright notice and
 *   accompanying disclaimer is not modified or removed from the software.
 *
 *   DISCLAIMER: The software is distributed "AS IS" without any express or
 *   implied warranty, including but not limited to, any implied warranties of
 *   merchantability or fitness for a particular purpose or any warranty of
 *   non-infringement of any current or pending patent rights. The authors of
 *   the software make no representations about the suitability of this
 *   software for any particular purpose. The entire risk as to the quality
 *   and performance of the software is with the user. Should the software
 *   prove defective, the user assumes the cost of all necessary servicing,
 *   repair or correction. In particular, neither Rensselaer Polytechnic
 *   Institute, nor the authors of the software are liable for any indirect,
 *   special, consequential, or incidental damages related to the software,
 *   to the maximum extent the law permits.-->@
 *
 *************************************************************************/

#ifndef	_misc_h_
#define	_misc_h_

#ifdef	__cplusplus
extern "C"
#endif	// __cplusplus
void	parse();

typedef enum CTState
{
  CT_Initial,
  CT_Steady,
  CT_Changing
} CTState;

typedef enum BackOffType
{
  BOT_SSR,
  BOT_SHR,
  BOT_Incorrect
} BackOffType;

typedef union MyUnion
{
  int		boolVal;
  int		intVal;
  float		floatVal;
  BackOffType	backOff;
  CTState	state;
} MyUnion;

#endif	// _misc_h_
