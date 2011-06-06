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
   ".......#........",
   "........#.......",
   "........#.......",
   "........#.......",
   ".#...#..#.#...#.",
   "..#.#...#.#...#.",
   "...#....#..#.#..",
   "...#....#..#.#..",
   "..#.#...#...#...",
   ".#...#..#...#...",
   "........#..#....",
   ".......#...#....",
   "................",
   "................",
   "................"
};

REGISTER_PLUGIN_BASIC(OpticksTutorial, neutralwhite);

neutralwhite::neutralwhite() :
   mpMouseMode(NULL),
   mpMouseModeAction3(NULL)
{
   AlgorithmShell::setName("Custom Mouse Mode Plug-In");
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setDescription("Demonstrates creating a custom mouse mode.");
   setDescriptorId("{e538f9a0-8e27-11e0-91e4-0800200c9a66}");
   //setMenuLocation("[ABC]/neutralwhite");
   

   setProductionStatus(false);
   allowMultipleInstances(false);
   executeOnStartup(true);
   destroyAfterExecute(false);
   setAbortSupported(false);
   setWizardSupported(false);
}

neutralwhite::~neutralwhite()
{
   Service<DesktopServices> pDesktop;

   // Remove the toolbar button and delete the mouse mode action
   
   ToolBar* pToolBar = dynamic_cast<ToolBar*>(pDesktop->getWindow("Demo2", TOOLBAR));
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
   StepResource pStep("mMode", "pratik", "5EA0CC75-9E0B-4c3d-BA23-6DB7157BBD54");

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
   mpMouseModeAction3->setStatusTip("Displays the coordinate of a pixel selected with the mouse");

   // Add a button to the Demo toolbar
   
   ToolBar* pToolBar = dynamic_cast<ToolBar*>(pDesktop->getWindow("Demo2", TOOLBAR));
   if (pToolBar == NULL)
   {
      pToolBar = dynamic_cast<ToolBar*>(pDesktop->createWindow("Demo2",TOOLBAR));
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
                              RasterElement* pRaster = pLayerList->getPrimaryRasterElement();
                              if (pRaster != NULL)
                              {
                                 Layer* pLayer = pLayerList->getLayer(RASTER, pRaster);
                                 if (pLayer != NULL)
                                 {
                                    LocationType dataCoord;
                                    pLayer->translateWorldToData(pixelCoord.mX, pixelCoord.mY,
                                       dataCoord.mX, dataCoord.mY);

                                    // Get the original pixel coordinates
                                    const RasterDataDescriptor* pDescriptor =
                                       dynamic_cast<const RasterDataDescriptor*>(pRaster->getDataDescriptor());
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

                                          unsigned int originalSceneX = columnDim.getOriginalNumber() + 1;
                                          unsigned int originalSceneY = rowDim.getOriginalNumber() + 1;

                                          QMessageBox::information(pViewWidget, "Display Pixel Coordinate",
                                             "The coordinate of the selected pixel is (" +
                                             QString::number(originalSceneX) + ", " +
                                             QString::number(originalSceneY) + ")");
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