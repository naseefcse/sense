/*************************************************************************
 *   @<title> SHR Protocol Simulation (Adjacency Matrix Channel) </title>@
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
 *
 * @<h1> SHR Protocol Simulation (Adjacency Matrix Channel) </h1>@
 *
 * Building a wireless sensor network simulation in SENSE consists of
 * the following steps:
 * @<UL>@
 * @<LI>@ Designing a sensor node component
 * @<LI>@ Constructing a sensor network derived from @CostSimEng@
 * @<LI>@ Configuring the system and running the simulation
 * @</UL>@
 * 
 * Here, we assume that all components needed by a sensor node component
 * are available from the component repository. If this is not the case, the user
 * must develop new components, as described by other @<a href=tutorial.html>tutorials</a>@. We should
 * also mention that the first step of designing a sensor node component
 * is not always necessary, if a standard sensor node is to be used.
 *
 * This first line of this source file demands that @HeapQueue@ must be
 * used as the priority queue for event management.  For wireless network
 * simulation, because of the inherent drawback of @CalendarQueue@, and also
 * because of the particular channel component being used, @HeapQueue@
 * is often faster. 
 *************************************************************************/

#define queue_t HeapQueue

/*******************************************************************
 * These header files are absolutely required
 *******************************************************************/

#include <ctime>
#include "../../common/sense.h"
#include "../../libraries/visualizer/inc/Visualizer.h"
#include "../../libraries/parseConfig/inc/ConfigInfoWrapper.h"


/*******************************************************************
 * The following header files are necessary only if the corresponding
 * components are needed by the sensor node component.
 *
 * Note: The CompC++ compiler doesn't fully support #ifdef. It is not possible
 * to use ifdef-else to include one file exclusive of the other. As a result,
 * this file includes the appropriate components then includes the file
 * sim_shr.h which contains the executable code of that used to be in this file.
 *******************************************************************/

/*******************************************************************
 * #cxxdef is similar to #define, except it is only recognized by the
 * CompC++ compiler.  The following two lines state that the flooding
 * component will be used for the network layer. These two macros
 * can also be overridden by command line macros definitions (whose
 * format is '-D<macro>=<something>'.
 *******************************************************************/

/*******************************************************************
 * SSR simulation that uses an adjacency matrix component for connectivity.
 *******************************************************************/

#cxxdef	ChannelType	SimpleChannelLoc
#cxxdef	ImmobileType	ImmobileLoc
#cxxdef	TransceiverType	DuplexTransceiverLoc

#include "../../app/cbr.h"
#include "../../mob/immobile_loc.h"
#include "../../net/ssab.h"
void	addToNeighborMatrix( int, int);
#include "../../net/shr.h"
#include "../../mac/bcast_mac.h"
#include "../../phy/transceiver_loc.h"
#include "../../phy/simple_channel_loc.h"
#include "../../energy/battery.h"
#include "../../energy/power.h"

const char	ChannelModel[] = "Adjacency Matrix";

#include "sim_shr.h"

/* end file sim_shr_loc.cc */
