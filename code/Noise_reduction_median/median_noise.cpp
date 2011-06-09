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
#include "median_noise.h"
#include <limits>

REGISTER_PLUGIN_BASIC(OpticksTutorial, median_noise);

namespace
{
   template<typename T>
   void medianfilter(T* pData, DataAccessor pSrcAcc, int row, int col, int rowSize, int colSize)
   {
      int prevCol = std::max(col - 1, 0);
      int prevRow = std::max(row - 1, 0);
      int nextCol = std::min(col + 1, colSize - 1);
      int nextRow = std::min(row + 1, rowSize - 1);
      
	  
	  
	  
	  double sampledata[9];
	  double val,median=0;

      
	  pSrcAcc->toPixel(prevRow, prevCol);
      VERIFYNRV(pSrcAcc.isValid());
      sampledata[0]= *reinterpret_cast<T*>(pSrcAcc->getColumn());

      pSrcAcc->toPixel(prevRow, col);
      VERIFYNRV(pSrcAcc.isValid());
      sampledata[1]= *reinterpret_cast<T*>(pSrcAcc->getColumn());

      pSrcAcc->toPixel(prevRow, nextCol);
      VERIFYNRV(pSrcAcc.isValid());
      sampledata[2] = *reinterpret_cast<T*>(pSrcAcc->getColumn());

      pSrcAcc->toPixel(row, prevCol);
      VERIFYNRV(pSrcAcc.isValid());
      sampledata[3] = *reinterpret_cast<T*>(pSrcAcc->getColumn());

	  pSrcAcc->toPixel(row, col);
      VERIFYNRV(pSrcAcc.isValid());
      sampledata[4]= *reinterpret_cast<T*>(pSrcAcc->getColumn());

      pSrcAcc->toPixel(row, nextCol);
      VERIFYNRV(pSrcAcc.isValid());
      sampledata[5]= *reinterpret_cast<T*>(pSrcAcc->getColumn());

      pSrcAcc->toPixel(nextRow, prevCol);
      VERIFYNRV(pSrcAcc.isValid());
      sampledata[6]= *reinterpret_cast<T*>(pSrcAcc->getColumn());

      pSrcAcc->toPixel(nextRow, col);
      VERIFYNRV(pSrcAcc.isValid());
      sampledata[7]= *reinterpret_cast<T*>(pSrcAcc->getColumn());

      pSrcAcc->toPixel(nextRow, nextCol);
      VERIFYNRV(pSrcAcc.isValid());
      sampledata[8] = *reinterpret_cast<T*>(pSrcAcc->getColumn());

	  
    
	  //insertion sort
	  
	  int i=0,j=0;
	  for(i=1;i<9;i++)
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
	  median=sampledata[4];
      *pData = static_cast<T>(median); 
		
	  

   }
};


bool median_noise::copyImage3(RasterElement *pRaster,RasterElement *dRaster,int i,Progress* pProgress)
{   
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
		
		switchOnEncoding(pDesc->getDataType(), medianfilter, pDestAcc->getColumn(), thirdBandDa, curRow, curCol,
        pDesc->getRowCount(), pDesc->getColumnCount());
		pDestAcc->nextColumn();
	  }
	        
	  pDestAcc->nextRow();

   }

   return true;
}


median_noise::median_noise()
{
   setDescriptorId("{8210FB96-1EE7-495A-A72A-2605BE41629C}");
   setName("median_noise");
   setDescription("Perform noise reduction on an image using median filter"
      "of the provided raster element.");
   setCreator("Pratik Anand");
   setVersion("0.1");
   setCopyright("Copyright (C) 2011, Pratik Anand <pratik@pratikanand.com>");
   setProductionStatus(false);
   setType("Algorithm");
   setSubtype("Noise reduction");
   setMenuLocation("[Photography]/Median_noise");
   setAbortSupported(false);
}

median_noise::~median_noise()
{
}

bool median_noise::getInputSpecification(PlugInArgList*& pInArgList)
{
   VERIFY(pInArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pInArgList->addArg<Progress>(Executable::ProgressArg(), NULL, "Progress reporter");
   pInArgList->addArg<RasterElement>(Executable::DataElementArg(), "Perform edge detection on this data element");
   return true;
}

bool median_noise::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   VERIFY(pOutArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pOutArgList->addArg<RasterElement>("Result", NULL);
   return true;
}

bool median_noise::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("median", "noise", "5EA0CC75-9E0B-4c3d-BA23-6DB7157BBD55");
   if (pInArgList == NULL || pOutArgList == NULL)
   {
      return false;
   }

   std::string msg="Noise reduction by Median filter ";
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
   
   
   copyImage3(pCube,dRas,0,pProgress);
   pProgress->updateProgress(msg+"RED complete", 60, NORMAL);
   
   copyImage3(pCube,dRas,1,pProgress);
   pProgress->updateProgress(msg+"GREEN complete", 70, NORMAL);
   
   copyImage3(pCube,dRas,2,pProgress);
   pProgress->updateProgress(msg+"BLUE complete", 80, NORMAL);



   //new model resource
   RasterDataDescriptor* rDesc = dynamic_cast<RasterDataDescriptor*>(dRas->getDataDescriptor());
   rDesc->setDisplayMode(RGB_MODE);										//enable color mode
   rDesc->setDisplayBand(RED,pDesc->getActiveBand(0));
   rDesc->setDisplayBand(GREEN,pDesc->getActiveBand(1));
   rDesc->setDisplayBand(BLUE,pDesc->getActiveBand(2));

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
      pProgress->updateProgress("median_noise is compete.", 100, NORMAL);
   }

   pOutArgList->setPlugInArgValue("median_noise_Result", pResultCube.release());	//saving data

   pStep->finalize();
   return true;
}