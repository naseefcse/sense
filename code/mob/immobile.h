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

#ifndef immobile_h
#define immobile_h

component Immobile : public TypeII
{
 public:
  Timer <trigger_t>	timer;    
  outport void		pos_out( coordinate_t& pos, int id);
  inport void		announce_pos( trigger_t& t);

  Immobile() { connect timer.to_component, announce_pos; }
  void		Start();
  void		Stop();
  void		Setup();
  void		setX( double x)	{ InitX = x; }
  double	getX()		{ return InitX; }
  void		setY( double y)	{ InitY = y; }
  double	getY()		{ return InitY; }
  void		setID( int id)	{ ID = id; }
  int		getID()		{ return ID; }
  static void	setVisualizer( Visualizer *ptr);
 private:
  double	InitX;
  double	InitY;
  int		ID;
  static Visualizer	*visualizer;
};

Visualizer	*Immobile::visualizer = NULL;

void Immobile::setVisualizer(
  Visualizer	*ptr)
{
  visualizer = ptr;
  return;
}

void Immobile::announce_pos(trigger_t&)
{
  coordinate_t pos=coordinate_t(InitX,InitY);
  pos_out(pos,ID);
  Printf((true, "%d @ %f %f\n",ID, InitX, InitY));
#ifdef VISUAL_ROUTE
  printf("@ %f %d %f %f\n", SimTime(), ID, InitX, InitY);
#endif
  visualizer->nodeLocation( ID, InitX, InitY);
  return;
}

void Immobile::Setup()
{
  return;
}

void Immobile::Start()
{
  timer.Set(0.0);
  return;
}

void Immobile::Stop()
{
  return;
}

#endif /* immobile_h*/
