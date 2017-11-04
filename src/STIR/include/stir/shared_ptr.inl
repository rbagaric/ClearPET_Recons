/*!
  \file
  \ingroup buildblock
  
  \brief Implementation of class shared_ptr
    
  \author Mustapha Sadki (minor modifications by Kris Thielemans)
  \author PARAPET project
      
  $Date: 2001/12/20 21:18:04 $        
  $Revision: 1.3 $
*/         
/*
    Copyright (C) 2000 PARAPET partners
    Copyright (C) 2000- $Date: 2001/12/20 21:18:04 $, IRSL
    See STIR/LICENSE.txt for details
*/

START_NAMESPACE_STIR

#ifndef STIR_USE_BOOST

template <class T>
T*
shared_ptr<T>::
get() const
{ return ptr == NULL ? NULL : ptr->data; }

template <class T>
shared_ptr<T>::
shared_ptr(T * t)
{
  if (t) {			 
    ptr = new couple(t);
  }
  else	ptr = NULL;	  
}


template <class T>
shared_ptr<T>::~shared_ptr() 
{
  if (ptr && --(ptr->count) == 0)
    delete ptr;
}

template <class T>
shared_ptr<T>::
shared_ptr(const shared_ptr<T> & cp):ptr(cp.ptr)
{ if (ptr)  ++(ptr->count);  }

#ifndef STIR_NO_AUTO_PTR
template <class T>
shared_ptr<T>::
shared_ptr(auto_ptr<T>& r) 
{  
  if (r.get())
  {
    ptr = new couple(r.get()); // may throw
    r.release();               // moved here to stop leak if new throws
  }
  else
    ptr = NULL;
  
} 
#endif

template <class T>
shared_ptr<T> &
shared_ptr<T>::operator= (const shared_ptr<T> & cp)  
{
  
  if (ptr && cp.ptr && ptr->data == cp.ptr->data)
    return *this;
  
  if (ptr && --(ptr->count) == 0)
    delete ptr;
  ptr = cp.ptr;
  if (ptr) ++(ptr->count);
  return *this;
}

template <class T>
T * 
shared_ptr<T>::
operator-> () const
{
  assert ( ptr != NULL );
  // KT removed if. it's checked with an assert, and might as well crash now as later
  //if (ptr)
  return (ptr->data);
  //else
  //return NULL; 
}

// KT added const, removed if
template <class T>
T &  
shared_ptr<T>::
operator* () const {
  assert ( ptr );
  /*if(ptr)*/
  return (*(ptr->data));
  
}

#if 0
template <class T, class newType>
shared_ptr<T>::    
shared_ptr<newType> operator()
{
  return shared_ptr<newType>(ptr->data);
}
#endif

// KT added const
template <class T>
bool  
shared_ptr<T>::
operator== (const shared_ptr<T> & cp) const {
  return (ptr == cp.ptr);
}

template <class T>
bool  
shared_ptr<T>::
operator!= (const shared_ptr<T> & cp) const {
  return !(*this == cp);
}



// KT 10/05/2000 removed as not in boost::shared_ptr
#if 0
template <class T>
void 
shared_ptr<T>::
release() 
{
  if (ptr && --(ptr->count) == 0)
    delete ptr;
  ptr = NULL;
}

template <class T>
bool shared_ptr<T>::
is_null() const
{ return ptr == NULL; } 
#endif

template <class T>
long 
shared_ptr<T>::
use_count() const
{
  return ptr==NULL ? 0 : ptr->count;
}

#endif // STIR_USE_BOOST

END_NAMESPACE_STIR
