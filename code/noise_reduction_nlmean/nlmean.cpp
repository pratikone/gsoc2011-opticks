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

namespace
{
   template<typename T>
   void nonlocalmeans(T* pData, DataAccessor pSrcAcc, int row, int col, int rowSize, int colSize)
   {
      int prevCol = std::max(col - 1, 0);
      int prevRow = std::max(row - 1, 0);
      int nextCol = std::min(col + 1, colSize - 1);
      int nextRow = std::min(row + 1, rowSize - 1);

	  double h=0.55*SIGMA;
	  double max;
	  
      
      pSrcAcc->toPixel(prevRow, prevCol);
      VERIFYNRV(pSrcAcc.isValid());
      T upperLeftVal = *reinterpret_cast<T*>(pSrcAcc->getColumn());

      pSrcAcc->toPixel(prevRow, col);
      VERIFYNRV(pSrcAcc.isValid());
      T upVal = *reinterpret_cast<T*>(pSrcAcc->getColumn());

      pSrcAcc->toPixel(prevRow, nextCol);
      VERIFYNRV(pSrcAcc.isValid());
      T upperRightVal = *reinterpret_cast<T*>(pSrcAcc->getColumn());

      pSrcAcc->toPixel(row, prevCol);
      VERIFYNRV(pSrcAcc.isValid());
      T leftVal = *reinterpret_cast<T*>(pSrcAcc->getColumn());

      pSrcAcc->toPixel(row, nextCol);
      VERIFYNRV(pSrcAcc.isValid());
      T rightVal = *reinterpret_cast<T*>(pSrcAcc->getColumn());

      pSrcAcc->toPixel(nextRow, prevCol);
      VERIFYNRV(pSrcAcc.isValid());
      T lowerLeftVal = *reinterpret_cast<T*>(pSrcAcc->getColumn());

      pSrcAcc->toPixel(nextRow, col);
      VERIFYNRV(pSrcAcc.isValid());
      T downVal = *reinterpret_cast<T*>(pSrcAcc->getColumn());

      pSrcAcc->toPixel(nextRow, nextCol);
      VERIFYNRV(pSrcAcc.isValid());
      T lowerRightVal = *reinterpret_cast<T*>(pSrcAcc->getColumn());

	  pSrcAcc->toPixel(row, col);
      VERIFYNRV(pSrcAcc.isValid());
      T midVal = *reinterpret_cast<T*>(pSrcAcc->getColumn());

      //weight of neighbourhood pixels
	  double w00=expf(-2/(h*h));
	  double w01=expf(-1/(h*h));
	  double w02=expf(-2/(h*h));
	  
	  double w10=expf(-1/(h*h));
	  double w11=expf(0);
	  double w12=expf(-1/(h*h));

	  double w20=expf(-2/(h*h));
	  double w21=expf(-1/(h*h));
	  double w22=expf(-2/(h*h));

	  //weight of central pixel is max value out of all other pixels
	  max=std::max(w00,w01);
	  max=std::max(max,w02);
	  max=std::max(max,w10);
	  max=std::max(max,w12);
	  max=std::max(max,w20);
	  max=std::max(max,w21);
	  max=std::max(max,w22);
	  w11=max;

	  double totalWeight=w00+w01+w02+w10+w11+w12+w20+w21+w22;
	  
	  
	  double finalVal=w00*upperLeftVal+w01*upVal+w02*upperRightVal+w10*leftVal+w11*midVal+w12*rightVal+w20*lowerLeftVal+w21*downVal+w22*lowerRightVal;
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
