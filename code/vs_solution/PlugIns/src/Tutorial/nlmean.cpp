/*
 * The information in this file is
 * Copyright (C) 2011, Pratik Anand <pratik@pratikanand.com>
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
#include "nlmean_ui.h"
#include <limits>


REGISTER_PLUGIN_BASIC(PhotographyProcessingTools, nlmean);




namespace
{
   template<typename T>
   void nonlocalmeans(T* pData, int row, int col,const int rowSize,const int colSize,double** image,const double SIGMA,const int size,const int csize)
   {
	  
	  //based on noise
	  double h=0.55*SIGMA;
	  double max;
	  double h2=h*h;
	  double totalWeight=0;
	  double finalVal=0;
	  
	  //allocate memory
	  double **weights=new double*[size];
	  for(int i=0;i<size;i++)
		  weights[i]=new double[size];
	  

	  double p=0,q=0;
	  double res=0;
	  
      T **data=new T*[size];
	  for(int i=0;i<size;i++)
		  data[i]=new T[size];
	  

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
							 //pSrcAcc->toPixel(std::max(prow2,0), std::max(pcol2,0));
							 //VERIFYNRV(pSrcAcc.isValid());
							 //p= *reinterpret_cast<T*>(pSrcAcc->getColumn());
							   p=image[std::max(prow2,0)][std::max(pcol2,0)];

							 //q
							 //pSrcAcc->toPixel(std::max(qrow2,0), std::max(qcol2,0));
							 //VERIFYNRV(pSrcAcc.isValid());
							 //q= *reinterpret_cast<T*>(pSrcAcc->getColumn());
							   q=image[std::max(qrow2,0)][std::max(qcol2,0)];

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
      					 
						 //pSrcAcc->toPixel(std::min(rtemp,rowSize-1), std::min(ctemp,colSize-1));
						 //VERIFYNRV(pSrcAcc.isValid());
						 //p= *reinterpret_cast<T*>(pSrcAcc->getColumn());
						   p=image[std::min(rtemp,rowSize-1)][std::min(ctemp,colSize-1)];

						 //q
						 rtemp=std::max(qrow2,0);
						 ctemp=std::max(qcol2,0);

						 //pSrcAcc->toPixel(std::min(rtemp,rowSize-1), std::min(ctemp,colSize-1));
						 //VERIFYNRV(pSrcAcc.isValid());
						 //q= *reinterpret_cast<T*>(pSrcAcc->getColumn());
						   q=image[std::min(rtemp,rowSize-1)][std::min(ctemp,colSize-1)];
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

					//pSrcAcc->toPixel(std::max(row2,0), std::max(col2,0));
					//VERIFYNRV(pSrcAcc.isValid());
					//data[r][c]= *reinterpret_cast<T*>(pSrcAcc->getColumn());
					  data[r][c]=image[std::max(row2,0)][std::max(col2,0)];
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
							 //pSrcAcc->toPixel(std::max(rtemp,0), std::max(ctemp,0));
							 //VERIFYNRV(pSrcAcc.isValid());
							 //p= *reinterpret_cast<T*>(pSrcAcc->getColumn());
							   p=image[std::max(rtemp,0)][std::max(ctemp,0)];

							 //q
							 rtemp=std::min(qrow2,rowSize);
							 ctemp=std::min(qcol2,colSize);
							 //pSrcAcc->toPixel(std::max(rtemp,0), std::max(ctemp,0));
							 //VERIFYNRV(pSrcAcc.isValid());
							 //q= *reinterpret_cast<T*>(pSrcAcc->getColumn());
							   q=image[std::max(rtemp,0)][std::max(ctemp,0)];
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
						 //pSrcAcc->toPixel(std::min(prow2,rowSize-1), std::min(pcol2,colSize-1));
						 //VERIFYNRV(pSrcAcc.isValid());
						 //p= *reinterpret_cast<T*>(pSrcAcc->getColumn());
						   p=image[std::min(prow2,rowSize-1)][std::min(pcol2,colSize-1)];

						 //q
						 //pSrcAcc->toPixel(std::min(qrow2,rowSize-1), std::min(qcol2,colSize-1));
						 //VERIFYNRV(pSrcAcc.isValid());
						 //q= *reinterpret_cast<T*>(pSrcAcc->getColumn());
						   q=image[std::min(qrow2,rowSize-1)][std::min(qcol2,colSize-1)];

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

					//pSrcAcc->toPixel(std::min(row2,rowSize-1), std::min(col2,colSize-1));
					//VERIFYNRV(pSrcAcc.isValid());
					//data[r][c]= *reinterpret_cast<T*>(pSrcAcc->getColumn());
					  data[r][c]=image[std::min(row2,rowSize-1)][std::min(col2,colSize-1)];
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



	  finalVal=finalVal/totalWeight;

   
	  *pData=static_cast<T>(finalVal);


	  //deallocate memory
	  for(int i=0;i<size;i++)
		  delete[] weights[i];
	  delete[] weights;

	  for(int i=0;i<size;i++)
		  delete[] data[i];
	  delete[] data;
   
   }
};


bool nlmean::standard_deviation(RasterElement *pRaster, RasterElement *dRaster, int i,const double SIGMA,const int size,const int csize, Progress *pProgress)
{
	VERIFY(pRaster != NULL);
	RasterDataDescriptor* pDesc = dynamic_cast<RasterDataDescriptor*>(pRaster->getDataDescriptor());
	VERIFY(dRaster != NULL);
	RasterDataDescriptor* rDesc = dynamic_cast<RasterDataDescriptor*>(dRaster->getDataDescriptor());

	std::string msg=" ";
	const unsigned int rowSize=pDesc->getRowCount();
	const unsigned int colSize=pDesc->getColumnCount();
	//image array
	double **image=new double*[rowSize];
	for(int indx=0;indx<rowSize;++indx)
		image[indx]=new double[colSize];

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
   
   //get image pixel data of band i in 2D array image
   for (unsigned int curRow = 0; curRow < pDesc->getRowCount(); ++curRow)
   {
	   for (unsigned int curCol = 0; curCol < pDesc->getColumnCount(); ++curCol)
	  {	  
		
		//switchOnEncoding(pDesc->getDataType(), nonlocalmeans, pDestAcc->getColumn(), thirdBandDa, curRow, curCol,
         //pDesc->getRowCount(), pDesc->getColumnCount());
		
		//msg="Working on "+StringUtilities::toDisplayString(curRow)+" , "+StringUtilities::toDisplayString(curCol);
		//pProgress->updateProgress(msg,40,NORMAL);
		  
		  image[curRow][curCol]=thirdBandDa->getColumnAsDouble();
		  thirdBandDa->nextColumn();
	  }
	 
	   thirdBandDa->nextRow();

   }


   //calculate values
   for (unsigned int curRow = 0; curRow < pDesc->getRowCount(); ++curRow)
   {
	   for (unsigned int curCol = 0; curCol < pDesc->getColumnCount(); ++curCol)
	  {	  
		
		  switchOnEncoding(pDesc->getDataType(), nonlocalmeans, pDestAcc->getColumn(), curRow, curCol,
           pDesc->getRowCount(), pDesc->getColumnCount(), image,SIGMA,size,csize);
		  
		  pDestAcc->nextColumn();
	  }
	 
	   pDestAcc->nextRow();

   }
  
	//deletion
    for(int indx2=0;indx2<rowSize;++indx2)
		delete[] image[indx2];
	delete[] image;

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
   setProductionStatus(true);
   setType("Algorithm");
   setSubtype("Noise reduction");
   setMenuLocation("[Photography]/nonlocal mean");
   setAbortSupported(false);
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


   double SIGMA=15.0;  //standard deviation of noise
   int size=17;
   int csize=3;
   
   //Dialog
   Service<DesktopServices> pDesktop;
   
   nlmean_ui dlg(pDesktop->getMainWidget());
   int stat=dlg.exec();
   if (stat == QDialog::Accepted)
   {
   
	 SIGMA = dlg.getSigmaValue();
	 size = dlg.getResearchValue();
	 csize = dlg.getCompareValue();

   }
   
   pProgress->updateProgress("Starting calculations", 10, NORMAL);
   std::string msg="Peforming non local mean algorithm";
   //nlmean::standard_deviation(pCube,pProgress);

   RasterDataDescriptor* pDesc = static_cast<RasterDataDescriptor*>(pCube->getDataDescriptor());
   VERIFY(pDesc != NULL);
   RasterElement *dRas=RasterUtilities::createRasterElement(pCube->getName()+"RGB",
      pDesc->getRowCount(), pDesc->getColumnCount(),3, pDesc->getDataType(),BSQ);
   
   pProgress->updateProgress("Starting calculations", 10, NORMAL);
   
   
   standard_deviation(pCube,dRas, 0,SIGMA,size,csize, pProgress);
   pProgress->updateProgress(msg+"RED complete", 60, NORMAL);
   standard_deviation(pCube,dRas, 1,SIGMA,size,csize, pProgress);
   pProgress->updateProgress(msg+"GREEN complete", 70, NORMAL);
   standard_deviation(pCube,dRas, 2,SIGMA,size,csize, pProgress);
   pProgress->updateProgress(msg+"BLUE complete", 80, NORMAL);

  
   //new model resource
   RasterDataDescriptor* rDesc = dynamic_cast<RasterDataDescriptor*>(dRas->getDataDescriptor());
   rDesc->setDisplayMode(RGB_MODE);										//enable color mode
   rDesc->setDisplayBand(RED,rDesc->getActiveBand(0));
   rDesc->setDisplayBand(GREEN,rDesc->getActiveBand(1));
   rDesc->setDisplayBand(BLUE,rDesc->getActiveBand(2));
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
