/*************************************************************************
 *   @<title> Visualizer.h </title>@
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

/*
** The Visualizer class is used to create files that may be used as input for
** gnuplot so that the nodes locations and packet routes may be visualized.
**
** The class must be instantiated with either the base name of the files to
** be created or with no arguments, in which case no output is generated.
**
** Files created
** baseName.nodes: location of all nodes
** baseName.paths: all of the routes taken by successful packets
** baseName.drops: all of the routes taken by packet that were eventually
**		   dropped.
*/

#ifndef	_Visualizer_h_
#define	_Visualizer_h_

#include <cstdio>
#include <map>

class Visualizer
{
 public:
  static Visualizer	*instantiate( const char *baseName);
  static Visualizer	*instantiate();
		~Visualizer();
  void		nodeLocation( int addr, double x, double y);
  void		writePath( int src, int dest, int seqNumber, Path path,
			   bool atDestination);	// false if pkt dropped due to exceeding TTL
  bool		startConnectFile( double maxX, double maxY);
  void		addConnection( double srcX, double srcY, double destX,
			       double destY);
  void		endConnectFile();
 private:
  enum whichFile
  {
    wFNode = 0,
    wFPath = 1,
    wFDrop = 2
  };
		Visualizer( const char *baseName);
  bool		displayNode( int addr, whichFile path);
  static Visualizer	*instance;
  struct LocInfo
  {
    double	x;
    double	y;
    LocInfo( double xIn, double yIn) { x = xIn; y = yIn; }
  };
  typedef std::map< int, struct LocInfo *>	LocationList;
  LocationList	nodes;
  FILE		*nodeFile;
  FILE		*pathFile;
  FILE		*dropFile;
  bool		enabled;
  char		*baseName;
  FILE		*connectFile;
  bool		connectFileOpen;
  double	cfMaxX;
  double	cfMaxY;
};

#endif	// _Visualizer_h_

/* end file Visualizer.h */
