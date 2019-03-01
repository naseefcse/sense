
#line 48 "sim_ssab.cc"
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

#line 54 "sim_ssab.cc"


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



#line 55 "sim_ssab.cc"








#line 1 "../../app/cbr.h"


































#ifndef cbr_h
#define cbr_h
#include <vector>
#include <set>











struct CBR_Struct
{






	struct packet_t
	{
		ether_addr_t src_addr;
		ether_addr_t dst_addr;
		int data_size;
		simtime_t send_time;








		std::string dump() const 
		{
		    char buffer[50];
		    sprintf(buffer,"%d %d %d",(int)src_addr,(int)dst_addr,data_size);
	    	return buffer;
		}
		void dump(std::string& str) const { str=dump();}
	};








	
    typedef triple<ether_addr_t,int,double> connection_t;
    std::vector<connection_t> Connections;
};













#line 303 "../../app/cbr.h"
#endif /* cbr_h*/

#line 62 "sim_ssab.cc"


#line 1 "../../mob/immobile.h"























#ifndef immobile_h
#define immobile_h


#line 89 "../../mob/immobile.h"
#endif /* immobile_h*/

#line 63 "sim_ssab.cc"


#line 1 "../../net/ssab.h"



























#ifndef SSAB_h
#define SSAB_h
#include <map>
#include <list>

template <class PLD>
struct SSAB_Struct
{
    struct hdr_struct
    {
	unsigned int seq_number;
	unsigned int size;
	double send_time;
	unsigned int hop;
	
	bool dump(std::string& str) const 
	{ 
	    char buffer[30];
	    sprintf(buffer,"%d %d",seq_number,size); 
	    str=buffer;
	    return true;
	}
    };

    typedef PLD payload_t;
    typedef smart_packet_t<hdr_struct,PLD> packet_t;

};




#line 330 "../../net/ssab.h"
#endif /*SSAB_h*/

#line 64 "sim_ssab.cc"


#line 1 "../../mac/bcast_mac.h"
























#ifndef broadcast_mac_h
#define broadcast_mac_h
#include <map>

template <class PLD>
struct BcastMAC_Struct
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
	
        struct frame_control    dh_fc;
        u_int16_t               dh_duration;
        ether_addr_t            dh_da;
        ether_addr_t            dh_sa;
        ether_addr_t            dh_bssid;
        u_int16_t               dh_scontrol;

	
	
	double tx_time;                              
	double wave_length;                          
	unsigned int size;

        bool dump(std::string& str) const;
    };

    typedef PLD payload_t;
    typedef smart_packet_t<hdr_struct,PLD> packet_t;

    
    static double SlotTime;
    static double SIFSTime;
    
    


    
    static int PLCP_HDR_LEN;
    static int HDR_LEN;
    static int FCS_LEN;
    
    
    
    static double DataRate;
    static double BasicRate;
    static double CPThreshold;

};


template <class PLD> double BcastMAC_Struct<PLD>::SlotTime=0.000020;



template <class PLD> int BcastMAC_Struct<PLD>::PLCP_HDR_LEN = (144+48) >> 3;
template <class PLD> int BcastMAC_Struct<PLD>::HDR_LEN = PLCP_HDR_LEN + FCS_LEN + 4*ether_addr_t::LENGTH + 6;
template <class PLD> int BcastMAC_Struct<PLD>::FCS_LEN = 4;

template <class PLD> double BcastMAC_Struct<PLD>::DataRate=1.0e6/8;
template <class PLD> double BcastMAC_Struct<PLD>::BasicRate=1.0e6/8;
template <class PLD> double BcastMAC_Struct<PLD>::CPThreshold=10;



#line 556 "../../mac/bcast_mac.h"
template <class PLD>
bool BcastMAC_Struct<PLD>::hdr_struct::dump(
  std::string	&str) const
{
  std::string type;
  bool payload=false;
  switch(dh_fc.fc_type)
  {
    case BcastMAC_Struct<PLD>::MAC_Type_Control:
      switch(dh_fc.fc_subtype)
      {
	case BcastMAC_Struct<PLD>::MAC_Subtype_RTS:
	  type="RTS";
	  break;
	case BcastMAC_Struct<PLD>::MAC_Subtype_CTS:
	  type="CST";
	  break;
	case BcastMAC_Struct<PLD>::MAC_Subtype_ACK:
	  type="ACK";
	  break;
	default:
	  type="UNKNOWN";
      }
      break;
    case BcastMAC_Struct<PLD>::MAC_Type_Data:
      if(dh_fc.fc_subtype == BcastMAC_Struct<PLD>::MAC_Subtype_Data)
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

#endif /* broadcast_mac_h */

#line 65 "sim_ssab.cc"


#line 1 "../../phy/transceiver.h"























#ifndef transceiver_h
#define transceiver_h









#line 132 "../../phy/transceiver.h"
#endif /* transceiver_h */



#line 66 "sim_ssab.cc"


#line 1 "../../phy/simple_channel.h"
























#ifndef simple_channel_h
#define simple_channel_h

#include <algorithm>


#line 305 "../../phy/simple_channel.h"
#endif /* simple_channel_h */

#line 67 "sim_ssab.cc"


#line 1 "../../energy/battery.h"























#ifndef battery_h
#define battery_h


#line 206 "../../energy/battery.h"
#endif /* battery_h */


#line 68 "sim_ssab.cc"


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

#line 69 "sim_ssab.cc"




































typedef CBR_Struct::packet_t app_packet_t;
typedef SSAB_Struct<app_packet_t>::packet_t net_packet_t;
typedef BcastMAC_Struct<net_packet_t*>::packet_t mac_packet_t;














#include "compcxx_sim_ssab.h"
class compcxx_SimpleChannel_18;
#line 401 "../../common/cost.h"
/*template <class T> */
#line 401 "../../common/cost.h"
class compcxx_InfiTimer_8 : public compcxx_component, public TimerBase
{
 public:
  struct event_t : public CostEvent { triple<mac_packet_t *, double, int>  data; };
  compcxx_InfiTimer_8();
  virtual ~compcxx_InfiTimer_8();
	
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
public:compcxx_SimpleChannel_18* p_compcxx_parent;};

/*template <class PACKET>

*/
#line 30 "../../phy/simple_channel.h"
class compcxx_SimpleChannel_18 : public compcxx_component, public TypeII
{
 public:
  bool		initSources( int numSources, bool oneSink);
  void		getSourcePair( int &src, int &dest);

  /*inport */void		from_phy( mac_packet_t  * p, double power, int id);
  class my_SimpleChannel_to_phy_f_t:public compcxx_functor<SimpleChannel_to_phy_f_t>{ public:void	 operator() ( mac_packet_t  * p, double power ) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])( p,power ); return (c[0]->*f[0])( p,power );};};compcxx_array<my_SimpleChannel_to_phy_f_t > to_phy;/*outportvoid	to_phy( mac_packet_t  * p, double power )*/;
  /*inport */void		pos_in( const coordinate_t& pos, int id);
	
  compcxx_InfiTimer_8/*< triple<mac_packet_t  *, double, int> > */propagation_delay;
  /*inport */void		depart( const triple<mac_packet_t  *, double, int> & data, unsigned int );

  compcxx_SimpleChannel_18() { propagation_delay.p_compcxx_parent=this /*connect propagation_delay.to_component, */; }
  virtual ~compcxx_SimpleChannel_18() {}
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

class compcxx_CBR_10;
#line 303 "../../common/cost.h"
/*template <class T> */
#line 303 "../../common/cost.h"
class compcxx_MultiTimer_2 : public compcxx_component, public TimerBase
{
 public:
  struct event_t : public CostEvent { trigger_t  data; };
  compcxx_MultiTimer_2();
  virtual ~compcxx_MultiTimer_2();
	
  /*outport void to_component(trigger_t  &, unsigned int i)*/;
 
  inline void Set(double t, unsigned int index =0);
  inline void Set( trigger_t  const & data, double t,unsigned int index=0);
  void Cancel (unsigned int index=0);
  void activate(CostEvent*event);

  inline bool Active(unsigned int index=0) { return GetEvent(index)->active; }
  inline double GetTime(unsigned int index=0) { return GetEvent(index)->time; }
  inline trigger_t  & GetData(unsigned int index=0) { return GetEvent(index)->data; }
  inline void SetData(trigger_t  const &d, unsigned int index) { GetEvent(index)->data = d; }

  event_t* GetEvent(unsigned int index);
				 
 private:
  std::vector<event_t*> m_events;
  CostSimEng* m_simeng;
public:compcxx_CBR_10* p_compcxx_parent;};

class compcxx_SSAB_11;
#line 104 "../../app/cbr.h"
class compcxx_CBR_10 : public compcxx_component, public TypeII, public CBR_Struct
{
public:













	/*outport void to_transport (packet_t & packet, ether_addr_t & dst, unsigned int size)*/;
	/*inport */inline void from_transport ( packet_t & pkt);










    compcxx_MultiTimer_2 /*< trigger_t > */connection_timer;





    /*inport */inline void create(trigger_t& t, int i);
    
    











    ether_addr_t MyEtherAddr;
    bool DumpPackets;
    double FinishTime;






    long SentPackets, RecvPackets;
    double TotalDelay;
    void	clearStats() { SentPackets = RecvPackets = 0; TotalDelay = 0; }







    compcxx_CBR_10();
    virtual ~compcxx_CBR_10();
    void Start();
    void Stop();





    typedef std::set<ether_addr_t, ether_addr_t::compare> addr_table_t;
    addr_table_t m_sources;

public:compcxx_SSAB_11* p_compcxx_SSAB_11;};






#line 27 "../../energy/battery.h"
class compcxx_SimpleBattery_14 : public compcxx_component, public TypeII
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

class compcxx_BcastMAC_12;/*template <class T> */
#line 241 "../../common/cost.h"
class compcxx_Timer_7 : public compcxx_component, public TimerBase
{
 public:
  struct event_t : public CostEvent { bool data; };
  

  compcxx_Timer_7() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
  inline void Set(bool const &, double );
  inline void Set(double );
  inline double GetTime() { return m_event.time; }
  inline bool Active() { return m_event.active; }
  inline bool & GetData() { return m_event.data; }
  inline void SetData(bool const &d) { m_event.data = d; }
  void Cancel();
  /*outport void to_component(bool &)*/;
  void activate(CostEvent*);
 private:
  CostSimEng* m_simeng;
  event_t m_event;
public:compcxx_BcastMAC_12* p_compcxx_parent;};

class compcxx_BcastMAC_12;/*template <class T> */
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
public:compcxx_BcastMAC_12* p_compcxx_parent;};

class compcxx_BcastMAC_12;/*template <class T> */
#line 241 "../../common/cost.h"
class compcxx_Timer_5 : public compcxx_component, public TimerBase
{
 public:
  struct event_t : public CostEvent { trigger_t data; };
  

  compcxx_Timer_5() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
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
public:compcxx_BcastMAC_12* p_compcxx_parent;};

class compcxx_BcastMAC_12;/*template <class T> */
#line 241 "../../common/cost.h"
class compcxx_Timer_6 : public compcxx_component, public TimerBase
{
 public:
  struct event_t : public CostEvent { trigger_t data; };
  

  compcxx_Timer_6() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
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
public:compcxx_BcastMAC_12* p_compcxx_parent;};

class compcxx_SSAB_11;class compcxx_DuplexTransceiver_13;/*template <class PLD>
*/
#line 118 "../../mac/bcast_mac.h"
class compcxx_BcastMAC_12 : public compcxx_component, public TypeII, public BcastMAC_Struct<net_packet_t* >
{
 public:
    enum MAC_STATE {
      MAC_IDLE        = 0x0000,   
      MAC_DEFER       = 0x0001,   
      MAC_SEND        = 0x0002,   
    };

    ether_addr_t	MyEtherAddr;
    bool	DumpPackets;
    bool	Promiscuity;
    double	IFS;
    long	SentPackets;
    long	RecvPackets;
    long	rrCollisions;

    

    
    
    

    typedef triple<net_packet_t* ,unsigned int,double> net_inf_t;

    
    
    /*inport */void from_network( net_packet_t* const& pld, unsigned int size, double backoff);
    /*inport */void cancel ();
    /*outport void to_network_data ( net_packet_t* pld, double receive_power )*/;
    /*outport void to_network_ack ( bool ack)*/;
    
    /*outport void to_network_recv_recv_coll( net_packet_t* pld1, net_packet_t* pld2)*/;
    
    /*inport */void from_phy (packet_t* pkt, bool errflag, double power);
    /*outport void to_phy (packet_t* pkt)*/;

    compcxx_Timer_4 /*<trigger_t> */defer_timer;                   
    compcxx_Timer_5 /*<trigger_t> */recv_timer;                    
    compcxx_Timer_6 /*<trigger_t> */send_timer;
    compcxx_Timer_7 /*<bool> */ack_timer;

    compcxx_BcastMAC_12();
    virtual ~compcxx_BcastMAC_12();
        
    void	Start();		
    void	Stop();			
    void	clearStats() { SentPackets = 0; RecvPackets = 0; rrCollisions = 0;}

    
    
    /*inport */void DeferTimer(trigger_t&);
    /*inport */void RecvTimer(trigger_t&);
    /*inport */void SendTimer(trigger_t&);
    /*inport */void AckTimer(bool&);

 private:

    void RecvData(packet_t* p);

    void           StartDefer();
    inline void    ResumeDefer();

    
    void DropPacket(packet_t* p, const char*);

    MAC_STATE m_state;       

    struct {
        payload_t pld;
        unsigned int size;
        double delay;
    }m_pldinf;    
    simtime_t m_tx_end;   
    struct {
        packet_t* packet;
        bool errflag;
        double power;
    } m_rxinf;
    u_int16_t m_seq_no;      
    simtime_t m_defer_start;
    bool m_sr_collision;

    
    
    
    typedef std::map<ether_addr_t, u_int16_t, ether_addr_t::compare> cache_t;
    cache_t m_recv_cache;
public:compcxx_SSAB_11* p_compcxx_SSAB_11;public:compcxx_DuplexTransceiver_13* p_compcxx_DuplexTransceiver_13;};

class compcxx_Immobile_16;/*template <class T> */
#line 241 "../../common/cost.h"
class compcxx_Timer_3 : public compcxx_component, public TimerBase
{
 public:
  struct event_t : public CostEvent { trigger_t data; };
  

  compcxx_Timer_3() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
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
public:compcxx_Immobile_16* p_compcxx_parent;};

class compcxx_SensorNode_17;
#line 27 "../../mob/immobile.h"
class compcxx_Immobile_16 : public compcxx_component, public TypeII
{
 public:
  compcxx_Timer_3 /*<trigger_t>	*/timer;    
  /*outport */void		pos_out( coordinate_t& pos, int id);
  /*inport */void		announce_pos( trigger_t& t);

  compcxx_Immobile_16() { timer.p_compcxx_parent=this /*connect timer.to_component, */; }
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
public:compcxx_SensorNode_17* p_compcxx_parent;};

class compcxx_CBR_10;class compcxx_BcastMAC_12;/*template <class PLD>
*/
#line 59 "../../net/ssab.h"
class compcxx_SSAB_11 : public compcxx_component, public TypeII, public SSAB_Struct<app_packet_t >
{
 public:

    ether_addr_t MyEtherAddr;
    simtime_t ForwardDelay;
    double RXThresh;
    bool DumpPackets;

    int SentPackets;
    int RecvPackets;
    int RecvUniPackets;
    double TotalDelay;
    int TotalSamples;
    int TotalHop;
    

    /*inport */inline void from_transport( payload_t& pld, ether_addr_t& dst, unsigned int size);
    /*inport */inline void from_mac_data (packet_t* pkt, double power);
    /*inport */inline void from_mac_ack (bool errflag);
    /*inport */inline void	from_mac_recv_recv_coll( packet_t *pkt1, packet_t *pkt2);

    
    /*inport */void	from_pm_node_up();

    /*outport void cancel ()*/;
    /*outport void to_transport ( payload_t& pld )*/;
    /*outport void to_mac (packet_t* pkt, unsigned int size, double backoff)*/;

    void Start();
    void Stop();
    compcxx_SSAB_11();
    virtual ~compcxx_SSAB_11();
	
 protected:

    bool m_mac_busy;                  
    unsigned int m_seq_number;        
    typedef std::list<pair<packet_t*,double> > packet_buffer_t;
    packet_buffer_t m_packet_buffer;
    packet_t* m_active_packet;

    class seq_number_t
    {
    public:
	seq_number_t () : current(0),bits(0u) {}
	bool check(int n)
	{
	    if( n + (int)sizeof(unsigned long) <= current ) return true;
	    if ( n > current )
	    {
		bits = bits << (n - current);
		current = n;
	    }

	    unsigned long flag = 1 << (current - n);
	    unsigned long r = flag & bits;
	    bits |= flag;
	    return r;
	}
    private:
	int current;
	uint32_t bits;
    };
	






    typedef std::map<ether_addr_t, seq_number_t, ether_addr_t::compare> cache_t;
    cache_t m_seq_cache;

public:compcxx_CBR_10* p_compcxx_CBR_10;public:compcxx_BcastMAC_12* p_compcxx_BcastMAC_12;};

class compcxx_BcastMAC_12;class compcxx_PowerManager_15;class compcxx_SensorNode_17;/*template <class PACKET>
*/
#line 34 "../../phy/transceiver.h"
class compcxx_DuplexTransceiver_13  : public compcxx_component, public TypeII
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
public:compcxx_BcastMAC_12* p_compcxx_BcastMAC_12;public:compcxx_PowerManager_15* p_compcxx_PowerManager_15;public:compcxx_SensorNode_17* p_compcxx_parent;};

class compcxx_PowerManager_15;/*template <class T> */
#line 241 "../../common/cost.h"
class compcxx_Timer_9 : public compcxx_component, public TimerBase
{
 public:
  struct event_t : public CostEvent { trigger_t data; };
  

  compcxx_Timer_9() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
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
public:compcxx_PowerManager_15* p_compcxx_parent;};

class compcxx_SimpleBattery_14;class compcxx_SSAB_11;
#line 11 "../../energy/power.h"
class compcxx_PowerManager_15 : public compcxx_component, public TypeII, public PM_Struct
{
 public:
  compcxx_PowerManager_15();

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
  compcxx_Timer_9/*<trigger_t> */switch_timer;
 private:
  int		m_state; 
  bool		m_switch_on;
  bool		transient;
  double	upTime;
  double	downTime;
  double	initTime;
public:compcxx_SimpleBattery_14* p_compcxx_SimpleBattery_14;public:compcxx_SSAB_11* p_compcxx_SSAB_11;};


#line 122 "sim_ssab.cc"
class compcxx_SensorNode_17 : public compcxx_component, public TypeII
{
public:

    compcxx_CBR_10 app;
    compcxx_SSAB_11 /*<app_packet_t> */net;
    compcxx_BcastMAC_12 /*<net_packet_t*> */mac;
    
    
    compcxx_DuplexTransceiver_13 /*< mac_packet_t > */phy;
    
    compcxx_SimpleBattery_14 battery;
    
    compcxx_PowerManager_15 pm;
    
    compcxx_Immobile_16 mob;
    
    double MaxX, MaxY;  
    ether_addr_t MyEtherAddr; 
    int ID; 

    virtual ~compcxx_SensorNode_17();
    void Start();
    void Stop();
    void Setup();






	class my_SensorNode_to_channel_packet_f_t:public compcxx_functor<SensorNode_to_channel_packet_f_t>{ public:void  operator() (mac_packet_t* packet, double power, int id) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(packet,power,id); return (c[0]->*f[0])(packet,power,id);};};my_SensorNode_to_channel_packet_f_t to_channel_packet_f;/*outport */void to_channel_packet(mac_packet_t* packet, double power, int id);
	/*inport */void from_channel (mac_packet_t* packet, double power);
	class my_SensorNode_to_channel_pos_f_t:public compcxx_functor<SensorNode_to_channel_pos_f_t>{ public:void  operator() (coordinate_t& pos, int id) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(pos,id); return (c[0]->*f[0])(pos,id);};};my_SensorNode_to_channel_pos_f_t to_channel_pos_f;/*outport */void to_channel_pos(coordinate_t& pos, int id);
};


#line 315 "sim_ssab.cc"
class compcxx_RoutingSim_19 : public compcxx_component, public CostSimEng
{
public:
    void Start();
    void Stop();






    double MaxX, MaxY;
    int NumNodes;
    int NumSourceNodes;
    int NumConnections;
    int PacketSize;
    double Interval;






    compcxx_array<compcxx_SensorNode_17 >nodes;
    compcxx_SimpleChannel_18 /*< mac_packet_t > */channel;

    void Setup();
};


#line 434 "../../common/cost.h"
compcxx_InfiTimer_8
#line 433 "../../common/cost.h"
/*template <class T>
*//*<T>*/::compcxx_InfiTimer_8()
{
  m_simeng = CostSimEng::Instance();
  m_allocator = m_simeng->GetAllocator(sizeof(event_t));
  GetEvent(0);
}


#line 442 "../../common/cost.h"
compcxx_InfiTimer_8
#line 441 "../../common/cost.h"
/*template <class T>
*//*<T>*/::~compcxx_InfiTimer_8()
{
  for(unsigned int i=0;i<m_events.size();i++)
    m_allocator->free(m_events[i]);
}


#line 448 "../../common/cost.h"
/*template <class T>
*//*typename */compcxx_InfiTimer_8/*<T>*/::event_t* compcxx_InfiTimer_8/*<triple<mac_packet_t *, double, int>  >*/::GetEvent(unsigned int index)
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
*/unsigned int compcxx_InfiTimer_8/*<triple<mac_packet_t *, double, int>  >*/::Set(triple<mac_packet_t *, double, int>  const & data, double time)
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
*/unsigned int compcxx_InfiTimer_8/*<triple<mac_packet_t *, double, int>  >*/::Set(double time)
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
*/void compcxx_InfiTimer_8/*<triple<mac_packet_t *, double, int>  >*/::Cancel(unsigned int index)
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
*/void compcxx_InfiTimer_8/*<triple<mac_packet_t *, double, int>  >*/::activate(CostEvent*e)
{
  event_t * event = (event_t*)e;
  event->active = false;
  ReleaseSlot(event->index);
  (p_compcxx_parent->depart(event->data,event->index));
}


#line 510 "../../common/cost.h"

#line 510 "../../common/cost.h"
/*template <class T>
*/unsigned int compcxx_InfiTimer_8/*<triple<mac_packet_t *, double, int>  >*/::GetSlot()
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
*/void compcxx_SimpleChannel_18/*<mac_packet_t  >*/::Setup()
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
*/void compcxx_SimpleChannel_18/*<mac_packet_t  >*/::Start()
{
}


#line 127 "../../phy/simple_channel.h"

#line 127 "../../phy/simple_channel.h"
/*template <class PACKET>
*/void compcxx_SimpleChannel_18/*<mac_packet_t  >*/::Stop()
{
}


#line 132 "../../phy/simple_channel.h"

#line 132 "../../phy/simple_channel.h"
/*template <class PACKET>
*/void compcxx_SimpleChannel_18/*<mac_packet_t  >*/::pos_in(const coordinate_t & pos, int id)
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
*/bool compcxx_SimpleChannel_18/*<mac_packet_t  >*/::isNeighbor(
  double	power,
  int		src,
  int		dest)
{
  return forward( NULL, power, src, dest, false);
}


#line 181 "../../phy/simple_channel.h"

#line 181 "../../phy/simple_channel.h"
/*template <class PACKET>
*/bool compcxx_SimpleChannel_18/*<mac_packet_t  >*/::forward ( mac_packet_t  * p, double power, int in, int out, bool flag)
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
*/void compcxx_SimpleChannel_18/*<mac_packet_t  >*/::from_phy ( mac_packet_t  * packet, double power, int in)
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
*/void compcxx_SimpleChannel_18/*<mac_packet_t  >*/::depart ( const triple<mac_packet_t  *, double, int> & data , unsigned int index)
{
  
  to_phy[data.third](data.first,data.second);
  return;
}


#line 289 "../../phy/simple_channel.h"

#line 289 "../../phy/simple_channel.h"
/*template <class PACKET>
*/bool compcxx_SimpleChannel_18/*<mac_packet_t  >*/::initSources(
  int		,	
  bool		)	
{
  return true;		
}


#line 297 "../../phy/simple_channel.h"

#line 297 "../../phy/simple_channel.h"
/*template <class PACKET>
*/void compcxx_SimpleChannel_18/*<mac_packet_t  >*/::getSourcePair(
  int		&,
  int		&)
{
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
*//*typename */compcxx_MultiTimer_2/*<T>*/::event_t* compcxx_MultiTimer_2/*<trigger_t  >*/::GetEvent(unsigned int index)
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
*/void compcxx_MultiTimer_2/*<trigger_t  >*/::Set(trigger_t  const & data, double time, unsigned int index)
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
*/void compcxx_MultiTimer_2/*<trigger_t  >*/::Set(double time, unsigned int index)
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
*/void compcxx_MultiTimer_2/*<trigger_t  >*/::Cancel(unsigned int index)
{
  event_t * e = GetEvent(index);
  if(e->active)
    m_simeng->CancelEvent(e);
  e->active = false;
}


#line 390 "../../common/cost.h"

#line 390 "../../common/cost.h"
/*template <class T>
*/void compcxx_MultiTimer_2/*<trigger_t  >*/::activate(CostEvent*e)
{
  event_t * event = (event_t*)e;
  event->active = false;
  (p_compcxx_parent->create(event->data,event->index));
}





#line 138 "../../app/cbr.h"

#line 189 "../../app/cbr.h"
compcxx_CBR_10::compcxx_CBR_10()
{
	connection_timer.p_compcxx_parent=this /*connect connection_timer.to_component, */;
}


#line 194 "../../app/cbr.h"
compcxx_CBR_10::~compcxx_CBR_10()
{
}








#line 204 "../../app/cbr.h"
void compcxx_CBR_10::Start()
{
    SentPackets=RecvPackets=0l;
    TotalDelay=0.0;
    for(unsigned int i=0;i<Connections.size();i++)
    {
#ifdef COST_DEBUG
	printf("connection: %d -> %d, packet size: %d, interval: %f\n",
	(int)MyEtherAddr, (int)Connections[i].first, Connections[i].second, Connections[i].third); 
#endif
		connection_timer.Set(Random(Connections[i].third),i);
    }
}






#line 222 "../../app/cbr.h"
void compcxx_CBR_10::Stop()
{
#ifdef COST_DEBUG
    if(SentPackets!=0)
    {
        printf("%d sent %ld packets to ",(int)MyEtherAddr,SentPackets);
        for(unsigned int i=0;i<Connections.size();i++)
        {
		    printf("%d ",(int)Connections[i].first);
        }
        printf("\n");
    }
    if(RecvPackets!=0) 
    {
        
        addr_table_t::iterator iter;
        printf("                       %d received %ld packets from ", (int)MyEtherAddr, RecvPackets);
        for(iter=m_sources.begin();iter!=m_sources.end();iter++)
        {
            printf("%d ", (int)(*iter));
        }
        printf("\n");
    }
#endif
}






#line 252 "../../app/cbr.h"
void compcxx_CBR_10::from_transport(packet_t& p)
{









    Printf((DumpPackets,"receives %s\n",p.dump().c_str()));

    if(p.dst_addr==MyEtherAddr)
    {
		RecvPackets++;
		TotalDelay += SimTime() - p.send_time;
		m_sources.insert(p.src_addr);
    }
    else
    {
		printf("cbr %d received a packet destined for %d\n",
	       (int)MyEtherAddr,(int)p.dst_addr);
    }
}










#line 286 "../../app/cbr.h"
void compcxx_CBR_10::create(trigger_t&, int index)
{
    if(SimTime()>FinishTime) return;

    packet_t p;
    p.data_size=Connections[index].second + 2*ether_addr_t::LENGTH;
    p.dst_addr=Connections[index].first;
    p.src_addr=MyEtherAddr;
    p.send_time=SimTime();

    Printf((DumpPackets,"sends %s\n",p.dump().c_str()));

    (p_compcxx_SSAB_11->from_transport(p,p.dst_addr,p.data_size));
    SentPackets++;
    connection_timer.Set(SimTime()+Connections[index].third,index);
}


#line 43 "../../energy/battery.h"
void compcxx_SimpleBattery_14::Start()
{
    RemainingEnergy=InitialEnergy;
    m_last_time=0.0;
}


#line 49 "../../energy/battery.h"
void compcxx_SimpleBattery_14::Stop()
{
}


#line 53 "../../energy/battery.h"
double compcxx_SimpleBattery_14::power_in(double power, simtime_t t)
{
	
	if(t>m_last_time)
	{
        RemainingEnergy -= (t-m_last_time)*power;
        m_last_time=t;
    }
    return RemainingEnergy;
}

#line 63 "../../energy/battery.h"
double compcxx_SimpleBattery_14::query_in()
{
    return RemainingEnergy;
}












































































































































#line 262 "../../common/cost.h"

#line 262 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_7/*<bool >*/::Set(bool const & data, double time)
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
*/void compcxx_Timer_7/*<bool >*/::Set(double time)
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
*/void compcxx_Timer_7/*<bool >*/::Cancel()
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.active = false;
}


#line 293 "../../common/cost.h"

#line 293 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_7/*<bool >*/::activate(CostEvent*e)
{
  assert(e==&m_event);
  m_event.active=false;
  (p_compcxx_parent->AckTimer(m_event.data));
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
  (p_compcxx_parent->DeferTimer(m_event.data));
}




#line 262 "../../common/cost.h"

#line 262 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_5/*<trigger_t >*/::Set(trigger_t const & data, double time)
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
*/void compcxx_Timer_5/*<trigger_t >*/::Set(double time)
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
*/void compcxx_Timer_5/*<trigger_t >*/::Cancel()
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.active = false;
}


#line 293 "../../common/cost.h"

#line 293 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_5/*<trigger_t >*/::activate(CostEvent*e)
{
  assert(e==&m_event);
  m_event.active=false;
  (p_compcxx_parent->RecvTimer(m_event.data));
}




#line 262 "../../common/cost.h"

#line 262 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_6/*<trigger_t >*/::Set(trigger_t const & data, double time)
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
*/void compcxx_Timer_6/*<trigger_t >*/::Set(double time)
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
*/void compcxx_Timer_6/*<trigger_t >*/::Cancel()
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.active = false;
}


#line 293 "../../common/cost.h"

#line 293 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_6/*<trigger_t >*/::activate(CostEvent*e)
{
  assert(e==&m_event);
  m_event.active=false;
  (p_compcxx_parent->SendTimer(m_event.data));
}




#line 170 "../../mac/bcast_mac.h"

#line 171 "../../mac/bcast_mac.h"

#line 172 "../../mac/bcast_mac.h"

#line 173 "../../mac/bcast_mac.h"

#line 210 "../../mac/bcast_mac.h"
compcxx_BcastMAC_12
#line 209 "../../mac/bcast_mac.h"
/*template <class PLD>
*//*<PLD>*/::compcxx_BcastMAC_12()
{
  defer_timer.p_compcxx_parent=this /*connect defer_timer.to_component, */;
  recv_timer.p_compcxx_parent=this /*connect recv_timer.to_component, */;
  send_timer.p_compcxx_parent=this /*connect send_timer.to_component, */;
  ack_timer.p_compcxx_parent=this /*connect ack_timer.to_component, */;
}


#line 219 "../../mac/bcast_mac.h"
compcxx_BcastMAC_12
#line 218 "../../mac/bcast_mac.h"
/*template <class PLD>
*//*<PLD>*/::~compcxx_BcastMAC_12()
{
}


#line 223 "../../mac/bcast_mac.h"

#line 223 "../../mac/bcast_mac.h"
/*template <class PLD>
*/void compcxx_BcastMAC_12/*<net_packet_t* >*/::Start()
{
  
  m_state=MAC_IDLE;
  m_tx_end=0.0;
  m_seq_no=0;
    
  SentPackets=0l;
  RecvPackets=0l;
  rrCollisions = 0L;
}


#line 236 "../../mac/bcast_mac.h"

#line 236 "../../mac/bcast_mac.h"
/*template <class PLD>
*/void compcxx_BcastMAC_12/*<net_packet_t* >*/::Stop()
{
}


#line 241 "../../mac/bcast_mac.h"

#line 241 "../../mac/bcast_mac.h"
/*template <class PLD>
*/void compcxx_BcastMAC_12/*<net_packet_t* >*/::from_network(
  net_packet_t* const	&pld,
  unsigned int	size,
  double	backoff_delay)
{
  Printf( (DumpPackets, "mac%d backoff delay: %f\n", (int) MyEtherAddr,
	   backoff_delay));

  
  if(m_state!=MAC_IDLE||m_tx_end>SimTime())
  {
    packet_t::free_pld(pld);
    
    
    
    
    assert(ack_timer.Active()==false);
    ack_timer.Set( false, SimTime());
    return;
  }
    
  m_pldinf.pld=pld;
  m_pldinf.size=size;
  m_pldinf.delay=backoff_delay;
  m_seq_no++;

  StartDefer();
}


#line 271 "../../mac/bcast_mac.h"

#line 271 "../../mac/bcast_mac.h"
/*template <class PLD>
*/void compcxx_BcastMAC_12/*<net_packet_t* >*/::DeferTimer(
  trigger_t	&)
{
  

  if(m_state!=MAC_DEFER)
  {
    Printf((DumpPackets,"mac%d not in defer state \n",(int)MyEtherAddr));
  }
  assert(m_state==MAC_DEFER);
  Printf((DumpPackets,"mac%d deferal timer fires \n",(int)MyEtherAddr));
  packet_t * np = packet_t::alloc();              

  
  np->hdr.dh_fc.fc_protocol_version = MAC_ProtocolVersion;
  np->hdr.dh_fc.fc_type             = MAC_Type_Data;
  np->hdr.dh_fc.fc_subtype          = MAC_Subtype_Data;
  np->hdr.dh_da                     = ether_addr_t::BROADCAST;
  np->hdr.dh_sa                     = MyEtherAddr;
  np->hdr.dh_scontrol               = m_seq_no;

  np->hdr.size = m_pldinf.size + HDR_LEN;
  np->hdr.tx_time = np->hdr.size / DataRate;
  np->pld = m_pldinf.pld;

  
  np->hdr.tx_time = np->hdr.size /BasicRate;
  np->hdr.dh_duration = 0;
  
  m_tx_end=SimTime() + np->hdr.tx_time;
  m_sr_collision=false;
  send_timer.Set(m_tx_end);
  m_state=MAC_SEND;

  Printf((DumpPackets,"mac%d sends %s until %f\n",(int)MyEtherAddr,np->dump().c_str(),m_tx_end));
  (p_compcxx_DuplexTransceiver_13->from_mac(np));
  SentPackets++;
}


#line 311 "../../mac/bcast_mac.h"

#line 311 "../../mac/bcast_mac.h"
/*template <class PLD>
*/void compcxx_BcastMAC_12/*<net_packet_t* >*/::SendTimer(trigger_t&)
{
  m_state=MAC_IDLE;
  (p_compcxx_SSAB_11->from_mac_ack(!m_sr_collision));
}


#line 318 "../../mac/bcast_mac.h"

#line 318 "../../mac/bcast_mac.h"
/*template <class PLD>
*/void compcxx_BcastMAC_12/*<net_packet_t* >*/::AckTimer(
  bool		&flag)
{
  m_state=MAC_IDLE;
  (p_compcxx_SSAB_11->from_mac_ack(flag));
  return;
}



#line 328 "../../mac/bcast_mac.h"

#line 328 "../../mac/bcast_mac.h"
/*template <class PLD>
*/void compcxx_BcastMAC_12/*<net_packet_t* >*/::cancel()
{
  if(m_state==MAC_DEFER)
  {
    defer_timer.Cancel();
    m_state=MAC_IDLE;
    packet_t::free_pld(m_pldinf.pld);
    assert(ack_timer.Active()==false);
    ack_timer.Set(true,SimTime());
  }
}


#line 341 "../../mac/bcast_mac.h"

#line 341 "../../mac/bcast_mac.h"
/*template <class PLD>
*/void compcxx_BcastMAC_12/*<net_packet_t* >*/::from_phy(
  packet_t	*pkt,
  bool		errflag,
  double	power)
{
  RecvPackets++;
  m_sr_collision=true;

  
  

  
  if( recv_timer.Active() )
  {
    packet_t* rx_packet = m_rxinf.packet;
	
    
    if( power > m_rxinf.power * CPThreshold)
    {
      DropPacket(m_rxinf.packet,"power too weak");
      m_rxinf.packet=pkt;
      m_rxinf.errflag=errflag;
      m_rxinf.power=power;
      recv_timer.Set(SimTime()+pkt->hdr.tx_time);
    }
    
    else if (m_rxinf.power > power * CPThreshold)
    {
      DropPacket(pkt, "power too weak");
    }
    
    else
    {
      packet_t	*p1 = pkt, *p2 = m_rxinf.packet;
      simtime_t end_time = SimTime() + pkt->hdr.tx_time;
      
      if( end_time > recv_timer.GetTime() )
      {
	recv_timer.Set(end_time);
	DropPacket(rx_packet, "receive-receive collision");
	m_rxinf.packet=pkt;
	m_rxinf.errflag=errflag;
	m_rxinf.power=power;
      }
      else
      {
	DropPacket(pkt,"receive-receive collision");
      }
      rrCollisions++;
      m_rxinf.errflag = true;		
      (p_compcxx_SSAB_11->from_mac_recv_recv_coll( p1->pld, p2->pld));	

    }
  }
  else
  {
    m_rxinf.packet=pkt;
    m_rxinf.errflag=errflag;
    m_rxinf.power=power;
    
    recv_timer.Set(pkt->hdr.tx_time + SimTime());
    if(m_state==MAC_DEFER)
    {
      
      
      assert(defer_timer.Active());
      
      defer_timer.Cancel();
      m_pldinf.delay -= SimTime() - m_defer_start;
    }
  }
}


#line 415 "../../mac/bcast_mac.h"

#line 415 "../../mac/bcast_mac.h"
/*template <class PLD>
*/void compcxx_BcastMAC_12/*<net_packet_t* >*/::RecvTimer(
  trigger_t	&)
{
  
  
  
  
  
  

  packet_t* p=m_rxinf.packet;

  Printf((DumpPackets,"mac%d receives %s (%e,%d)\n",(int)MyEtherAddr,p->dump().c_str(),m_rxinf.power,m_rxinf.errflag));

  if(m_rxinf.errflag==1)  
  {  
    DropPacket(p, "frame error");
    ResumeDefer();
    return ;
  }
  simtime_t now = SimTime();
  
  if( now - p->hdr.tx_time < m_tx_end )
  {
    
    DropPacket(p,"receive-send collision");
    ResumeDefer();
    return;
  }

  u_int8_t type = p->hdr.dh_fc.fc_type;
  u_int8_t subtype = p->hdr.dh_fc.fc_subtype;  

  switch(type)
  {
    case MAC_Type_Management:
      DropPacket(p, "unknown MAC packet");
      break;
    case MAC_Type_Control:
      DropPacket(p, "not a DATA packet");
      break;
    case MAC_Type_Data:
      switch(subtype)
      {
        case MAC_Subtype_Data:
	  ResumeDefer();
	  RecvData(p);
	  return;
        default:
	  fprintf(stderr, "mac%d recv_timer(2):Invalid MAC Data Subtype %x\n",(int)MyEtherAddr,
		  subtype);
	  break;
      }
      break;
    default:
      fprintf(stderr, "mac%d recv_timer(3):Invalid MAC Type %x\n", (int)MyEtherAddr,subtype);
      break;
  }
  ResumeDefer();
}


#line 477 "../../mac/bcast_mac.h"

#line 477 "../../mac/bcast_mac.h"
/*template <class PLD>
*/void compcxx_BcastMAC_12/*<net_packet_t* >*/::RecvData(
  packet_t	*p)
{
  
  
  cache_t::iterator src=m_recv_cache.find(p->hdr.dh_sa);
  if(src!=m_recv_cache.end())
  {
    
    
    if(src->second==p->hdr.dh_scontrol)
    {
      
      
      
      DropPacket(p,"duplicated DATA packet");
      return;
    }
    else
    {
      
      src->second=p->hdr.dh_scontrol;
    }
  }
  else
  {
    
    m_recv_cache.insert(make_pair(p->hdr.dh_sa,p->hdr.dh_scontrol));
  }

  
  
  
  p->inc_pld_ref();
  
  (p_compcxx_SSAB_11->from_mac_data(p->pld,m_rxinf.power));
  p->free(); 
}


#line 517 "../../mac/bcast_mac.h"

#line 517 "../../mac/bcast_mac.h"
/*template <class PLD>
*/void compcxx_BcastMAC_12/*<net_packet_t* >*/::StartDefer()
{
  
  assert(m_state==MAC_IDLE);
  assert(defer_timer.Active()==false);

  Printf((DumpPackets, "mac%d deferring\n",(int)MyEtherAddr));
  
  
  
  if(!recv_timer.Active())                    
  {
    m_defer_start = SimTime();
    defer_timer.Set( m_defer_start+m_pldinf.delay);
  }
  m_state=MAC_DEFER;
}


#line 536 "../../mac/bcast_mac.h"

#line 536 "../../mac/bcast_mac.h"
/*template <class PLD>
*/void compcxx_BcastMAC_12/*<net_packet_t* >*/::ResumeDefer()
{
  if(m_state!=MAC_DEFER)return;

  assert(defer_timer.Active()==false);
  m_defer_start=SimTime();
  defer_timer.Set(m_defer_start+m_pldinf.delay+IFS);
  Printf((DumpPackets, "mac%d resumes deferring: %f \n",(int)MyEtherAddr,m_pldinf.delay));
}


#line 547 "../../mac/bcast_mac.h"

#line 547 "../../mac/bcast_mac.h"
/*template <class PLD>
*/void compcxx_BcastMAC_12/*<net_packet_t* >*/::DropPacket(
  packet_t	*p,
  const char	*reason)
{
  Printf((DumpPackets,"mac%d drops %s (%s)\n",(int)MyEtherAddr,p->dump().c_str(),reason));
  p->free();
}


#line 262 "../../common/cost.h"

#line 262 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_3/*<trigger_t >*/::Set(trigger_t const & data, double time)
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
*/void compcxx_Timer_3/*<trigger_t >*/::Set(double time)
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
*/void compcxx_Timer_3/*<trigger_t >*/::Cancel()
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.active = false;
}


#line 293 "../../common/cost.h"

#line 293 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_3/*<trigger_t >*/::activate(CostEvent*e)
{
  assert(e==&m_event);
  m_event.active=false;
  (p_compcxx_parent->announce_pos(m_event.data));
}




#line 31 "../../mob/immobile.h"
void		compcxx_Immobile_16::pos_out( coordinate_t& pos, int id){return (p_compcxx_parent->to_channel_pos( pos,id));}
#line 32 "../../mob/immobile.h"

#line 52 "../../mob/immobile.h"
Visualizer	*compcxx_Immobile_16::visualizer = NULL;


#line 54 "../../mob/immobile.h"
void compcxx_Immobile_16::setVisualizer(
  Visualizer	*ptr)
{
  visualizer = ptr;
  return;
}


#line 61 "../../mob/immobile.h"
void compcxx_Immobile_16::announce_pos(trigger_t&)
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
void compcxx_Immobile_16::Setup()
{
  return;
}


#line 78 "../../mob/immobile.h"
void compcxx_Immobile_16::Start()
{
  timer.Set(0.0);
  return;
}


#line 84 "../../mob/immobile.h"
void compcxx_Immobile_16::Stop()
{
  return;
}


#line 137 "../../net/ssab.h"
compcxx_SSAB_11
#line 136 "../../net/ssab.h"
/*template <class PLD>
*//*<PLD>*/::compcxx_SSAB_11()
{

}


#line 143 "../../net/ssab.h"
compcxx_SSAB_11
#line 142 "../../net/ssab.h"
/*template <class PLD>
*//*<PLD>*/::~compcxx_SSAB_11()
{
}


#line 147 "../../net/ssab.h"

#line 147 "../../net/ssab.h"
/*template <class PLD>
*/void compcxx_SSAB_11/*<app_packet_t >*/::Start() 
{
    m_mac_busy=false;
    m_seq_number=1u;
    m_seq_cache.insert(make_pair(MyEtherAddr,seq_number_t()));

    SentPackets=RecvPackets=RecvUniPackets=0l;
    TotalDelay=0.0;
    TotalHop=0;
    TotalSamples=0;
}

#line 159 "../../net/ssab.h"

#line 159 "../../net/ssab.h"
/*template <class PLD>
*/void compcxx_SSAB_11/*<app_packet_t >*/::Stop() 
{
    
    
}


#line 166 "../../net/ssab.h"

#line 166 "../../net/ssab.h"
/*template <class PLD>
*/void compcxx_SSAB_11/*<app_packet_t >*/::from_transport( payload_t& pld, ether_addr_t& dst, unsigned int size)
{
    packet_t* p=packet_t::alloc();
    p->pld=pld;
    m_seq_number++;
    m_seq_cache[MyEtherAddr].check(m_seq_number);
    p->hdr.seq_number=m_seq_number;
    p->hdr.size=size+2*sizeof(unsigned int);
    p->hdr.hop=0;
    SentPackets++;

    Printf((DumpPackets,"ssab%d creates %s\n",(int)MyEtherAddr,p->dump().c_str()));

    p->hdr.send_time=SimTime();
    if(!m_mac_busy)
    {
	m_mac_busy=true;
	m_active_packet=p;
	(p_compcxx_BcastMAC_12->from_network(p,size,0.0));
    }
    else
    {
	packet_buffer_t::iterator iter=m_packet_buffer.begin();
	while(iter!=m_packet_buffer.end()&&(*iter).second<=0.0)iter++;
	m_packet_buffer.insert(iter,make_pair(p,0.0));
    }

}






#line 200 "../../net/ssab.h"

#line 200 "../../net/ssab.h"
/*template <class PLD>
*/void compcxx_SSAB_11/*<app_packet_t >*/::from_mac_data (packet_t* pkt, double power)
{
    ether_addr_t src;
    src=pkt->access_pld().src_addr;
    RecvPackets++;

    Printf((DumpPackets,"ssab%d receives %s \n",(int)MyEtherAddr,pkt->dump().c_str()));
    cache_t::iterator iter=m_seq_cache.find(src); 
    if(iter==m_seq_cache.end())
    {
	
	
	iter=(m_seq_cache.insert(make_pair(src,seq_number_t()))).first;
    }
    if( !iter->second.check(pkt->hdr.seq_number) )
    {
	
	RecvUniPackets++;
	if(pkt->access_pld().dst_addr==MyEtherAddr||pkt->access_pld().dst_addr==ether_addr_t::BROADCAST)
	{
	    
	    pkt->inc_pld_ref();
	    (p_compcxx_CBR_10->from_transport(pkt->pld));
	    TotalHop+=pkt->hdr.hop;
	}
	else  
	{
	    double delay;
#ifdef USING_SSAB
	    delay=Random();
	    delay=ForwardDelay*(1-RXThresh/power);
#else
	    delay=Random(ForwardDelay);
#endif
	    TotalDelay+=delay;
	    TotalSamples++;
	    

	    packet_t* np = pkt->copy();
	    np->hdr.hop++;
	    pkt->free();


	    if(m_mac_busy==false)
	    {
		Printf((DumpPackets,"ssab%d forwards %s\n",(int)MyEtherAddr,np->dump().c_str()));

		SentPackets++;
		np->hdr.send_time=SimTime();
		m_mac_busy=true;
		m_active_packet=np;
		(p_compcxx_BcastMAC_12->from_network(np,np->hdr.size,delay));
	    }
	    else
	    {
                packet_buffer_t::iterator iter=m_packet_buffer.begin();
                while(iter!=m_packet_buffer.end()&&(*iter).second<=delay)iter++;
                m_packet_buffer.insert(iter,make_pair(np,delay));
	    }
	    return;
	}
    }
    else
    {
        if(m_mac_busy==true&&m_active_packet->access_pld().src_addr==pkt->access_pld().src_addr&&
	   m_active_packet->hdr.seq_number==pkt->hdr.seq_number)
        {
	    Printf((DumpPackets,"ssab%d cancels the current packet\n",(int)MyEtherAddr));
            (p_compcxx_BcastMAC_12->cancel());
        }
        else
        {
            packet_buffer_t::iterator iter;
            for(iter=m_packet_buffer.begin();iter!=m_packet_buffer.end();iter++)
            {
                if((*iter).first->access_pld().src_addr==pkt->access_pld().src_addr&&
                    (*iter).first->hdr.seq_number==pkt->hdr.seq_number)
                {
		    Printf((DumpPackets,"ssab%d cancels a buffered packet\n",(int)MyEtherAddr));fflush(stdout);
                    (*iter).first->free();
                    m_packet_buffer.erase(iter);
                    break;
                }
            }
        }
    }
    pkt->free();
}






#line 294 "../../net/ssab.h"

#line 294 "../../net/ssab.h"
/*template <class PLD>
*/void compcxx_SSAB_11/*<app_packet_t >*/::from_mac_recv_recv_coll(
  packet_t	*pkt1,
  packet_t	*pkt2)
{
  return;
}


#line 302 "../../net/ssab.h"

#line 302 "../../net/ssab.h"
/*template <class PLD>
*/void compcxx_SSAB_11/*<app_packet_t >*/::from_mac_ack(bool)
{
    Printf((DumpPackets,"ssab%d recevies an ack\n",(int)MyEtherAddr));
    if(m_packet_buffer.size())
    {
        packet_t* packet=m_packet_buffer.front().first;
        double delay=m_packet_buffer.front().second;
        m_packet_buffer.pop_front();
    	Printf((DumpPackets,"ssab%d forwards %s from buffer\n",(int)MyEtherAddr,packet->dump().c_str()));
    	m_active_packet=packet;
        (p_compcxx_BcastMAC_12->from_network(packet,packet->hdr.size,delay));
    }
    else
        m_mac_busy=false;
    return;
}






#line 324 "../../net/ssab.h"

#line 324 "../../net/ssab.h"
/*template <class PLD>
*/void compcxx_SSAB_11/*<app_packet_t >*/::from_pm_node_up()
{
  return;
}


#line 75 "../../phy/transceiver.h"

#line 75 "../../phy/transceiver.h"
/*template <class PACKET>
*/void compcxx_DuplexTransceiver_13/*<mac_packet_t  >*/::Start()
{
}


#line 80 "../../phy/transceiver.h"

#line 80 "../../phy/transceiver.h"
/*template <class PACKET>
*/void compcxx_DuplexTransceiver_13/*<mac_packet_t  >*/::Stop()
{
}


#line 85 "../../phy/transceiver.h"

#line 85 "../../phy/transceiver.h"
/*template <class PACKET>
*/void compcxx_DuplexTransceiver_13/*<mac_packet_t  >*/::from_mac(mac_packet_t  * p)
{
	double now = SimTime();
	if((p_compcxx_PowerManager_15->switch_state(PM_Struct::TX,now))<=0.0)  
    {
    	p->free();
		return;
    }	 
   
    if((p_compcxx_PowerManager_15->switch_state(PM_Struct::IDLE,now+p->hdr.tx_time))<=0.0)
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
*/void compcxx_DuplexTransceiver_13/*<mac_packet_t  >*/::from_channel(mac_packet_t  * p, double power)
{
	double now = SimTime();
    if((p_compcxx_PowerManager_15->switch_state(PM_Struct::RX,now))<= 0.0)  
    {
       p->free();
       return;
    }
          
    if((p_compcxx_PowerManager_15->switch_state(PM_Struct::IDLE, now+p->hdr.tx_time))<=0.0)
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
	  	(p_compcxx_BcastMAC_12->from_phy( p, error, recv_power ));
    }
}


#line 262 "../../common/cost.h"

#line 262 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_9/*<trigger_t >*/::Set(trigger_t const & data, double time)
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
*/void compcxx_Timer_9/*<trigger_t >*/::Set(double time)
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
*/void compcxx_Timer_9/*<trigger_t >*/::Cancel()
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.active = false;
}


#line 293 "../../common/cost.h"

#line 293 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_9/*<trigger_t >*/::activate(CostEvent*e)
{
  assert(e==&m_event);
  m_event.active=false;
  (p_compcxx_parent->power_switch(m_event.data));
}




#line 48 "../../energy/power.h"
compcxx_PowerManager_15::compcxx_PowerManager_15()
{
  upTime = 1.0;
  downTime = 0.0;
  initTime = 0.0;
  transient = false;
  switch_timer.p_compcxx_parent=this /*connect switch_timer.to_component, */;
  return;
}





#line 61 "../../energy/power.h"
void compcxx_PowerManager_15::failureStats(
  double	initializeTime)	
{
  transient = false;
  initTime = initializeTime;
  return;
}





#line 72 "../../energy/power.h"
void compcxx_PowerManager_15::failureStats(
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
void compcxx_PowerManager_15::Start()
{
  m_state = IDLE;
  m_switch_on = true;
  if( initTime != 0)
    switch_timer.Set( BeginTime + initTime);
  return;
}


#line 101 "../../energy/power.h"
void compcxx_PowerManager_15::Stop()
{
  return;
}






#line 110 "../../energy/power.h"
void compcxx_PowerManager_15::power_switch(
  trigger_t	&)
{
  
  if( m_switch_on == true)
  {
    m_state = OFF;
    (p_compcxx_SimpleBattery_14->power_in( 0.0, SimTime()));
    m_switch_on = false;
    if( transient == true)
      switch_timer.Set( SimTime() + Exponential( downTime));
  }
  else		
  {
    m_state = IDLE;
    (p_compcxx_SimpleBattery_14->power_in( IdlePower, SimTime()));
    m_switch_on = true;
    (p_compcxx_SSAB_11->from_pm_node_up());		
    switch_timer.Set( SimTime() + Exponential( upTime));
  }
  return;
}


#line 133 "../../energy/power.h"
double compcxx_PowerManager_15::switch_state(
  int		state,
  double	time) 
{
  double	power = 0.0;	
  if( time < BeginTime || time > FinishTime)
    return power;

  switch( m_state)
  {
    case TX:
      power = (p_compcxx_SimpleBattery_14->power_in( TXPower, time));
      break;
    case RX:
      power = (p_compcxx_SimpleBattery_14->power_in( RXPower, time));
      break;
    case IDLE:
      power = (p_compcxx_SimpleBattery_14->power_in( IdlePower, time));
      break;
    case SLEEP:
      power = (p_compcxx_SimpleBattery_14->power_in( SleepPower, time));
      break;
    case OFF:
      power = (p_compcxx_SimpleBattery_14->power_in( 0.0, time));
      return 0.0;
    case ON:
      power = (p_compcxx_SimpleBattery_14->power_in( IdlePower, time));
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
int compcxx_PowerManager_15::state_query()
{
  return m_state; 
}


#line 175 "../../energy/power.h"
double compcxx_PowerManager_15::energy_query()
{
  return (p_compcxx_SimpleBattery_14->query_in());
}


#line 153 "sim_ssab.cc"
void compcxx_SensorNode_17::to_channel_packet(mac_packet_t* packet, double power, int id){return (to_channel_packet_f(packet,power,id));}
#line 154 "sim_ssab.cc"
void compcxx_SensorNode_17::from_channel(mac_packet_t* packet, double power){return (phy.from_channel(packet,power));}
#line 155 "sim_ssab.cc"
void compcxx_SensorNode_17::to_channel_pos(coordinate_t& pos, int id){return (to_channel_pos_f(pos,id));}
#line 158 "sim_ssab.cc"
compcxx_SensorNode_17::~compcxx_SensorNode_17()
{
}


#line 162 "sim_ssab.cc"
void compcxx_SensorNode_17::Start()
{
}


#line 166 "sim_ssab.cc"
void compcxx_SensorNode_17::Stop()
{
}





#line 173 "sim_ssab.cc"
void compcxx_SensorNode_17::Setup()
{




    battery.InitialEnergy=1e6;
    









    
    app.MyEtherAddr=MyEtherAddr;
    app.FinishTime=StopTime()*0.9;
    app.DumpPackets=false;
    






    mob.setX( Random( MaxX));
    mob.setY( Random( MaxY));
    mob.setID( ID);










    net.MyEtherAddr=MyEtherAddr;
    net.ForwardDelay=0.1;
    net.DumpPackets=true;
    





 
    mac.MyEtherAddr=MyEtherAddr;
    mac.Promiscuity=true;
    mac.DumpPackets=false;
   






   	pm.TXPower=1.6;
   	pm.RXPower=1.2;
   	pm.IdlePower=1.15;











    phy.setTXPower( 0.0280);
    phy.setTXGain( 1.0);
    phy.setRXGain( 1.0); 
    phy.setFrequency( 9.14e8);
    phy.setRXThresh( 3.652e-10);
    phy.setCSThresh( 1.559e-11);
    phy.setID( ID);

    net.RXThresh = phy.getRXThresh();






    app.p_compcxx_SSAB_11=&net /*connect app.to_transport, net.from_transport*/;
    net.p_compcxx_CBR_10=&app /*connect net.to_transport, app.from_transport*/;
    
    mac.p_compcxx_SSAB_11=&net /*connect mac.to_network_data, net.from_mac_data */;
    mac.p_compcxx_SSAB_11=&net /*connect mac.to_network_ack, net.from_mac_ack*/;
    net.p_compcxx_BcastMAC_12=&mac /*connect net.to_mac, mac.from_network*/;
    net.p_compcxx_BcastMAC_12=&mac /*connect net.cancel, mac.cancel*/;
    mac.p_compcxx_SSAB_11=&net /*connect mac.to_network_recv_recv_coll, net.from_mac_recv_recv_coll*/;
    
    mac.p_compcxx_DuplexTransceiver_13=&phy /*connect mac.to_phy, phy.from_mac*/;
    phy.p_compcxx_BcastMAC_12=&mac /*connect phy.to_mac, mac.from_phy*/;

    phy.p_compcxx_PowerManager_15=&pm /*connect phy.to_power_switch, pm.switch_state*/;
    pm.p_compcxx_SimpleBattery_14=&battery /*connect pm.to_battery_query, battery.query_in*/;
    pm.p_compcxx_SimpleBattery_14=&battery /*connect pm.to_battery_power, battery.power_in*/;















    phy.p_compcxx_parent=this /*connect phy.to_channel, */;
    mob.p_compcxx_parent=this /*connect mob.pos_out, */;
     /*connect , phy.from_channel*/;





    pm.p_compcxx_SSAB_11=&net /*connect pm.from_pm_node_up, net.from_pm_node_up*/;

    return;
}











#line 344 "sim_ssab.cc"
void compcxx_RoutingSim_19 :: Start()
{

}






#line 353 "sim_ssab.cc"
void compcxx_RoutingSim_19 :: Stop()
{
    int i,sent,recv,hop,samples;
    double app_recv;
    double delay;
    for(sent=recv=i=0,delay=0.0;i<NumNodes;i++)
    {
		sent+=nodes[i].app.SentPackets;
		recv+=nodes[i].app.RecvPackets;
		delay+=nodes[i].app.TotalDelay;
    }
    printf("APP -- packets sent: %d, received: %d, success rate: %.3f, end-to-end delay: %.3f\n",
		sent,recv,(double)recv/sent,delay/recv);
    app_recv=recv;
    delay=0.0;
    samples=0;
    for(hop=sent=recv=i=0;i<NumNodes;i++)
    {
		sent+=nodes[i].net.SentPackets;
		recv+=nodes[i].net.RecvPackets;
		hop+=nodes[i].net.TotalHop;
		delay+=nodes[i].net.TotalDelay;
		samples+=nodes[i].net.TotalSamples;
    }
    printf("NET -- packets sent: %d, received: %d, average hop: %.3f, backoff delay: %.3f\n",sent,recv,hop/app_recv, delay/samples);
    for(sent=recv=i=0;i<NumNodes;i++)
    {
		sent+=nodes[i].mac.SentPackets;
		recv+=nodes[i].mac.RecvPackets;
    }
    printf("MAC -- packets sent: %d, received: %d\n",sent,recv);
}














#line 398 "sim_ssab.cc"
void compcxx_RoutingSim_19 :: Setup()
{
    int i,j;
    




    
    nodes.SetSize(NumNodes);
    for(i=0;i<NumNodes;i++)
    {
		nodes[i].MaxX=MaxX;
		nodes[i].MaxY=MaxY;
		nodes[i].MyEtherAddr=i;
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
		nodes[i].to_channel_packet_f.Connect(channel,(compcxx_component::SensorNode_to_channel_packet_f_t)&compcxx_SimpleChannel_18::from_phy) /*connect nodes[i].to_channel_packet,channel.from_phy*/;
		nodes[i].to_channel_pos_f.Connect(channel,(compcxx_component::SensorNode_to_channel_pos_f_t)&compcxx_SimpleChannel_18::pos_in) /*connect nodes[i].to_channel_pos,channel.pos_in*/;
		channel.to_phy[i].Connect(nodes[i],(compcxx_component::SimpleChannel_to_phy_f_t)&compcxx_SensorNode_17::from_channel) /*connect channel.to_phy[i],nodes[i].from_channel */;
    }




    int src,dst;
    for(i=0;i<NumSourceNodes;i++)
    {
		for(j=0;j<NumConnections;j++)
		{
	    	do
	    	{
				src=Random(NumNodes);
				dst=Random(NumNodes);
	    	}while(src==dst); 
	    	nodes[src].app.Connections.push_back(
				make_triple(ether_addr_t(dst),Random(PacketSize)+PacketSize/2,
				Random(Interval)+Interval/2));
		}
    }




    for(i=0;i<NumNodes;i++)
    {
      nodes[i].pm.BeginTime=0.0;
      nodes[i].pm.FinishTime=StopTime();
      nodes[i].pm.failureStats( 0.0);
    }
    return;
}






















#line 511 "sim_ssab.cc"
int main(int argc, char* argv[])
{
    compcxx_RoutingSim_19 sim;

    sim.StopTime( 1000);
    sim.Seed = 234;

    sim.MaxX = 2000;
    sim.MaxY = 2000;

    sim.NumNodes = 110;
    sim.NumConnections = 2;
    sim.PacketSize = 2000;
    sim.Interval = 100.0;
   
    if(argc >= 2) sim.StopTime( atof(argv[1]));
    if(argc >= 3) sim.NumNodes = atoi(argv[2]);
    sim.NumSourceNodes = sim.NumNodes / 10;
    if(argc >= 4) sim.MaxX = sim.MaxY = atof(argv[3]);
    if(argc >= 5) sim.NumSourceNodes = atoi(argv[4]);
    if(argc >= 6) sim.PacketSize = atoi(argv[5]);
    if(argc >= 7) sim.Interval = atof(argv[6]); 

#ifdef USING_SSAB
    printf("Using SSAB\n");
#else
    printf("Not using SSAB\n");
#endif   

    printf("StopTime: %.0f, Number of Nodes: %d, Terrain: %.0f by %.0f\n",
	   sim.StopTime(), sim.NumNodes, sim.MaxX, sim.MaxY);
    printf("Number of Sources: %d, Packet Size: %d, Interval: %f\n",
	   sim.NumSourceNodes, sim.PacketSize, sim.Interval);

    sim.Setup();
    sim.Run();

    return 0;
}
