/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#include "AppVerify.h"
#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "DataRequest.h"
#include "DesktopServices.h"
#include "MessageLogResource.h"
#include "ObjectResource.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "PlugInResource.h"
#include "Progress.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterUtilities.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "StringUtilities.h"
#include "switchOnEncoding.h"
#include "autowhite.h"
#include <limits>

REGISTER_PLUGIN_BASIC(OpticksTutorial, autowhite);






namespace
{
   template<typename T>
   void copywhite(T* pData, DataAccessor pSrcAcc, int row, int col, int rowSize, int colSize,double correct)
   {
      
	  pSrcAcc->toPixel(row, col);
      VERIFYNRV(pSrcAcc.isValid());
      T midVal = *reinterpret_cast<T*>(pSrcAcc->getColumn());
	  
	  midVal=midVal*correct;
	  
	  *pData=static_cast<T>(midVal);
   }
};



autowhite::autowhite()
{
   setDescriptorId("{cd8af530-89ca-11e0-9d78-0800200c9a66}");
   setName("autowhite");
   setDescription("Auto white balance on an image");
   setCreator("Pratik Anand");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setProductionStatus(false);
   setType("Sample");
   setSubtype("Statistics");
   setMenuLocation("[Tutorial]/autowhite");
   setAbortSupported(true);
}

autowhite::~autowhite()
{
}

bool autowhite::getInputSpecification(PlugInArgList* &pInArgList)
{
   VERIFY(pInArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pInArgList->addArg<Progress>(Executable::ProgressArg(), NULL, "Progress reporter");
   pInArgList->addArg<RasterElement>(Executable::DataElementArg(), "Generate statistics for this raster element");
   return true;
}

bool autowhite::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   VERIFY(pOutArgList = Service<PlugInManagerServices>()->getPlugInArgList());
  
   return true;
}



double testfunc(RasterElement *pRaster,int i)
{
	RasterDataDescriptor* pDesc = dynamic_cast<RasterDataDescriptor*>(pRaster->getDataDescriptor());
	DimensionDescriptor thirdBand = pDesc->getActiveBand(i);
	
   FactoryResource<DataRequest> pRequest;
   pRequest->setInterleaveFormat(BSQ);
   pRequest->setBands(thirdBand, thirdBand);

   double pixelVal,max=0;

   DataAccessor thirdBandDa = pRaster->getDataAccessor(pRequest.release());

   for (unsigned int curRow = 0; curRow < pDesc->getRowCount(); ++curRow)

   {

      for (unsigned int curCol = 0; curCol < pDesc->getColumnCount(); ++curCol)

      {
		  max=std::max(max,thirdBandDa->getColumnAsDouble());
		  thirdBandDa->nextColumn();
	  }

      thirdBandDa->nextRow();

   }

   return max;
   
}

void copyImage(RasterElement *pRaster,RasterElement *dRaster,int i,Progress* pProgress,double correct)
{   

	RasterDataDescriptor* pDesc = dynamic_cast<RasterDataDescriptor*>(pRaster->getDataDescriptor());
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

   

   double pixelVal,max=0;
   void *ptr=NULL,*ptr2=NULL;

   
   

   

   for (unsigned int curRow = 0; curRow < pDesc->getRowCount(); ++curRow)

   {

      for (unsigned int curCol = 0; curCol < pDesc->getColumnCount(); ++curCol)

	  {	  
		  
		  switchOnEncoding(pDesc->getDataType(), copywhite, pDestAcc->getColumn(), thirdBandDa, curRow, curCol,
            pDesc->getRowCount(), pDesc->getColumnCount(),correct);
		 
		 

		  
		  pDestAcc->nextColumn();
	  }

     
	  pDestAcc->nextRow();

   }



   
   
}









bool autowhite::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   
	
	StepResource pStep("autowhite", "pratik", "27170298-10CE-4E6C-AD7A-97E8058C29FF");
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
   RasterDataDescriptor* pDesc = static_cast<RasterDataDescriptor*>(pCube->getDataDescriptor());
   VERIFY(pDesc != NULL);

   
   double max0=testfunc(pCube,0);
   double max1=testfunc(pCube,1);
   double max2=testfunc(pCube,2);

   std::string msg="Old values: RED Band 0:"+StringUtilities::toDisplayString(max0)+"\n"
				  +"GREEN Band 1:"+StringUtilities::toDisplayString(max1)+"\n"
				  +"BLUE Band 2:"+StringUtilities::toDisplayString(max2)+"\n";
   
   pProgress->updateProgress(msg,100,NORMAL);							//initial R,G and B values

  
   //auto white correction
   double correct[3];
   if(max0>255||max1>255||max2>255)    //if image is 16-bit
   {
	 correct[0]= (65535/max0);
    correct[1]= (65535/max1);
    correct[2]= (65535/max2);

   }
   else
   {								   //if image is 8-bit
	correct[0]= (255/max0);
    correct[1]= (255/max1);
    correct[2]= (255/max2);
   }

   RasterElement *dRas=RasterUtilities::createRasterElement("RGB"+pCube->getName(),
      pDesc->getRowCount(), pDesc->getColumnCount(),3, pDesc->getDataType(),BSQ);

   
   


   //request
   pProgress->updateProgress(msg, 50, NORMAL);
   
   copyImage(pCube,dRas,0,pProgress,correct[0]);
   pProgress->updateProgress(msg+"RED complete", 60, NORMAL);
   
   copyImage(pCube,dRas,1,pProgress,correct[1]);
   pProgress->updateProgress(msg+"GREEN complete", 70, NORMAL);
   
   copyImage(pCube,dRas,2,pProgress,correct[2]);
   pProgress->updateProgress(msg+"BLUE complete", 80, NORMAL);


   //new model resource
   RasterDataDescriptor* rDesc = dynamic_cast<RasterDataDescriptor*>(dRas->getDataDescriptor());
   rDesc->setDisplayMode(RGB_MODE);							//enable color mode
   rDesc->setDisplayBand(RED,pDesc->getActiveBand(0));
   rDesc->setDisplayBand(GREEN,pDesc->getActiveBand(1));
   rDesc->setDisplayBand(BLUE,pDesc->getActiveBand(2));
   ModelResource<RasterElement> pResultCube(dRas);
   
   
   pProgress->updateProgress("Final", 100, NORMAL);
   
   //new statistics
   
   max0=testfunc(dRas,0);
   max1=testfunc(dRas,1);
   max2=testfunc(dRas,2);
   msg="New values:  RED Band 0:"+StringUtilities::toDisplayString(max0)+"\n"
				  +"GREEN Band 1:"+StringUtilities::toDisplayString(max1)+"\n"
				  +"BLUE Band 2:"+StringUtilities::toDisplayString(max2);
   pProgress->updateProgress(msg,100,NORMAL);						//final R,G and B values

   //create window
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
   

   pOutArgList->setPlugInArgValue("autowhite_Result", pResultCube.release());  //for saving data

   pStep->finalize();
   return true;
}