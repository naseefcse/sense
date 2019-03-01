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

#ifndef battery_h
#define battery_h

component SimpleBattery : public TypeII
{
 public:
  double	InitialEnergy;
  double	RemainingEnergy;

  inport inline double	power_in( double power, simtime_t t);
  inport inline double	query_in();
    
  void		Start();
  void		Stop();
  void		clearStats()	{ InitialEnergy = RemainingEnergy; }
 private:
  double	m_last_time;
};

void SimpleBattery::Start()
{
    RemainingEnergy=InitialEnergy;
    m_last_time=0.0;
}

void SimpleBattery::Stop()
{
}

double SimpleBattery::power_in(double power, simtime_t t)
{
	//printf("t: %f, m_last_time: %f\n", t, m_last_time);
	if(t>m_last_time)
	{
        RemainingEnergy -= (t-m_last_time)*power;
        m_last_time=t;
    }
    return RemainingEnergy;
}
double SimpleBattery::query_in()
{
    return RemainingEnergy;
}
/*
class RealBattery : public TypeII
{
 public:
 
    enum{Normal=0,HighCurrent,Relaxation};
       
    double InitialEnergy;
    double RemainingEnergy;
        
    double Voltage;
    double DRDependence; //discharge rate dependence parameter
    double LCurrent;   //low currentthreshold value
    double HCurrent;  //high current threshold value
    double RelaxThreshold; //HighCurrent time 
    double RecoveryRate;
    double GrowthRatio;
    
    int CurDep;  //1: include the current dependence model; 0: only the relaxation model
    
    typedef std::pair <double, simtime_t> energy_t;
    Inport <double, const energy_t> energy_in;
    Inport <double, const trigger_t> query_in;
    
    void Start();
    void Stop();
    void Setup(TypeII* c, const char* name);

 private:
    double EnergyIn(const energy_t&, int);
    double QueryIn(const trigger_t&, int);
   
    int m_state; 
    double m_last_time;
    double m_last_energy;
 

    double m_high_time;  //the time in HighCurrent state
    double m_relax_time;
    double m_last_relaxed_time;
    double m_relax_energy;
    
};

void RealBattery::Start()
{
    RemainingEnergy=m_last_energy=InitialEnergy;
    m_state=Normal;
    m_last_time=0.0;
    m_high_time=0.0;  
    m_relax_time=0.0;
    m_last_relaxed_time=0.0;   //for use of any continuous relaxation
    m_relax_energy=m_last_energy;    
}

void RealBattery::Stop()
{
}

void RealBattery::Setup(TypeII* c, const char* name)
{
    energy_in.Setup(this,&RealBattery::EnergyIn,"energy_in");
    query_in.Setup(this,&RealBattery::QueryIn,"query_in");
    TypeII::Setup(c,name);
}
double RealBattery::EnergyIn(const energy_t& p, int)
{
   double current=p.first/Voltage;
   double now=p.second;
   assert(now > m_last_time);
   switch(m_state)
   {
   case Normal:
     {
     	if(current>HCurrent)
     	{
     	  m_state=HighCurrent;
     	  m_high_time=now-m_last_time;
     	  m_last_energy=RemainingEnergy;
     	}      
     	break;
     }
   case HighCurrent:
     {
     	if(current<LCurrent && m_high_time > RelaxThreshold)
     	{
     	 // relaxation
     	 m_state=Relaxation;
     	 m_relax_time=now-m_last_time;
     	 m_relax_energy=(m_last_energy < RemainingEnergy*GrowthRatio? m_last_energy:RemainingEnergy*GrowthRatio)-RemainingEnergy;
        }
        if(LCurrent<=current && current<=HCurrent)
        {
          m_state=Normal;	
        }
        if(current>HCurrent)
        {
           m_high_time+=now-m_last_time;
        }
        break;
      }
    case Relaxation:
      {
      	if(current<LCurrent)
      	{	
      	  //continue to relax
      	  m_relax_time=m_last_relaxed_time+now-m_last_time;
        }
        if(current>HCurrent)
        {
          m_state=HighCurrent;
          m_high_time=now-m_last_time;
        }
        else  m_state=Normal;
        m_last_relaxed_time=0.0;
        break;
      }
   }
   if(CurDep)
     RemainingEnergy=RemainingEnergy/(1.0+DRDependence*current)-p.first*(now-m_last_time);  //current dependence model
   else RemainingEnergy-= p.first*(now-m_last_time);
   m_last_time=now;
   if(m_state==Relaxation)
   {
     //printf("*************relaxation*********************\n");			
     RemainingEnergy+=m_relax_energy*(exp(-RecoveryRate*m_last_relaxed_time)-exp(-RecoveryRate*m_relax_time));
     m_last_relaxed_time=m_relax_time;
     m_high_time=0.0;
   } 
   printf("remaining is %f\n", RemainingEnergy); 
   return RemainingEnergy;
}       		 	 	 	 
 

double RealBattery::QueryIn(const trigger_t& e, int)
{
    return RemainingEnergy;
}
*/
#endif /* battery_h */

