
/*************************************************************************
 *   Copyright 2003 Gilbert (Gang) Chen, Boleslaw K. Szymanski and
 *   Rensselaer Polytechnic Institute. All worldwide rights reserved.
 *   A license to use, copy, modify and distribute this software for
 *   non-commercial research purposes only is hereby granted, provided
 *   that this copyright notice and accompanying disclaimer is not
 *   modified or removed from the software.
 *
 *   DISCLAIMER: The software is distributed "AS IS" without any
 *   express or implied warranty, including but not limited to, any
 *   implied warranties of merchantability or fitness for a particular
 *   purpose or any warranty of non-infringement of any current or
 *   pending patent rights. The authors of the software make no
 *   representations about the suitability of this software for any
 *   particular purpose. The entire risk as to the quality and
 *   performance of the software is with the user. Should the software
 *   prove defective, the user assumes the cost of all necessary
 *   servicing, repair or correction. In particular, neither Rensselaer
 *   Polytechnic Institute, nor the authors of the software are liable
 *   for any indirect, special, consequential, or incidental damages
 *   related to the software, to the maximum extent the law permits.
 *************************************************************************/

#ifndef simple_channel_h
#define simple_channel_h

#include <algorithm>

template <class PACKET>

component SimpleChannel : public TypeII
{
 public:
  bool		initSources( int numSources, bool oneSink);
  void		getSourcePair( int &src, int &dest);

  inport void		from_phy( PACKET* p, double power, int id);
  outport[] void	to_phy( PACKET* p, double power );
  inport void		pos_in( const coordinate_t& pos, int id);
	
  InfiTimer< triple<PACKET*, double, int> > propagation_delay;
  inport void		depart( const triple<PACKET*, double, int> & data, unsigned int );

  SimpleChannel() { connect propagation_delay.to_component, depart; }
  virtual ~SimpleChannel() {}
  void		Start();
  void		Stop();
  void		Setup();
  bool		isNeighbor( double power, int src, int dest);

  void		setNumNodes( int n)		{ NumNodes = n; }
  void		setDumpPackets( bool flag)	{ DumpPackets = flag; }
  void		setCSThresh( double thresh)	{ CSThresh = thresh; }
  void		setRXThresh( double thresh)	{ RXThresh = thresh; }
  void		setWaveLength( double wl)	{ WaveLength= wl; }
  void		useFreeSpace()			{ PropagationModel = FreeSpace; }
  void		useTwoRay()			{ PropagationModel = TwoRay; }
  void		setX( double x)			{ X = x; }
  void		setY( double y)			{ Y = y; }
  void		setGridEnabled( bool flag)	{ GridEnabled = flag; }
  void		setMaxTXPower( double pwr)	{ MaxTXPower = pwr; }

 private:    
  enum PModelType { FreeSpace, TwoRay };
  int		NumNodes;
  bool		DumpPackets;
  double	CSThresh;
  double	RXThresh;
  double	WaveLength;
  PModelType	PropagationModel;
  double	X;
  double	Y;
  bool		GridEnabled;
  double	MaxTXPower;
  bool		forward( PACKET* p, double power, int src, int dest, bool);
  int		nx, ny;
  double	gridsize;
  struct node_t : public coordinate_t
  {
    node_t() : coordinate_t(0.0,0.0), prev(NULL), next(NULL), grid(-1) {};
    node_t* prev;
    node_t* next;
    int grid;
  };
  std::vector< node_t* > m_grids;
  std::vector< node_t > m_positions;
};

template <class PACKET>
void SimpleChannel<PACKET>::Setup()
{
  m_positions.reserve(NumNodes);
  for(int i=0;i<NumNodes;i++)
    m_positions.push_back( node_t() );
  to_phy.SetSize(NumNodes);

  if(PropagationModel==FreeSpace)
    gridsize = sqrt(MaxTXPower/CSThresh) * WaveLength / (4.0*3.14159265);
  else
    gridsize = sqrt ( sqrt (RXThresh/CSThresh) * MaxTXPower ) * WaveLength / (4.0*3.14159265);

  printf("X: %f, Y: %f, gridsize: %f\n",X,Y,gridsize);
  nx = (int) (X/gridsize) + 1;
  ny = (int) (Y/gridsize) + 1;

  for(int i=0;i<nx*ny;i++)
  {
    m_grids.push_back(NULL);
  }

  if(GridEnabled && nx < 6 && ny < 6 )
    GridEnabled = false;

  if(GridEnabled)
    printf("Grid enabled\n");
  else
    printf("Grid disabled\n");

}

template <class PACKET>
void SimpleChannel<PACKET>::Start()
{
}

template <class PACKET>
void SimpleChannel<PACKET>::Stop()
{
}

template <class PACKET>
void SimpleChannel<PACKET>::pos_in(const coordinate_t & pos, int id)
{
  m_positions[id].x=pos.x;
  m_positions[id].y=pos.y;

  if(GridEnabled)
  {
    int x,y,n;
    x=(int)(pos.x/gridsize);
    y=(int)(pos.y/gridsize);
    n=x+y*nx;
    if(n!=m_positions[id].grid)
    {
      if(m_positions[id].grid!=-1)
      {
	if(m_positions[id].prev==NULL)
	{
	  assert(m_grids[n]==&m_positions[id]);
	  m_grids[n]=m_positions[id].next;
	  if(m_grids[n]!=NULL)m_grids[n]->prev=NULL;
	}
	else
	{
	  m_positions[id].prev->next=m_positions[id].next;
	  if(m_positions[id].next!=NULL)
	    m_positions[id].next->prev=m_positions[id].prev;		    
	}
      }
	    
      m_positions[id].grid=n;
      m_positions[id].prev=NULL;
      m_positions[id].next=m_grids[n];
      m_grids[n]=&m_positions[id];
      if(m_positions[id].next!=NULL)
	m_positions[id].next->prev=m_grids[n];
    }
  }
}

template <class PACKET>
bool SimpleChannel<PACKET>::isNeighbor(
  double	power,
  int		src,
  int		dest)
{
  return forward( NULL, power, src, dest, false);
}

template <class PACKET>
bool SimpleChannel<PACKET>::forward ( PACKET* p, double power, int in, int out, bool flag)
{
  double rx_power,sqd,v,dx,dy;
  double now = SimTime();

  dx = m_positions[in].x - m_positions[out].x;
  dy = m_positions[in].y - m_positions[out].y;
  sqd = dx*dx + dy*dy ;
  v = WaveLength / (4.0*3.14159265);

  switch( PropagationModel)
  {
    default:
      printf( "Invalid propagation model\n");
      // fall through
    case FreeSpace:
      rx_power = power * v * v / sqd;
      break;
    case TwoRay:
      rx_power = v*v*v*v*power*power / ( sqd * sqd * RXThresh );
      break;
  }

  if(rx_power>CSThresh)
  {
    if (flag)
    {
      p->inc_ref();
      Printf((DumpPackets,"transmits from %f %f to %f %f\n", 
	      m_positions[in].x, m_positions[in].y,
	      m_positions[out].x,m_positions[out].y));
      printf( "SC::f: transmits from %f %f to %f %f\n", m_positions[in].x,
	      m_positions[in].y, m_positions[out].x, m_positions[out].y);
      propagation_delay.Set(make_triple(p,rx_power,out),now+sqrt(sqd)/speed_of_light);
    }
    return true;
  }
	else
	printf( "SC::f: dropping from %f %f to %f %f\n", m_positions[in].x,
	m_positions[in].y, m_positions[out].x, m_positions[out].y);
  return false;
}

template <class PACKET>
void SimpleChannel<PACKET>::from_phy ( PACKET* packet, double power, int in)
{   
  printf("[%f] %d transmits %s\n", SimTime(), in, packet->dump().c_str());
  
  if(!GridEnabled)
  {
    for(int out=0;out<NumNodes;out++)
    {
      if(out!=in)
      {
	forward (packet, power, in, out, true);
      }
	    
    }
  }
  else
  {
    int i,j,m,n,out;
    node_t* p;
    m = m_positions[in].grid % nx;
    n = m_positions[in].grid / ny;
    for ( i = (m>0?m-1:0); i <= (m<nx-1?m+1:nx-1) ; i ++)
      for ( j = (n>0?n-1:0); j <= (n<ny-1?n+1:ny-1) ; j ++)
      {
	p=m_grids[i+j*nx];
	while(p!=NULL)
	{
	  out=p-&m_positions[0];
	  if(out!=in)
	  {
	    forward (packet, power, in, out, true);
	  }
	  p=p->next;
	}
      }	
  }

  /* std::sort(out_list2.begin(), out_list2.end());
     if(out_list1!=out_list2)
     {
     printf("%d --> ", out_list1.size());
     for(unsigned int i=0;i<out_list1.size();i++)
     printf("%d ", out_list1[i]);
     printf("\n");
     printf("%d -> ", out_list2.size());
     for(unsigned int i=0;i<out_list2.size();i++)
     printf("%d ", out_list2[i]);
     printf("\n");
     }*/
  packet->free();
}

/*
** This is called when the propagation_delay timer expires.
*/
template <class PACKET>
void SimpleChannel<PACKET>::depart ( const triple<PACKET*, double, int> & data , unsigned int index)
{
  //to_phy[dest]( pkt, rx_power)  
  to_phy[data.third](data.first,data.second);
  return;
}

template <class PACKET>
bool SimpleChannel<PACKET>::initSources(
  int		,	// numSources
  bool		)	// oneSink
{
  return true;		// let channel determine source nodes
}

template <class PACKET>
void SimpleChannel<PACKET>::getSourcePair(
  int		&,
  int		&)
{
  return;
}

#endif /* simple_channel_h */
