//
// $Id: OutputFileFormat.cxx,v 1.5 2003/05/23 16:17:46 kris Exp $
//
/*!
  \file
  \ingroup IO
  
  \brief  implementation of the OutputFileFormat class 
  \author Kris Thielemans
      
  $Date: 2003/05/23 16:17:46 $
  $Revision: 1.5 $
*/
/*
    Copyright (C) 2002- $Date: 2003/05/23 16:17:46 $, IRSL
    See STIR/LICENSE.txt for details
*/


#include "stir/IO/OutputFileFormat.h"
#include "stir/Succeeded.h"

START_NAMESPACE_STIR

ASCIIlist_type
OutputFileFormat::number_format_values;

ASCIIlist_type
OutputFileFormat::byte_order_values;

OutputFileFormat::
OutputFileFormat(const NumericType& type,
		 const ByteOrder& byte_order)
  : type_of_numbers(type), file_byte_order(byte_order)
{ 
}

void 
OutputFileFormat::
set_defaults()
{
  file_byte_order = ByteOrder::native;
  type_of_numbers = NumericType::FLOAT;

  set_key_values();
}

void 
OutputFileFormat::
initialise_keymap()
{

  if (number_format_values.size()==0)
  {
    number_format_values.push_back("bit");
    number_format_values.push_back("ascii");
    number_format_values.push_back("signed integer");
    number_format_values.push_back("unsigned integer");
    number_format_values.push_back("float");

    assert(byte_order_values.size()==0);
    byte_order_values.push_back("LITTLEENDIAN");
    byte_order_values.push_back("BIGENDIAN");
  }
  parser.add_key("byte order",
		 &byte_order_index,
		 &byte_order_values);
  parser.add_key("number format",
		 &number_format_index,
		 &number_format_values);
  parser.add_key("number of bytes per pixel",
		 &bytes_per_pixel);
}

void 
OutputFileFormat::
set_key_values()
{
  byte_order_index =
    file_byte_order == ByteOrder::little_endian ?
      0 : 1;

  string number_format; 
  size_t bytes_per_pixel_in_size_t;
  type_of_numbers.get_Interfile_info(number_format, 
                                     bytes_per_pixel_in_size_t);
  bytes_per_pixel = static_cast<int>(bytes_per_pixel_in_size_t);
  number_format_index =
     parser.find_in_ASCIIlist(number_format, number_format_values); 

}

bool
OutputFileFormat::
post_processing()
{
  if (number_format_index<0 ||
      static_cast<ASCIIlist_type::size_type>(number_format_index)>=
        number_format_values.size())
  {
    warning("OutputFileFormat parser internal error: "
            "'number_format_index' out of range\n");
    return true;
  }
  // check if bytes_per_pixel is set if the data type is not 'bit'
  if (number_format_index!=0 && bytes_per_pixel<=0)
  {
    warning("OutputFileFormat parser: "
            "'number_of_bytes_per_pixel' should be > 0\n");
    return true;
  }

  type_of_numbers = NumericType(number_format_values[number_format_index], 
                                bytes_per_pixel);

  if (type_of_numbers == NumericType::UNKNOWN_TYPE)
  {
    warning("OutputFileFormat parser: "
        "unrecognised data type. Check either "
        "'number_of_bytes_per_pixel' or 'number format'\n");
    return true;
  }

  switch(byte_order_index)
  {
    case -1:
	file_byte_order = ByteOrder::native; break;
    case 0:
	file_byte_order = ByteOrder::little_endian; break;
    case 1:
	file_byte_order = ByteOrder::big_endian; break;
    default:
        warning("OutputFileFormat parser internal error: "
                "'byte_order_index' out of range\n");
        return true;
  }

  set_byte_order(file_byte_order, true);
  set_type_of_numbers(type_of_numbers, true);

  return false;
}

NumericType 
OutputFileFormat::
get_type_of_numbers() const
{ return type_of_numbers; }

ByteOrder 
OutputFileFormat::
get_byte_order()
{ return file_byte_order; }

NumericType 
OutputFileFormat::
set_type_of_numbers(const NumericType& new_type, const bool warn)
{ return type_of_numbers =  new_type; }

ByteOrder 
OutputFileFormat::
set_byte_order(const ByteOrder& new_byte_order, const bool warn)
{ return file_byte_order = new_byte_order; }

void
OutputFileFormat::
set_byte_order_and_type_of_numbers(ByteOrder& new_byte_order, NumericType& new_type, const bool warn)
{
  new_byte_order = set_byte_order(new_byte_order, warn);
  new_type = set_type_of_numbers(new_type, warn);
}

Succeeded  
OutputFileFormat::
write_to_file(string& filename, 
	      const DiscretisedDensity<3,float>& density) const
{
  return actual_write_to_file(filename, density);
}

Succeeded  
OutputFileFormat::
write_to_file(const string& filename, 
	      const DiscretisedDensity<3,float>& density) const
{
  string filename_to_use = filename;
  return 
    write_to_file(filename_to_use, density);
};
 

END_NAMESPACE_STIR
