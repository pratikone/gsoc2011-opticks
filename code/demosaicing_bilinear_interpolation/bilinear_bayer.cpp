/*
 * The information in this file is
 * Copyright (C) 2011, Pratik Anand <pratik@pratikanand.com>
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
#include "bilinear_bayer.h"
#include <limits>

//#include<windows.h>


REGISTER_PLUGIN_BASIC(OpticksTutorial, bilinear_bayer);





namespace
{
	double getValChannel(RasterElement *pRaster, int row, int col, int i)
   {
	//find value at a given point of a given band
   
   VERIFY(pRaster!=NULL)
   RasterDataDescriptor* pDesc = dynamic_cast<RasterDataDescriptor*>(pRaster->getDataDescriptor());
   
   DimensionDescriptor Band = pDesc->getActiveBand(i);
   FactoryResource<DataRequest> pRequest;
   pRequest->setInterleaveFormat(BSQ);
   pRequest->setBands(Band, Band);
   DataAccessor BandDa = pRaster->getDataAccessor(pRequest.release());
	
   BandDa->toPixel(row,col);
   double val=BandDa->getColumnAsDouble();

   return val;
   }




   template<typename T>
   void bilinear(T* pData, DataAccessor pSrcAcc, int row, int col, int rowSize, int colSize, int i, RasterElement *pRaster)
   {
      
	  pSrcAcc->toPixel(row, col);
      VERIFYNRV(pSrcAcc.isValid());
	  double rgb[3]={0.0,0.0,0.0};
	  double midVal = *reinterpret_cast<T*>(pSrcAcc->getColumn());
	  double temp=0,flag=0;
	  double r=0,g=0,b=0;
	  
	  //RGB value at the point
	  rgb[0]=getValChannel(pRaster,row,col,0);
	  rgb[1]=getValChannel(pRaster,row,col,1);
	  rgb[2]=getValChannel(pRaster,row,col,2);

	  double max=0;
	  max=std::max(max,rgb[0]);
	  max=std::max(max,rgb[1]);
	  max=std::max(max,rgb[2]);

	  if(max==rgb[0])	//RED pixel
	  {
		  //RED
		  if(i==0)
		  {
		  }

		  //GREEN
		  else if(i==1)
		  {       //green = (green[x][y-1] + green[x-1][y] + green[x+1][y] + green[x][y+1]) / 4;
			      temp=0;
			      //green[x][y-1]
			      pSrcAcc->toPixel(std::max(row-1,0), col);
				  VERIFYNRV(pSrcAcc.isValid());
				  temp+= *reinterpret_cast<T*>(pSrcAcc->getColumn());

				  //green[x-1][y]
			      pSrcAcc->toPixel(row, std::max(col-1,0));
				  VERIFYNRV(pSrcAcc.isValid());
				  temp+= *reinterpret_cast<T*>(pSrcAcc->getColumn());

				  //green[x+1][y]
				  pSrcAcc->toPixel(row, std::min(col+1,colSize-1));
				  VERIFYNRV(pSrcAcc.isValid());
				  temp+= *reinterpret_cast<T*>(pSrcAcc->getColumn());

				  //green[x][y+1]
				  pSrcAcc->toPixel(std::min(row+1,rowSize-1), col);
				  VERIFYNRV(pSrcAcc.isValid());
				  temp+= *reinterpret_cast<T*>(pSrcAcc->getColumn());

				  temp/=4;
				  midVal=temp;
		  }

		  //BLUE
		  else if(i==2)
		  {       //blue = (blue[x-1][y-1] + blue[x+1][y-1] + blue[x-1][y+1] + blue[x+1][y+1]) / 4;
			      temp=0;
			      //blue[x-1][y-1]
			      pSrcAcc->toPixel(std::max(row-1,0), std::max(col-1,0));
				  VERIFYNRV(pSrcAcc.isValid());
				  temp+= *reinterpret_cast<T*>(pSrcAcc->getColumn());

				  //blue[x+1][y-1]
				  pSrcAcc->toPixel(std::max(row-1,0), std::min(col+1,colSize-1));
				  VERIFYNRV(pSrcAcc.isValid());
				  temp+= *reinterpret_cast<T*>(pSrcAcc->getColumn());

				  //blue[x-1][y+1]
				  pSrcAcc->toPixel(std::min(row+1,rowSize-1), std::max(col-1,0));
				  VERIFYNRV(pSrcAcc.isValid());
				  temp+= *reinterpret_cast<T*>(pSrcAcc->getColumn());

				  //blue[x+1][y+1]
				  pSrcAcc->toPixel(std::min(row+1, rowSize-1), std::min(col+1,colSize-1));
				  VERIFYNRV(pSrcAcc.isValid());
				  temp+= *reinterpret_cast<T*>(pSrcAcc->getColumn());

				  temp/=4;
				  midVal=temp;
		  }

	  }

	  else if(max==rgb[1])	//GREEN pixel
		  {

			  //GREEN
			  if(i==1)
			  {
			  }

			  //to find whether of RG or GB
			  if(col<colSize-1)
				{   
					r=getValChannel(pRaster,row,col+1,0);
			        g=getValChannel(pRaster,row,col+1,1);
					b=getValChannel(pRaster,row,col+1,2);
					temp=0;
					temp=std::max(temp,r);
					temp=std::max(temp,g);
					temp=std::max(temp,b);
			        if(temp==r)  //rg
					 flag=1;
					else		 //gb
					 flag=2;
	             
			    }
			  else
			  {
				    r=getValChannel(pRaster,row,col-1,0);
			        g=getValChannel(pRaster,row,col-1,1);
					b=getValChannel(pRaster,row,col-1,2);
					temp=0;
					temp=std::max(temp,r);
					temp=std::max(temp,g);
					temp=std::max(temp,b);
			        if(temp==r)  //rg
					 flag=1;
					else		 //gb
					 flag=2;

			  }

			  //RED
			  if(i==0)
			  {   temp=0;
				  //red = (red[x-1][y] + red[x+1][y])  / 2;
				  if(flag==1)  //rg
				  {      
						//red[x-1][y]
						pSrcAcc->toPixel(row, std::max(col-1,0));
						VERIFYNRV(pSrcAcc.isValid());
						temp+= *reinterpret_cast<T*>(pSrcAcc->getColumn());

						//red[x+1][y]
						pSrcAcc->toPixel(row, std::min(col+1,colSize-1));
						VERIFYNRV(pSrcAcc.isValid());
						temp+= *reinterpret_cast<T*>(pSrcAcc->getColumn());
						temp/=2;
						midVal=temp;

				  }

				  else	//gb
				  {
					  //red = (red[x][y-1] + red[x][y+1]) / 2;
					  
					  //red[x][y-1]
					  pSrcAcc->toPixel(std::max(row-1,0), col);
					  VERIFYNRV(pSrcAcc.isValid());
					  temp+= *reinterpret_cast<T*>(pSrcAcc->getColumn());

					  //red[x][y+1]
					  pSrcAcc->toPixel(std::min(row+1,rowSize-1), col);
					  VERIFYNRV(pSrcAcc.isValid());
					  temp+= *reinterpret_cast<T*>(pSrcAcc->getColumn());
					  temp/=2;
					  midVal=temp;
				  }
			  }

			  //BLUE
			  if(i==2)
			  {   temp=0;
				  //blue = (blue[x][y-1] + blue[x][y+1]) / 2;
				  if(flag==1)  //rg
				  {      
						//blue[x][y-1]
						pSrcAcc->toPixel(std::max(row-1,0),col);
						VERIFYNRV(pSrcAcc.isValid());
						temp+= *reinterpret_cast<T*>(pSrcAcc->getColumn());

						//blue[x][y+1]
						pSrcAcc->toPixel(std::min(row+1,rowSize-1),col);
						VERIFYNRV(pSrcAcc.isValid());
						temp+= *reinterpret_cast<T*>(pSrcAcc->getColumn());
						temp/=2;
						midVal=temp;

				  }

				  else	//gb
				  {
					  //blue = (blue[x-1][y] + blue[x+1][y]) / 2;
					  
					  //blue[x-1][y]
					  pSrcAcc->toPixel(row, std::max(col-1,0));
				      VERIFYNRV(pSrcAcc.isValid());
					  temp+= *reinterpret_cast<T*>(pSrcAcc->getColumn());

					  //blue[x+1][y]
					  pSrcAcc->toPixel(row, std::min(col+1,colSize-1));
					  VERIFYNRV(pSrcAcc.isValid());
					  temp+= *reinterpret_cast<T*>(pSrcAcc->getColumn());
					  temp/=2;
					  midVal=temp;
					  
				  }
			  }


		  }
		else if(max==rgb[2])	//BLUE pixel
		{    
		  temp=0;
		  //BLUE
		  if(i==2)
		  {
		  }

		  //GREEN
		  else if(i==1)
		  {       //green = (green[x][y-1] + green[x-1][y] + green[x+1][y] + green[x][y+1]) / 4;
			      temp=0;
			      //green[x][y-1]
			      pSrcAcc->toPixel(std::max(row-1,0), col);
				  VERIFYNRV(pSrcAcc.isValid());
				  temp+= *reinterpret_cast<T*>(pSrcAcc->getColumn());

				  //green[x-1][y]
			      pSrcAcc->toPixel(row, std::max(col-1,0));
				  VERIFYNRV(pSrcAcc.isValid());
				  temp+= *reinterpret_cast<T*>(pSrcAcc->getColumn());

				  //green[x+1][y]
				  pSrcAcc->toPixel(row, std::min(col+1,colSize-1));
				  VERIFYNRV(pSrcAcc.isValid());
				  temp+= *reinterpret_cast<T*>(pSrcAcc->getColumn());

				  //green[x][y+1]
				  pSrcAcc->toPixel(std::min(row+1,rowSize-1), col);
				  VERIFYNRV(pSrcAcc.isValid());
				  temp+= *reinterpret_cast<T*>(pSrcAcc->getColumn());

				  temp/=4;
				  midVal=temp;
		  }

		  //RED
		  else if(i==0)
		  {       //red = (red[x-1][y-1] + red[x+1][y-1] + red[x-1][y+1] + red[x+1][y+1]) / 4;
			      temp=0;
			      //red[x-1][y-1]
			      pSrcAcc->toPixel(std::max(row-1,0), std::max(col-1,0));
				  VERIFYNRV(pSrcAcc.isValid());
				  temp+= *reinterpret_cast<T*>(pSrcAcc->getColumn());

				  //red[x+1][y-1]
				  pSrcAcc->toPixel(std::max(row-1,0), std::min(col+1,colSize-1));
				  VERIFYNRV(pSrcAcc.isValid());
				  temp+= *reinterpret_cast<T*>(pSrcAcc->getColumn());

				  //red[x-1][y+1]
				  pSrcAcc->toPixel(std::min(row+1,rowSize-1), std::max(col-1,0));
				  VERIFYNRV(pSrcAcc.isValid());
				  temp+= *reinterpret_cast<T*>(pSrcAcc->getColumn());

				  //red[x+1][y+1]
				  pSrcAcc->toPixel(std::min(row+1, rowSize-1), std::min(col+1,colSize-1));
				  VERIFYNRV(pSrcAcc.isValid());
				  temp+= *reinterpret_cast<T*>(pSrcAcc->getColumn());

				  temp/=4;
				  midVal=temp;
	    }

	}

	  *pData=static_cast<T>(midVal);
   
	  
 }

};


bilinear_bayer::bilinear_bayer()
{
   setDescriptorId("{77A5FDA5-1C6A-420F-AAEA-98602A38B382}");
   setName("bilinear_bayer");
   setDescription("Auto white balance on an image");
   setCreator("Pratik Anand");
   setVersion("0.1");
   setCopyright("Copyright (C) 2011, Pratik Anand <pratik@pratikanand.com>");
   setProductionStatus(false);
   setType("Algorithm");
   setSubtype("White Balance");
   setMenuLocation("[Photography]/bilinear_bayer");
   setAbortSupported(false);
}

bilinear_bayer::~bilinear_bayer()
{
}

bool bilinear_bayer::getInputSpecification(PlugInArgList* &pInArgList)
{
   VERIFY(pInArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pInArgList->addArg<Progress>(Executable::ProgressArg(), NULL, "Progress reporter");
   pInArgList->addArg<RasterElement>(Executable::DataElementArg(), "Demosaicing  for an image");
   return true;
}

bool bilinear_bayer::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   VERIFY(pOutArgList = Service<PlugInManagerServices>()->getPlugInArgList());
  
   return true;
}




bool bilinear_bayer::copyImage(RasterElement *pRaster,RasterElement *dRaster,int i,Progress* pProgress)
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
		
		switchOnEncoding(pDesc->getDataType(), bilinear, pDestAcc->getColumn(), thirdBandDa, curRow, curCol,
        pDesc->getRowCount(), pDesc->getColumnCount(), i, pRaster);
		pDestAcc->nextColumn();
	  }
	        
	  pDestAcc->nextRow();

   }

   return true;
}






bool bilinear_bayer::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   
	
   StepResource pStep("bilinear_bayer", "pratik", "27170298-10CE-4E6C-AD7A-97E8058C29FF");
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
   RasterDataDescriptor* pDesc = static_cast<RasterDataDescriptor*>(pCube->getDataDescriptor());
   VERIFY(pDesc != NULL);

   
   std::string msg="De-bayerize by bilinear interpolation \n";
   pProgress->updateProgress(msg,20,NORMAL);							//show initial R,G and B values

   RasterElement *dRas=RasterUtilities::createRasterElement(pCube->getName()+"RGB",
      pDesc->getRowCount(), pDesc->getColumnCount(),3, pDesc->getDataType(),BSQ);

   //request
   
   pProgress->updateProgress(msg, 50, NORMAL);
   
   copyImage(pCube,dRas,0,pProgress);
   pProgress->updateProgress(msg+"RED complete", 60, NORMAL);
   
   copyImage(pCube,dRas,1,pProgress);
   pProgress->updateProgress(msg+"GREEN complete", 70, NORMAL);
   
   copyImage(pCube,dRas,2,pProgress);
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
   pOutArgList->setPlugInArgValue("bilinear_bayer_Result", pResultCube.release());  //for saving data

   pStep->finalize();
   return true;
}
