
#line 40 "mmm1.cc"
#define queue_t SimpleQueue


#line 1 "../../common/cost.h"
#ifndef queue_t
#define queue_t SimpleQueue
#endif

#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <deque>
#include <vector>
#include <assert.h>
#include <string.h>
#include <stdio.h>


#line 1 "../../common/priority_q.h"























#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H














template < class ITEM >
class SimpleQueue 
{
 public:
  SimpleQueue() :m_head(NULL) {};
  void EnQueue(ITEM*);
  ITEM* DeQueue();
  void Delete(ITEM*);
  ITEM* NextEvent() const { return m_head; };
  const char* GetName();
 protected:
  ITEM* m_head;
};

template <class ITEM>
const char* SimpleQueue<ITEM>::GetName()
{
  static const char* name = "SimpleQueue";
  return name;
}

template <class ITEM>
void SimpleQueue<ITEM>::EnQueue(ITEM* item)
{
  if( m_head==NULL || item->time < m_head->time )
  {
    if(m_head!=NULL)m_head->prev=item;
    item->next=m_head;
    m_head=item;
    item->prev=NULL;
    return;
  }
    
  ITEM* i=m_head;
  while( i->next!=NULL && item->time > i->next->time)
    i=i->next;
  item->next=i->next;
  if(i->next!=NULL)i->next->prev=item;
  i->next=item;
  item->prev=i;

}

template <class ITEM>
ITEM* SimpleQueue<ITEM> ::DeQueue()
{
  if(m_head==NULL)return NULL;
  ITEM* item = m_head;
  m_head=m_head->next;
  if(m_head!=NULL)m_head->prev=NULL;
  return item;
}

template <class ITEM>
void SimpleQueue<ITEM>::Delete(ITEM* item)
{
  if(item==NULL) return;

  if(item==m_head)
  {
    m_head=m_head->next;
    if(m_head!=NULL)m_head->prev=NULL;
  }
  else
  {
    item->prev->next=item->next;
    if(item->next!=NULL)
      item->next->prev=item->prev;
  }

}

template <class ITEM>
class GuardedQueue : public SimpleQueue<ITEM>
{
 public:
  void Delete(ITEM*);
  void EnQueue(ITEM*);
  bool Validate(const char*);
};
template <class ITEM>
void GuardedQueue<ITEM>::EnQueue(ITEM* item)
{

  ITEM* i=SimpleQueue<ITEM>::m_head;
  while(i!=NULL)
  {
    if(i==item)
    {
      pthread_printf("queue error: item %f(%p) is already in the queue\n",item->time,item);
    }
    i=i->next;
  }
  SimpleQueue<ITEM>::EnQueue(item);
}

template <class ITEM>
void GuardedQueue<ITEM>::Delete(ITEM* item)
{
  ITEM* i=SimpleQueue<ITEM>::m_head;
  while(i!=item&&i!=NULL)
    i=i->next;
  if(i==NULL)
    pthread_printf("error: cannot find the to-be-deleted event %f(%p)\n",item->time,item);
  else
    SimpleQueue<ITEM>::Delete(item);
}

template <class ITEM>
bool GuardedQueue<ITEM>::Validate(const char* s)
{
  char out[1000],buff[100];

  ITEM* i=SimpleQueue<ITEM>::m_head;
  bool qerror=false;

  sprintf(out,"queue error %s : ",s);
  while(i!=NULL)
  {
    sprintf(buff,"%f ",i->time);
    strcat(out,buff);
    if(i->next!=NULL)
      if(i->next->prev!=i)
      {
	qerror=true;
	sprintf(buff," {broken} ");
	strcat(out,buff);
      }
    if(i==i->next)
    {
      qerror=true;
      sprintf(buff,"{loop}");
      strcat(out,buff);
      break;
    }
    i=i->next;
  }
  if(qerror)
    printf("%s\n",out);
  return qerror;
}

template <class ITEM>
class ErrorQueue : public SimpleQueue<ITEM>
{
 public:
  ITEM* DeQueue(double);
  const char* GetName();
};

template <class ITEM>
const char* ErrorQueue<ITEM>::GetName()
{
  static const char* name = "ErrorQueue";
  return name;
}

template <class ITEM>
ITEM* ErrorQueue<ITEM> ::DeQueue(double stoptime)
{
  

  if(drand48()>0.5)
    return SimpleQueue<ITEM>::DeQueue();

  int s=0;
  ITEM* e;
  e=SimpleQueue<ITEM>::m_head;
  while(e!=NULL&&e->time<stoptime)
  {
    s++;
    e=e->next;
  }
  e=SimpleQueue<ITEM>::m_head;
  s=(int)(s*drand48());
  while(s!=0)
  {
    e=e->next;
    s--;
  }
  Delete(e);
  return e;
}

template < class ITEM >
class HeapQueue 
{
 public:
  HeapQueue();
  ~HeapQueue();
  void EnQueue(ITEM*);
  ITEM* DeQueue();
  void Delete(ITEM*);
  const char* GetName();
  ITEM* NextEvent() const { return num_of_elems?elems[0]:NULL; };
 private:
  void SiftDown(int);
  void PercolateUp(int);
  void Validate(const char*);
        
  ITEM** elems;
  int num_of_elems;
  int curr_max;
};

template <class ITEM>
const char* HeapQueue<ITEM>::GetName()
{
  static const char* name = "HeapQueue";
  return name;
}

template <class ITEM>
void HeapQueue<ITEM>::Validate(const char* s)
{
  int i,j;
  char out[1000],buff[100];
  for(i=0;i<num_of_elems;i++)
    if(  ((2*i+1)<num_of_elems&&elems[i]->time>elems[2*i+1]->time) ||
	 ((2*i+2)<num_of_elems&&elems[i]->time>elems[2*i+2]->time) )
    {
      sprintf(out,"queue error %s : ",s);
      for(j=0;j<num_of_elems;j++)
      {
	if(i!=j)
	  sprintf(buff,"%f(%d) ",elems[j]->time,j);
	else
	  sprintf(buff,"{%f(%d)} ",elems[j]->time,j);
	strcat(out,buff);
      }
      printf("%s\n",out);
    }
}
template <class ITEM>
HeapQueue<ITEM>::HeapQueue()
{
  curr_max=16;
  elems=new ITEM*[curr_max];
  num_of_elems=0;
}
template <class ITEM>
HeapQueue<ITEM>::~HeapQueue()
{
  delete [] elems;
}
template <class ITEM>
void HeapQueue<ITEM>::SiftDown(int node)
{
  if(num_of_elems<=1) return;
  int i=node,k,c1,c2;
  ITEM* temp;
        
  do{
    k=i;
    c1=c2=2*i+1;
    c2++;
    if(c1<num_of_elems && elems[c1]->time < elems[i]->time)
      i=c1;
    if(c2<num_of_elems && elems[c2]->time < elems[i]->time)
      i=c2;
    if(k!=i)
    {
      temp=elems[i];
      elems[i]=elems[k];
      elems[k]=temp;
      elems[k]->pos=k;
      elems[i]->pos=i;
    }
  }while(k!=i);
}
template <class ITEM>
void HeapQueue<ITEM>::PercolateUp(int node)
{
  int i=node,k,p;
  ITEM* temp;
        
  do{
    k=i;
    if( (p=(i+1)/2) != 0)
    {
      --p;
      if(elems[i]->time < elems[p]->time)
      {
	i=p;
	temp=elems[i];
	elems[i]=elems[k];
	elems[k]=temp;
	elems[k]->pos=k;
	elems[i]->pos=i;
      }
    }
  }while(k!=i);
}

template <class ITEM>
void HeapQueue<ITEM>::EnQueue(ITEM* item)
{
  if(num_of_elems>=curr_max)
  {
    curr_max*=2;
    ITEM** buffer=new ITEM*[curr_max];
    for(int i=0;i<num_of_elems;i++)
      buffer[i]=elems[i];
    delete[] elems;
    elems=buffer;
  }
        
  elems[num_of_elems]=item;
  elems[num_of_elems]->pos=num_of_elems;
  num_of_elems++;
  PercolateUp(num_of_elems-1);
}

template <class ITEM>
ITEM* HeapQueue<ITEM>::DeQueue()
{
  if(num_of_elems<=0)return NULL;
        
  ITEM* item=elems[0];
  num_of_elems--;
  elems[0]=elems[num_of_elems];
  elems[0]->pos=0;
  SiftDown(0);
  return item;
}

template <class ITEM>
void HeapQueue<ITEM>::Delete(ITEM* item)
{
  int i=item->pos;

  num_of_elems--;
  elems[i]=elems[num_of_elems];
  elems[i]->pos=i;
  SiftDown(i);
  PercolateUp(i);
}



#define CQ_MAX_SAMPLES 25

template <class ITEM>
class CalendarQueue 
{
 public:
  CalendarQueue();
  const char* GetName();
  ~CalendarQueue();
  void enqueue(ITEM*);
  ITEM* dequeue();
  void EnQueue(ITEM*);
  ITEM* DeQueue();
  ITEM* NextEvent() const { return m_head;}
  void Delete(ITEM*);
 private:
  long last_bucket,number_of_buckets;
  double bucket_width;
        
  void ReSize(long);
  double NewWidth();

  ITEM ** buckets;
  long total_number;
  double bucket_top;
  long bottom_threshold;
  long top_threshold;
  double last_priority;
  bool resizable;

  ITEM* m_head;
  char m_name[100];
};


template <class ITEM>
const char* CalendarQueue<ITEM> :: GetName()
{
  sprintf(m_name,"Calendar Queue (bucket width: %.2e, size: %ld) ",
	  bucket_width,number_of_buckets);
  return m_name;
}
template <class ITEM>
CalendarQueue<ITEM>::CalendarQueue()
{
  long i;
        
  number_of_buckets=16;
  bucket_width=1.0;
  bucket_top=bucket_width;
  total_number=0;
  last_bucket=0;
  last_priority=0.0;
  top_threshold=number_of_buckets*2;
  bottom_threshold=number_of_buckets/2-2;
  resizable=true;
        
  buckets= new ITEM*[number_of_buckets];
  for(i=0;i<number_of_buckets;i++)
    buckets[i]=NULL;
  m_head=NULL;

}
template <class ITEM>
CalendarQueue<ITEM>::~CalendarQueue()
{
  delete [] buckets;
}
template <class ITEM>
void CalendarQueue<ITEM>::ReSize(long newsize)
{
  long i;
  ITEM** old_buckets=buckets;
  long old_number=number_of_buckets;
        
  resizable=false;
  bucket_width=NewWidth();
  buckets= new ITEM*[newsize];
  number_of_buckets=newsize;
  for(i=0;i<newsize;i++)
    buckets[i]=NULL;
  last_bucket=0;
  total_number=0;

  
        
  ITEM *item;
  for(i=0;i<old_number;i++)
  {
    while(old_buckets[i]!=NULL)
    {
      item=old_buckets[i];
      old_buckets[i]=item->next;
      enqueue(item);
    }
  }
  resizable=true;
  delete[] old_buckets;
  number_of_buckets=newsize;
  top_threshold=number_of_buckets*2;
  bottom_threshold=number_of_buckets/2-2;
  bucket_top=bucket_width*((long)(last_priority/bucket_width)+1)+bucket_width*0.5;
  last_bucket = long(last_priority/bucket_width) % number_of_buckets;

}
template <class ITEM>
ITEM* CalendarQueue<ITEM>::DeQueue()
{
  ITEM* head=m_head;
  m_head=dequeue();
  return head;
}
template <class ITEM>
ITEM* CalendarQueue<ITEM>::dequeue()
{
  long i;
  for(i=last_bucket;;)
  {
    if(buckets[i]!=NULL&&buckets[i]->time<bucket_top)
    {
      ITEM * item=buckets[i];
      buckets[i]=buckets[i]->next;
      total_number--;
      last_bucket=i;
      last_priority=item->time;
                        
      if(resizable&&total_number<bottom_threshold)
	ReSize(number_of_buckets/2);
      item->next=NULL;
      return item;
    }
    else
    {
      i++;
      if(i==number_of_buckets)i=0;
      bucket_top+=bucket_width;
      if(i==last_bucket)
	break;
    }
  }
        
  
  long smallest;
  for(smallest=0;smallest<number_of_buckets;smallest++)
    if(buckets[smallest]!=NULL)break;

  if(smallest >= number_of_buckets)
  {
    last_priority=bucket_top;
    return NULL;
  }

  for(i=smallest+1;i<number_of_buckets;i++)
  {
    if(buckets[i]==NULL)
      continue;
    else
      if(buckets[i]->time<buckets[smallest]->time)
	smallest=i;
  }
  ITEM * item=buckets[smallest];
  buckets[smallest]=buckets[smallest]->next;
  total_number--;
  last_bucket=smallest;
  last_priority=item->time;
  bucket_top=bucket_width*((long)(last_priority/bucket_width)+1)+bucket_width*0.5;
  item->next=NULL;
  return item;
}
template <class ITEM>
void CalendarQueue<ITEM>::EnQueue(ITEM* item)
{
  
  if(m_head==NULL)
  {
    m_head=item;
    return;
  }
  if(m_head->time>item->time)
  {
    enqueue(m_head);
    m_head=item;
  }
  else
    enqueue(item);
}
template <class ITEM>
void CalendarQueue<ITEM>::enqueue(ITEM* item)
{
  long i;
  if(item->time<last_priority)
  {
    i=(long)(item->time/bucket_width);
    last_priority=item->time;
    bucket_top=bucket_width*(i+1)+bucket_width*0.5;
    i=i%number_of_buckets;
    last_bucket=i;
  }
  else
  {
    i=(long)(item->time/bucket_width);
    i=i%number_of_buckets;
  }

        
  

  if(buckets[i]==NULL||item->time<buckets[i]->time)
  {
    item->next=buckets[i];
    buckets[i]=item;
  }
  else
  {

    ITEM* pos=buckets[i];
    while(pos->next!=NULL&&item->time>pos->next->time)
    {
      pos=pos->next;
    }
    item->next=pos->next;
    pos->next=item;
  }
  total_number++;
  if(resizable&&total_number>top_threshold)
    ReSize(number_of_buckets*2);
}
template <class ITEM>
void CalendarQueue<ITEM>::Delete(ITEM* item)
{
  if(item==m_head)
  {
    m_head=dequeue();
    return;
  }
  long j;
  j=(long)(item->time/bucket_width);
  j=j%number_of_buckets;
        
  

  
  

  ITEM** p = &buckets[j];
  
  ITEM* i=buckets[j];
    
  while(i!=NULL)
  {
    if(i==item)
    { 
      (*p)=item->next;
      total_number--;
      if(resizable&&total_number<bottom_threshold)
	ReSize(number_of_buckets/2);
      return;
    }
    p=&(i->next);
    i=i->next;
  }   
}
template <class ITEM>
double CalendarQueue<ITEM>::NewWidth()
{
  long i, nsamples;
        
  if(total_number<2) return 1.0;
  if(total_number<=5)
    nsamples=total_number;
  else
    nsamples=5+total_number/10;
  if(nsamples>CQ_MAX_SAMPLES) nsamples=CQ_MAX_SAMPLES;
        
  long _last_bucket=last_bucket;
  double _bucket_top=bucket_top;
  double _last_priority=last_priority;
        
  double AVG[CQ_MAX_SAMPLES],avg1=0,avg2=0;
  ITEM* list,*next,*item;
        
  list=dequeue(); 
  long real_samples=0;
  while(real_samples<nsamples)
  {
    item=dequeue();
    if(item==NULL)
    {
      item=list;
      while(item!=NULL)
      {
	next=item->next;
	enqueue(item);
	item=next;      
      }

      last_bucket=_last_bucket;
      bucket_top=_bucket_top;
      last_priority=_last_priority;

                        
      return 1.0;
    }
    AVG[real_samples]=item->time-list->time;
    avg1+=AVG[real_samples];
    if(AVG[real_samples]!=0.0)
      real_samples++;
    item->next=list;
    list=item;
  }
  item=list;
  while(item!=NULL)
  {
    next=item->next;
    enqueue(item);
    item=next;      
  }
        
  last_bucket=_last_bucket;
  bucket_top=_bucket_top;
  last_priority=_last_priority;
        
  avg1=avg1/(double)(real_samples-1);
  avg1=avg1*2.0;
        
  
  long count=0;
  for(i=0;i<real_samples-1;i++)
  {
    if(AVG[i]<avg1&&AVG[i]!=0)
    {
      avg2+=AVG[i];
      count++;
    }
  }
  if(count==0||avg2==0)   return 1.0;
        
  avg2 /= (double) count;
  avg2 *= 3.0;
        
  return avg2;
}

#endif /*PRIORITY_QUEUE_H*/

#line 15 "../../common/cost.h"


#line 1 "../../common/corsa_alloc.h"
































#ifndef corsa_allocator_h
#define corsa_allocator_h

#include <typeinfo>
#include <string>

class CorsaAllocator
{
private:
    struct DT{
#ifdef CORSA_DEBUG
	DT* self;
#endif
	DT* next;
    };
public:
    CorsaAllocator(unsigned int );         
    CorsaAllocator(unsigned int, int);     
    ~CorsaAllocator();		
    void *alloc();		
    void free(void*);
    unsigned int datasize() 
    {
#ifdef CORSA_DEBUG
	return m_datasize-sizeof(DT*);
#else
	return m_datasize; 
#endif
    }
    int size() { return m_size; }
    int capacity() { return m_capacity; }			
    
    const char* GetName() { return m_name.c_str(); }
    void SetName( const char* name) { m_name=name; } 

private:
    CorsaAllocator(const CorsaAllocator& ) {}  
    void Setup(unsigned int,int); 
    void InitSegment(int);
  
    unsigned int m_datasize;
    char** m_segments;	          
    int m_segment_number;         
    int m_segment_max;      
    int m_segment_size;	          
				  
    DT* m_free_list; 
    int m_size;
    int m_capacity;

    int m_free_times,m_alloc_times;
    int m_max_allocs;

    std::string m_name;
};
#ifndef CORSA_NODEF
CorsaAllocator::CorsaAllocator(unsigned int datasize)
{
    Setup(datasize,256);	  
}

CorsaAllocator::CorsaAllocator(unsigned int datasize, int segsize)
{
    Setup(datasize,segsize);
}

CorsaAllocator::~CorsaAllocator()
{
    #ifdef CORSA_DEBUG
    printf("%s -- alloc: %d, free: %d, max: %d\n",GetName(),
	   m_alloc_times,m_free_times,m_max_allocs);
    #endif

    for(int i=0;i<m_segment_number;i++)
	delete[] m_segments[i];	   
    delete[] m_segments;			
}

void CorsaAllocator::Setup(unsigned int datasize,int seg_size)
{

    char buffer[50];
    sprintf(buffer,"%s[%d]",typeid(*this).name(),datasize);
    m_name = buffer;

#ifdef CORSA_DEBUG
    datasize+=sizeof(DT*);  
#endif

    if(datasize<sizeof(DT))datasize=sizeof(DT);
    m_datasize=datasize;
    if(seg_size<16)seg_size=16;    
    m_segment_size=seg_size;			
    m_segment_number=1;		   
    m_segment_max=seg_size;	   
    m_segments= new char* [ m_segment_max ] ;   
    m_segments[0]= new char [m_segment_size*m_datasize];  

    m_size=0;
    m_capacity=0;
    InitSegment(0);

    m_free_times=m_alloc_times=m_max_allocs=00;
}

void CorsaAllocator::InitSegment(int s)
{
    char* p=m_segments[s];
    m_free_list=reinterpret_cast<DT*>(p);
    for(int i=0;i<m_segment_size-1;i++,p+=m_datasize)
    {
	reinterpret_cast<DT*>(p)->next=
	    reinterpret_cast<DT*>(p+m_datasize);
    }
    reinterpret_cast<DT*>(p)->next=NULL;
    m_capacity+=m_segment_size;
}

void* CorsaAllocator::alloc()
{
    #ifdef CORSA_DEBUG
    m_alloc_times++;
    if(m_alloc_times-m_free_times>m_max_allocs)
	m_max_allocs=m_alloc_times-m_free_times;
    #endif
    if(m_free_list==NULL)	
    
    {
	int i;
	if(m_segment_number==m_segment_max)	
	
	
	{
	    m_segment_max*=2;		
	    char** buff;
	    buff=new char* [m_segment_max];   
#ifdef CORSA_DEBUG
	    if(buff==NULL)
	    {
		printf("CorsaAllocator runs out of memeory.\n");
		exit(1);
	    }
#endif
	    for(i=0;i<m_segment_number;i++)
		buff[i]=m_segments[i];	
	    delete [] m_segments;		
	    m_segments=buff;
	}
	m_segment_size*=2;
	m_segments[m_segment_number]=new char[m_segment_size*m_datasize];
#ifdef CORSA_DEBUG
	    if(m_segments[m_segment_number]==NULL)
	    {
		printf("CorsaAllocator runs out of memeory.\n");
		exit(1);
	    }
#endif
	InitSegment(m_segment_number);
	m_segment_number++;
    }

    DT* item=m_free_list;		
    m_free_list=m_free_list->next;
    m_size++;

#ifdef CORSA_DEBUG
    item->self=item;
    char* p=reinterpret_cast<char*>(item);
    p+=sizeof(DT*);
    
    return static_cast<void*>(p);
#else
    return static_cast<void*>(item);
#endif
}

void CorsaAllocator::free(void* data)
{
#ifdef CORSA_DEBUG
    m_free_times++;
    char* p=static_cast<char*>(data);
    p-=sizeof(DT*);
    DT* item=reinterpret_cast<DT*>(p);
    
    if(item!=item->self)
    {
	if(item->self==(DT*)0xabcd1234)
	    printf("%s: packet at %p has already been released\n",GetName(),p+sizeof(DT*)); 
	else
	    printf("%s: %p is probably not a pointer to a packet\n",GetName(),p+sizeof(DT*));
    }
    assert(item==item->self);
    item->self=(DT*)0xabcd1234;
#else
    DT* item=static_cast<DT*>(data);
#endif

    item->next=m_free_list;
    m_free_list=item;
    m_size--;
}
#endif /* CORSA_NODEF */

#endif /* corsa_allocator_h */

#line 16 "../../common/cost.h"


class trigger_t {};
typedef double simtime_t;

#ifdef COST_DEBUG
#define Printf(x) Print x
#else
#define Printf(x)
#endif



class TimerBase;



struct CostEvent
{
  double time;
  CostEvent* next;
  union {
    CostEvent* prev;
    int pos;  
  };
  TimerBase* object;
  int index;
  unsigned char active;
};



class TimerBase
{
 public:
  virtual void activate(CostEvent*) = 0;
  inline virtual ~TimerBase() {}	
};

class TypeII;



class CostSimEng
{
 public:

  class seed_t
      {
       public:
	void operator = (long seed) { srand48(seed); };
      };
  seed_t		Seed;
  CostSimEng() 
      : stopTime( 0), clearStatsTime( 0)
      {
        if( m_instance == NULL)
	  m_instance = this;
        else
	  printf("Error: only one simulation engine can be created\n");
      }
  virtual		~CostSimEng()	{ }
  static CostSimEng	*Instance()
      {
        if(m_instance==NULL)
        {
	  printf("Error: a simulation engine has not been initialized\n");
	  m_instance = new CostSimEng;
        }
        return m_instance;
      }
  CorsaAllocator	*GetAllocator(unsigned int datasize)
      {
    	for(unsigned int i=0;i<m_allocators.size();i++)
    	{
	  if(m_allocators[i]->datasize()==datasize)return m_allocators[i];
    	} 
    	CorsaAllocator* allocator=new CorsaAllocator(datasize);
    	char buffer[25];
    	sprintf(buffer,"EventAllocator[%d]",datasize);
    	allocator->SetName(buffer);
    	m_allocators.push_back(allocator);
    	return allocator;
      }
  void		AddComponent(TypeII*c)
      {
        m_components.push_back(c);
      }
  void		ScheduleEvent(CostEvent*e)
      {
        assert(e->time>=m_clock);
        
        m_queue.EnQueue(e);
      }
  void		CancelEvent(CostEvent*e)
      {
        
        m_queue.Delete(e);
      }
  double	Random( double v=1.0)	{ return v*drand48();}
  int		Random( int v)		{ return (int)(v*drand48()); }
  double	Exponential(double mean)	{ return -mean*log(Random());}
  virtual void	Start()		{}
  virtual void	Stop()		{}
  void		Run();
  double	SimTime()	{ return m_clock; } 
  void		StopTime( double t)	{ stopTime = t; }
  double	StopTime() const	{ return stopTime; }
  void		ClearStatsTime( double t)	{ clearStatsTime = t; }
  double	ClearStatsTime() const	{ return clearStatsTime; }
  virtual void	ClearStats()	{}
 private:
  double	stopTime;
  double	clearStatsTime;	
  double	eventRate;
  double	runningTime;
  long		eventsProcessed;
  double	m_clock;
  queue_t<CostEvent>	m_queue;
  std::vector<TypeII*>	m_components;
  static CostSimEng	*m_instance;
  std::vector<CorsaAllocator*>	m_allocators;
};




class TypeII
{
 public: 
  virtual void Start() {};
  virtual void Stop() {};
  inline virtual ~TypeII() {}		
  TypeII()
      {
        m_simeng=CostSimEng::Instance();
        m_simeng->AddComponent(this);
      }

#ifdef COST_DEBUG
  void Print(const bool, const char*, ...);
#endif
    
  double Random(double v=1.0) { return v*drand48();}
  int Random(int v) { return (int)(v*drand48());}
  double Exponential(double mean) { return -mean*log(Random());}
  inline double SimTime() const { return m_simeng->SimTime(); }
  inline double StopTime() const { return m_simeng->StopTime(); }
 private:
  CostSimEng* m_simeng;
}; 

#ifdef COST_DEBUG
void TypeII::Print(const bool flag, const char* format, ...)
{
  if(flag==false) return;
  va_list ap;
  va_start(ap, format);
  printf("[%.10f] ",SimTime());
  vprintf(format,ap);
  va_end(ap);
}
#endif

CostSimEng* CostSimEng::m_instance = NULL;

void CostSimEng::Run()
{
  double	nextTime = (clearStatsTime != 0.0 && clearStatsTime < stopTime) ? clearStatsTime : stopTime;

  m_clock = 0.0;
  eventsProcessed = 0l;
  std::vector<TypeII*>::iterator iter;
      
  struct timeval start_time;    
  gettimeofday( &start_time, NULL);

  Start();
    
  for( iter = m_components.begin(); iter != m_components.end(); iter++)
    (*iter)->Start();

  CostEvent* e=m_queue.DeQueue();
  while( e != NULL)
  {
    if( e->time >= nextTime)
    {
      if( nextTime == stopTime)
	break;
      
      printf( "Clearing statistics @ %f\n", nextTime);
      nextTime = stopTime;
      ClearStats();
    }
    
    assert( e->time >= m_clock);
    m_clock = e->time;
    e->object->activate( e);
    eventsProcessed++;
    e = m_queue.DeQueue();
  }
  m_clock = stopTime;
  for(iter = m_components.begin(); iter != m_components.end(); iter++)
    (*iter)->Stop();
	    
  Stop();

  struct timeval stop_time;    
  gettimeofday(&stop_time,NULL);

  runningTime = stop_time.tv_sec - start_time.tv_sec +
      (stop_time.tv_usec - start_time.tv_usec) / 1000000.0;
  eventRate = eventsProcessed/runningTime;

  printf("---------------------------------------------------------------------------\n");	
  printf("CostSimEng with %s, stopped at %f\n ", m_queue.GetName(), stopTime);	
  printf("%ld events processed in %.3f seconds, event processing rate: %.0f\n",	
	 eventsProcessed, runningTime, eventRate);
}







#line 42 "mmm1.cc"




struct packet_t
{
    int seq_number;
    int index;
    double arrival_time,departure_time;
};



#include "compcxx_mmm1.h"

#line 97 "mmm1.cc"
/*template < class DATATYPE >
*/
#line 97 "mmm1.cc"
class compcxx_FIFO_5 : public compcxx_component, public TypeII
{
public:
    compcxx_FIFO_5();
    virtual ~compcxx_FIFO_5();
    void Start();

    unsigned int queue_length;

    /*inport */void in(packet_t & p);
    class my_FIFO_out_f_t:public compcxx_functor<FIFO_out_f_t>{ public:void  operator() (packet_t & p) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(p); return (c[0]->*f[0])(p);};};my_FIFO_out_f_t out_f;/*outport void out(packet_t & p)*/;
    /*inport */void next();

private:
    bool m_busy;
    std::deque<packet_t > m_queue;
};

class compcxx_MultiServer_6;
#line 303 "../../common/cost.h"
/*template <class T> */
#line 303 "../../common/cost.h"
class compcxx_MultiTimer_3 : public compcxx_component, public TimerBase
{
 public:
  struct event_t : public CostEvent { trigger_t data; };
  compcxx_MultiTimer_3();
  virtual ~compcxx_MultiTimer_3();
	
  /*outport void to_component(trigger_t &, unsigned int i)*/;
 
  inline void Set(double t, unsigned int index =0);
  inline void Set( trigger_t const & data, double t,unsigned int index=0);
  void Cancel (unsigned int index=0);
  void activate(CostEvent*event);

  inline bool Active(unsigned int index=0) { return GetEvent(index)->active; }
  inline double GetTime(unsigned int index=0) { return GetEvent(index)->time; }
  inline trigger_t & GetData(unsigned int index=0) { return GetEvent(index)->data; }
  inline void SetData(trigger_t const &d, unsigned int index) { GetEvent(index)->data = d; }

  event_t* GetEvent(unsigned int index);
				 
 private:
  std::vector<event_t*> m_events;
  CostSimEng* m_simeng;
public:compcxx_MultiServer_6* p_compcxx_parent;};


#line 215 "mmm1.cc"
/*template <class DATATYPE>
*/
#line 215 "mmm1.cc"
class compcxx_MultiServer_6 : public compcxx_component, public TypeII
{
public:
    compcxx_MultiServer_6();
	virtual ~compcxx_MultiServer_6() ;
	void Setup();
	
    double service_time;
    int size;

    /*inport */void in(packet_t & p);
    class my_MultiServer_out_f_t:public compcxx_functor<MultiServer_out_f_t>{ public:void  operator() (packet_t & p) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(p); return (c[0]->*f[0])(p);};};compcxx_array<my_MultiServer_out_f_t > out;/*outportvoid out(packet_t & p)*/;
    class my_MultiServer_next_f_t:public compcxx_functor<MultiServer_next_f_t>{ public:void  operator() () { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(); return (c[0]->*f[0])();};};compcxx_array<my_MultiServer_next_f_t > next;/*outportvoid next()*/;
    /*inport */void depart(trigger_t&, unsigned int);








    compcxx_MultiTimer_3/*<trigger_t> */wait;

private:
    packet_t * m_packets;
};


#line 164 "mmm1.cc"
class compcxx_Sink_7 : public compcxx_component, public TypeII
{
public:
    /*inport */void in(packet_t&);

    void Start()
    {
        m_total=0.0;
        m_number=0;
    }
    compcxx_Sink_7() {}
    void Stop()
    {
        printf("Average packet delay is: %f (%d packets) \n",
	       m_total/m_number,m_number);
    }
private:
    double m_total;
    int m_number;
    packet_t m_packet;
};

class compcxx_Source_4;
#line 303 "../../common/cost.h"
/*template <class T> */
#line 303 "../../common/cost.h"
class compcxx_MultiTimer_2 : public compcxx_component, public TimerBase
{
 public:
  struct event_t : public CostEvent { trigger_t data; };
  compcxx_MultiTimer_2();
  virtual ~compcxx_MultiTimer_2();
	
  /*outport void to_component(trigger_t &, unsigned int i)*/;
 
  inline void Set(double t, unsigned int index =0);
  inline void Set( trigger_t const & data, double t,unsigned int index=0);
  void Cancel (unsigned int index=0);
  void activate(CostEvent*event);

  inline bool Active(unsigned int index=0) { return GetEvent(index)->active; }
  inline double GetTime(unsigned int index=0) { return GetEvent(index)->time; }
  inline trigger_t & GetData(unsigned int index=0) { return GetEvent(index)->data; }
  inline void SetData(trigger_t const &d, unsigned int index) { GetEvent(index)->data = d; }

  event_t* GetEvent(unsigned int index);
				 
 private:
  std::vector<event_t*> m_events;
  CostSimEng* m_simeng;
public:compcxx_Source_4* p_compcxx_parent;};


#line 55 "mmm1.cc"
class compcxx_Source_4 : public compcxx_component, public TypeII
{
public:
    double interval;
    int index;
    class my_Source_out_f_t:public compcxx_functor<Source_out_f_t>{ public:void  operator() (packet_t& p) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(p); return (c[0]->*f[0])(p);};};my_Source_out_f_t out_f;/*outport void out (packet_t& p)*/;
    compcxx_MultiTimer_2 /*<trigger_t> */wait;

public:
    compcxx_Source_4();
    virtual ~compcxx_Source_4() {}
    void Start();
    /*inport */void create(trigger_t& t, unsigned int);

private:
	packet_t m_packet;
    int m_seq_number;
};


#line 307 "mmm1.cc"
class compcxx_MMM1_8 : public compcxx_component, public CostSimEng
{
public:
    void Setup();

	int size;
    double interval;
    int queue_length;
    double service_time;

private:

    compcxx_array<compcxx_Source_4 >source;
    compcxx_array<compcxx_FIFO_5 /*<packet_t> */ >fifo;
    compcxx_MultiServer_6 /*<packet_t> */server;
    compcxx_array<compcxx_Sink_7 >sink;
};









#line 117 "mmm1.cc"
compcxx_FIFO_5 
#line 116 "mmm1.cc"
/*template < class DATATYPE >
*//*<DATATYPE> */:: compcxx_FIFO_5()
{
}


#line 122 "mmm1.cc"
compcxx_FIFO_5 
#line 121 "mmm1.cc"
/*template < class DATATYPE >
*//*<DATATYPE> */:: ~compcxx_FIFO_5()
{
}


#line 126 "mmm1.cc"

#line 126 "mmm1.cc"
/*template < class DATATYPE >
*/void compcxx_FIFO_5 /*<packet_t > */:: Start()
{
    m_busy=false;
}


#line 132 "mmm1.cc"

#line 132 "mmm1.cc"
/*template < class DATATYPE >
*/void compcxx_FIFO_5/*<packet_t >*/::in(packet_t & packet)
{
    if (!m_busy)
    { 
        (out_f(packet));
        m_busy=true;
    } 
    else if (m_queue.size()<queue_length)
    { 
        	m_queue.push_back(packet);
    }
    return;
}


#line 147 "mmm1.cc"

#line 147 "mmm1.cc"
/*template < class DATATYPE >
*/void compcxx_FIFO_5 /*<packet_t > */:: next()
{
    if (m_queue.size()>0)
    {
        (out_f(m_queue.front()));
        m_queue.pop_front();
    } 
    else
    {
        m_busy=false;
    }
    return;
}




#line 330 "../../common/cost.h"
compcxx_MultiTimer_3
#line 329 "../../common/cost.h"
/*template <class T>
*//*<T>*/::compcxx_MultiTimer_3()
{
  m_simeng = CostSimEng::Instance(); 
  GetEvent(0);
}


#line 337 "../../common/cost.h"
compcxx_MultiTimer_3
#line 336 "../../common/cost.h"
/*template <class T>
*//*<T>*/::~compcxx_MultiTimer_3()
{
  for(unsigned int i=0;i<m_events.size();i++)
    delete m_events[i];
}


#line 343 "../../common/cost.h"
/*template <class T>
*//*typename */compcxx_MultiTimer_3/*<T>*/::event_t* compcxx_MultiTimer_3/*<trigger_t >*/::GetEvent(unsigned int index)
{
  if (index>=m_events.size())
  {
    for (unsigned int i=m_events.size();i<=index;i++)
    {
      m_events.push_back(new event_t);
      m_events[i]->active=false;
      m_events[i]->index=i;
    }
  }
  return m_events[index];
}


#line 358 "../../common/cost.h"

#line 358 "../../common/cost.h"
/*template <class T>
*/void compcxx_MultiTimer_3/*<trigger_t >*/::Set(trigger_t const & data, double time, unsigned int index)
{
  event_t * e = GetEvent(index);
  if(e->active)m_simeng->CancelEvent(e);
  e->time = time;
  e->data = data;
  e->object = this;
  e->active = true;
  m_simeng->ScheduleEvent(e);
}


#line 370 "../../common/cost.h"

#line 370 "../../common/cost.h"
/*template <class T>
*/void compcxx_MultiTimer_3/*<trigger_t >*/::Set(double time, unsigned int index)
{
  event_t * e = GetEvent(index);
  if(e->active)m_simeng->CancelEvent(e);
  e->time = time;
  e->object = this;
  e->active = true;
  m_simeng->ScheduleEvent(e);
}


#line 381 "../../common/cost.h"

#line 381 "../../common/cost.h"
/*template <class T>
*/void compcxx_MultiTimer_3/*<trigger_t >*/::Cancel(unsigned int index)
{
  event_t * e = GetEvent(index);
  if(e->active)
    m_simeng->CancelEvent(e);
  e->active = false;
}


#line 390 "../../common/cost.h"

#line 390 "../../common/cost.h"
/*template <class T>
*/void compcxx_MultiTimer_3/*<trigger_t >*/::activate(CostEvent*e)
{
  event_t * event = (event_t*)e;
  event->active = false;
  (p_compcxx_parent->depart(event->data,event->index));
}





#line 245 "mmm1.cc"
compcxx_MultiServer_6
#line 244 "mmm1.cc"
/*template <class DATATYPE>
*//*<DATATYPE>*/::compcxx_MultiServer_6()
{
    wait.p_compcxx_parent=this /*connect wait.to_component,*/;
}


#line 251 "mmm1.cc"
compcxx_MultiServer_6
#line 250 "mmm1.cc"
/*template <class DATATYPE>
*//*<DATATYPE>*/::~compcxx_MultiServer_6()
{
    delete[] m_packets;
}










#line 264 "mmm1.cc"

#line 264 "mmm1.cc"
/*template <class DATATYPE>
*/void compcxx_MultiServer_6/*<packet_t >*/::Setup()
{
    m_packets = new packet_t [ size ];
    out.SetSize(size);
    next.SetSize(size);
}









#line 279 "mmm1.cc"

#line 279 "mmm1.cc"
/*template <class DATATYPE>
*/void compcxx_MultiServer_6/*<packet_t > */:: in(packet_t & packet)
{
	int index = packet.index;
    m_packets[index]=packet;
    wait.Set(SimTime()+Exponential(service_time),index);
    return;
}







#line 293 "mmm1.cc"

#line 293 "mmm1.cc"
/*template <class DATATYPE>
*/void compcxx_MultiServer_6 /*<packet_t > */:: depart(trigger_t&, unsigned int i)
{
    out[i](m_packets[i]);
    next[i]();
    return;
}








#line 186 "mmm1.cc"
void compcxx_Sink_7::in(packet_t &packet)
{
    m_packet=packet;
    m_packet.departure_time=SimTime();
    m_total+=m_packet.departure_time-m_packet.arrival_time;
    m_number++;
    return;
}






















#line 330 "../../common/cost.h"
compcxx_MultiTimer_2
#line 329 "../../common/cost.h"
/*template <class T>
*//*<T>*/::compcxx_MultiTimer_2()
{
  m_simeng = CostSimEng::Instance(); 
  GetEvent(0);
}


#line 337 "../../common/cost.h"
compcxx_MultiTimer_2
#line 336 "../../common/cost.h"
/*template <class T>
*//*<T>*/::~compcxx_MultiTimer_2()
{
  for(unsigned int i=0;i<m_events.size();i++)
    delete m_events[i];
}


#line 343 "../../common/cost.h"
/*template <class T>
*//*typename */compcxx_MultiTimer_2/*<T>*/::event_t* compcxx_MultiTimer_2/*<trigger_t >*/::GetEvent(unsigned int index)
{
  if (index>=m_events.size())
  {
    for (unsigned int i=m_events.size();i<=index;i++)
    {
      m_events.push_back(new event_t);
      m_events[i]->active=false;
      m_events[i]->index=i;
    }
  }
  return m_events[index];
}


#line 358 "../../common/cost.h"

#line 358 "../../common/cost.h"
/*template <class T>
*/void compcxx_MultiTimer_2/*<trigger_t >*/::Set(trigger_t const & data, double time, unsigned int index)
{
  event_t * e = GetEvent(index);
  if(e->active)m_simeng->CancelEvent(e);
  e->time = time;
  e->data = data;
  e->object = this;
  e->active = true;
  m_simeng->ScheduleEvent(e);
}


#line 370 "../../common/cost.h"

#line 370 "../../common/cost.h"
/*template <class T>
*/void compcxx_MultiTimer_2/*<trigger_t >*/::Set(double time, unsigned int index)
{
  event_t * e = GetEvent(index);
  if(e->active)m_simeng->CancelEvent(e);
  e->time = time;
  e->object = this;
  e->active = true;
  m_simeng->ScheduleEvent(e);
}


#line 381 "../../common/cost.h"

#line 381 "../../common/cost.h"
/*template <class T>
*/void compcxx_MultiTimer_2/*<trigger_t >*/::Cancel(unsigned int index)
{
  event_t * e = GetEvent(index);
  if(e->active)
    m_simeng->CancelEvent(e);
  e->active = false;
}


#line 390 "../../common/cost.h"

#line 390 "../../common/cost.h"
/*template <class T>
*/void compcxx_MultiTimer_2/*<trigger_t >*/::activate(CostEvent*e)
{
  event_t * event = (event_t*)e;
  event->active = false;
  (p_compcxx_parent->create(event->data,event->index));
}





#line 67 "mmm1.cc"

#line 74 "mmm1.cc"
compcxx_Source_4::compcxx_Source_4()
{
    wait.p_compcxx_parent=this /*connect wait.to_component,*/;
}


#line 79 "mmm1.cc"
void compcxx_Source_4::Start()
{
    m_seq_number=0;
    wait.Set(Exponential(interval));
}


#line 85 "mmm1.cc"
void compcxx_Source_4::create(trigger_t&, unsigned int)
{
    wait.Set(SimTime()+Exponential(interval) );
    m_packet.seq_number=m_seq_number++;
    m_packet.arrival_time = SimTime();
    m_packet.index = index;
    (out_f(m_packet));
    return;
}




#line 332 "mmm1.cc"
void compcxx_MMM1_8::Setup()
{
	int i;
	
	source.SetSize(size);
	fifo.SetSize(size);
	sink.SetSize(size);
	
	for(i=0;i<size;i++)
	{
    	source[i].interval=interval;
    	source[i].index=i;
	    fifo[i].queue_length=queue_length;
	}
	
    server.service_time=service_time;
    server.size=size;
    server.Setup();

	for(i=0;i<size;i++)
	{
	    source[i].out_f.Connect(fifo[i],(compcxx_component::Source_out_f_t)&compcxx_FIFO_5::in) /*connect source[i].out,fifo[i].in*/;
    	fifo[i].out_f.Connect(server,(compcxx_component::FIFO_out_f_t)&compcxx_MultiServer_6::in) /*connect fifo[i].out,server.in*/;
    	server.next[i].Connect(fifo[i],(compcxx_component::MultiServer_next_f_t)&compcxx_FIFO_5::next) /*connect server.next[i],fifo[i].next*/;
    	server.out[i].Connect(sink[i],(compcxx_component::MultiServer_out_f_t)&compcxx_Sink_7::in) /*connect server.out[i],sink[i].in*/;
    }
}











#line 369 "mmm1.cc"
int main(int argc, char* argv[])
{
    compcxx_MMM1_8 mmm1;

	mmm1.size=4;
    mmm1.interval=1;
    mmm1.queue_length=100;
    mmm1.service_time=0.5;
    mmm1.StopTime( 100000.0);
    mmm1.Seed=10;

    if (argc>=2)
      mmm1.StopTime( atof(argv[1]));
    if (argc>=3)
      mmm1.Seed=atoi(argv[2]);

    mmm1.Setup(); 
    mmm1.Run(); 

    return 0;
}



