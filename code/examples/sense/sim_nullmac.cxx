
#line 1 "sim_80211.cc"
#define queue_t HeapQueue


#line 1 "../../common/sense.h"































#ifndef SENSE_H
#define SENSE_H


#line 1 "../../common/config.h"
#define HAVE_INT8_T 1
#define HAVE_INT16_T 1
#define HAVE_INT32_T 1
#define HAVE_INT64_T 1

#define HAVE_UINT8_T 1
#define HAVE_UINT16_T 1
#define HAVE_UINT32_T 1
#define HAVE_UINT64_T 1

#if 1
#include <inttypes.h>
#endif

#if !HAVE_INT8_T
typedef signed char int8_t;
#endif
#if !HAVE_INT16_T
typedef signed short int16_t;
#endif
#if !HAVE_INT32_T
typedef signed long int32_t;
#endif
#if !HAVE_INT64_T
typedef signed long long int64_t;
#endif


#if !HAVE_UINT8_T
typedef unsigned char uint8_t;
#endif
#if !HAVE_UINT16_T
typedef unsigned short uint16_t;
#endif
#if !HAVE_UINT32_T
typedef unsigned long uint32_t;
#endif
#if !HAVE_UINT64_T
typedef unsigned long long uint64_t;
#endif



#line 35 "../../common/sense.h"

const double speed_of_light=299792458.0;

#include <string>
using std::string;







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







#line 46 "../../common/sense.h"


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

#line 47 "../../common/sense.h"































#line 1 "../../common/ether_addr.h"
#ifndef	_ether_addr_h_
#define	_ether_addr_h_







class ether_addr_t
{
 public:
  enum { LENGTH = 6, BROADCAST = -1 };

  ether_addr_t () : addr (0) {} ;
  ether_addr_t (int a) : addr (a) {};

  bool operator == (const ether_addr_t& another) const { return addr==another.addr; }
  bool operator == (const int& another) const { return addr==another; }
  bool operator < (const ether_addr_t& another) const { return addr<another.addr; }
  bool operator > (const ether_addr_t& another) const { return addr>another.addr; }

  operator const int& () const { return addr; };
  struct compare 
  {
    bool operator() ( const ether_addr_t & e1, const ether_addr_t & e2)
      {
	return e1.addr > e2.addr;
      }
  };
 private:
  int addr;
};

#endif	// _ether_addr_h_

#line 77 "../../common/sense.h"














template <class T>
class packet_trait
{
 public:
  typedef T nonpointer_t;
  static void free(const T&) { }
  static void free(T&t) { t.~T(); }
  static void init(T&t) { new (&t) T; }
  static void init(const T&) { }
  static void inc_ref(const T&) { }
  static T& access_hdr(T&t) { return t; }
  static void dump(std::string& str, const T&p) { p.dump(str) ;}  
  static void check_ref(const T&, int) { }
};






















template <class H, class P>
class smart_packet_t
{
 public:
  inline static smart_packet_t<H,P> * alloc();
  inline void free();
  inline void destroy();
  inline void inc_ref();
  inline smart_packet_t<H,P> * copy();
  inline void inc_pld_ref() const;
  
  inline bool check_ref(int r) { return r<=refcount; }
  
  P& access_pld() { return packet_trait<P>::access_hdr(pld); }
  static void free_pld(P const &p) { packet_trait<P>::free(p); }
  
  H* operator ->() { return &hdr; }
  
  std::string dump() const;
  
  H hdr;
  P pld;
  
 private:
  static CorsaAllocator m_allocator;
  int refcount;
};







template <class H, class P> CorsaAllocator smart_packet_t<H,P>::m_allocator (sizeof(smart_packet_t<H,P>));






template <class H, class P>
smart_packet_t<H,P>* smart_packet_t<H,P>::alloc()
{
  smart_packet_t<H,P>* p=static_cast<smart_packet_t<H,P>*>(m_allocator.alloc());
  packet_trait<H>::init(p->hdr);
  packet_trait<P>::init(p->pld);
  p->refcount=1;
  return p;
}





template <class H, class P>
smart_packet_t<H,P>* smart_packet_t<H,P>::copy()
{
  smart_packet_t<H,P>* p=static_cast<smart_packet_t<H,P>*>(m_allocator.alloc());
  packet_trait<H>::init(p->hdr);
  packet_trait<P>::init(p->pld);
  p->hdr=hdr;
  p->pld=pld;
  inc_pld_ref();
  p->refcount=1;
  return p;
}







template <class H, class P>
void smart_packet_t<H,P>::free()
{
#ifdef SENSE_DEBUG
  packet_trait<P>::check_ref(pld,refcount);
#endif
  packet_trait<P>::free(pld);
  refcount--;
  if(refcount==0)
  {
    packet_trait<H>::free(hdr);
    m_allocator.free(this);
  }
}





template <class H, class P>
void smart_packet_t<H,P>::destroy()
{
  packet_trait<H>::free(hdr);
  packet_trait<P>::free(pld);
  m_allocator.free(this);
}






template <class H, class P>
void smart_packet_t<H,P>::inc_ref()
{
  packet_trait<P>::inc_ref(pld);
  refcount++;
  return;
}








template <class H, class P>
void smart_packet_t<H,P>::inc_pld_ref() const
{
  packet_trait<P>::inc_ref(pld);
}







template <class H, class P>
std::string smart_packet_t<H,P>::dump() const
{
  std::string str="[";
  std::string h;
  char		mwlString[20];
  sprintf( mwlString, " rc %d", refcount);
  if( hdr.dump(h) )
  {
    std::string m;
    packet_trait<P>::dump(m,pld);
    str = str + h + ", " + m + mwlString + "]";
  }
  else
  {
    str = str + h + ", NULL "+ mwlString + "]";
  }
  return str;
}












template <class H, class P>
class packet_trait< smart_packet_t<H,P>* >
{
 public:
  typedef smart_packet_t<H,P> nonpointer_t;
  static void free(nonpointer_t* const &p) { if(p!=NULL) p->free(); };
  static void inc_ref(nonpointer_t* const &p) { if(p!=NULL) p->inc_ref(); };
  static H& access_hdr( nonpointer_t * p) { return p->hdr; }
  static void init(nonpointer_t* &p) { p=NULL; }
  static void dump(std::string& str,nonpointer_t* const &p)
      { if(p!=NULL) str= p->dump(); else str="NULL"; }
  static void check_ref(nonpointer_t* const &p, int ref)
      {
	if(p!=NULL&&!p->check_ref(ref))
	  printf("Packet 0x%08X: Payload refcount is smaller than the current refcount\n", (int) p);
      }

};







class coordinate_t
{
 public:
  coordinate_t () : x(0.0), y(0.0) { }
  coordinate_t ( double _x, double _y ) : x(_x), y(_y) { }
  double x,y;
};









#include <utility>
using std::make_pair;
using std::pair;









template <class T1, class T2, class T3>
class triple
{
 public:
  typedef T1 first_type;
  typedef T2 second_type;
  typedef T3 third_type;

  T1 first;
  T2 second;
  T3 third;

  triple() : first(T1()), second(T2()), third(T3()) {}
  triple(const T1& a, const T2& b, const T3& c) : first(a), second(b), third(c) {}

  template <class U1, class U2, class U3>
      triple(const triple<U1, U2, U3>& t) : first(t.first), second(t.second), third(t.third) {}
};

template <class T1, class T2, class T3>
inline bool operator == (const triple<T1, T2, T3>& x, const triple<T1, T2, T3>& y)
{
  return x.first == y.first && x.second == y.second && x.third == y.third;
}

template <class T1, class T2, class T3>
inline triple<T1, T2, T3> make_triple(const T1& x, const T2& y, const T3& z)
{
  return triple<T1, T2, T3>(x, y, z);
}














































#line 1 "../../common/path.h"
#ifndef	_path_h_
#define	_path_h_

#include <string>
#include <stdio.h>
using std::string;

template <int size>
class path_t
{
 public:
  path_t(): length(0), index( 0), overFlow( false) {}
  void AddNode(ether_addr_t& n)
    {
      if( length < size)
	nodes[ length] = n;
      else
	overFlow = true;
      length++;
      return;
    }
  void Clear() { length=0; }
  const char* ToString()
      {
	static string str;
	str = "";
	char buff[10];
	int	L2=length > 15 ? 15 : length;
	
	sprintf( buff, " {%2d}", length);
	str += buff;
	for(int i=0;i<L2;i++)
	{
	  sprintf(buff," %3d", (int) nodes[i] );
	  str += buff;
	}
	return str.c_str();
      }
  bool	firstNode( int &node)
    {
      index = 0;
      return nextNode( node);
    }
  bool	nextNode( int &node)
    {
      if( index < length)
      {
	node = (int) nodes[ index++];
	return true;
      }
      else
	return false;
    }
  bool	getOverFlow() const	{ return overFlow; }
  int	getLength() const	{ return length; }
 private:
  ether_addr_t	nodes[size];
  int		length;
  int		index;
  bool		overFlow;
};

#ifndef	VR_SIZE
#define VR_SIZE 20
#endif	//VR_SIZE

typedef path_t<VR_SIZE>	Path;

#endif	// _path_h_

#line 419 "../../common/sense.h"


#endif /* SENSE_H */

#line 3 "sim_80211.cc"


#line 1 "../../libraries/visualizer/inc/Visualizer.h"






































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
			   bool atDestination);	
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



#line 4 "sim_80211.cc"


#line 1 "../../app/random_neighbor.h"























#ifndef random_neighbor_h
#define random_neighbor_h

struct RandomNeighbor_Struct
{
    struct packet_t
    {
	    ether_addr_t src_addr;
    	ether_addr_t dst_addr;
	    int data_size;
	    std::string dump() const 
	    {
	        char buffer[50];
	        sprintf(buffer,"%d %d %d",(int)src_addr,(int)dst_addr,data_size);
	        return buffer;
	    }
	    void dump(std::string& str) const { str=dump();}
    };
};


#line 165 "../../app/random_neighbor.h"
#endif /* random_neighbor_h*/

#line 5 "sim_80211.cc"


#line 1 "../../mob/immobile.h"























#ifndef immobile_h
#define immobile_h


#line 89 "../../mob/immobile.h"
#endif /* immobile_h*/

#line 6 "sim_80211.cc"


#line 1 "../../mac/mac_80211.h"























#ifndef mac80211_h
#define mac80211_h
#include <map>

template <class PLD>
struct MAC80211_Struct
{
    
    enum 
    {
		MAC_ProtocolVersion=0x00,
		MAC_Type_Management=0x00,
		MAC_Type_Control=0x01,
		MAC_Type_Data=0x02,
		MAC_Type_Reserved=0x03,   
		MAC_Subtype_RTS=0x0b,
		MAC_Subtype_CTS=0x0c,
		MAC_Subtype_ACK=0x0d,
		MAC_Subtype_Data=0x00
    };

    
    struct frame_control 
    {
        u_char          fc_subtype              : 4;
        u_char          fc_type                 : 2;
        u_char          fc_protocol_version     : 2;

        u_char          fc_order                : 1;
        u_char          fc_wep                  : 1;
        u_char          fc_more_data            : 1;
        u_char          fc_pwr_mgt              : 1;
        u_char          fc_retry                : 1;
        u_char          fc_more_frag            : 1;
        u_char          fc_from_ds              : 1;
        u_char          fc_to_ds                : 1;
    }; 

    struct hdr_struct
    {
		
        frame_control    	dh_fc;
        uint16_t            dh_duration;
        ether_addr_t        dh_da;
        ether_addr_t        dh_sa;
        ether_addr_t        dh_bssid;
        uint16_t            dh_scontrol;

		
		
		double tx_time;                              
		double wave_length;                          
		unsigned int size;

        bool dump(std::string& str) const
        {
            std::string type;
            bool payload=false;
            switch(dh_fc.fc_type)
            {
            case MAC_Type_Control:
	            switch(dh_fc.fc_subtype)
	            {
	            case MAC_Subtype_RTS:
	                type="RTS";
	                break;
	            case MAC_Subtype_CTS:
	                type="CST";
	                break;
	            case MAC_Subtype_ACK:
	                type="ACK";
	                break;
	            default:
	                type="UNKNOWN";
	            }
	            break;
            case MAC_Type_Data:
	            if(dh_fc.fc_subtype == MAC_Subtype_Data)
	            {
	                payload=true;
	                if(dh_da!=ether_addr_t::BROADCAST)
		                type="DATA";
	                else
		                type="BROADCAST";
	            }
	            else
	                type="UNKNOWN";
	            break;
            default:
	            type="UNKNOWN";
	            break;
            }
            char buffer[100];
            sprintf(buffer,"%f %s %d -> %d", tx_time, type.c_str(), (int)(dh_sa), (int)(dh_da));
            str=buffer;
            return payload;
        }        
    };	

    typedef smart_packet_t<hdr_struct,PLD> packet_t;
    typedef PLD payload_t;
    
    
    static int CWMin;
    static int CWMax;
    static double SlotTime;
    static double SIFSTime;

    
    static unsigned int RTSThreshold;
    static unsigned int ShortRetryLimit;
    static unsigned int LongRetryLimit;


    
    static int PLCP_HDR_LEN;
    static int HDR_LEN;
    static int RTS_LEN;
    static int CTS_LEN;
    static int ACK_LEN;
    static int FCS_LEN;
    
    
    
    static double DataRate;
    static double BasicRate;
    static double CPThreshold;    
};


template <class PLD> int MAC80211_Struct<PLD>::CWMin=31;
template <class PLD> int MAC80211_Struct<PLD>::CWMax=1023;
template <class PLD> double MAC80211_Struct<PLD>::SlotTime=0.000020;
template <class PLD> double MAC80211_Struct<PLD>::SIFSTime=0.000010;

template <class PLD> unsigned int MAC80211_Struct<PLD>::RTSThreshold=3000;
template <class PLD> unsigned int MAC80211_Struct<PLD>::ShortRetryLimit=3;
template <class PLD> unsigned int MAC80211_Struct<PLD>::LongRetryLimit=4;

template <class PLD> int MAC80211_Struct<PLD>::PLCP_HDR_LEN = (144 + 48) >> 3;
template <class PLD> int MAC80211_Struct<PLD>::HDR_LEN = PLCP_HDR_LEN + FCS_LEN + 4*ether_addr_t::LENGTH + 6;
template <class PLD> int MAC80211_Struct<PLD>::RTS_LEN = PLCP_HDR_LEN + FCS_LEN + 2*ether_addr_t::LENGTH + 6;
template <class PLD> int MAC80211_Struct<PLD>::CTS_LEN = PLCP_HDR_LEN + FCS_LEN + 1*ether_addr_t::LENGTH + 6;
template <class PLD> int MAC80211_Struct<PLD>::ACK_LEN = PLCP_HDR_LEN + FCS_LEN + 1*ether_addr_t::LENGTH + 6;
template <class PLD> int MAC80211_Struct<PLD>::FCS_LEN = 4;

template <class PLD> double MAC80211_Struct<PLD>::DataRate=1.0e6/8;
template <class PLD> double MAC80211_Struct<PLD>::BasicRate=1.0e6/8;
template <class PLD> double MAC80211_Struct<PLD>::CPThreshold=10; 










#line 987 "../../mac/mac_80211.h"
#endif /* mac80211_h */

#line 7 "sim_80211.cc"


#line 1 "../../mac/null_mac.h"



































#ifndef simple_mac_h
#define simple_mac_h















template <class PLD>
struct NullMAC_Struct
{
    



    
	struct hdr_t
	{
		ether_addr_t dst_addr;
		ether_addr_t src_addr;
		unsigned int size;

		double tx_time;       
		double wave_length;   










		bool dump(std::string& str) const 
		{ 
	    	char buffer[50];
	    	sprintf(buffer,"%d %d %d",int(src_addr),int(dst_addr),size); 
	    	str=buffer;
	    	return true;
		}
	};










	typedef PLD payload_t;
	typedef smart_packet_t<hdr_t, PLD> packet_t;

};












#line 261 "../../mac/null_mac.h"
#endif /* simple_mac_h */

#line 8 "sim_80211.cc"


#line 1 "../../phy/transceiver.h"























#ifndef transceiver_h
#define transceiver_h









#line 132 "../../phy/transceiver.h"
#endif /* transceiver_h */



#line 9 "sim_80211.cc"


#line 1 "../../phy/simple_channel.h"
























#ifndef simple_channel_h
#define simple_channel_h

#include <algorithm>


#line 305 "../../phy/simple_channel.h"
#endif /* simple_channel_h */

#line 10 "sim_80211.cc"


#line 1 "../../energy/battery.h"























#ifndef battery_h
#define battery_h


#line 206 "../../energy/battery.h"
#endif /* battery_h */


#line 11 "sim_80211.cc"


#line 1 "../../energy/power.h"
#ifndef power_h
#define power_h

#include <utility>

struct PM_Struct
{
  enum { TX = 0, RX, IDLE, SLEEP, OFF, ON};
};


#line 180 "../../energy/power.h"
#endif /* power_h */

#line 12 "sim_80211.cc"



typedef NullMAC_Struct <RandomNeighbor_Struct::packet_t> :: packet_t mac_packet_t;

#include "compcxx_sim_80211.h"
class compcxx_SimpleChannel_21;
#line 401 "../../common/cost.h"
/*template <class T> */
#line 401 "../../common/cost.h"
class compcxx_InfiTimer_12 : public compcxx_component, public TimerBase
{
 public:
  struct event_t : public CostEvent { triple<mac_packet_t *, double, int>  data; };
  compcxx_InfiTimer_12();
  virtual ~compcxx_InfiTimer_12();
	
  /*outport void to_component(triple<mac_packet_t *, double, int>  &, unsigned int i)*/;
 
  void activate(CostEvent*event);

  inline unsigned int Set(double t);
  inline unsigned int Set( triple<mac_packet_t *, double, int>  const & data, double t);
  inline void Cancel (unsigned int index);
  inline event_t* GetEvent(unsigned int index);

  inline bool Active(unsigned int index) { return GetEvent(index)->active; }
  inline double GetTime(unsigned int index) { return GetEvent(index)->time; }
  inline triple<mac_packet_t *, double, int>  & GetData(unsigned int index) { return GetEvent(index)->data; }
  inline void SetData(triple<mac_packet_t *, double, int>  const &d, unsigned int index) { GetEvent(index)->data = d; }

 private:
  inline void ReleaseSlot(unsigned int i) { m_free_slots.push_back(i); }
  inline unsigned int GetSlot();
					 
  std::vector<event_t*> m_events;
  std::vector<unsigned int> m_free_slots;
  CostSimEng* m_simeng;

  CorsaAllocator* m_allocator;
public:compcxx_SimpleChannel_21* p_compcxx_parent;};

/*template <class PACKET>

*/
#line 30 "../../phy/simple_channel.h"
class compcxx_SimpleChannel_21 : public compcxx_component, public TypeII
{
 public:
  bool		initSources( int numSources, bool oneSink);
  void		getSourcePair( int &src, int &dest);

  /*inport */void		from_phy( mac_packet_t  * p, double power, int id);
  class my_SimpleChannel_to_phy_f_t:public compcxx_functor<SimpleChannel_to_phy_f_t>{ public:void	 operator() ( mac_packet_t  * p, double power ) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])( p,power ); return (c[0]->*f[0])( p,power );};};compcxx_array<my_SimpleChannel_to_phy_f_t > to_phy;/*outportvoid	to_phy( mac_packet_t  * p, double power )*/;
  /*inport */void		pos_in( const coordinate_t& pos, int id);
	
  compcxx_InfiTimer_12/*< triple<mac_packet_t  *, double, int> > */propagation_delay;
  /*inport */void		depart( const triple<mac_packet_t  *, double, int> & data, unsigned int );

  compcxx_SimpleChannel_21() { propagation_delay.p_compcxx_parent=this /*connect propagation_delay.to_component, */; }
  virtual ~compcxx_SimpleChannel_21() {}
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
  bool		forward( mac_packet_t  * p, double power, int src, int dest, bool);
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

class compcxx_RandomNeighbor_14;/*template <class T> */
#line 241 "../../common/cost.h"
class compcxx_Timer_2 : public compcxx_component, public TimerBase
{
 public:
  struct event_t : public CostEvent { trigger_t  data; };
  

  compcxx_Timer_2() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
  inline void Set(trigger_t  const &, double );
  inline void Set(double );
  inline double GetTime() { return m_event.time; }
  inline bool Active() { return m_event.active; }
  inline trigger_t  & GetData() { return m_event.data; }
  inline void SetData(trigger_t  const &d) { m_event.data = d; }
  void Cancel();
  /*outport void to_component(trigger_t  &)*/;
  void activate(CostEvent*);
 private:
  CostSimEng* m_simeng;
  event_t m_event;
public:compcxx_RandomNeighbor_14* p_compcxx_parent;};

class compcxx_RandomNeighbor_14;/*template <class T> */
#line 241 "../../common/cost.h"
class compcxx_Timer_3 : public compcxx_component, public TimerBase
{
 public:
  struct event_t : public CostEvent { trigger_t  data; };
  

  compcxx_Timer_3() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
  inline void Set(trigger_t  const &, double );
  inline void Set(double );
  inline double GetTime() { return m_event.time; }
  inline bool Active() { return m_event.active; }
  inline trigger_t  & GetData() { return m_event.data; }
  inline void SetData(trigger_t  const &d) { m_event.data = d; }
  void Cancel();
  /*outport void to_component(trigger_t  &)*/;
  void activate(CostEvent*);
 private:
  CostSimEng* m_simeng;
  event_t m_event;
public:compcxx_RandomNeighbor_14* p_compcxx_parent;};

class compcxx_NullMAC_16;
#line 44 "../../app/random_neighbor.h"
class compcxx_RandomNeighbor_14 : public compcxx_component, public TypeII, public RandomNeighbor_Struct
{
public:
    
 	/*outport void to_transport ( packet_t& pld, const ether_addr_t& dst, unsigned int size )*/;
 	/*inport */void from_transport_data ( packet_t& pld, const ether_addr_t & dst);
 	/*inport */void from_transport_ack ( bool flag);
    
    
    compcxx_Timer_2 /*< trigger_t > */broadcast_timer;
    compcxx_Timer_3 /*< trigger_t > */unicast_timer;

    simtime_t BroadcastInterval;
    simtime_t UnicastInterval;
    ether_addr_t MyEtherAddr;
    double MaxX, MaxY, MaxPacketSize;
    bool DumpPackets;

    int SentPackets, RecvPackets;
    int SendFailures;

    compcxx_RandomNeighbor_14();
    virtual ~compcxx_RandomNeighbor_14() ;
    void Start();
    void Stop();

    /*inport */void BroadcastTimer(trigger_t&);
    /*inport */void UnicastTimer(trigger_t&);

 private:
    
    std::vector<ether_addr_t> m_neighbors;
    bool m_transport_busy;

public:compcxx_NullMAC_16* p_compcxx_NullMAC_16;};


#line 27 "../../energy/battery.h"
class compcxx_SimpleBattery_18 : public compcxx_component, public TypeII
{
 public:
  double	InitialEnergy;
  double	RemainingEnergy;

  /*inport */inline double	power_in( double power, simtime_t t);
  /*inport */inline double	query_in();
    
  void		Start();
  void		Stop();
  void		clearStats()	{ InitialEnergy = RemainingEnergy; }
 private:
  double	m_last_time;
};

class compcxx_RandomNeighbor_14;class compcxx_DuplexTransceiver_17;/*template <class PLD>
*/
#line 112 "../../mac/null_mac.h"
class compcxx_NullMAC_16 : public compcxx_component, public TypeII, public NullMAC_Struct <RandomNeighbor_Struct::packet_t >
{
 public:












    static unsigned int RTSThreshold;
    ether_addr_t MyEtherAddr;
    bool DumpPackets;
    bool Promiscuity;

	int SentPackets, RecvPackets;

    










 
 	/*inport */inline void from_network ( payload_t& pld, const ether_addr_t& dst, unsigned int size );
 	/*outport void to_network_data ( payload_t& pld, const ether_addr_t & dst)*/;
 	/*outport void to_network_ack ( bool errflag )*/;





 	
 	/*inport */inline void from_phy ( packet_t* pkt, bool errflag, double  power );
 	/*outport void to_phy (packet_t* pkt)*/;
    
    void Start();                                    
    void Stop();                                     

public:compcxx_RandomNeighbor_14* p_compcxx_RandomNeighbor_14;public:compcxx_DuplexTransceiver_17* p_compcxx_DuplexTransceiver_17;};
class compcxx_Immobile_15;/*template <class T> */
#line 241 "../../common/cost.h"
class compcxx_Timer_4 : public compcxx_component, public TimerBase
{
 public:
  struct event_t : public CostEvent { trigger_t data; };
  

  compcxx_Timer_4() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
  inline void Set(trigger_t const &, double );
  inline void Set(double );
  inline double GetTime() { return m_event.time; }
  inline bool Active() { return m_event.active; }
  inline trigger_t & GetData() { return m_event.data; }
  inline void SetData(trigger_t const &d) { m_event.data = d; }
  void Cancel();
  /*outport void to_component(trigger_t &)*/;
  void activate(CostEvent*);
 private:
  CostSimEng* m_simeng;
  event_t m_event;
public:compcxx_Immobile_15* p_compcxx_parent;};

class compcxx_SensorNode_20;
#line 27 "../../mob/immobile.h"
class compcxx_Immobile_15 : public compcxx_component, public TypeII
{
 public:
  compcxx_Timer_4 /*<trigger_t>	*/timer;    
  /*outport */void		pos_out( coordinate_t& pos, int id);
  /*inport */void		announce_pos( trigger_t& t);

  compcxx_Immobile_15() { timer.p_compcxx_parent=this /*connect timer.to_component, */; }
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
public:compcxx_SensorNode_20* p_compcxx_parent;};

class compcxx_NullMAC_16;class compcxx_SensorNode_20;class compcxx_PowerManager_19;/*template <class PACKET>
*/
#line 34 "../../phy/transceiver.h"
class compcxx_DuplexTransceiver_17  : public compcxx_component, public TypeII
{
 public:
    
  
  
  
  
     
  void		setTXPower( double pwr)		{ TXPower = pwr; }
  double	getTXPower()			{ return TXPower; }
  void		setTXGain( double pwr)		{ TXGain = pwr; }
  void		setRXGain( double pwr)		{ RXGain = pwr; }
  void		setRXThresh( double pwr)	{ RXThresh = pwr; }
  double	getRXThresh()			{ return RXThresh; }
  void		setCSThresh( double pwr)	{ CSThresh = pwr; }
  double	getCSThresh()			{ return CSThresh; }
  void		setFrequency( double freq)	{ Frequency = freq; }
  double	getFrequency()			{ return Frequency; }
  void		setID( int id)			{ ID = id; }

  /*inport */inline void from_mac (mac_packet_t  * p);    
  /*outport void to_mac (mac_packet_t  * p, bool errflag, unsigned int size)*/;
  /*inport */inline void from_channel (mac_packet_t  * p, double power);
  /*outport void to_channel (mac_packet_t  * p, double power, int id)*/;
  /*outport double to_power_switch ( int state, double time)*/;
    
  void Start();
  void Stop();

 private:
  int ID;
  double	TXPower;
  double	TXGain;
  double	RXGain;
  double	Frequency;
  double RXThresh;       
  double CSThresh;       
public:compcxx_NullMAC_16* p_compcxx_NullMAC_16;public:compcxx_SensorNode_20* p_compcxx_parent;public:compcxx_PowerManager_19* p_compcxx_PowerManager_19;};

class compcxx_PowerManager_19;/*template <class T> */
#line 241 "../../common/cost.h"
class compcxx_Timer_13 : public compcxx_component, public TimerBase
{
 public:
  struct event_t : public CostEvent { trigger_t data; };
  

  compcxx_Timer_13() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
  inline void Set(trigger_t const &, double );
  inline void Set(double );
  inline double GetTime() { return m_event.time; }
  inline bool Active() { return m_event.active; }
  inline trigger_t & GetData() { return m_event.data; }
  inline void SetData(trigger_t const &d) { m_event.data = d; }
  void Cancel();
  /*outport void to_component(trigger_t &)*/;
  void activate(CostEvent*);
 private:
  CostSimEng* m_simeng;
  event_t m_event;
public:compcxx_PowerManager_19* p_compcxx_parent;};

class compcxx_SimpleBattery_18;class compcxx_SensorNode_20;
#line 11 "../../energy/power.h"
class compcxx_PowerManager_19 : public compcxx_component, public TypeII, public PM_Struct
{
 public:
  compcxx_PowerManager_19();

  double	RXPower;
  double	TXPower;
  double	IdlePower;
  double	SleepPower;

  double	BeginTime;
  double	FinishTime;
  void		failureStats( double initTime, double cycleTime,
			      double percentUp);
  void		failureStats( double downTime);

  /*inport */inline double	switch_state( int state, double time);
  /*inport */inline int	state_query();
  /*inport */inline double	energy_query();
  /*inport */inline void	power_switch(trigger_t&);
  /*outport double	to_battery_power( double power, simtime_t time)*/;
  /*outport double	to_battery_query()*/;
  
  /*outport void	from_pm_node_up()*/;
	
  void		Start();
  void		Stop();
  compcxx_Timer_13/*<trigger_t> */switch_timer;
 private:
  int		m_state; 
  bool		m_switch_on;
  bool		transient;
  double	upTime;
  double	downTime;
  double	initTime;
public:compcxx_SimpleBattery_18* p_compcxx_SimpleBattery_18;public:compcxx_SensorNode_20* p_compcxx_parent;};


#line 19 "sim_80211.cc"
class compcxx_SensorNode_20 : public compcxx_component, public TypeII
{
public:

    compcxx_RandomNeighbor_14 app;
    compcxx_Immobile_15 mob;
    compcxx_NullMAC_16 /*<RandomNeighbor_Struct::packet_t> */mac;
    compcxx_DuplexTransceiver_17 /*< mac_packet_t > */phy;
    compcxx_SimpleBattery_18 battery;
    compcxx_PowerManager_19 pm;

    double MaxX, MaxY;
    ether_addr_t MyEtherAddr;
    simtime_t BroadcastInterval;
    simtime_t UnicastInterval;
    int ID;

    int SentPackets, RecvPackets;

    compcxx_SensorNode_20();
    virtual ~compcxx_SensorNode_20();
    void Start();
    void Stop();
    void Setup();

    class my_SensorNode_to_channel_packet_f_t:public compcxx_functor<SensorNode_to_channel_packet_f_t>{ public:void  operator() (mac_packet_t* packet, double power, int id) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(packet,power,id); return (c[0]->*f[0])(packet,power,id);};};my_SensorNode_to_channel_packet_f_t to_channel_packet_f;/*outport */void to_channel_packet(mac_packet_t* packet, double power, int id);
    /*inport */void from_channel (mac_packet_t* packet, double power);
    class my_SensorNode_to_channel_pos_f_t:public compcxx_functor<SensorNode_to_channel_pos_f_t>{ public:void  operator() (coordinate_t& pos, int id) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(pos,id); return (c[0]->*f[0])(pos,id);};};my_SensorNode_to_channel_pos_f_t to_channel_pos_f;/*outport */void to_channel_pos(coordinate_t& pos, int id);
    
    /*inport */void	from_pm_node_up();
};


#line 129 "sim_80211.cc"
class compcxx_Sim_80211_22 : public compcxx_component, public CostSimEng
{
public:

    void Stop();

    double MaxX, MaxY;
    simtime_t BroadcastInterval;
    simtime_t UnicastInterval;
    int NumNodes;

    compcxx_array<compcxx_SensorNode_20  >nodes;
    compcxx_SimpleChannel_21 /*< mac_packet_t > */channel;
    
    void Setup();
};


#line 434 "../../common/cost.h"
compcxx_InfiTimer_12
#line 433 "../../common/cost.h"
/*template <class T>
*//*<T>*/::compcxx_InfiTimer_12()
{
  m_simeng = CostSimEng::Instance();
  m_allocator = m_simeng->GetAllocator(sizeof(event_t));
  GetEvent(0);
}


#line 442 "../../common/cost.h"
compcxx_InfiTimer_12
#line 441 "../../common/cost.h"
/*template <class T>
*//*<T>*/::~compcxx_InfiTimer_12()
{
  for(unsigned int i=0;i<m_events.size();i++)
    m_allocator->free(m_events[i]);
}


#line 448 "../../common/cost.h"
/*template <class T>
*//*typename */compcxx_InfiTimer_12/*<T>*/::event_t* compcxx_InfiTimer_12/*<triple<mac_packet_t *, double, int>  >*/::GetEvent(unsigned int index)
{
  if (index>=m_events.size())
  {
    for (unsigned int i=m_events.size();i<=index;i++)
    {
      m_events.push_back( (event_t*) m_allocator->alloc() );
      m_events[i]->active=false;
      m_events[i]->index=i;
      m_free_slots.push_back(i);
    }
  }
  return m_events[index];
}


#line 464 "../../common/cost.h"

#line 464 "../../common/cost.h"
/*template <class T>
*/unsigned int compcxx_InfiTimer_12/*<triple<mac_packet_t *, double, int>  >*/::Set(triple<mac_packet_t *, double, int>  const & data, double time)
{
  int index=GetSlot();
  event_t * e = GetEvent(index);
  assert(e->active==false);
  e->time = time;
  e->data = data;
  e->object = this;
  e->active = true;
  m_simeng->ScheduleEvent(e);
  return index;
}


#line 478 "../../common/cost.h"

#line 478 "../../common/cost.h"
/*template <class T>
*/unsigned int compcxx_InfiTimer_12/*<triple<mac_packet_t *, double, int>  >*/::Set(double time)
{
  int index=GetSlot();
  event_t * e = GetEvent(index);
  assert(e->active==false);
  e->time = time;
  e->object = this;
  e->active = true;
  m_simeng->ScheduleEvent(e);
  return index;
}


#line 491 "../../common/cost.h"

#line 491 "../../common/cost.h"
/*template <class T>
*/void compcxx_InfiTimer_12/*<triple<mac_packet_t *, double, int>  >*/::Cancel(unsigned int index)
{
  event_t * e = GetEvent(index);
  assert(e->active);
  m_simeng->CancelEvent(e);
  ReleaseSlot(index);
  e->active = false;
}


#line 501 "../../common/cost.h"

#line 501 "../../common/cost.h"
/*template <class T>
*/void compcxx_InfiTimer_12/*<triple<mac_packet_t *, double, int>  >*/::activate(CostEvent*e)
{
  event_t * event = (event_t*)e;
  event->active = false;
  ReleaseSlot(event->index);
  (p_compcxx_parent->depart(event->data,event->index));
}


#line 510 "../../common/cost.h"

#line 510 "../../common/cost.h"
/*template <class T>
*/unsigned int compcxx_InfiTimer_12/*<triple<mac_packet_t *, double, int>  >*/::GetSlot()
{
  if(m_free_slots.empty())
    GetEvent(m_events.size()*2-1);
  int i=m_free_slots.back();
  m_free_slots.pop_back();
  return i;
}

#line 43 "../../phy/simple_channel.h"

#line 90 "../../phy/simple_channel.h"

#line 90 "../../phy/simple_channel.h"
/*template <class PACKET>
*/void compcxx_SimpleChannel_21/*<mac_packet_t  >*/::Setup()
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


#line 122 "../../phy/simple_channel.h"

#line 122 "../../phy/simple_channel.h"
/*template <class PACKET>
*/void compcxx_SimpleChannel_21/*<mac_packet_t  >*/::Start()
{
}


#line 127 "../../phy/simple_channel.h"

#line 127 "../../phy/simple_channel.h"
/*template <class PACKET>
*/void compcxx_SimpleChannel_21/*<mac_packet_t  >*/::Stop()
{
}


#line 132 "../../phy/simple_channel.h"

#line 132 "../../phy/simple_channel.h"
/*template <class PACKET>
*/void compcxx_SimpleChannel_21/*<mac_packet_t  >*/::pos_in(const coordinate_t & pos, int id)
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


#line 172 "../../phy/simple_channel.h"

#line 172 "../../phy/simple_channel.h"
/*template <class PACKET>
*/bool compcxx_SimpleChannel_21/*<mac_packet_t  >*/::isNeighbor(
  double	power,
  int		src,
  int		dest)
{
  return forward( NULL, power, src, dest, false);
}


#line 181 "../../phy/simple_channel.h"

#line 181 "../../phy/simple_channel.h"
/*template <class PACKET>
*/bool compcxx_SimpleChannel_21/*<mac_packet_t  >*/::forward ( mac_packet_t  * p, double power, int in, int out, bool flag)
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


#line 225 "../../phy/simple_channel.h"

#line 225 "../../phy/simple_channel.h"
/*template <class PACKET>
*/void compcxx_SimpleChannel_21/*<mac_packet_t  >*/::from_phy ( mac_packet_t  * packet, double power, int in)
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

  











  packet->free();
}





#line 281 "../../phy/simple_channel.h"

#line 281 "../../phy/simple_channel.h"
/*template <class PACKET>
*/void compcxx_SimpleChannel_21/*<mac_packet_t  >*/::depart ( const triple<mac_packet_t  *, double, int> & data , unsigned int index)
{
  
  to_phy[data.third](data.first,data.second);
  return;
}


#line 289 "../../phy/simple_channel.h"

#line 289 "../../phy/simple_channel.h"
/*template <class PACKET>
*/bool compcxx_SimpleChannel_21/*<mac_packet_t  >*/::initSources(
  int		,	
  bool		)	
{
  return true;		
}


#line 297 "../../phy/simple_channel.h"

#line 297 "../../phy/simple_channel.h"
/*template <class PACKET>
*/void compcxx_SimpleChannel_21/*<mac_packet_t  >*/::getSourcePair(
  int		&,
  int		&)
{
  return;
}


#line 262 "../../common/cost.h"

#line 262 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_2/*<trigger_t  >*/::Set(trigger_t  const & data, double time)
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.time = time;
  m_event.data = data;
  m_event.object = this;
  m_event.active=true;
  m_simeng->ScheduleEvent(&m_event);
}


#line 274 "../../common/cost.h"

#line 274 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_2/*<trigger_t  >*/::Set(double time)
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.time = time;
  m_event.object = this;
  m_event.active=true;
  m_simeng->ScheduleEvent(&m_event);
}


#line 285 "../../common/cost.h"

#line 285 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_2/*<trigger_t  >*/::Cancel()
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.active = false;
}


#line 293 "../../common/cost.h"

#line 293 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_2/*<trigger_t  >*/::activate(CostEvent*e)
{
  assert(e==&m_event);
  m_event.active=false;
  (p_compcxx_parent->BroadcastTimer(m_event.data));
}




#line 262 "../../common/cost.h"

#line 262 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_3/*<trigger_t  >*/::Set(trigger_t  const & data, double time)
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.time = time;
  m_event.data = data;
  m_event.object = this;
  m_event.active=true;
  m_simeng->ScheduleEvent(&m_event);
}


#line 274 "../../common/cost.h"

#line 274 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_3/*<trigger_t  >*/::Set(double time)
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.time = time;
  m_event.object = this;
  m_event.active=true;
  m_simeng->ScheduleEvent(&m_event);
}


#line 285 "../../common/cost.h"

#line 285 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_3/*<trigger_t  >*/::Cancel()
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.active = false;
}


#line 293 "../../common/cost.h"

#line 293 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_3/*<trigger_t  >*/::activate(CostEvent*e)
{
  assert(e==&m_event);
  m_event.active=false;
  (p_compcxx_parent->UnicastTimer(m_event.data));
}




#line 70 "../../app/random_neighbor.h"

#line 71 "../../app/random_neighbor.h"

#line 80 "../../app/random_neighbor.h"
compcxx_RandomNeighbor_14::compcxx_RandomNeighbor_14()
{
    broadcast_timer.p_compcxx_parent=this /*connect broadcast_timer.to_component, */;
    unicast_timer.p_compcxx_parent=this /*connect unicast_timer.to_component, */;
}

#line 85 "../../app/random_neighbor.h"
compcxx_RandomNeighbor_14::~compcxx_RandomNeighbor_14()
{
}

#line 88 "../../app/random_neighbor.h"
void compcxx_RandomNeighbor_14::Start()
{
    m_transport_busy=false;
    SentPackets=0;
    RecvPackets=0;
    SendFailures=0;
    broadcast_timer.Set(Random(BroadcastInterval));
    unicast_timer.Set(Random(UnicastInterval));
}

#line 97 "../../app/random_neighbor.h"
void compcxx_RandomNeighbor_14::Stop()
{
    

}

#line 102 "../../app/random_neighbor.h"
void compcxx_RandomNeighbor_14::from_transport_ack ( bool flag)
{
    if(flag==false)SendFailures++;
    m_transport_busy=false;
}

#line 107 "../../app/random_neighbor.h"
void compcxx_RandomNeighbor_14::from_transport_data ( packet_t& pld, const ether_addr_t & dst)
{
    const packet_t & p = pld;

    Printf((DumpPackets,"receives %s\n",p.dump().c_str()));

    unsigned int i;
    if(p.dst_addr==ether_addr_t::BROADCAST)
    {
	    for(i=0;i<m_neighbors.size();i++)
	    {
	        if(m_neighbors[i]==p.src_addr)break;
	    }
	    if(i>=m_neighbors.size())
	    {
	        Printf((DumpPackets,"detects a new neighbor station%d\n",int(p.src_addr)));
	        m_neighbors.push_back(p.src_addr);
	    }
    }
    else
    {
	    if(p.dst_addr==MyEtherAddr)
	    {
	        RecvPackets++;
	    }
	    else
	    {
	        printf("rn%d received a packet with wrong destination!\n",(int)MyEtherAddr);
	    }
    }
}

#line 138 "../../app/random_neighbor.h"
void compcxx_RandomNeighbor_14::BroadcastTimer(trigger_t&)
{
    broadcast_timer.Set(Random(BroadcastInterval)+SimTime());
    if(m_transport_busy==true) return;
    packet_t p;
    p.src_addr=MyEtherAddr;
    p.dst_addr=ether_addr_t::BROADCAST;
    p.data_size=(int)Random(MaxPacketSize);
    m_transport_busy=true;
    Printf((DumpPackets,"broadcasts %s\n",p.dump().c_str()));
    (p_compcxx_NullMAC_16->from_network(p,p.dst_addr,p.data_size+2*ether_addr_t::LENGTH));
}

#line 150 "../../app/random_neighbor.h"
void compcxx_RandomNeighbor_14::UnicastTimer(trigger_t&)
{
    unicast_timer.Set(Random(UnicastInterval)+SimTime());
    if(m_transport_busy==true||m_neighbors.size()==0) return;
    packet_t p;
    p.src_addr=MyEtherAddr;
    p.dst_addr=m_neighbors[(int)Random((double)m_neighbors.size())];
    p.data_size=(int)Random(MaxPacketSize);
    m_transport_busy=true;
    Printf((DumpPackets,"unicasts %s\n",p.dump().c_str()));
    SentPackets++;
    (p_compcxx_NullMAC_16->from_network(p,p.dst_addr,p.data_size+2*ether_addr_t::LENGTH));
}



#line 43 "../../energy/battery.h"
void compcxx_SimpleBattery_18::Start()
{
    RemainingEnergy=InitialEnergy;
    m_last_time=0.0;
}


#line 49 "../../energy/battery.h"
void compcxx_SimpleBattery_18::Stop()
{
}


#line 53 "../../energy/battery.h"
double compcxx_SimpleBattery_18::power_in(double power, simtime_t t)
{
	
	if(t>m_last_time)
	{
        RemainingEnergy -= (t-m_last_time)*power;
        m_last_time=t;
    }
    return RemainingEnergy;
}

#line 63 "../../energy/battery.h"
double compcxx_SimpleBattery_18::query_in()
{
    return RemainingEnergy;
}












































































































































#line 163 "../../mac/null_mac.h"

#line 163 "../../mac/null_mac.h"
/*template <class PLD>
*/unsigned int compcxx_NullMAC_16/*<RandomNeighbor_Struct::packet_t >*/::RTSThreshold=0;


#line 166 "../../mac/null_mac.h"

#line 166 "../../mac/null_mac.h"
/*template <class PLD>
*/void compcxx_NullMAC_16/*<RandomNeighbor_Struct::packet_t >*/::Start()
{
	SentPackets=RecvPackets=0;
}


#line 172 "../../mac/null_mac.h"

#line 172 "../../mac/null_mac.h"
/*template <class PLD>
*/void compcxx_NullMAC_16/*<RandomNeighbor_Struct::packet_t >*/::Stop()
{
}




































#line 211 "../../mac/null_mac.h"

#line 211 "../../mac/null_mac.h"
/*template <class PLD>
*/void compcxx_NullMAC_16/*<RandomNeighbor_Struct::packet_t >*/::from_network(payload_t& pld, const ether_addr_t& dst, unsigned int size)
{
    packet_t* p     = packet_t::alloc();
    p->hdr.dst_addr = dst;
    p->hdr.src_addr = MyEtherAddr;
    p->pld          = pld;
    p->hdr.size     = size + sizeof(hdr_t);

    Printf((DumpPackets,"creates %s\n",p->dump().c_str()));

    (p_compcxx_DuplexTransceiver_17->from_mac(p));                 
    (p_compcxx_RandomNeighbor_14->from_transport_ack(true));      
    SentPackets++;
}











#line 236 "../../mac/null_mac.h"

#line 236 "../../mac/null_mac.h"
/*template <class PLD>
*/void compcxx_NullMAC_16/*<RandomNeighbor_Struct::packet_t >*/::from_phy(packet_t* pkt, bool errflag, double  power)
{
	RecvPackets++;
    Printf((DumpPackets,"receives %s with power %f\n",pkt->dump().c_str(),power));

    if(pkt->hdr.dst_addr==MyEtherAddr||pkt->hdr.dst_addr==ether_addr_t::BROADCAST||Promiscuity)
    {






		pkt->inc_pld_ref();
		(p_compcxx_RandomNeighbor_14->from_transport_data(pkt->pld,pkt->hdr.dst_addr));
    }




    pkt->free();
}



#line 262 "../../common/cost.h"

#line 262 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_4/*<trigger_t >*/::Set(trigger_t const & data, double time)
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.time = time;
  m_event.data = data;
  m_event.object = this;
  m_event.active=true;
  m_simeng->ScheduleEvent(&m_event);
}


#line 274 "../../common/cost.h"

#line 274 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_4/*<trigger_t >*/::Set(double time)
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.time = time;
  m_event.object = this;
  m_event.active=true;
  m_simeng->ScheduleEvent(&m_event);
}


#line 285 "../../common/cost.h"

#line 285 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_4/*<trigger_t >*/::Cancel()
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.active = false;
}


#line 293 "../../common/cost.h"

#line 293 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_4/*<trigger_t >*/::activate(CostEvent*e)
{
  assert(e==&m_event);
  m_event.active=false;
  (p_compcxx_parent->announce_pos(m_event.data));
}




#line 31 "../../mob/immobile.h"
void		compcxx_Immobile_15::pos_out( coordinate_t& pos, int id){return (p_compcxx_parent->to_channel_pos( pos,id));}
#line 32 "../../mob/immobile.h"

#line 52 "../../mob/immobile.h"
Visualizer	*compcxx_Immobile_15::visualizer = NULL;


#line 54 "../../mob/immobile.h"
void compcxx_Immobile_15::setVisualizer(
  Visualizer	*ptr)
{
  visualizer = ptr;
  return;
}


#line 61 "../../mob/immobile.h"
void compcxx_Immobile_15::announce_pos(trigger_t&)
{
  coordinate_t pos=coordinate_t(InitX,InitY);
  (p_compcxx_parent->to_channel_pos(pos,ID));
  Printf((true, "%d @ %f %f\n",ID, InitX, InitY));
#ifdef VISUAL_ROUTE
  printf("@ %f %d %f %f\n", SimTime(), ID, InitX, InitY);
#endif
  visualizer->nodeLocation( ID, InitX, InitY);
  return;
}


#line 73 "../../mob/immobile.h"
void compcxx_Immobile_15::Setup()
{
  return;
}


#line 78 "../../mob/immobile.h"
void compcxx_Immobile_15::Start()
{
  timer.Set(0.0);
  return;
}


#line 84 "../../mob/immobile.h"
void compcxx_Immobile_15::Stop()
{
  return;
}


#line 75 "../../phy/transceiver.h"

#line 75 "../../phy/transceiver.h"
/*template <class PACKET>
*/void compcxx_DuplexTransceiver_17/*<mac_packet_t  >*/::Start()
{
}


#line 80 "../../phy/transceiver.h"

#line 80 "../../phy/transceiver.h"
/*template <class PACKET>
*/void compcxx_DuplexTransceiver_17/*<mac_packet_t  >*/::Stop()
{
}


#line 85 "../../phy/transceiver.h"

#line 85 "../../phy/transceiver.h"
/*template <class PACKET>
*/void compcxx_DuplexTransceiver_17/*<mac_packet_t  >*/::from_mac(mac_packet_t  * p)
{
	double now = SimTime();
	if((p_compcxx_PowerManager_19->switch_state(PM_Struct::TX,now))<=0.0)  
    {
    	p->free();
		return;
    }	 
   
    if((p_compcxx_PowerManager_19->switch_state(PM_Struct::IDLE,now+p->hdr.tx_time))<=0.0)
    {
    	p->free();
    	return;
    } 
    p->hdr.wave_length=speed_of_light/Frequency;
    (p_compcxx_parent->to_channel_packet(p,TXPower * TXGain, ID));
}


#line 104 "../../phy/transceiver.h"

#line 104 "../../phy/transceiver.h"
/*template <class PACKET>
*/void compcxx_DuplexTransceiver_17/*<mac_packet_t  >*/::from_channel(mac_packet_t  * p, double power)
{
	double now = SimTime();
    if((p_compcxx_PowerManager_19->switch_state(PM_Struct::RX,now))<= 0.0)  
    {
       p->free();
       return;
    }
          
    if((p_compcxx_PowerManager_19->switch_state(PM_Struct::IDLE, now+p->hdr.tx_time))<=0.0)
    {
		p->free();
		return;
    } 
  
    double recv_power = power * RXGain;
    if( recv_power < CSThresh )
    {
    	p->free(); 
    }
	else
	{
		bool error = ( recv_power < RXThresh ) ;
	  	(p_compcxx_NullMAC_16->from_phy( p, error, recv_power ));
    }
}


#line 262 "../../common/cost.h"

#line 262 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_13/*<trigger_t >*/::Set(trigger_t const & data, double time)
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.time = time;
  m_event.data = data;
  m_event.object = this;
  m_event.active=true;
  m_simeng->ScheduleEvent(&m_event);
}


#line 274 "../../common/cost.h"

#line 274 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_13/*<trigger_t >*/::Set(double time)
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.time = time;
  m_event.object = this;
  m_event.active=true;
  m_simeng->ScheduleEvent(&m_event);
}


#line 285 "../../common/cost.h"

#line 285 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_13/*<trigger_t >*/::Cancel()
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.active = false;
}


#line 293 "../../common/cost.h"

#line 293 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_13/*<trigger_t >*/::activate(CostEvent*e)
{
  assert(e==&m_event);
  m_event.active=false;
  (p_compcxx_parent->power_switch(m_event.data));
}




#line 48 "../../energy/power.h"
compcxx_PowerManager_19::compcxx_PowerManager_19()
{
  upTime = 1.0;
  downTime = 0.0;
  initTime = 0.0;
  transient = false;
  switch_timer.p_compcxx_parent=this /*connect switch_timer.to_component, */;
  return;
}





#line 61 "../../energy/power.h"
void compcxx_PowerManager_19::failureStats(
  double	initializeTime)	
{
  transient = false;
  initTime = initializeTime;
  return;
}





#line 72 "../../energy/power.h"
void compcxx_PowerManager_19::failureStats(
  double	initializeTime,	
  double	cycleTime,	
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


#line 92 "../../energy/power.h"
void compcxx_PowerManager_19::Start()
{
  m_state = IDLE;
  m_switch_on = true;
  if( initTime != 0)
    switch_timer.Set( BeginTime + initTime);
  return;
}


#line 101 "../../energy/power.h"
void compcxx_PowerManager_19::Stop()
{
  return;
}






#line 110 "../../energy/power.h"
void compcxx_PowerManager_19::power_switch(
  trigger_t	&)
{
  
  if( m_switch_on == true)
  {
    m_state = OFF;
    (p_compcxx_SimpleBattery_18->power_in( 0.0, SimTime()));
    m_switch_on = false;
    if( transient == true)
      switch_timer.Set( SimTime() + Exponential( downTime));
  }
  else		
  {
    m_state = IDLE;
    (p_compcxx_SimpleBattery_18->power_in( IdlePower, SimTime()));
    m_switch_on = true;
    (p_compcxx_parent->from_pm_node_up());		
    switch_timer.Set( SimTime() + Exponential( upTime));
  }
  return;
}


#line 133 "../../energy/power.h"
double compcxx_PowerManager_19::switch_state(
  int		state,
  double	time) 
{
  double	power = 0.0;	
  if( time < BeginTime || time > FinishTime)
    return power;

  switch( m_state)
  {
    case TX:
      power = (p_compcxx_SimpleBattery_18->power_in( TXPower, time));
      break;
    case RX:
      power = (p_compcxx_SimpleBattery_18->power_in( RXPower, time));
      break;
    case IDLE:
      power = (p_compcxx_SimpleBattery_18->power_in( IdlePower, time));
      break;
    case SLEEP:
      power = (p_compcxx_SimpleBattery_18->power_in( SleepPower, time));
      break;
    case OFF:
      power = (p_compcxx_SimpleBattery_18->power_in( 0.0, time));
      return 0.0;
    case ON:
      power = (p_compcxx_SimpleBattery_18->power_in( IdlePower, time));
      m_state = IDLE;
      return power;
    default:
      fprintf( stderr,"invalid state to power manager\n");
      break;
  }
  m_state=state;
  return power;
} 
       

#line 170 "../../energy/power.h"
int compcxx_PowerManager_19::state_query()
{
  return m_state; 
}


#line 175 "../../energy/power.h"
double compcxx_PowerManager_19::energy_query()
{
  return (p_compcxx_SimpleBattery_18->query_in());
}


#line 44 "sim_80211.cc"
void compcxx_SensorNode_20::to_channel_packet(mac_packet_t* packet, double power, int id){return (to_channel_packet_f(packet,power,id));}
#line 45 "sim_80211.cc"
void compcxx_SensorNode_20::from_channel(mac_packet_t* packet, double power){return (phy.from_channel(packet,power));}
#line 46 "sim_80211.cc"
void compcxx_SensorNode_20::to_channel_pos(coordinate_t& pos, int id){return (to_channel_pos_f(pos,id));}
#line 48 "sim_80211.cc"

#line 51 "sim_80211.cc"
compcxx_SensorNode_20::compcxx_SensorNode_20()
{
}

#line 54 "sim_80211.cc"
compcxx_SensorNode_20::~compcxx_SensorNode_20()
{
}


#line 58 "sim_80211.cc"
void compcxx_SensorNode_20::Start()
{

}


#line 63 "sim_80211.cc"
void compcxx_SensorNode_20::Stop()
{
    SentPackets=app.SentPackets;
    RecvPackets=app.RecvPackets;
}


#line 69 "sim_80211.cc"
void compcxx_SensorNode_20::Setup()
{

    battery.InitialEnergy=1e6;
    
    app.BroadcastInterval=BroadcastInterval;
    app.UnicastInterval=UnicastInterval;
    app.MyEtherAddr=MyEtherAddr;
    app.MaxPacketSize=2*mac.RTSThreshold;
    app.DumpPackets=true;

    mob.setX( Random( MaxX));
    mob.setY( Random( MaxY));
    mob.setID( ID);

    mac.MyEtherAddr=MyEtherAddr;
    mac.DumpPackets=true;
    mac.Promiscuity=false;

   	pm.TXPower=1.6;
   	pm.RXPower=1.2;
   	pm.IdlePower=1.15;

    phy.setTXPower( 0.02283);
    phy.setTXGain( 1.0);
    phy.setRXGain( 1.0);
    phy.setFrequency( 9.14e8);
    phy.setRXThresh( 3.652e-10);
    phy.setCSThresh( 1.559e-11);
    phy.setID( ID);

    app.p_compcxx_NullMAC_16=&mac /*connect app.to_transport, mac.from_network*/;
    mac.p_compcxx_RandomNeighbor_14=&app /*connect mac.to_network_data, app.from_transport_data*/;
    mac.p_compcxx_RandomNeighbor_14=&app /*connect mac.to_network_ack, app.from_transport_ack*/;

    mac.p_compcxx_DuplexTransceiver_17=&phy /*connect mac.to_phy, phy.from_mac*/;
    phy.p_compcxx_NullMAC_16=&mac /*connect phy.to_mac, mac.from_phy*/;

    phy.p_compcxx_parent=this /*connect phy.to_channel, */;
     /*connect , phy.from_channel*/;
    phy.p_compcxx_PowerManager_19=&pm /*connect phy.to_power_switch, pm.switch_state*/;
    
    pm.p_compcxx_SimpleBattery_18=&battery /*connect pm.to_battery_query, battery.query_in*/;
    pm.p_compcxx_SimpleBattery_18=&battery /*connect pm.to_battery_power, battery.power_in*/;

    pm.p_compcxx_parent=this /*connect pm.from_pm_node_up, */;

    mob.p_compcxx_parent=this /*connect mob.pos_out, */;
    return;
}






#line 124 "sim_80211.cc"
void compcxx_SensorNode_20::from_pm_node_up()
{
  return;
}


#line 146 "sim_80211.cc"
void compcxx_Sim_80211_22 :: Stop()
{
    int i,sent=0,recv=0;
    for(i=0;i<NumNodes;i++)
    {
	    sent+=nodes[i].SentPackets;
	    recv+=nodes[i].RecvPackets;
    }

    printf("total packets sent: %d, received: %d\n",sent,recv);
}

#line 157 "sim_80211.cc"
void compcxx_Sim_80211_22 :: Setup()
{
    int i;
    nodes.SetSize(NumNodes);
    for(i=0;i<NumNodes;i++)
    {
      nodes[i].MaxX=MaxX;
      nodes[i].MaxY=MaxY;
      nodes[i].MyEtherAddr=i;
      nodes[i].BroadcastInterval=BroadcastInterval;
      nodes[i].UnicastInterval=UnicastInterval;
      nodes[i].ID=i;
      nodes[i].Setup();
    }
      






    nodes[0].mob.setVisualizer( Visualizer::instantiate());

    channel.setNumNodes( NumNodes);
    channel.setDumpPackets( false);
    channel.setCSThresh( nodes[0].phy.getCSThresh());
    channel.setRXThresh( nodes[0].phy.getRXThresh());
    channel.useFreeSpace();

    channel.setWaveLength( speed_of_light/nodes[0].phy.getFrequency());
    channel.setX( MaxX);
    channel.setY( MaxY);
    channel.setGridEnabled( false);
    channel.setMaxTXPower( nodes[0].phy.getTXPower());
    
    channel.Setup();

    for(i=0;i<NumNodes;i++)
    {
      nodes[i].to_channel_packet_f.Connect(channel,(compcxx_component::SensorNode_to_channel_packet_f_t)&compcxx_SimpleChannel_21::from_phy) /*connect nodes[i].to_channel_packet,channel.from_phy*/;
      nodes[i].to_channel_pos_f.Connect(channel,(compcxx_component::SensorNode_to_channel_pos_f_t)&compcxx_SimpleChannel_21::pos_in) /*connect nodes[i].to_channel_pos,channel.pos_in*/;
      channel.to_phy[i].Connect(nodes[i],(compcxx_component::SimpleChannel_to_phy_f_t)&compcxx_SensorNode_20::from_channel) /*connect channel.to_phy[i],nodes[i].from_channel */;
    }




    for(i=0;i<NumNodes;i++)
    {
      nodes[i].pm.BeginTime=0.0;
      nodes[i].pm.FinishTime=StopTime();
      nodes[i].pm.failureStats( 0.0);
    }
    return;
}


#line 213 "sim_80211.cc"
int main(int argc, char* argv[])
{
    compcxx_Sim_80211_22 sim;

    sim.StopTime( 1000);
    sim.Seed=1234;

    sim.MaxX=5000;
    sim.MaxY=5000;
    sim.BroadcastInterval=5;
    sim.UnicastInterval=10;
    sim.NumNodes=100;

    if(argc>=2)sim.StopTime( atof(argv[1]));
    if(argc>=3)sim.NumNodes=atoi(argv[2]);
    if(argc>=4)sim.MaxX=sim.MaxY=atof(argv[3]);

    printf("StopTime: %.1f, NumNodes: %d, Terrain: %.0f by %.0f\n", 
	   sim.StopTime(), sim.NumNodes, sim.MaxX, sim.MaxY);

    sim.Setup();
    sim.Run();

    return 0;
}
