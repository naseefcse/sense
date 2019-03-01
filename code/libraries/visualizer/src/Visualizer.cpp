/*************************************************************************
 *   @<title> Visualizer.cpp </title>@
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

#include "ether_addr.h"
#include "path.h"
#include "Visualizer.h"
#include <string.h>
#include <assert.h>

using namespace std;

Visualizer	*Visualizer::instance = NULL;
const char	intro[] = "#Use the gnuplot command 'plot \"%s\" using 2:3 with lines' to display.\n"
			  "#Use scripts/extractPaths to extract a subset of paths.\n#\n";

Visualizer::Visualizer(
  const char	*base) 
    : enabled( false), connectFileOpen( false), cfMaxX( 0.0), cfMaxY( 0.0)
{
  if( base != NULL)
  {
    int		len = strlen( base);
    char	*str = new char[ len + 6 + 1];

    strcpy( str, base);
    strcpy( str + len, ".nodes");
    nodeFile = fopen( str, "w");
    if( nodeFile == NULL)
      printf( "Unable to open file %s for writing\n", str);
    else
    {
      fprintf( nodeFile, "#use the gnuplot command "
	       "'plot \"%s\" using 2:3' to display\n#\n", str);
      strcpy( str + len, ".paths");
      pathFile = fopen( str, "w");
      if( pathFile == NULL)
      {
	printf( "Unable to open file %s for writing\n", str);
	fclose( nodeFile);
	nodeFile = NULL;
      }
      else
      {
	fprintf( pathFile, intro, str);
	strcpy( str + len, ".drops");
	dropFile = fopen( str, "w");
	if( dropFile == NULL)
	{
	  printf( "Unable to open file %s for writing\n", str);
	  fclose( nodeFile);
	  fclose( pathFile);
	  nodeFile = NULL;
	  pathFile = NULL;
	}
	else
	{
	  fprintf( dropFile, intro, str);
	  enabled = true;
	  baseName = new char[ len + 6 + 1];	// extra room for extension
	  if( baseName != NULL)
	    strcpy( baseName, base);
	}
      }
    }
    delete [] str;
  }
  return;
}

Visualizer::~Visualizer()
{
  if( enabled == true)
  {
    fclose( nodeFile);
    fclose( pathFile);
    fclose( dropFile);
    if( baseName != NULL)
      delete [] baseName;
  }
  if( connectFileOpen == true)
    endConnectFile();
  enabled = false;
  LocationList::iterator	iter;
  for( iter = nodes.begin(); iter != nodes.end(); iter++)
  {
    delete (*iter).second;
    nodes.erase( iter);
  }
  return;
}

Visualizer *Visualizer::instantiate()
{
  return instantiate( NULL);
}

Visualizer *Visualizer::instantiate(
  const char	*baseName)
{
  if( instance == NULL)
    instance = new Visualizer( baseName);
  return instance;
}

void Visualizer::nodeLocation(
  int	addr,
  double	xLoc,
  double	yLoc)
{
  if( instance == NULL)
  {
    printf( "***** Visualizer not instantiated.\n");
    assert( instance != NULL);
  }
  if( enabled == false)
    return;

  LocationList::iterator	iter;
  LocInfo	*li;

  iter = nodes.find( addr);
  if( iter != nodes.end())
  {
    delete iter->second;
    nodes.erase( iter);
  }
  li = new LocInfo( xLoc, yLoc);
  nodes.insert( make_pair( addr, li));
  displayNode( addr, wFNode);
  return;
}

void Visualizer::writePath(
  int		src,
  int		dest,
  int		seqNumber,
  Path		path,
  bool		atDestination)
{
  if( enabled == false)
    return;

  int		id;
  bool		status;
  FILE		*fp = atDestination == true ? pathFile : dropFile;
  whichFile	wF = atDestination == true ? wFPath : wFDrop;

  fprintf( fp, "\n# src %3d; dest %3d; sn%08X\n", src, dest, seqNumber);
  if( displayNode( src, wF) == false)
    return;
  for( status = path.firstNode( id); status == true;
       status = path.nextNode( id))
    if( displayNode( id, wF) == false)
      break;
  if( path.getOverFlow() == true)
    fprintf( fp, "888 10000 10000\n***** Path incomplete *****\n");
  fprintf( fp, "##\n");
  return;
}

bool Visualizer::displayNode(
  int		node,
  whichFile	wF)
{
  LocationList::iterator	iter;
  FILE		*fp = wF == wFNode ? nodeFile : wF == wFPath ? pathFile : dropFile;
  
  iter = nodes.find( node);
  if( iter == nodes.end())
  {
    fprintf( fp, "********** node %d not in map **********\n", (int) node);
    return false;
  }
  fprintf( fp, "%d %f %f\n", (int) node, iter->second->x, iter->second->y);
  return true;
}

bool Visualizer::startConnectFile(
  double	maxX,
  double	maxY)
{
  if( enabled == false)
    return false;
  // constructor allocated enough space to hold file name extension
  int		len = strlen( baseName);

  strcpy( baseName + len, ".links");
  connectFile = fopen( baseName, "w");
  baseName[ len] = 0;	// restore baseName
  if( connectFile == NULL)
    return false;
  cfMaxX = maxX;
  cfMaxY = maxY;
  fprintf( connectFile, "# This file may be used as input for gnuplot, "
	   "Version 4.0\n# It provides a graphical representation of the "
	   "topology described in the configuration file.\n# Links are "
	   "unidirectional unless there are arrow heads at both ends.\n#\n");
  return true;
}

void Visualizer::addConnection(
  double	srcX,
  double	srcY,
  double	destX,
  double	destY)
{
  if( enabled == false)
    return;
  fprintf( connectFile, "set arrow from %f,%f to %f,%f\n", srcX, srcY, destX,
	   destY);
  return;
}

void Visualizer::endConnectFile()
{
  if( enabled == false)
    return;
  fprintf( connectFile, "plot [0:%f] [0:%f] \"%s.nodes\" using 2:3 with "
	   "points lt 3 pt 6 ps 2\n", cfMaxX, cfMaxY, baseName);
  fclose( connectFile);
  connectFileOpen = false;
  return;
}

/* end file Visualizer.cpp */
