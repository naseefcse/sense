#include <cstdio>
#include <algorithm>
#include "SimInfo.h"
#include "commonRtns.h"

void printInt(
  int		val)
{
  printf( "%3d ", val);
  return;
}

void printNodeInfo(
  const NodeList::value_type	info)
{
  NodeInfo	*node = info.second;

  printf( "%3d: \nneighborList: ", info.first);
  for_each( node->neighborList.begin(), node->neighborList.end(), printInt);
  printf( "\n");
  for_each( node->costTable.begin(), node->costTable.end(), printCTInfo);
  printf( "\n");
  return;
}

void printCTInfo(
  const CTList::value_type	info)
{
  CTInfo	*ct = info.second;
  const char	*sstr;

  switch( ct->state)
  {
    case CT_Initial:
      sstr = " Initial";
      break;
    case CT_Steady:
      sstr = "  Steady";
      break;
    case CT_Changing:
      sstr = "Changing";
      break;
    default:			// not used, but keeps gcc quiet
      sstr = "unused";
      break;
  }

  printf( "%3d: %s; current HC %2d; pending HC %2d; counter %2d\n", info.first,
	  sstr, ct->currentHC, ct->pendingHC, ct->updateCtr);
  return;
}
