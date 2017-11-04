
#include "stir/listmode/SubtractSinoScatterNegA0.h"

#ifndef STIR_NO_NAMESPACES
using std::cerr;
#endif

USING_NAMESPACE_STIR



int main(int argc, char * argv[])
{
  
  if ( argc!=2 ) {
    cerr << "Usage: " << argv[0] << " [par_file]\n";
    exit(EXIT_FAILURE);
  }
  SubtractSinoScatterNegA0 application(argc==2 ? argv[1] : 0);
  application.process_data();

  return EXIT_SUCCESS;
}

