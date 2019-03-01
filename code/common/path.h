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
