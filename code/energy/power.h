#ifndef power_h
#define power_h

#include <utility>

struct PM_Struct
{
  enum { TX = 0, RX, IDLE, SLEEP, OFF, ON};
};

component PowerManager : public TypeII, public PM_Struct
{
 public:
  PowerManager();

  double	RXPower;
  double	TXPower;
  double	IdlePower;
  double	SleepPower;

  double	BeginTime;
  double	FinishTime;
  void		failureStats( double initTime, double cycleTime,
			      double percentUp);
  void		failureStats( double downTime);

  inport inline double	switch_state( int state, double time);
  inport inline int	state_query();
  inport inline double	energy_query();
  inport inline void	power_switch(trigger_t&);
  outport double	to_battery_power( double power, simtime_t time);
  outport double	to_battery_query();
  // This is called when the power manager wakes up the node.
  outport void	from_pm_node_up();
	
  void		Start();
  void		Stop();
  Timer<trigger_t> switch_timer;
 private:
  int		m_state; 
  bool		m_switch_on;
  bool		transient;
  double	upTime;
  double	downTime;
  double	initTime;
};

PowerManager::PowerManager()
{
  upTime = 1.0;
  downTime = 0.0;
  initTime = 0.0;
  transient = false;
  connect switch_timer.to_component, power_switch;
  return;
}

/*
** Set failure stats for nodes that die permanently
*/
void PowerManager::failureStats(
  double	initializeTime)	// seconds until node fails (0 = no failure)
{
  transient = false;
  initTime = initializeTime;
  return;
}

/*
** Set failure stats for transient nodes
*/
void PowerManager::failureStats(
  double	initializeTime,	// seconds until node fails for first time
  double	cycleTime,	// mean cycle time
  double	percentUp)
{
  if( percentUp < 1.0)
  {
    transient = true;
    upTime = cycleTime * percentUp;
    downTime = cycleTime * (1 - percentUp);
    initTime = initializeTime;
  }
  else
  {
    transient = false;
    initTime = 0.0;
  }
  return;
}

void PowerManager::Start()
{
  m_state = IDLE;
  m_switch_on = true;
  if( initTime != 0)
    switch_timer.Set( BeginTime + initTime);
  return;
}

void PowerManager::Stop()
{
  return;
}

/*
** This method is called when the timer expires.
** Toggle the state of the power manager.
*/
void PowerManager::power_switch(
  trigger_t	&)
{
  // node is going down
  if( m_switch_on == true)
  {
    m_state = OFF;
    to_battery_power( 0.0, SimTime());
    m_switch_on = false;
    if( transient == true)
      switch_timer.Set( SimTime() + Exponential( downTime));
  }
  else		// node is going up
  {
    m_state = IDLE;
    to_battery_power( IdlePower, SimTime());
    m_switch_on = true;
    from_pm_node_up();		// notify node that it's now awake
    switch_timer.Set( SimTime() + Exponential( upTime));
  }
  return;
}

double PowerManager::switch_state(
  int		state,
  double	time) 
{
  double	power = 0.0;	
  if( time < BeginTime || time > FinishTime)
    return power;

  switch( m_state)
  {
    case TX:
      power = to_battery_power( TXPower, time);
      break;
    case RX:
      power = to_battery_power( RXPower, time);
      break;
    case IDLE:
      power = to_battery_power( IdlePower, time);
      break;
    case SLEEP:
      power = to_battery_power( SleepPower, time);
      break;
    case OFF:
      power = to_battery_power( 0.0, time);
      return 0.0;
    case ON:
      power = to_battery_power( IdlePower, time);
      m_state = IDLE;
      return power;
    default:
      fprintf( stderr,"invalid state to power manager\n");
      break;
  }
  m_state=state;
  return power;
} 
       
int PowerManager::state_query()
{
  return m_state; 
}

double PowerManager::energy_query()
{
  return to_battery_query();
}

#endif /* power_h */
