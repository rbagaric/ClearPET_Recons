//
// $Id: ByteOrder.inl,v 1.4 2001/12/20 21:18:03 kris Exp $
//
/*!
  \file 
 
  \brief This file declares the ByteOrder class.

  \author Kris Thielemans 
  \author Alexey Zverovich
  \author PARAPET project

  $Date: 2001/12/20 21:18:03 $

  $Revision: 1.4 $

*/
/*
    Copyright (C) 2000 PARAPET partners
    Copyright (C) 2000- $Date: 2001/12/20 21:18:03 $, IRSL
    See STIR/LICENSE.txt for details
*/

START_NAMESPACE_STIR

ByteOrder::ByteOrder(Order byte_order)
  : byte_order(byte_order)
{}

/*! for efficiency, this refers to the static member native_order.*/
inline ByteOrder::Order ByteOrder::get_native_order()
{ 
  return native_order;
}

inline bool ByteOrder::is_native_order() const
{
  return 
    byte_order == native ||
    byte_order == get_native_order();
}

/*! This takes care of interpreting 'native' and 'swapped'. */
bool ByteOrder::operator==(const ByteOrder order2) const
{
  // Simple comparison (byte_order == order2.byte_order)
  // does not work because of 4 possible values of the enum.
  return 
    (is_native_order() && order2.is_native_order()) ||
    (!is_native_order() && !order2.is_native_order());
}

bool ByteOrder::operator!=(const ByteOrder order2) const
{
return !(*this == order2);
}

END_NAMESPACE_STIR
