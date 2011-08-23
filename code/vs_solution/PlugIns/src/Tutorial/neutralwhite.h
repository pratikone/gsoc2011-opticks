/*
 * The information in this file is
 * Copyright (C) 2011, Pratik Anand <pratik@pratikanand.com>
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef neutralwhite_H
#define neutralwhite_H

#include <QtCore/QObject>
#include <QtGui/QAction>


#include "AlgorithmShell.h"
#include "SessionItemDeserializer.h"
#include "SessionItemSerializer.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterUtilities.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include <boost/any.hpp>
#include <string>

class MouseMode;
class SpatialDataView;

class neutralwhite : public QObject, public AlgorithmShell
{
public:
   neutralwhite();
   ~neutralwhite();

   double correct[3];
   RasterElement *pRaster,*dRaster;
   RasterDataDescriptor *pDescriptor,*rDesc;
   SpatialDataView *pView;
   SpatialDataWindow *pWindow;

   bool setBatch();
   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

   virtual bool serialize(SessionItemSerializer& serializer) const;
   virtual bool deserialize(SessionItemDeserializer& deserializer);

protected:
   bool eventFilter(QObject* pObject, QEvent* pEvent);
   void windowAdded(Subject& subject, const std::string& signal, const boost::any& value);
   void windowActivated(Subject& subject, const std::string& signal, const boost::any& value);
   void windowRemoved(Subject& subject, const std::string& signal, const boost::any& value);

   void addMouseMode(SpatialDataView* pView);
   void removeMouseMode(SpatialDataView* pView);
   void enableAction();
   bool copyImage2(RasterElement *pRaster,RasterElement *dRaster,int i,double max,double correct);
   bool neutralwhite::getRGB(RasterElement *pRaster,RasterDataDescriptor* pDesc,int x,int y,double correct[]);

private:
   MouseMode* mpMouseMode;
   QAction* mpMouseModeAction3;
};

#endif
