/*-------------------------------------------------------

           List Mode Format 
                        
     --  cutEnergyModule.c  --                      

     Luc.Simon@iphe.unil.ch

     Crystal Clear Collaboration
     Copyright (C) 2003 IPHE/UNIL, CH-1015 Lausanne

     This software is distributed under the terms 
     of the GNU Lesser General 
     Public Licence (LGPL)
     See LMF/LICENSE.txt for further details

-------------------------------------------------------*/

/*-------------------------------------------------------

     Description of cutEnergyModule.c

	 Apply an energy window cut on event records

-------------------------------------------------------*/

#include <stdio.h>
#include "lmf.h"
#include "constantsLMF_ccs.h"

static FILE *entree;
static u16 *pcrist;
static float energyCalib[65536];
static int upLimit = 750; // keV
static int downLimit = 350; // keV
static int FPGANeighSelect = 100;
static u64 nRejected = 0;
static int nb_rsectors, nb_modules, nb_submodules, nb_crystals, nb_layers;

EVENT_RECORD *cutEnergy(const ENCODING_HEADER *pEncoH,
			const EVENT_HEADER *pEH,
			const GATE_DIGI_HEADER *pGDH,
			EVENT_RECORD *pER)
{
  
  int keepIT = TRUE;  /*  = FALSE if we dont keep this event */
  int energy=0;
  int energy1=0;
  float cal_energy, cal_energy1;
  int layer,crystal,submodule,module,rsector;
  int layer1,crystal1,submodule1,module1,rsector1;
  int index,index1;
  //int neighbour, neighbour1;
  
  nb_rsectors = pEncoH->scannerTopology.totalNumberOfRsectors;
  //nb_modules = pEncoH->scannerTopology.totalNumberOfModules;
  nb_modules = 1;
  nb_submodules = pEncoH->scannerTopology.totalNumberOfSubmodules;
  nb_crystals = pEncoH->scannerTopology.totalNumberOfCrystals;
  nb_layers = pEncoH->scannerTopology.totalNumberOfLayers;
  
  if(pEH->detectorIDBool == TRUE) {
    pcrist=demakeid(pER->crystalIDs[0], pEncoH);
    rsector = pcrist[4];
    module = pcrist[3];
    submodule = pcrist[2];
    crystal = pcrist[1];
    layer = pcrist[0];
    free(pcrist);
  
    if (pEH->coincidenceBool == TRUE) {
      pcrist=demakeid(pER->crystalIDs[pEH->numberOfNeighbours+1], pEncoH);
      rsector1 = pcrist[4];
      module1 = pcrist[3];
      submodule1 = pcrist[2];
      crystal1 = pcrist[1];
      layer1 = pcrist[0];
      free(pcrist);
    }
  }

  if(pEH->energyBool == FALSE) 
    {
      printf("*** error : cutEnergyModule.c : you can cut energy only if you have energy stored in your file\n");
      exit(0);
    }  
	  
  if (pEH->fpgaNeighBool == FALSE) {      
    printf("*** error : cutEnergyModule.c : you can select singles only if you have FGPA neighbours stored in your file\n");
    exit(0);  
  }

  energy =pER->energy[0];
    
  index = rsector*nb_modules*nb_submodules*nb_crystals*nb_layers;
  index += module*nb_submodules*nb_crystals*nb_layers;
  index += submodule*nb_crystals*nb_layers;
  index += crystal*nb_layers;
  index += layer;
  
  cal_energy = energy*energyCalib[index];
  //if ((layer==1)&&(crystal==0)&&(rsector==0)) printf("%d %d %d %d %d index %d %f %f\n",layer,crystal,submodule,module,rsector,index,energyCalib[index],cal_energy); 

  if (pEH->coincidenceBool == TRUE) {
    energy1 =pER->energy[1];

    index1 = rsector1*nb_modules*nb_submodules*nb_crystals*nb_layers;
    index1 += module1*nb_submodules*nb_crystals*nb_layers;
    index1 += submodule1*nb_crystals*nb_layers;
    index1 += crystal1*nb_layers;
    index1 += layer1;
    
    
    cal_energy1 = energy1*energyCalib[index1];
    //if ((layer1==1)&&(crystal1==0)&&(rsector1==0)) printf("%d %d %d %d %d index1 %d %f %f\n",layer1,crystal1,submodule1,module1,rsector1,index1,energyCalib[index1],cal_energy1);
   }
  
  if (pEH->coincidenceBool == TRUE) {
    //Select LSO
    //if ((layer==1)||(layer1==1)) keepIT = FALSE;
    //Select LuYAP
    //if ((layer==0)||(layer1==0)) keepIT = FALSE;
  }
  
  if(upLimit != 0) {
    if(cal_energy > upLimit) keepIT = FALSE ;
    if ((pEH->coincidenceBool == TRUE)&&(cal_energy1 > upLimit)) keepIT = FALSE;
  }
  
  if(downLimit != 0) {
    if(cal_energy < downLimit) keepIT = FALSE ; 
    if ((pEH->coincidenceBool == TRUE)&&(cal_energy1 < downLimit)) keepIT = FALSE;
  }

  /*if (FPGANeighSelect != 100) {
    neighbour = pER->fpgaNeighInfo[0];
    if (neighbour != FPGANeighSelect) keepIT = FALSE;
    if (pEH->coincidenceBool == TRUE) {
      neighbour1 = pER->fpgaNeighInfo[1];
      if (neighbour1 != FPGANeighSelect) keepIT = FALSE;
    }
    }*/

  if(keepIT) return(pER);
  else {
    nRejected++;
    return(NULL);    
  }
}

void setEnergyCalib(char energyCalibFile[40],int nb_layers,int nb_crystals,int nb_submodules,int nb_modules,int nb_rsectors)
{
  printf("Cut energy Module: calibration with %s\n",energyCalibFile);
  char t;
  float mean,sigma;
  float *calib;
  int layer,crystal,submodule,module,rsector;
  
  //init calib to 0.0
  calib = energyCalib;
  for (rsector=0;rsector<nb_rsectors;rsector++) {
    for (module=0;module<nb_modules;module++) { 
      for (submodule=0;submodule<nb_submodules;submodule++) {
	for (crystal=0;crystal<nb_crystals;crystal++) {
	  for (layer=0;layer<nb_layers;layer++) {
	    *calib++ = 0.;
	  }
	}
      }
    }
  }
  printf("Cut energy Module: calibration init done !\n");
   
  entree=fopen(energyCalibFile,"r");

  calib = energyCalib;
  printf("Cut energy Module: calib file %s\n",energyCalibFile);

  for (rsector=0;rsector<nb_rsectors;rsector++) {
    for (module=0;module<nb_modules;module++) { 
      for (submodule=0;submodule<nb_submodules;submodule++) {
	for (crystal=0;crystal<nb_crystals;crystal++) {
	  for (layer=0;layer<nb_layers;layer++) {   
	    fscanf(entree,"%f%c%f",&mean,&t,&sigma);
	    *calib++ = 511./mean;
	  }
	}
      }
    }
  }
  fclose(entree);
}

void setUpEnergyLimit(int upKeVLimit)
{
  upLimit = upKeVLimit;
  printf("Cut Energy Module : upper limit set to %d\n",upKeVLimit);
}

void setDownEnergyLimit(int downKeVLimit)
{
  downLimit = downKeVLimit;
  printf("Cut Energy Module : lower limit set to %d\n",downKeVLimit);
}

void setFPGANeighSelect(int fpgaNeighSelect)
{
  FPGANeighSelect = fpgaNeighSelect;
  printf("Cut Energy Module : FPGA neighbour selection set to %d\n",FPGANeighSelect);
}


u64 printEnergyModuleRejectedEventNumber(void)
{
  printf("Number of rejected records by cutEnergyModule.c : %llu\n",nRejected);
  return(nRejected);
}
