/*
 * The information in this file is
 * Copyright (C) 2011, Pratik Anand <pratik@pratikanand.com>
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

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
#include "switchOnEncoding.h"
#include "StringUtilities.h"
#include "adaptive_median.h"
#include <limits>



REGISTER_PLUGIN_BASIC(PhotographyProcessingTools, adaptive_median);

#define MAX_SIZE 15

namespace
{
   template<typename T>
   void adaptivemedian(T* pData, DataAccessor pSrcAcc, int row, int col, int rowSize, int colSize, int size, int sizeMax,Progress* pProgress,int *flagptr)
   {
      double sampledata[625];
	  double val,median=0;
	  
	  double zmin=0,zmax=0,zxy=0;

	  int r=0,c=0;
	  int ctr=0;
	  int row2=row,col2=col;
	  
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
				  pSrcAcc->toPixel(std::max(row2,0), std::max(col2,0));
				  VERIFYNRV(pSrcAcc.isValid());
				  sampledata[ctr++]= *reinterpret_cast<T*>(pSrcAcc->getColumn());
				  
				  c--;
				  col2--;
			  }
		  }

	  
	  
	  row2=row,col2=col+1;

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
				         
      			  pSrcAcc->toPixel(std::min(row2,rowSize-1), std::min(col2,colSize-1));
				  VERIFYNRV(pSrcAcc.isValid());
				  sampledata[ctr++]= *reinterpret_cast<T*>(pSrcAcc->getColumn());
				  
				  c++;
				  col2++;
				  
			  
			  }

		  }

	  
	  //insertion sort
	  
	  int i=0,j=0;
	  for(i=1;i<ctr;i++)
	  { val=sampledata[i];
	    j=i-1;
	    while(j>=0 && sampledata[j]>val)
		{
			sampledata[j+1]=sampledata[j];
			j--;
		}

		sampledata[j+1]=val;
	  }
	  
	  
	  //median of the entire sample data
      int index=ctr/2;
	  median=sampledata[index];
	  
	  zmin=sampledata[0];
	  zmax=sampledata[ctr-1];
	  
	  //storing zxy
	  pSrcAcc->toPixel(row,col);
	  VERIFYNRV(pSrcAcc.isValid());
	  zxy= *reinterpret_cast<T*>(pSrcAcc->getColumn());
	  
	  
	  if(zmax==median || zmin==median)
	  {
		  if(size>=sizeMax)
		  {
			*pData = static_cast<T>(zxy); 
			*flagptr=0;
			return;
		  }
		  else
		  {
			 *flagptr=1;
			 return;
		  }
	  }

	 
     	  
	  if(zmax==zxy || zmin==zxy)
	  {
         *pData = static_cast<T>(median); 
		
	  }

	  
	  else
	  {
		  *pData = static_cast<T>(zxy);
		  
	  }
	  

	  *flagptr=0;
   }
};


bool adaptive_median::copyImage4(RasterElement *pRaster,RasterElement *dRaster,int i,Progress* pProgress)
{   

   int flag=0;
   int size=3;
   int sizeMax=MAX_SIZE;

	VERIFY(pRaster != NULL);
	RasterDataDescriptor* pDesc = dynamic_cast<RasterDataDescriptor*>(pRaster->getDataDescriptor());
	VERIFY(dRaster != NULL);
	RasterDataDescriptor* rDesc = dynamic_cast<RasterDataDescriptor*>(dRaster->getDataDescriptor());

	
	
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
		VERIFY(pDestAcc.isValid());
		switchOnEncoding(pDesc->getDataType(), adaptivemedian, pDestAcc->getColumn(), thirdBandDa, curRow, curCol,
			pDesc->getRowCount(), pDesc->getColumnCount(), size, sizeMax, pProgress, &flag);
		
		
		if(flag==1 && size<=sizeMax)
			{
				//increase window size
				size=size+2;
				curCol--;
				
			}
		
		else
			{
				pDestAcc->nextColumn();
				size=3;
				flag=0;
			}
	  }
	        
	  pDestAcc->nextRow();

   }

   return true;
}


adaptive_median::adaptive_median()
{
   setDescriptorId("{FF1EFA03-0888-4199-AB40-0503C9FABC80}");
   setName("adaptive_median");
   setDescription("Perform noise reduction on an image using an adaptive median filter");
   setCreator("Pratik Anand");
   setVersion("0.1");
   setCopyright("Copyright (C) 2011, Pratik Anand <pratik@pratikanand.com>");
   setProductionStatus(true);
   setType("Algorithm");
   setSubtype("Noise reduction");
   setMenuLocation("[Photography]/adaptive_median");
   setAbortSupported(false);
}

adaptive_median::~adaptive_median()
{
}

bool adaptive_median::getInputSpecification(PlugInArgList*& pInArgList)
{
   VERIFY(pInArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pInArgList->addArg<Progress>(Executable::ProgressArg(), NULL, "Progress reporter");
   pInArgList->addArg<RasterElement>(Executable::DataElementArg(), "Perform noise reduction on this data element");
   return true;
}

bool adaptive_median::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   VERIFY(pOutArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pOutArgList->addArg<RasterElement>("Result", NULL);
   return true;
}

bool adaptive_median::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("adap_median", "noise", "5EA0CC75-9E0B-4c3d-BA23-6DB7157BBD55");
   if (pInArgList == NULL || pOutArgList == NULL)
   {
      return false;
   }

   std::string msg="Noise reduction by Adaptive Median filter ";
   Progress* pProgress = pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg());
   RasterElement* pCube = pInArgList->getPlugInArgValue<RasterElement>(Executable::DataElementArg());
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
   RasterDataDescriptor* pDesc = static_cast<RasterDataDescriptor*>(pCube->getDataDescriptor());
   VERIFY(pDesc != NULL);
   if (pDesc->getDataType() == INT4SCOMPLEX || pDesc->getDataType() == FLT8COMPLEX)
   {
      std::string msg = "Noise reduction cannot be performed on complex types.";
      pStep->finalize(Message::Failure, msg);
      if (pProgress != NULL) 
      {
         pProgress->updateProgress(msg, 0, ERRORS);
      }
      return false;
   }

   FactoryResource<DataRequest> pRequest;
   pRequest->setInterleaveFormat(BSQ);
   DataAccessor pSrcAcc = pCube->getDataAccessor(pRequest.release());

   RasterElement *dRas=RasterUtilities::createRasterElement(pCube->getName()+"Noise_reduction_Median_filter",
      pDesc->getRowCount(), pDesc->getColumnCount(),3, pDesc->getDataType(),BSQ);

   pProgress->updateProgress(msg, 50, NORMAL);
   
   
   copyImage4(pCube,dRas,0,pProgress);
   pProgress->updateProgress(msg+"RED complete", 60, NORMAL);
   
   copyImage4(pCube,dRas,1,pProgress);
   pProgress->updateProgress(msg+"GREEN complete", 70, NORMAL);
   
   copyImage4(pCube,dRas,2,pProgress);
   pProgress->updateProgress(msg+"BLUE complete", 80, NORMAL);



   //new model resource
   RasterDataDescriptor* rDesc = dynamic_cast<RasterDataDescriptor*>(dRas->getDataDescriptor());
   rDesc->setDisplayMode(RGB_MODE);										//enable color mode
   rDesc->setDisplayBand(RED,rDesc->getActiveBand(0));
   rDesc->setDisplayBand(GREEN,rDesc->getActiveBand(1));
   rDesc->setDisplayBand(BLUE,rDesc->getActiveBand(2));

   ModelResource<RasterElement> pResultCube(dRas);
   
   if (pResultCube.get() == NULL)
   {
      std::string msg = "A raster cube could not be created.";
      pStep->finalize(Message::Failure, msg);
      if (pProgress != NULL) 
      {
         pProgress->updateProgress(msg, 0, ERRORS);
      }
      return false;
   }
   
   pProgress->updateProgress("Final", 100, NORMAL);
   
   pProgress->updateProgress(msg,100,NORMAL);

   if (!isBatch())
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

   if (pProgress != NULL)
   {
      pProgress->updateProgress("adaptive_median is compete.", 100, NORMAL);
   }

   pOutArgList->setPlugInArgValue("adaptive_median_Result", pResultCube.release());	//saving data

   pStep->finalize();
   return true;
}
