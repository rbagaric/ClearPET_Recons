/*-------------------------------------------------------

           List Mode Format 
                        
     --  readOneGateDigiRecord.c  --                      

     Luc.Simon@iphe.unil.ch

     Crystal Clear Collaboration
     Copyright (C) 2003 IPHE/UNIL, CH-1015 Lausanne

     This software is distributed under the terms 
     of the GNU Lesser General 
     Public Licence (LGPL)
     See LMF/LICENSE.txt for further details

-------------------------------------------------------*/

/*-------------------------------------------------------

     Description of readOneGateDigiRecord.c:
     Fills the LMF_ccs_gateDigiRecord structure with 
     a block of read bytes (block size must be check before)
     The block is pointed by *pBufGateDigi

-------------------------------------------------------*/
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include "lmf.h"


static struct LMF_ccs_gateDigiRecord *pGDR; 
static int allocGDR = 0;


u8 firstHalfOf(u8 a)
{
  a = a >> 4;
  return (a);/*  // return the value of the 4 first bits of a byte */
}
u8 secondHalfOf(u8 a)
{
  a = a & 15;
  return (a); /* // return the value of the 4 last bits of a byte */
}


struct LMF_ccs_gateDigiRecord(*readOneGateDigiRecord(u8 *pBufGateDigi,
						     u16 GDpattern,
						     struct LMF_ccs_gateDigiHeader *pGDH,
						     struct LMF_ccs_eventHeader *pEH))
{



  /***************************************************************/ 
  int i;                       /* Counter                        */
  u32 *pi32=NULL;   /* i32 pointer (4 bytes)         */
  u16 *punch=NULL;  /* i16. pointer (2 bytes)       */
  u8 *puni8=NULL; /* i8 pointer (1 byte)          */
  i16 *pi16=NULL;          /* i16 pointer                  */
  /***************************************************************/ 
 
  if (allocGDR==0)
    {                   /* just one time this block*/
      if((pGDR = (struct LMF_ccs_gateDigiRecord *)malloc(sizeof(struct LMF_ccs_gateDigiRecord)))==NULL)
	printf("\n***ERROR : in readOneGateDigiRecord.c : impossible to do : malloc()\n");
      
      allocGDR=1;
    }

  punch = (u16*)pBufGateDigi;/* initial position of the pointers*/
  puni8 = (u8*)pBufGateDigi;
  pi32 = (u32*)pBufGateDigi;

 
   
  pi32 = (u32*)puni8;
  punch = (u16*)puni8;
  

   if((GDpattern&BIT8)==BIT8) /* if r=1 */
    {
      *pi32=ntohl(*pi32); /* // byte order */
      pGDR->runID = *pi32;

      pi32++;
    }


  if((GDpattern&BIT9)==BIT9) /* if e=1 */
    {
      *pi32=ntohl(*pi32); /* // byte order */
      pGDR->eventID[0] = *pi32;
      pi32++;
      
      if(pEH->coincidenceBool)
	{
	  *pi32=ntohl(*pi32); /* // byte order */
	  pGDR->eventID[1] = *pi32;
	  pi32++;
	}
	
    }

  if((GDpattern&BIT6)==BIT6) /* if M=1 */
    {
      if(pEH->coincidenceBool)  /*if  c=1*/
	{
	  *pi32 = ntohs(*pi32);
	  pGDR->multipleID = *pi32;
	  pi32++; 
 	}
    }

  punch = (u16*)pi32;


  if((GDpattern&BIT11)==BIT11) /* if S=1 */
    {
      *punch = ntohs(*punch);
      pGDR->sourceID[0] = *punch;
      punch++; /* Avance de 1 uns i16*/
  

      if(pEH->coincidenceBool)  /*if s=1 & c=1*/
	{
	  *punch = ntohs(*punch);
	  pGDR->sourceID[1] = *punch;
	  punch++; /* Avance de 1 uns i16*/
 	}
    }

  pi16 = (i16*)punch;

  if((GDpattern&BIT10)==BIT10) /* if p=1 */
    {
      *pi16 = ntohs(*pi16);
      pGDR->sourcePos[0].X = *pi16;
      pi16++; /* Avance de 1 uns i16*/

      *pi16 = ntohs(*pi16);
      pGDR->sourcePos[0].Y = *pi16;
      pi16++; /* Avance de 1 uns i16*/

      *pi16 = ntohs(*pi16);
      pGDR->sourcePos[0].Z = *pi16;
      pi16++; /* Avance de 1 uns i16*/



      if(pEH->coincidenceBool)  /*if p=1 & c= 1*/
	{ 
	  *pi16 = ntohs(*pi16);
	  pGDR->sourcePos[1].X = *pi16;
	  pi16++; /* Avance de 1 uns i16*/
	  
	  *pi16 = ntohs(*pi16);
	  pGDR->sourcePos[1].Y = *pi16;
	  pi16++; /* Avance de 1 uns i16*/
	  
	  *pi16 = ntohs(*pi16);
	  pGDR->sourcePos[1].Z = *pi16;
	  pi16++; /* Avance de 1 uns i16*/

	}
    }
  
 if((GDpattern&BIT7)==BIT7) /* if G=1 */
   {
     *pi16 = ntohs(*pi16);
     pGDR->globalPos[0].X = *pi16;
     pi16++;
     *pi16 = ntohs(*pi16);
     pGDR->globalPos[0].Y = *pi16;
     pi16++;
     *pi16 = ntohs(*pi16);
     pGDR->globalPos[0].Z = *pi16;
     pi16++;

     if(pEH->neighbourBool)
       {
	 for(i=1;i < pEH->numberOfNeighbours + 1;i++)
	   {
	     *pi16 = ntohs(*pi16);
	     pGDR->globalPos[i].X = *pi16;
	     pi16++;
	     *pi16 = ntohs(*pi16);
	     pGDR->globalPos[i].Y = *pi16;
	     pi16++;
	     *pi16 = ntohs(*pi16);
	     pGDR->globalPos[i].Z = *pi16;
	     pi16++;
	     
	   }
       }


     if(pEH->coincidenceBool)
       {
	 *pi16 = ntohs(*pi16);
	 pGDR->globalPos[pEH->numberOfNeighbours + 1].X = *pi16;
	 pi16++;
	 *pi16 = ntohs(*pi16);
	 pGDR->globalPos[pEH->numberOfNeighbours + 1].Y = *pi16;
	 pi16++;
	 *pi16 = ntohs(*pi16);
	 pGDR->globalPos[pEH->numberOfNeighbours + 1].Z = *pi16;
	 pi16++;
       
	  if(pEH->neighbourBool)
	    {
	      for(i=pEH->numberOfNeighbours + 2;i <= (2*pEH->numberOfNeighbours)+1;i++)
		{

		  *pi16 = ntohs(*pi16);
		  pGDR->globalPos[i].X = *pi16;
		  pi16++;
		  *pi16 = ntohs(*pi16);
		  pGDR->globalPos[i].Y = *pi16;
		  pi16++;
		  *pi16 = ntohs(*pi16);
		  pGDR->globalPos[i].Z = *pi16;
		  pi16++;
 		}
	    }
       }
   }
 

 puni8=(u8*)pi16;
 if((GDpattern&BIT12)==BIT12) /*if C = 1*/
   {
      if(pEH->coincidenceBool) /*if c = 1*/
	{
	  pGDR->numberCompton[0] = firstHalfOf(*puni8);
	  pGDR->numberCompton[1] = secondHalfOf(*puni8);
	  puni8++;
	}
      else
	{
	  pGDR->numberCompton[0] = *puni8;
	  puni8++;
	}
   }
  
 if((GDpattern&BIT5)==BIT5) /*if D = 1*/
   {



      if(pEH->coincidenceBool) /*if c = 1*/
	{

	  pGDR->numberDetectorCompton[0] = firstHalfOf(*puni8);
	  pGDR->numberDetectorCompton[1] = secondHalfOf(*puni8);
	  puni8++;
	}
      else
	{

	  pGDR->numberDetectorCompton[0] = *puni8; 
	  puni8++;
	}
   }
  
   return(pGDR);
}


void destroyGDRreader(GATE_DIGI_RECORD *pGDR)
{



  free(pGDR); 
  allocGDR = 0;


}
