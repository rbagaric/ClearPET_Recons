/*-------------------------------------------------------

List Mode Format 
                        
--  LMFCbuilder.c  --                      

Luc.Simon@iphe.unil.ch

Crystal Clear Collaboration
Copyright (C) 2003 IPHE/UNIL, CH-1015 Lausanne

This software is distributed under the terms 
of the GNU Lesser General 
Public Licence (LGPL)
See LMF/LICENSE.txt for further details

-------------------------------------------------------*/

/*-------------------------------------------------------

Description of LMFCbuilder.c:
This file is a copy of LMFbuilder.c but designed,
and optimized for the coincidence output module.


-------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include "lmf.h"


  
/* -=-=-=-=-=-=  FUNCTION LMF BUILDER -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

FILE *LMFCbuilder(struct LMF_ccs_encodingHeader *pEncoH,
		  struct LMF_ccs_eventHeader *pEH,      
		  struct LMF_ccs_gateDigiHeader *pGDH,      
		  struct LMF_ccs_currentContent *pcC,   
		  struct LMF_ccs_eventRecord *pER,      
		  FILE *pfile,
		  const i8 *nameOfFile)
{   
  /*- - - - - - - - - - -  -=-=-=-=-=-=-=--=-=-=-=- - - - - - - - - - - - - - - */ 
  /*                       Write the .ccs file HEAD                             */
  /*- - - - - - - - - - -  -=-=-=-=-=-=-=--=-=-=-=- - - - - - - - - - - - - - - */ 
  /* This if-block is just done at the 1st call*/
  if (pfile == NULL)
    { 
      pfile = fopen (nameOfFile, "w+b");    /*create and Opening  WRITE_FILE*/
     
     
      buildHead(pEncoH,pEH,pGDH,NULL,pfile);/* build and write head of .ccs file */
     
      fflush(pfile);
      /* the head is not to build anymore  */
      fclose(pfile);            /* close the file in read write mode and... */
      pfile = fopen (nameOfFile, "a+b");   /*...open it  in "apend  mode"   */
    }                                                                      /* */
  /*- - - - - - - - - - -  -=-=-=-=-=-=-=--=-=-=-=- - - - - - - - - - - - - - - */ 
  /*                       Write the .ccs file BODY                             */
  /*- - - - - - - - - - -  -=-=-=-=-=-=-=--=-=-=-=- - - - - - - - - - - - - - - */ 
  if(pcC->typeOfCarrier == pEncoH->scanContent.eventRecordTag)             /* */
    {
      buildER(pEH,pEncoH->scanEncodingIDLength,pER,pfile); /*build and write  an event record*/
      /* if (fflush(pfile) != 0) */                /*force the writting of the record*/
      /* 	printf("\n*** ERROR : LMFBuilder : problem with fflush\n");        
       */
      if(pEH->gateDigiBool)
	{
	  buildGDR(pGDH,pER->pGDR,pEH,pfile);               /*build and write a gate digi record*/
	  /* if (fflush(pfile) != 0)  */               /*force the writting of the record*/
	  /* 	printf("\n*** ERROR : LMFBuilder : problem with fflush\n");*/            /* */   
	} 
    }



  if(pcC->typeOfCarrier == pEncoH->scanContent.countRateRecordTag)         /* */
    { 
      printf("*** ERROR : LMFCbuilder.c : find a count rate here !\n");
    } 



 
  return(pfile);                                 /* return the File pointer */
}                                                                          /* */
/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-==-=-=-=-=-=-=-=-=-=*/


void CloseLMFCfile(FILE *pfile)
{  /* Close the file */
  if(pfile)
    fclose(pfile);  

 
}


/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-==-=-=-=-=-=-=-=-=-=*/


void FreeLMFCBuilderCarrier(struct LMF_ccs_encodingHeader *pEncoH,
			    struct LMF_ccs_eventHeader *pEH,
			    struct LMF_ccs_gateDigiHeader *pGDH,
			    struct LMF_ccs_currentContent *pcC,
			    struct LMF_ccs_eventRecord *pER)
{
  if(pEncoH->scanContent.eventRecordBool == 1)
    {
      free(pER->crystalIDs);
      free(pER->energy);
      free(pEH);
      if(pEncoH->scanContent.gateDigiRecordBool == 1)
	{
	  if(pGDH)
	    free(pGDH);
	  if(pER->pGDR)
	    free(pER->pGDR);
	}
     
      free(pER);
      
    }


  if(pcC)
    free(pcC);
  if(pEncoH)
    free(pEncoH);
  printf("coincidence LMF file built succesfully by LMFCbuilder...ok\n");
}

