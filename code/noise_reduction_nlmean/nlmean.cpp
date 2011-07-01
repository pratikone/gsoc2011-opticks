/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include<math.h>
#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "DataRequest.h"
#include "DesktopServices.h"
#include "MessageLogResource.h"
#include "ObjectResource.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "Progress.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterUtilities.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "StringUtilities.h"
#include "switchOnEncoding.h"
#include "nlmean.h"
#include <limits>

REGISTER_PLUGIN_BASIC(OpticksTutorial, nlmean);

#define SIGMA 15.0  //standard deviation of noise
#define size 17
#define csize 3


namespace
{
   template<typename T>
   void nonlocalmeans(T* pData, DataAccessor pSrcAcc, int row, int col, int rowSize, int colSize)
   {
	  
	  //based on noise
	  double h=0.55*SIGMA;
	  double max;
	  double h2=h*h;
	  double totalWeight=0;
	  double finalVal=0;
	  double sampledata[1000];
	  double weights[50][50];

	  double p=0,q=0;
	  double res=0;

      T data[50][50];
	  

	  int mul=2*SIGMA*SIGMA;
      
	  int r=0,c=0,d2=0;
	  int pr=0,qr=0,pc=0,qc=0;
	  int ctr=0;
	  int row2=row,col2=col;
	  int prow2=row,qrow2=row,pcol2=col,qcol2=col;
	  int rtemp=0,ctemp=0;
	  

	  //value and comparison window

	  //before

	  for(r=size/2;r>=0;r--,row2--)
		 { 
			if(r==(int)size/2)
				   c=size/2;
			else {
					c=size-1;
					col2=col2+size-1;
				 }

			while(c>=0)
			 {   
				   
					//pSrcAcc->toPixel(std::max(row2,0), std::max(col2,0));
					//VERIFYNRV(pSrcAcc.isValid());
					//data[r][c]= *reinterpret_cast<T*>(pSrcAcc->getColumn());
					//d2=(r-(size/2))*(r-(size/2))+(c-(size/2))*(c-(size/2));
					//weights[r][c]=exp(-(std::max(d2-mul,0))/(h2));
					
					
				    //before-inner1
				    prow2=row;pcol2=col;	//p
					qrow2=row2;qcol2=col2;	//q
					for(pr=csize/2,qr=csize/2;pr>=0 && qr>=0;pr--,qr--,prow2--,qrow2--)
					 {  
						//p
						if(pr==(int)csize/2)
							   pc=csize/2;
						else {
								pc=csize-1;
								pcol2=pcol2+csize-1;
							 }
						//q
						if(qr==(int)csize/2)
							   qc=csize/2;
						else {
								qc=csize-1;
								qcol2=qcol2+csize-1;
							 }

						while(pc>=0 && qc>=0)
						   {   
				         	 //p
							 pSrcAcc->toPixel(std::max(prow2,0), std::max(pcol2,0));
							// VERIFYNRV(pSrcAcc.isValid());
							 p= *reinterpret_cast<T*>(pSrcAcc->getColumn());
							 //q
							 pSrcAcc->toPixel(std::max(qrow2,0), std::max(qcol2,0));
							 VERIFYNRV(pSrcAcc.isValid());
							 q= *reinterpret_cast<T*>(pSrcAcc->getColumn());

							 //result
							 res+=(p-q)*(p-q);
							 

							  pc--;
							  qc--;
							  pcol2--;
							  qcol2--;
				       	  }
					 
					}

					//after-inner1
					prow2=row,pcol2=col+1;		//p
					qrow2=row2,qcol2=col2+1;	//q

					for(pr=csize/2,qr=csize/2;pr<csize && qr<csize;pr++,qr++,prow2++,qrow2++)
					{  
			          //p
					  if(pr==(int)csize/2)
					  {
						  pc=csize/2+1;
					  }
					  else
					  {
						  pc=0;
						  pcol2=pcol2-csize+1;
					  }
			          
					  //q
					  if(qr==(int)csize/2)
					  {
						  qc=csize/2+1;
					  }
					  else
					  {
						  qc=0;
						  qcol2=qcol2-csize+1;
					  }

					  while(pc<csize && qc<csize)
					  {   
				         //p
                         rtemp=std::max(prow2,0);
						 ctemp=std::max(pcol2,0);
      					 pSrcAcc->toPixel(std::min(rtemp,rowSize-1), std::min(ctemp,colSize-1));
						 VERIFYNRV(pSrcAcc.isValid());
						 p= *reinterpret_cast<T*>(pSrcAcc->getColumn());

						 //q
						 rtemp=std::max(qrow2,0);
						 ctemp=std::max(qcol2,0);
						 pSrcAcc->toPixel(std::min(rtemp,rowSize-1), std::min(ctemp,colSize-1));
						 //VERIFYNRV(pSrcAcc.isValid());
						 q= *reinterpret_cast<T*>(pSrcAcc->getColumn());

						 //result
						 res+=(p-q)*(p-q);
						 
						 

						  pc++;
						  qc++;
						  pcol2++;
						  qcol2++;
				  
			  
					  }

					}
					d2=res/(csize*csize);		//d2
					weights[r][c]=exp(-(std::max(d2-mul,0))/(h2));

					pSrcAcc->toPixel(std::max(row2,0), std::max(col2,0));
					VERIFYNRV(pSrcAcc.isValid());
					data[r][c]= *reinterpret_cast<T*>(pSrcAcc->getColumn());

					c--;
				    col2--;
					
			  }
		  }

	  
	  
       
      r=0;c=0;d2=0;
	  row2=row;col2=col+1;
	  

	  //after
      for(r=size/2;r<size;r++,row2++)
		 {  
			 
			  if(r==(int)size/2)
			  {
				  c=size/2+1;
			  }
			  else
			  {
				  c=0;
				  col2=col2-size+1;
			  }
			    
			  while(c<size)
			  {   
				 /*        
      			 pSrcAcc->toPixel(std::min(row2,rowSize-1), std::min(col2,colSize-1));
				 VERIFYNRV(pSrcAcc.isValid());
				 data[r][c]= *reinterpret_cast<T*>(pSrcAcc->getColumn());
				 d2=(r-(size/2))*(r-(size/2))+(c-(size/2))*(c-(size/2));
				 weights[r][c]=exp(-(std::max(d2-mul,0))/(h2));
				 */

				  //before-inner2
				  prow2=row;pcol2=col;qrow2=row;qcol2=col2;

				  for(pr=csize/2,qr=csize/2;pr>=0 && qr>=0;pr--,qr--,prow2--,qrow2--)
					 {  
						//p
						if(pr==(int)csize/2)
							   pc=csize/2;
						else {
								pc=csize-1;
								pcol2=pcol2+csize-1;
							 }
						//q
						if(qr==(int)csize/2)
							   qc=csize/2;
						else {
								qc=csize-1;
								qcol2=qcol2+csize-1;
							 }

						while(pc>=0 && qc>=0)
						   {   
				         	 //p
							 rtemp=std::min(prow2,rowSize);
							 ctemp=std::min(pcol2,colSize);
							 pSrcAcc->toPixel(std::max(rtemp,0), std::max(ctemp,0));
							 VERIFYNRV(pSrcAcc.isValid());
							 p= *reinterpret_cast<T*>(pSrcAcc->getColumn());
							 
							 //q
							 rtemp=std::min(qrow2,rowSize);
							 ctemp=std::min(qcol2,colSize);
							 pSrcAcc->toPixel(std::max(rtemp,0), std::max(ctemp,0));
							 VERIFYNRV(pSrcAcc.isValid());
							 q= *reinterpret_cast<T*>(pSrcAcc->getColumn());

							 //result
							 res+=(p-q)*(p-q);
							 

							  pc--;
							  qc--;
							  pcol2--;
							  qcol2--;
				       	  }
					 
					}

					//after-inner2
					prow2=row,pcol2=col+1;		//p
					qrow2=row2,qcol2=col2+1;	//q

					for(pr=csize/2,qr=csize/2;pr<csize && qr<csize;pr++,qr++,prow2++,qrow2++)
					{  
			          //p
					  if(pr==(int)csize/2)
					  {
						  pc=csize/2+1;
					  }
					  else
					  {
						  pc=0;
						  pcol2=pcol2-csize+1;
					  }
			          
					  //q
					  if(qr==(int)csize/2)
					  {
						  qc=csize/2+1;
					  }
					  else
					  {
						  qc=0;
						  qcol2=qcol2-csize+1;
					  }

					  while(pc<csize && qc<csize)
					  {   
				         //p
						 pSrcAcc->toPixel(std::min(prow2,rowSize-1), std::min(pcol2,colSize-1));
						 VERIFYNRV(pSrcAcc.isValid());
						 p= *reinterpret_cast<T*>(pSrcAcc->getColumn());

						 //q
						 pSrcAcc->toPixel(std::min(qrow2,rowSize-1), std::min(qcol2,colSize-1));
						 VERIFYNRV(pSrcAcc.isValid());
						 q= *reinterpret_cast<T*>(pSrcAcc->getColumn());

						 //result
						 res+=(p-q)*(p-q);
						 
						 

						  pc++;
						  qc++;
						  pcol2++;
						  qcol2++;
				  
			  
					  }

					}

					d2=res/(csize*csize);		//d2
					weights[r][c]=exp(-(std::max(d2-mul,0))/(h2));

					pSrcAcc->toPixel(std::min(row2,rowSize-1), std::min(col2,colSize-1));
					VERIFYNRV(pSrcAcc.isValid());
					data[r][c]= *reinterpret_cast<T*>(pSrcAcc->getColumn());
                  res=0; //reset res
				  c++;
				  col2++;
				  
			  
			  }

		  }

	  int count=0,count2=0;
      /*
      //for central pixel
      max=weights[0][0];
	  for(count=0;count<size;count++)
		  for(count2=0;count2<size;count2++)
	  {
		  max=std::max(max,weights[count][count2]);
	  }
	  weights[size/2][size/2]=max;
	  */
	  
	  //calculate sum of values
	  for(count=0;count<size;count++)
		  for(count2=0;count2<size;count2++)
	  {
		  finalVal+=data[count][count2]*weights[count][count2];
		  totalWeight+=weights[count][count2];
		  
	  }


/*
	  //search window

	  r=0;c=0;d2=0;
	  ctr=0;
	  row2=row;col2=col;
	  
	  //before
	  for(r=searchsize/2;r>=0;r--,row2--)
		 { 
			if(r==(int)searchsize/2)
				   c=searchsize/2;
			else {
					c=searchsize-1;
					col2=col2+searchsize-1;
				 }

			while(c>=0)
			 {   
				  if(r!=row || c!=col)
					{ 
						d2=(r-(searchsize/2))*(r-(searchsize/2))+(c-(searchsize/2))*(c-(searchsize/2));
						sampledata[ctr++]= exp(-(std::max(d2-mul,0))/(h2));
					}
				  c--;
				  col2--;
			  }
		  }

	  
	  
	  //after

	  row2=row,col2=col+1;
	  
	  for(r=searchsize/2;r<searchsize;r++,row2++)
		 {  
			 
			  if(r==(int)searchsize/2)
			  {
				  c=searchsize/2+1;
			  }
			  else
			  {
				  c=0;
				  col2=col2-searchsize+1;
			  }
			    
			  while(c<searchsize)
			  {   
				         
      			 d2=(r-(searchsize/2))*(r-(searchsize/2))+(c-(searchsize/2))*(c-(searchsize/2));
				 sampledata[ctr++]= exp(-(std::max(d2-mul,0))/(h2));
					

				  c++;
				  col2++;
				  
			  
			  }

		  }

	  
	  //totalWeight
	  for(count=0;count<ctr;count++)
	  {
		  
		  totalWeight+=sampledata[count];
	  }

	  totalWeight+=max;
	  
*/  
	  	  
	  //normalized value
	  finalVal=finalVal/totalWeight;

   
	  *pData=static_cast<T>(finalVal);
   
   }
};


bool nlmean::standard_deviation(RasterElement *pRaster, RasterElement *dRaster, int i, Progress *pProgress)
{
	VERIFY(pRaster != NULL);
	RasterDataDescriptor* pDesc = dynamic_cast<RasterDataDescriptor*>(pRaster->getDataDescriptor());
	VERIFY(dRaster != NULL);
	RasterDataDescriptor* rDesc = dynamic_cast<RasterDataDescriptor*>(dRaster->getDataDescriptor());

	std::string msg=" ";
	
	DimensionDescriptor thirdBand = pDesc->getActiveBand(i);   //get active band
	
   //source
   FactoryResource<DataRequest> pRequest;
   pRequest->setInterleaveFormat(BSQ);
   pRequest->setBands(thirdBand, thirdBand);
   DataAccessor thirdBandDa = pRaster->getDataAccessor(pRequest.release());

   
   thirdBand = rDesc->getActiveBand(i);

   //destination
   FactoryResource<DataRequest> pResultRequest;
   pResultRequest->setWritable(true);
   pRequest->setInterleaveFormat(BSQ);
   pResultRequest->setBands(thirdBand,thirdBand);
   DataAccessor pDestAcc = dRaster->getDataAccessor(pResultRequest.release());

   
   VERIFY(thirdBandDa.isValid());
   VERIFY(pDestAcc.isValid());
   

   for (unsigned int curRow = 0; curRow < pDesc->getRowCount(); ++curRow)

   {
	   for (unsigned int curCol = 0; curCol < pDesc->getColumnCount(); ++curCol)
	  {	  
		
		switchOnEncoding(pDesc->getDataType(), nonlocalmeans, pDestAcc->getColumn(), thirdBandDa, curRow, curCol,
         pDesc->getRowCount(), pDesc->getColumnCount());
		
		//msg="Working on "+StringUtilities::toDisplayString(curRow)+" , "+StringUtilities::toDisplayString(curCol);
		//pProgress->updateProgress(msg,40,NORMAL);
		pDestAcc->nextColumn();
	  }
	        
	  pDestAcc->nextRow();

   }
   return true;

}


nlmean::nlmean()
{
   setDescriptorId("{F7CBEA2C-84DC-4AA5-B45F-ACDE683AA1B0}");
   setName("Non-local Mean");
   setVersion("0.1");
   setDescription("Performs Non-local mean noise removal on"
	   " Additive White Gaussian Noise");
   setCreator("Pratik Anand");
   setCopyright("Copyright (C) 2011, Pratik Anand <pratik@pratikanand.com>");
   setProductionStatus(false);
   setType("Algorithm");
   setSubtype("Noise reduction");
   setMenuLocation("[Photography]/nlmean");
   setAbortSupported(true);
}

nlmean::~nlmean()
{
}

bool nlmean::getInputSpecification(PlugInArgList*& pInArgList)
{
   VERIFY(pInArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pInArgList->addArg<Progress>(Executable::ProgressArg(), NULL, "Progress reporter");
   pInArgList->addArg<RasterElement>(Executable::DataElementArg(), "Perform noise reduction on this data element");
   return true;
}

bool nlmean::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   VERIFY(pOutArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pOutArgList->addArg<RasterElement>("Result", NULL);
   return true;
}

bool nlmean::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("nlmean", "app", "5EA0CC75-9E0B-4c3d-BA23-6DB7157BBD78");
   if (pInArgList == NULL || pOutArgList == NULL)
   {
      return false;
   }

   Progress* pProgress = pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg());
   
   RasterElement* pCube = pInArgList->getPlugInArgValue<RasterElement>(Executable::DataElementArg());  //pCube
   
   if (pCube == NULL)
   {
      std::string msg = "A raster cube must be specified.";
      pStep->finalize(Message::Failure, msg);
      if (pProgress != NULL)
      {
         pProgress->updateProgress(msg, 0, ERRORS);
      }

      return false;
   }
   
   pProgress->updateProgress("Starting calculations", 10, NORMAL);
   std::string msg="Peforming non local mean algorithm";
   //nlmean::standard_deviation(pCube,pProgress);

   RasterDataDescriptor* pDesc = static_cast<RasterDataDescriptor*>(pCube->getDataDescriptor());
   VERIFY(pDesc != NULL);
   RasterElement *dRas=RasterUtilities::createRasterElement(pCube->getName()+"RGB",
      pDesc->getRowCount(), pDesc->getColumnCount(),3, pDesc->getDataType(),BSQ);
   
   pProgress->updateProgress("Starting calculations", 10, NORMAL);
   
   
   standard_deviation(pCube,dRas, 0, pProgress);
   pProgress->updateProgress(msg+"RED complete", 60, NORMAL);
   standard_deviation(pCube,dRas, 1, pProgress);
   pProgress->updateProgress(msg+"GREEN complete", 70, NORMAL);
   standard_deviation(pCube,dRas, 2, pProgress);
   pProgress->updateProgress(msg+"BLUE complete", 80, NORMAL);

  
   //new model resource
   RasterDataDescriptor* rDesc = dynamic_cast<RasterDataDescriptor*>(dRas->getDataDescriptor());
   rDesc->setDisplayMode(RGB_MODE);										//enable color mode
   rDesc->setDisplayBand(RED,pDesc->getActiveBand(0));
   rDesc->setDisplayBand(GREEN,pDesc->getActiveBand(1));
   rDesc->setDisplayBand(BLUE,pDesc->getActiveBand(2));
   ModelResource<RasterElement> pResultCube(dRas);
   
   
   pProgress->updateProgress("Final", 100, NORMAL);
  
  
   //create window

   if(!isBatch())
   {
    Service<DesktopServices> pDesktop;

    SpatialDataWindow* pWindow = static_cast<SpatialDataWindow*>(pDesktop->createWindow(pResultCube->getName(),
         SPATIAL_DATA_WINDOW));

    SpatialDataView* pView = (pWindow == NULL) ? NULL : pWindow->getSpatialDataView();
    if (pView == NULL)
      {
         std::string msg = "Unable to create view.";
         pStep->finalize(Message::Failure, msg);
         if (pProgress != NULL) 
         {
            pProgress->updateProgress(msg, 0, ERRORS);
         }
         return false;
      }

	
   

      pView->setPrimaryRasterElement(pResultCube.get());
      pView->createLayer(RASTER, pResultCube.get());
   
  }
   pOutArgList->setPlugInArgValue("nlmean", pResultCube.release());  //for saving data

   

   pStep->finalize();
   return true;
}
