/*-------------------------------------------------------

List Mode Format 
                        
--  blockSorter.c  --                      

Martin.Rey@epfl.ch

Crystal Clear Collaboration
Copyright (C) 2004 LPHE/EPFL, CH-1015 Lausanne

This software is distributed under the terms 
of the GNU Lesser General 
Public Licence (LGPL)
See LMF/LICENSE.txt for further details

-------------------------------------------------------*/

/*-------------------------------------------------------

Description of blockSorter

Allows to re-mix events comming from different sectors.
It is usefull when the events are stored by fixed size block
of specific sectors.
You have to specify the number of sectors you use and 
the size of one block

-------------------------------------------------------*/


#include <stdio.h>
#include "lmf.h"

static u16 sctNb;
static LIST *listEv;
static u16 *sctName;
static u64 *t;

static CURRENT_CONTENT *pcCC = NULL;     /* to build coinci file head */
static ENCODING_HEADER *pEncoHC = NULL;  /* to build coinci file head */
static EVENT_HEADER *pEHC = NULL;        /* to build coinci file head */
static GATE_DIGI_HEADER *pGDHC = NULL;    /* to build coinci file head */
static COUNT_RATE_HEADER *pCRHC = NULL;   /* to build coinci file head */
static COUNT_RATE_RECORD *pCRRC = NULL;   /* to build coinci file head */
static FILE *pfileC = NULL;
static char *fileNameC;

void setNbOfSct(u16 inlineSctNb)
{
  sctNb = inlineSctNb;
}

void copyHeads(const ENCODING_HEADER *pEncoH,
	       const EVENT_HEADER *pEH,
	       const GATE_DIGI_HEADER *pGDH,
	       const COUNT_RATE_HEADER *pCRH,
	       const CURRENT_CONTENT *pcC,
	       const COUNT_RATE_RECORD *pCRR,
	       char *fileNameCCS)
{
  pEncoHC = (ENCODING_HEADER *)malloc(sizeof(ENCODING_HEADER));
  pEHC = (EVENT_HEADER *)malloc(sizeof(EVENT_HEADER));
  pGDHC =  (GATE_DIGI_HEADER *)malloc(sizeof(GATE_DIGI_HEADER));
  pCRHC = (COUNT_RATE_HEADER *)malloc(sizeof(COUNT_RATE_HEADER));
  pcCC = (CURRENT_CONTENT *)malloc(sizeof(CURRENT_CONTENT));
  pCRRC = (COUNT_RATE_RECORD *)malloc(sizeof(COUNT_RATE_RECORD));
      
  if ((pEncoHC == NULL)||(pEHC == NULL)||(pGDHC == NULL)||(pCRHC == NULL)||(pcCC == NULL)||(pCRRC == NULL))
    printf("\n *** error : blockSorter.c : copyHeads : malloc\n");
      
  if(pEncoH)
    *pEncoHC = *pEncoH ; /* no pointer in this structure, it is safe */
  if(pEH)
    *pEHC = *pEH ;       /* no pointer in this structure, it is safe */
  if(pCRH)
    *pCRHC = *pCRH ;    /*  no pointer in this structure, it is safe */
  if(pGDH)
    *pGDHC = *pGDH ;    /*  no pointer in this structure, it is safe */
  if(pcC)
    *pcCC = *pcC ;    /*  no pointer in this structure, it is safe */

  fileNameC = fileNameCCS;
}

void sortBlocks(const ENCODING_HEADER *pEncoH,
		const EVENT_HEADER *pEH,
		const COUNT_RATE_HEADER *pCRH,
		const GATE_DIGI_HEADER *pGDH,
		const CURRENT_CONTENT *pcC,   
		const EVENT_RECORD *pER,
		const COUNT_RATE_RECORD *pCRR,
		char *fileNameCCS)
{
  static u32 viewOnce = 0;
  static u8 doneonce = 0; 
  static u8 check = 0;
  u8 notEmpty;

  EVENT_RECORD *pERin;
  ELEMENT *searcher, *destroyer;
  void *data;

  u16 sct, index, old;
  int error = 0;

  u16 *pcrist;
  u64 tOld;
  
  if(!doneonce)
    {
      if(!(listEv = (LIST *)malloc(sctNb * sizeof(LIST))))
	printf("\n *** error : blockSorter.c : sortBlocks : malloc\n");
      if(!(sctName = (u16 *) malloc(sctNb * sizeof(u16))))
	printf("\n *** error : blockSorter.c : sortBlocks : malloc\n");
      if(!(t = (u64*)malloc(sctNb * sizeof(u64))))
	printf("\n *** error : blockSorter.c : sortBlocks : malloc\n");
      
      copyHeads(pEncoH,pEH,pGDH,pCRH,pcC,pCRR,fileNameCCS);
      doneonce++;
    }

  pERin = newER(pEHC);    /* complete allocatation for an element */
	  
  copyER(pER,pERin,pEHC); /* *pERin = *pER but safe */

  pcrist = demakeid(pERin->crystalIDs[0], pEncoHC);
  sct = pcrist[4];
  free(pcrist);
  
  if (!((viewOnce>>sct)&1))
    {
      viewOnce |= 1 << sct;
      check = 0;
      for (index = 0; index < 8 * sizeof(u32); index++)
	check += (viewOnce>>index)&1;
      sctName[check - 1] = sct;
      dlist_init(&(listEv[check - 1]),(void *) freeER); /* init the List */
      if (check > sctNb)
	{
	  printf("nb of sector in file is greater than the one introduced\nPlease re-run\n");
	  exit(0);
	}
    }

  for (index = 0; index < check; index++)
    if (sct == sctName[index])
      break;

  if (dlist_ins_next(&(listEv[index]), dlist_tail(&(listEv[index])), pERin))
    error = 1;

  if (check == sctNb)
    {
      notEmpty = 0;
      for (index = 0; index < sctNb; index++)
	{
	  if(dlist_size(&(listEv[index])) > 0)
	    notEmpty++;
	}

      while(notEmpty == sctNb)
	{
	  tOld = -1;
	  for (index = 0; index < sctNb; index++)
	    {
	      searcher = dlist_head(&(listEv[index]));
	      t[index] = getTimeOfThisEVENT((EVENT_RECORD*)((searcher)->data));
	      if(t[index] < tOld)
		{
		  tOld = t[index];
		  old = index;
		}
	    }
	  destroyer =  dlist_head(&(listEv[old]));
	  LMFbuilder(pEncoHC,pEHC,pCRHC,pGDHC,pcCC,(EVENT_RECORD *)(destroyer->data),pCRRC,&pfileC,fileNameC);
	  if ((dlist_remove(&(listEv[old]), destroyer, (void **)&data)) == 0 && 
	      (listEv[old].destroy != NULL)) 
	    listEv[old].destroy(data);
	
	  notEmpty = 0;
	  for (index = 0; index < sctNb; index++)
	    {
	      if(dlist_size(&(listEv[index])) > 0)
		notEmpty++;
	    }

	}
    } 
}

void finalCleanListEv()
{
  ELEMENT *searcher, *destroyer;
  EVENT_RECORD *pERout;
  void *data;
  u16 index, old;
  u8 notEmpty;
  u64 tOld;

  notEmpty = sctNb;
  while(notEmpty)
    {      
      notEmpty = 0;
      tOld = -1;
      for(index = 0; index < sctNb; index++)
	{	
	  if(dlist_size(&(listEv[index])))
	    {
	      searcher = dlist_head(&(listEv[index]));
	      pERout = (EVENT_RECORD*)((searcher)->data);

	      t[index] = (u64)getTimeOfThisEVENT(pERout);
	      if(t[index] < tOld)
		{
		  tOld = t[index];
		  old = index;
		}
	      notEmpty++;
	    }
	}

      if(notEmpty)
	{
	  destroyer =  dlist_head(&(listEv[old]));
	  LMFbuilder(pEncoHC,pEHC,pCRHC,pGDHC,pcCC,(EVENT_RECORD *)(destroyer->data),pCRRC,&pfileC,fileNameC);
	  if ((dlist_remove(&(listEv[old]), destroyer, (void **)&data)) == 0 && 
	      (listEv[old].destroy != NULL)) 
	    listEv[old].destroy(data);
	}
    }

  for(index = 0; index < sctNb; index++)
    dlist_destroy(&(listEv[index]));

  free(listEv);
  free(sctName);
  free(t);

  if(pEncoHC->scanContent.eventRecordBool == 1)
    {
      free(pEHC);
      if(pEncoHC->scanContent.gateDigiRecordBool == 1)
	if(pGDHC)
	  free(pGDHC);
    }

  if(pcCC)
    free(pcCC);
  if(pEncoHC)
    free(pEncoHC);
  if(pGDHC)
    free(pGDHC);
  if(pCRHC)
    free(pCRHC);
  if(pCRRC)
    free(pCRRC);

  fclose(pfileC);
}
