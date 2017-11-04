#ifndef noncopy_h_included
#define noncopy_h_included

#if HAS_BOOST
#include <boost/noncopyable.hpp>
using boost::noncopyable;
#else

class noncopyable
{
protected:
  noncopyable() {}
  ~noncopyable() {}
private:
  noncopyable( const noncopyable& );
  const noncopyable& operator=( const noncopyable& );
};

#endif

#endif // noncopy_h_included
