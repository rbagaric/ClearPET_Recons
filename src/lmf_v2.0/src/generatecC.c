/*-------------------------------------------------------

           List Mode Format 
                        
     --  generatecC.c  --                      

     Luc.Simon@iphe.unil.ch

     Crystal Clear Collaboration
     Copyright (C) 2003 IPHE/UNIL, CH-1015 Lausanne

     This software is distributed under the terms 
     of the GNU Lesser General 
     Public Licence (LGPL)
     See LMF/LICENSE.txt for further details

-------------------------------------------------------*/

/*-------------------------------------------------------

     Description of generatecC.c:
     allocation for a current content structure.

-------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include "lmf.h"
struct LMF_ccs_currentContent *pcC=NULL;

struct LMF_ccs_currentContent (*generatecC())
{


  if((pcC = (struct LMF_ccs_currentContent *) malloc(sizeof(struct LMF_ccs_currentContent))) == NULL)
    printf("\n*** ERROR : in generatecC.c : impossible to do : malloc()\n");
  return(pcC);
}
void generatecCDestructor()
{
  if(pcC)
    free(pcC);
}
