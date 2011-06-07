/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QBitmap>
#include <QtGui/QMessageBox>
#include <QtGui/QMouseEvent>

#include "AppConfig.h"
#include "AppVerify.h"
#include "DesktopServices.h"
#include "DimensionDescriptor.h"
#include "Layer.h"
#include "LayerList.h"
#include "MouseMode.h"
#include "neutralwhite.h"
#include "PlugInRegistration.h"
#include "RasterElement.h"
#include "RasterDataDescriptor.h"
#include "SessionManager.h"
#include "Slot.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "ToolBar.h"
#include "xmlreader.h"
#include "xmlwriter.h"
#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "DataRequest.h"
#include "MessageLogResource.h"
#include "ObjectResource.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "Progress.h"
#include "RasterUtilities.h"
#include "switchOnEncoding.h"
#include <limits>

using namespace std;
XERCES_CPP_NAMESPACE_USE

static const char* const MouseModeIcon2[] =
{
   "16 16 2 1",
   "# c #000000",
   ". c #ffffff",
   "................",
   "................",
   "........#.......",
   "........#.......",
   "........#.......",
   ".#....#.#.#####.",
   ".##...#.#.#...#.",
   ".#.#..#.#.#..#..",
   ".#.#..#.#.#.#...",
   ".#..#.#.#.#.#...",
   ".#...#..#.#..#..",
   "........#.......",
   "................",
   "................",
   "................",
   "................"
};

REGISTER_PLUGIN_BASIC(OpticksTutorial, neutralwhite);



namespace
{
   template<typename T>
   void copywhite2(T* pData, DataAccessor pSrcAcc, int row, int col, int rowSize, int colSize,double correct,double max)
   {
      
	  pSrcAcc->toPixel(row, col);
      VERIFYNRV(pSrcAcc.isValid());
      T midVal = *reinterpret_cast<T*>(pSrcAcc->getColumn());
	  
	  midVal=midVal*correct;
	  //overflow check
	  if((pSrcAcc->getColumnAsDouble()*correct)>=max)			//comparison with midVal not possible
		  {
			  midVal=max;
		  }
	  
	  
	  *pData=static_cast<T>(midVal);
   }
};

void getRGB(RasterElement *pRaster,RasterDataDescriptor* pDesc,int x,int y,double correct[])
{

	//RED
   DimensionDescriptor firstBand = pDesc->getActiveBand(0);
   FactoryResource<DataRequest> pRequest;
   pRequest->setInterleaveFormat(BSQ);
   pRequest->setBands(firstBand, firstBand);
   DataAccessor firstBandDa = pRaster->getDataAccessor(pRequest.release());

   //GREEN
   DimensionDescriptor secondBand = pDesc->getActiveBand(1);
   FactoryResource<DataRequest> qRequest;
   qRequest->setInterleaveFormat(BSQ);
   qRequest->setBands(secondBand, secondBand);
   DataAccessor secondBandDa = pRaster->getDataAccessor(qRequest.release());

   //BLUE
   DimensionDescriptor thirdBand = pDesc->getActiveBand(2);
   FactoryResource<DataRequest> rRequest;
   rRequest->setInterleaveFormat(BSQ);
   rRequest->setBands(thirdBand, thirdBand);
   DataAccessor thirdBandDa = pRaster->getDataAccessor(rRequest.release());

   
   firstBandDa->toPixel(y,x);
   VERIFYNRV(firstBandDa.isValid());
   correct[0]=firstBandDa->getColumnAsDouble();		

   secondBandDa->toPixel(y,x);
   VERIFYNRV(secondBandDa.isValid());
   correct[1]=secondBandDa->getColumnAsDouble();

   thirdBandDa->toPixel(y,x);
   VERIFYNRV(thirdBandDa.isValid());
   correct[2]=thirdBandDa->getColumnAsDouble();



   //display msg

   Service<DesktopServices> pDesktop;

  SpatialDataView* pSpatialDataView =
               dynamic_cast<SpatialDataView*>(pDesktop->getCurrentWorkspaceWindowView());
  //if (pSpatialDataView != NULL)
       //{
        QWidget* pViewWidget2 = pSpatialDataView->getWidget();
       //}

   QMessageBox::information(pViewWidget2, "Display RGB values",
                                             "RGB values of the selected pixel are (" +
											 QString::number(correct[0]) + ", " +
                                             QString::number(correct[1]) + ", " +
											 QString::number(correct[2]) + ")");



}

bool copyImage2(RasterElement *pRaster,RasterElement *dRaster,int i,double max,double correct)
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
		
		switchOnEncoding(pDesc->getDataType(), copywhite2, pDestAcc->getColumn(), thirdBandDa, curRow, curCol,
			pDesc->getRowCount(), pDesc->getColumnCount(),correct,max);
		
		pDestAcc->nextColumn();
	  }
	        
	  pDestAcc->nextRow();

   }

   return true;
}

neutralwhite::neutralwhite() :
   mpMouseMode(NULL),
   mpMouseModeAction3(NULL)
{
   AlgorithmShell::setName("Neutral Reference Selection for white correction");
   setCreator("Pratik Anand");
   setVersion("0.1");
   setCopyright("Copyright (C) 2011, Pratik Anand <pratik@pratikanand.com>");
   setDescription("White balance correction of an image based in selected neutral reference");
   setDescriptorId("{e538f9a0-8e27-11e0-91e4-0800200c9a66}");
   //setMenuLocation("[ABC]/neutralwhite");
   

   setProductionStatus(false);
   allowMultipleInstances(false);
   executeOnStartup(true);
   destroyAfterExecute(false);
   setAbortSupported(false);
   setWizardSupported(false);

   //initialise the value
   correct[0]=correct[1]=correct[2]=0.0;  
   pRaster=NULL;
   dRaster=NULL;
   pDescriptor=NULL;
   rDesc=NULL;
   pView=NULL;
   pWindow=NULL;
}

neutralwhite::~neutralwhite()
{
   Service<DesktopServices> pDesktop;

   // Remove the toolbar button and delete the mouse mode action
   
   ToolBar* pToolBar = dynamic_cast<ToolBar*>(pDesktop->getWindow("Photography", TOOLBAR));
   if (pToolBar != NULL)
   {
      if (mpMouseModeAction3 != NULL)
      {
         pToolBar->removeItem(mpMouseModeAction3);
         delete mpMouseModeAction3;
      }
	  
   }
     

   // Detach from desktop services
   pDesktop->detach(SIGNAL_NAME(DesktopServices, WindowAdded), Slot(this, &neutralwhite::windowAdded));
   pDesktop->detach(SIGNAL_NAME(DesktopServices, WindowActivated), Slot(this, &neutralwhite::windowActivated));
   pDesktop->detach(SIGNAL_NAME(DesktopServices, WindowRemoved), Slot(this, &neutralwhite::windowRemoved));

   // Remove the mouse mode from the views
   vector<Window*> windows;
   pDesktop->getWindows(SPATIAL_DATA_WINDOW, windows);

   for (vector<Window*>::iterator iter = windows.begin(); iter != windows.end(); ++iter)
   {
      SpatialDataWindow* pWindow = static_cast<SpatialDataWindow*>(*iter);
      if (pWindow != NULL)
      {
         SpatialDataView* pView = pWindow->getSpatialDataView();
         if (pView != NULL)
         {
            QWidget* pViewWidget = pView->getWidget();
            if (pViewWidget != NULL)
            {
               pViewWidget->removeEventFilter(this);
            }

            removeMouseMode(pView);
         }
      }
   }

   // Delete the custom mouse mode
   if (mpMouseMode != NULL)
   {
      pDesktop->deleteMouseMode(mpMouseMode);
   }
}

bool neutralwhite::setBatch()
{
   AlgorithmShell::setBatch();
   return false;
}

bool neutralwhite::getInputSpecification(PlugInArgList*& pArgList)
{  
   //VERIFY(pArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   //pArgList->addArg<Progress>(AlgorithmShell::ProgressArg(), NULL, "Progress reporter");
   pArgList = NULL;
   return !isBatch();
}

bool neutralwhite::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return !isBatch();
}

bool neutralwhite::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("mMode", "m_whitecorr", "5EA0CC75-9E0B-4c3d-BA23-6DB7157BBD54");


   //Progress* pProgress = pInArgList->getPlugInArgValue<Progress>(AlgorithmShell::ProgressArg());
   RasterElement* pRaster=NULL;
   RasterDataDescriptor* pDescriptor=NULL;

   if (isBatch() == true)
   {
      return false;
   }

   Service<DesktopServices> pDesktop;

   // Create the custom mouse mode action
   
   QPixmap mouseModePix = QPixmap(MouseModeIcon2);
   mouseModePix.setMask(mouseModePix.createHeuristicMask());
   QIcon mouseModeIcon2(mouseModePix);

   mpMouseModeAction3 = new QAction(mouseModeIcon2, "Display Pixel Coordinate 2", this);
   mpMouseModeAction3->setAutoRepeat(false);
   mpMouseModeAction3->setCheckable(true);
   mpMouseModeAction3->setStatusTip("Select the neutral reference for white correction ");

   // Add a button to the Demo toolbar
   
   ToolBar* pToolBar = dynamic_cast<ToolBar*>(pDesktop->getWindow("Photography", TOOLBAR));
   if (pToolBar == NULL)
   {
      pToolBar = dynamic_cast<ToolBar*>(pDesktop->createWindow("Photography",TOOLBAR));
   }
   if (pToolBar != NULL)
   {
      pToolBar->addButton(mpMouseModeAction3);
   }
   
   // Initialization
   enableAction();

   // Connections
   pDesktop->attach(SIGNAL_NAME(DesktopServices, WindowAdded), Slot(this, &neutralwhite::windowAdded));
   pDesktop->attach(SIGNAL_NAME(DesktopServices, WindowActivated), Slot(this, &neutralwhite::windowActivated));
   pDesktop->attach(SIGNAL_NAME(DesktopServices, WindowRemoved), Slot(this, &neutralwhite::windowRemoved));


   //logging
   pStep->finalize();

   return true;
}

bool neutralwhite::serialize(SessionItemSerializer& serializer) const
{
   if (mpMouseModeAction3 != NULL)
   {
      QString windowName = QString::fromStdString(getName());
      windowName.remove(" ");

      XMLWriter writer(windowName.toUtf8());
      writer.addAttr("batch", isBatch());

      vector<Window*> windows;
      Service<DesktopServices>()->getWindows(SPATIAL_DATA_WINDOW, windows);
      for (vector<Window*>::iterator iter = windows.begin(); iter != windows.end(); ++iter)
      {
         SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(*iter);
         if (pWindow != NULL)
         {
            SpatialDataView* pView = pWindow->getSpatialDataView();
            if (pView != NULL)
            {
               writer.pushAddPoint(writer.addElement("view"));
               writer.addAttr("id", pView->getId());
               writer.popAddPoint();
            }
         }
      }

      return serializer.serialize(writer);
   }

   return false;
}

bool neutralwhite::deserialize(SessionItemDeserializer& deserializer)
{
   QString windowName = QString::fromStdString(getName());
   windowName.remove(" ");

   XmlReader reader(NULL, false);
   DOMElement* pRootElement = deserializer.deserialize(reader, windowName.toUtf8());
   if (pRootElement != NULL)
   {
      bool batch = StringUtilities::fromXmlString<bool>(A(pRootElement->getAttribute(X("batch"))));
      if (batch == true)
      {
         setBatch();
      }
      else
      {
         setInteractive();
      }
   }

   // Create the toolbar button and action
   if (execute(NULL, NULL) == false)
   {
      return false;
   }

   VERIFY(mpMouseModeAction3 != NULL);

   // Add the mouse mode to the spatial data views
   for (DOMNode* pNode = pRootElement->getFirstChild(); pNode != NULL; pNode = pNode->getNextSibling())
   {
      if (XMLString::equals(pNode->getNodeName(), X("view")))
      {
         DOMElement* pViewElement = static_cast<DOMElement*>(pNode);
         Service<SessionManager> pSession;

         SpatialDataView* pView =
            dynamic_cast<SpatialDataView*>(pSession->getSessionItem(A(pViewElement->getAttribute(X("id")))));
         if (pView != NULL)
         {
            QWidget* pViewWidget = pView->getWidget();
            if (pViewWidget != NULL)
            {
               pViewWidget->installEventFilter(this);
            }

            addMouseMode(pView);
         }
      }
   }

   return true;
}

bool neutralwhite::eventFilter(QObject* pObject, QEvent* pEvent)
{
	if ((pObject != NULL) && (pEvent != NULL))
   {
      if (pEvent->type() == QEvent::MouseButtonPress)
      {
         QMouseEvent* pMouseEvent = static_cast<QMouseEvent*> (pEvent);
         if (pMouseEvent->button() == Qt::LeftButton)
         {
            Service<DesktopServices> pDesktop;

            SpatialDataView* pSpatialDataView =
               dynamic_cast<SpatialDataView*>(pDesktop->getCurrentWorkspaceWindowView());
            if (pSpatialDataView != NULL)
            {
               QWidget* pViewWidget = pSpatialDataView->getWidget();
               if (pViewWidget == pObject)
               {
                  MouseMode* pMouseMode = pSpatialDataView->getCurrentMouseMode();
                  if (pMouseMode != NULL)
                  {
                     string mouseMode = "";
                     pMouseMode->getName(mouseMode);
                     if (mouseMode == "DisplayPixelCoordinateMode2")
                     {
                        QPoint ptMouse = pMouseEvent->pos();
                        ptMouse.setY(pViewWidget->height() - pMouseEvent->pos().y());

                        LocationType pixelCoord;
                        pSpatialDataView->translateScreenToWorld(ptMouse.x(), ptMouse.y(),
                           pixelCoord.mX, pixelCoord.mY);

                        double dMinX = 0.0;
                        double dMinY = 0.0;
                        double dMaxX = 0.0;
                        double dMaxY = 0.0;
                        pSpatialDataView->getExtents(dMinX, dMinY, dMaxX, dMaxY);

                        if ((pixelCoord.mX >= dMinX) && (pixelCoord.mX <= dMaxX) && (pixelCoord.mY >= dMinY) &&
                           (pixelCoord.mY <= dMaxY))
                        {
                           LayerList* pLayerList = pSpatialDataView->getLayerList();
                           if (pLayerList != NULL)
                           {
                             RasterElement*  pRaster = pLayerList->getPrimaryRasterElement();
                              if (pRaster != NULL)
                              {
                                 Layer* pLayer = pLayerList->getLayer(RASTER, pRaster);
                                 if (pLayer != NULL)
                                 {
                                    LocationType dataCoord;
                                    pLayer->translateWorldToData(pixelCoord.mX, pixelCoord.mY,
                                       dataCoord.mX, dataCoord.mY);

                                    // Get the original pixel coordinates
                                   RasterDataDescriptor* pDescriptor =
                                       dynamic_cast<RasterDataDescriptor*>(pRaster->getDataDescriptor());
                                    if (pDescriptor != NULL)
                                    {
                                       const vector<DimensionDescriptor>& activeRows = pDescriptor->getRows();
                                       const vector<DimensionDescriptor>& activeColumns = pDescriptor->getColumns();
                                       if ((dataCoord.mY >= 0) &&
                                          (activeRows.size() > static_cast<unsigned int>(dataCoord.mY)) &&
                                          (activeRows[dataCoord.mY].isValid()) &&
                                          (dataCoord.mX >= 0) &&
                                          (activeColumns.size() > static_cast<unsigned int>(dataCoord.mX)) &&
                                          (activeColumns[dataCoord.mX].isValid()))
                                       {
                                          DimensionDescriptor rowDim = activeRows[dataCoord.mY];
                                          DimensionDescriptor columnDim = activeColumns[dataCoord.mX];

                                          int originalSceneX = columnDim.getOriginalNumber() + 1;
                                          int originalSceneY = rowDim.getOriginalNumber() + 1;

                                          QMessageBox::information(pViewWidget, "Display Pixel Coordinate",
                                             "The coordinate of the selected pixel is (" +
											 QString::number(originalSceneX) + ", " +
                                             QString::number(originalSceneY) + ")");

										  //getting r,g & b values of selected neutral reference
										 
										  getRGB(pRaster,pDescriptor,originalSceneX,originalSceneY,correct);
										  
										  //new raster window
										   if(pDescriptor!=NULL)
										   {
									RasterElement *dRaster=RasterUtilities::createRasterElement(pRaster->getName()+"RGB",
											   pDescriptor->getRowCount(), pDescriptor->getColumnCount(),3, pDescriptor->getDataType(),BSQ);
										  
										  if(correct[0]>255||correct[1]>255||correct[2]>255)						//if image is 16-bit
											   {
												//underflow check
												if(correct[0]<1)
												{ correct[0]=1;}

												if(correct[1]<1)
												{ correct[1]=1;}
												
												if(correct[2]<1)
												{ correct[2]=1;}

												correct[0]= (65535/correct[0]);
												correct[1]= (65535/correct[1]);
												correct[2]= (65535/correct[2]);

												copyImage2(pRaster,dRaster,0,65535,correct[0]);
												copyImage2(pRaster,dRaster,1,65535,correct[1]);
												copyImage2(pRaster,dRaster,2,65535,correct[2]);

											   }
											   else
											   {																	//if image is 8-bit
												//underflow check
												if(correct[0]<1)
												{ correct[0]=1;}

												if(correct[1]<1)
												{ correct[1]=1;}
												
												if(correct[2]<1)
												{ correct[2]=1;}

												correct[0]= (255/correct[0]);
												correct[1]= (255/correct[1]);
												correct[2]= (255/correct[2]);
											   
												copyImage2(pRaster,dRaster,0,255,correct[0]);
												copyImage2(pRaster,dRaster,1,255,correct[1]);
												copyImage2(pRaster,dRaster,2,255,correct[2]);
										  
												}
										  
										     
																			  
										    
										  
										  
											//new model resource
									RasterDataDescriptor* rDesc = dynamic_cast<RasterDataDescriptor*>(dRaster->getDataDescriptor());
											rDesc->setDisplayMode(RGB_MODE);										//enable color mode
											rDesc->setDisplayBand(RED,pDescriptor->getActiveBand(0));
											rDesc->setDisplayBand(GREEN,pDescriptor->getActiveBand(1));
											rDesc->setDisplayBand(BLUE,pDescriptor->getActiveBand(2));
											ModelResource<RasterElement> pResultCube(dRaster);
										  
											//create new window

											if(!isBatch())
											{
											 Service<DesktopServices> pDesktop;

										  SpatialDataWindow* pWindow = static_cast<SpatialDataWindow*>(pDesktop->createWindow(pResultCube->getName(),
												SPATIAL_DATA_WINDOW));

										 SpatialDataView* pView = (pWindow == NULL) ? NULL : pWindow->getSpatialDataView();	
										    
											 pView->setPrimaryRasterElement(pResultCube.get());
											 pView->createLayer(RASTER, pResultCube.get());

											 pResultCube.release();  //saving data

											}

										   }
										    

                                       }
                                    }
                                 }
                              }
                           }
                        }
                     }
                  }
              
				  pSpatialDataView->setMouseMode(NULL);  //disable mouse mode
				  
			   }
            }
         }
      }
   }

   return QObject::eventFilter(pObject, pEvent);
}

void neutralwhite::windowAdded(Subject& subject, const string& signal, const boost::any& value)
{
   if (dynamic_cast<DesktopServices*>(&subject) != NULL)
   {
      Window* pWindow = boost::any_cast<Window*>(value);

      SpatialDataWindow* pSpatialDataWindow = dynamic_cast<SpatialDataWindow*>(pWindow);
      if (pSpatialDataWindow != NULL)
      {
         SpatialDataView* pView = pSpatialDataWindow->getSpatialDataView();
         if (pView != NULL)
         {
            QWidget* pViewWidget = pView->getWidget();
            if (pViewWidget != NULL)
            {
               pViewWidget->installEventFilter(this);
            }

            addMouseMode(pView);
         }
      }
   }
}

void neutralwhite::windowActivated(Subject& subject, const string& signal, const boost::any& value)
{
   enableAction();
}

void neutralwhite::windowRemoved(Subject& subject, const string& signal, const boost::any& value)
{
   if (dynamic_cast<DesktopServices*>(&subject) != NULL)
   {
      Window* pWindow = boost::any_cast<Window*>(value);

      SpatialDataWindow* pSpatialDataWindow = dynamic_cast<SpatialDataWindow*>(pWindow);
      if (pSpatialDataWindow != NULL)
      {
         SpatialDataView* pView = pSpatialDataWindow->getSpatialDataView();
         if (pView != NULL)
         {
            QWidget* pViewWidget = pView->getWidget();
            if (pViewWidget != NULL)
            {
               pViewWidget->removeEventFilter(this);
            }

            removeMouseMode(pView);
         }
      }
   }


}



void neutralwhite::addMouseMode(SpatialDataView* pView)
{
   if (pView == NULL)
   {
      return;
   }

   Service<DesktopServices> pDesktop;

   // Create the custom mouse mode
   if (mpMouseMode == NULL)
   {
      // To set a custom mouse cursor for the custom mouse mode change the
      // NULL pointers when creating the mouse mode to the custom cursor
      mpMouseMode = pDesktop->createMouseMode("DisplayPixelCoordinateMode2", NULL, NULL, -1, -1, mpMouseModeAction3);
   }

   // Add the mouse mode to the view
   if (mpMouseMode != NULL)
   {
      pView->addMouseMode(mpMouseMode);
   }
}

void neutralwhite::removeMouseMode(SpatialDataView* pView)
{
   // Remove the mouse mode from the view
   if ((pView != NULL) && (mpMouseMode != NULL))
   {
      pView->removeMouseMode(mpMouseMode);
   }
}

void neutralwhite::enableAction()
{
   if (mpMouseModeAction3 != NULL)
   {
      Service<DesktopServices> pDesktop;
      bool bEnable = false;

      SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(pDesktop->getCurrentWorkspaceWindow());
      if (pWindow != NULL)
      {
         SpatialDataView* pView = pWindow->getSpatialDataView();
         if (pView != NULL)
         {
            bEnable = true;
         }
      }

      mpMouseModeAction3->setEnabled(bEnable);
   }
}

