#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "lmf.h"

u16 *pcrist; /*  to demake id */

u16 getLayerID(const ENCODING_HEADER *pEncoH, const EVENT_RECORD *pER)
{
  u16 value;
  pcrist =  demakeid(pER->crystalIDs[0],pEncoH);
  value = pcrist[0];
  free(pcrist);  
  return(value);
}

u16 getCrystalID(const ENCODING_HEADER *pEncoH, const EVENT_RECORD *pER)
{
  u16 value;
  pcrist =  demakeid(pER->crystalIDs[0],pEncoH);
  value = pcrist[1];
  free(pcrist);  
  return(value);
}
u16 getSubmoduleID(const ENCODING_HEADER *pEncoH, const EVENT_RECORD *pER)
{
  u16 value;
  pcrist =  demakeid(pER->crystalIDs[0],pEncoH);
  value = pcrist[2];
  free(pcrist);  
  return(value);
}


u16 getModuleID(const ENCODING_HEADER *pEncoH, const EVENT_RECORD *pER)
{
  u16 value;
  pcrist =  demakeid(pER->crystalIDs[0],pEncoH);
  value = pcrist[3];
  free(pcrist);  
  return(value);
}
u16 getRsectorID(const ENCODING_HEADER *pEncoH, const EVENT_RECORD *pER)
{
  u16 value;
  pcrist =  demakeid(pER->crystalIDs[0],pEncoH);
  value = pcrist[4];
  free(pcrist);  
  return(value);
}
