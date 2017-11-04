/*-------------------------------------------------------

List Mode Format 
                        
--  LMFreader.c  --                      

Luc.Simon@iphe.unil.ch

Crystal Clear Collaboration
Copyright (C) 2003 IPHE/UNIL, CH-1015 Lausanne

This software is distributed under the terms 
of the GNU Lesser General 
Public Licence (LGPL)
See LMF/LICENSE.txt for further details

-------------------------------------------------------*/

/*-------------------------------------------------------

Description of LMFreader.c:
At the 1st call, the head of the LMF .ccs binary file
is read and allows to fill the encoding header, and 
record headers.

Then it reads a record in LMF .ccs binary file at each call,
and fill the LMF record carrier structures with the 
getted informations.
The record is given to processRecordCarrier.c with 
an reading mode. The mode (a string) specify what to do 
with the record: 
     
dump : display the event at screen.
countRecord : count the number of records.
...
see processRecordCarrier.c for further details.
     
     
     
-------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include "lmf.h"

/* definition of a LMF RECORD CARRIER and corresponding pointers  */
static struct LMF_ccs_encodingHeader *pEncoH; /* encoding header structure pointer */
static struct LMF_ccs_eventHeader *pEH;          /* event header structure pointer */
static struct LMF_ccs_gateDigiHeader *pGDH;  /* gate digi header structure pointer */
static struct LMF_ccs_countRateHeader *pCRH;/* count rate header structure pointer */
static struct LMF_ccs_currentContent *pcC;    /* current content structure pointer */
static struct LMF_ccs_eventRecord *pER;          /* event record structure pointer */
static struct LMF_ccs_countRateRecord *pCRR;/* count rate record structure pointer */

static u8 *pBufEvent,*pBufCountRate,*pBufGateDigi;       /* buffer for read*/

static int alreadyDonepcC = FALSE;
static int alreadyDoneBufE = FALSE;
static int alreadyDoneBufCR = FALSE;
static int alreadyDoneBufGD = FALSE;
static u8 FileCcsNotFound = FALSE;

/* -=-=-=-=-=-=  LMF READER -=-=-=-=-=-= */
int LMFreader(const i8 *nameOfFile, const i8 *processingMode)
{ 

  u16 Epattern=0,
    CRpattern=0,
    GDpattern=0,
    pattern=0,
    sizeE=0,
    sizeCR=0, 
    sizeGD=0,
    testread=0;
  u8 uni8=0;
  
  
  FILE *pf = NULL;
  /*------       End of declaration   -----*/


  printf("\n\n\n---- LMF READER ----\n\n\n");
  printf("LMFreader for %s in mode = %s",nameOfFile,processingMode);
 
  pf = fopen(nameOfFile,"rb");
  printf("\tOpen ok...\n");
    
  if(pf == NULL) /* test if the file exists and open it*/
    {  
      printf("\nFile %s not found by the LMFreader...\n",nameOfFile);
      exit(0);
    }  
  else
    {
      if(alreadyDonepcC == FALSE)                         /* this if block is done just once*/
	{
	  pcC = (struct LMF_ccs_currentContent *)malloc(sizeof(struct LMF_ccs_currentContent));
	  alreadyDonepcC=TRUE;
	}
      fseek(pf,0L,0);                                          /* find the begin of file */
      pEncoH = readHead(pf);                       /* fill the encoding header structure */
      fseek(pf,-(2 * pEncoH->scanContent.nRecord),1);
      
      /* -=-=-=-=-=-=-=-= READ THE PATTERNS AND FILL THE RECORD HEADERS  =-=-=-=- */
      if(pEncoH->scanContent.nRecord != 0)/* if there are records in the file...  */
	{
	  if(pEncoH->scanContent.eventRecordBool == 1)  /* is there an E pattern ?  */
	    {	 

	      fread(&pattern,sizeof(u16),1,pf);            /*read a i16  */
	      pattern = ntohs(pattern);
	      Epattern = pattern ;                     /* Buffer the event pattern*/
	      pEH = extractEpat(Epattern);           /*analyse the event pattern  */  
	      
	      sizeE = calculatesizeevent(Epattern);     /* size of 1 Event Record */
	      sizeE += 2 * (pEncoH->scanEncodingIDLength);
	      if (alreadyDoneBufE == FALSE)
		{                 /* check that this allocation is just done once */
		  pBufEvent = malloc(sizeE*sizeof(u8)); 
		  alreadyDoneBufE = TRUE; 
		}
	    }
	  if(pEncoH->scanContent.gateDigiRecordBool == 1)  /* is there an GD pattern ?  */
	    {
	     
	     
	      fread(&pattern,sizeof(u16),1,pf);            /*read a i16  */
	      pattern = ntohs(pattern);
	      GDpattern = pattern ;                     /* Buffer the gate digi pattern*/
	      pGDH = extractGDpat(GDpattern);           /*analyse the gate digi pattern  */  

	      
	      
	      sizeGD = calculatesizegatedigi(GDpattern,Epattern);     /* size of 1 Gate Digi Record */
	      if (alreadyDoneBufGD == FALSE)
		{                 /* check that this allocation is just done once */
		  pBufGateDigi = malloc(sizeGD*sizeof(u8)); 
		  alreadyDoneBufGD = TRUE; 
		 
		}
	    }
	  if(pEncoH->scanContent.countRateRecordBool == 1)   /* and a CR pattern ?  */
	    {
	      fread(&pattern,sizeof(u16),1,pf);/*        so read it*/
	      pattern = ntohs(pattern);                    /* byte order  */
	      CRpattern = pattern;
	      pCRH = extractCRpat(CRpattern);                   /*and analyse it*/
	      sizeCR = calculatesizecountrate(CRpattern,pEncoH);/* size of 1 CR Record*/
	      if (alreadyDoneBufCR == FALSE)
		{                 /* check that this allocation is just done once */
		  pBufCountRate = malloc(sizeCR*sizeof(u8)); 
		  alreadyDoneBufCR = TRUE;
		}
	    }
	}
      else
	{
	  printf("no records in file\n");
	  exit(0);
	}

      printf("size E %d size gate digi =%d\n",sizeE,sizeGD);

      setCoincidenceOutputMode(0);   /* we store the coincidences in a file (mode 0) */
      /* cf coincidenceOutputModule.c */
     
      printf("pattern read ok ...\n");
      
      /* -=-=-=-=-=-=-=-= READ THE RECORDS LOOP -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
      
      while((testread=(fread(&uni8,sizeof(u8),1,pf)))==1)   /* read 1 byte */
	{ 
	  if((uni8&BIT8)==0)                               /* TAG = 0 ; it s an event */  
	    {
	      pcC->typeOfCarrier = pEncoH->scanContent.eventRecordTag;
	      fseek(pf,-1,1);     /* one byte before , then read the whole event record */
	      testread=(fread(&pBufEvent[0],sizeof(u8),sizeE,pf));
	      if(testread == sizeE)       /* read the event record in once and fill the */
		pER = readOneEventRecord(&pBufEvent[0],Epattern,pEH,
					 pEncoH->scanEncodingIDLength); /* carrier */
	      else                                    /* is there a problem of reading ?*/
		printf("\nReading problem in LMFReader for an event record...");

	      if(pEH->gateDigiBool) /* extended event record for Gate Digi information */
		{
		  
		  testread=(fread(&pBufGateDigi[0],sizeof(u8),sizeGD,pf));
		  
		  if(testread == sizeGD)       /* read the event record in once and fill the */
		    {
		      pER->pGDR = readOneGateDigiRecord(&pBufGateDigi[0],GDpattern,pGDH,pEH); /* carrier */
		    }
		  else                                    /* is there a problem of reading ?*/
		    printf("\nReading problem in LMFReader for a gate digi record...");
		}

	    }
	  else if((uni8 & (~(BIT1+BIT2+BIT3+BIT4))) == BIT8) /* TAG = 1000  COUNTRATE */
	    {
	      pcC->typeOfCarrier = pEncoH->scanContent.countRateRecordTag;
	      fseek(pf,-1,1); /* one byte before then  read the whole Count Rate record */
	      testread=(fread(&pBufCountRate[0],sizeof(u8),sizeCR,pf));
	      if(testread == sizeCR)/* read the count rate record in once and fill the  */
		pCRR = readOneCountRateRecord(pEncoH,pBufCountRate,CRpattern);/*carrier */
	      else                                    /* is there a problem of reading ?*/
		printf("\nReading problem in LMFReader for a count rate record...");
	    }
	  fflush(stdin);

	  //	  if(strcmp("analyseCoinci",processingMode)==0) printf("o\n");
	  processRecordCarrier(pEncoH,pEH,pGDH,pCRH,pcC,pER,pCRR,processingMode,pf);
	  fflush(stdin);
	}   
     
      

      printf("Loop LMFreader over...\n");


/*
      if(strcmp("outputRoot",processingMode)==0)
	destroyOutputRootMgr();
*/
      if(strcmp("outputAscii",processingMode)==0)
	destroyOutputAsciiMgr();
 

      if(strcmp("analyseCoinci",processingMode)==0)
	destroyCoincidenceAnalyser();

      if(strcmp("treatAndCopy",processingMode)==0)
	destroyDeadTimeMgr();
 

      if(strcmp("juelichDeadTime",processingMode)==0)
      	destroyJuelichDeadTime();
         

      if(strcmp("delayLine",processingMode)==0)
      	destroyDelayLine();
 
      

      if(strcmp("sortTime",processingMode)==0)
	{

	  while(1)
	    {
	      if(finishTimeSorting(pEncoH,pEH,pCRH,pGDH,pcC,pER,pCRR,pf,bis_ccsFileName) == 0)
		break;
	    } 
	  
	  
	}
      if(strcmp("countRecords",processingMode)==0)
	destroyCounter();

      if(strcmp("sortCoincidence",processingMode)==0)
	{
	  if(pEncoH->scanContent.eventRecordBool)
	    {
	      destroyList();  /* in _coincidenceSorter.c */
	      destroyCoinciFile(); /* in _cleanKit.c */
	      outputCoincidenceModuleDestructor(); /* in outputCoincidenceModule.c */
	    }	  
	  else
	    printf("no event records to sort in this file");
	}

      if(strcmp("sortBlocks",processingMode)==0) 
	{
	  finalCleanListEv();
	}

      if(strcmp("setPosition",processingMode)==0)
	positionListDestructor();

      if(strcmp("correctDaqTime",processingMode)==0) 
	{
	  finalCleanTable();
	}

      if(pf)
	{
	  fclose(pf); /*close the file*/  
	  pf = NULL;
	}

      printf("\nFile %s read, processed and closed by the LMFreader...\n",nameOfFile);
    }
  
  /* -=-=-=-=-=-=-=-= END OF RECORDS READING  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */    
  return(0);

}




int LMFreaderDestructor(void)
{
  
  if(pEncoH->scanContent.eventRecordBool == 1)
    {
    

      if(pEH->gateDigiBool)
	{   
	  destroyGDRreader(pER->pGDR);
	  destroyExtractGDpat();
	}
 
      destroyERreader(); /* // free complete this pER in readOneEventRecord.c */
     
	destroyExtractEpat(); /* //      free pEH */

	  free(pBufEvent);

    }

  if(pEncoH->scanContent.countRateRecordBool == 1)
    {
     
      /*  // this 2 lines cannot be inverted  */
      destroyCRRreader(pCRH);
      destroyExtractCRpat();/*  //free(pCRH)  */
	free(pBufCountRate);
    } 

  free(pcC);
    
  destroyReadHead();
  
  FileCcsNotFound = FALSE;  
  alreadyDonepcC = FALSE;
  alreadyDoneBufE = FALSE;
  alreadyDoneBufCR = FALSE;


  return(0);
}





