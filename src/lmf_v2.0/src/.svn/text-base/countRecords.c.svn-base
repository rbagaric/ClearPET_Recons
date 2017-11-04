/*-------------------------------------------------------

           List Mode Format 
                        
     --  countRecords.c  --                      

     Luc.Simon@iphe.unil.ch

     Crystal Clear Collaboration
     Copyright (C) 2003 IPHE/UNIL, CH-1015 Lausanne

     This software is distributed under the terms 
     of the GNU Lesser General 
     Public Licence (LGPL)
     See LMF/LICENSE.txt for further details

-------------------------------------------------------*/

/*-------------------------------------------------------

     Description of countRecords.c:
     Called for each record read by LMFreader in mode
     countRecords
     This function returns at destruction, on sceen, and 
     eventually in a ASCII output file : 

     Number of records 
     Number of event records 
        Number of Singles 
	Number of Coincidences
     Number of count rate records
     Number of singles non chronologically sorted
 

-------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include "lmf.h"
  static int  headalreadyDone = FALSE;
static u64 recordNumber = 0,countRateNumber = 0, eventNumber = 0;
static u64 numberOfSingles = 0, numberOfCoincidences = 0,previousEventTime=0;
static u64 numberOfNonSortedSingles = 0;
void countRecords(const ENCODING_HEADER *pEncoH,
		  const EVENT_HEADER *pEH,
		  const COUNT_RATE_HEADER *pCRH,
		  const GATE_DIGI_HEADER *pGDH,
		  const CURRENT_CONTENT *pcC,   
		  const EVENT_RECORD *pER,
		  const COUNT_RATE_RECORD *pCRR)
{  


if(headalreadyDone == FALSE)
    {
      printf("\n");
      printf("*****************************\n");
      printf("* COUNT THE LMF BINARY FILE *\n");
      printf("*****************************\n");
      printf("\n");
      printf("******     HEAD      *******\n");
      printf("\n");
        
      //      dumpHead(pEncoH,pEH,pGDH,pCRH); 
      headalreadyDone = TRUE;
      printf("\n");
      printf("******    BODY      ********\n");
      printf("\n");
    }

  recordNumber++;


 if(pcC->typeOfCarrier == pEncoH->scanContent.eventRecordTag)
   {
   
     eventNumber++;
     if(pEH->coincidenceBool == 1)
       numberOfCoincidences ++;
     else
       {
	 numberOfSingles++;

	 
	 if(previousEventTime == 0)
	   previousEventTime=getTimeOfThisEVENT(pER);
	 else
	   {
	     if (previousEventTime > getTimeOfThisEVENT(pER))
	       {
		 numberOfNonSortedSingles++;	
	       }
	     else
	       {
		 previousEventTime=getTimeOfThisEVENT(pER); 
	       }
	   }
	 
       }
}
 if(pcC->typeOfCarrier == pEncoH->scanContent.countRateRecordTag)
   {
     
     countRateNumber++;
     

   } 


  
}


void destroyCounter()
{
 headalreadyDone = FALSE;
 printf("\n\n\n\n");
 printf("***************************************************\n");
 printf("All the file have been read :\n");
 printf("Number of records =\t%llu\n",recordNumber);
 recordNumber = 0;
 printf("\nNumber of event records =\t%llu\t\n", eventNumber);

 if(OF_is_Set())/*  // write event number in an ascii file */
   {
     fprintf(getAndSetOutputFileName(),"%llu\n",eventNumber);
   }

 eventNumber = 0;
 printf("\t\t\tSingles :\t%llu",numberOfSingles);
 numberOfSingles = 0;
 printf("\n\t\t\tCoincidences :\t%llu\n", numberOfCoincidences);
 numberOfCoincidences = 0;
 printf("\nNumber of count rate records :\t%llu\n",countRateNumber);
 countRateNumber = 0;
 printf("Number of singles non chronologically sorted = %llu\n",numberOfNonSortedSingles);



 previousEventTime=0;
 numberOfNonSortedSingles = 0;
 printf("***************************************************\n\n");


}

