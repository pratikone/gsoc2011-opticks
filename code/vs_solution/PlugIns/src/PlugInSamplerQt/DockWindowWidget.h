/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DOCKWINDOWWIDGET_H
#define DOCKWINDOWWIDGET_H

#include <boost/any.hpp>
#include <QtGui/QCheckBox>
#include <QtGui/QListWidget>
#include <QtGui/QTreeWidget>
#include <QtGui/QWidget>

#include "DesktopServices.h"

class ElidedLabel;
class PlotSet;
class PlotWidget;
class PlotWindow;
class Subject;

class DockWindowWidget : public QWidget
{
   Q_OBJECT

public:
   DockWindowWidget(QWidget* pParent = 0);
   ~DockWindowWidget();

   void windowAdded(Subject& subject, const std::string& signal, const boost::any& value);
   void windowActivated(Subject& subject, const std::string& signal, const boost::any& value);
   void windowRemoved(Subject& subject, const std::string& signal, const boost::any& value);

protected:
   void setSelectedPlotWindow(PlotWindow* pWindow);
   void setSelectedPlotSet(PlotSet* pPlotSet);
   void setSelectedPlot(PlotWidget* pPlot);
   PlotWindow* getSelectedPlotWindow() const;
   PlotWidget* getSelectedPlot() const;

protected slots:
   void addPlotWindow();
   void deletePlotWindow();
   void showPlotWindow();
   void hidePlotWindow();
   void updatePlotWindowList();
   void addPlot();
   void deletePlot();
   void updatePlotList();
   void activatePlot();
   void renamePlot();
   void editPlotProperties();

private:
   Service<DesktopServices> mpDesktop;

   QListWidget* mpWindowList;
   QCheckBox* mpDeleteWindowCheck;
   QTreeWidget* mpPlotTree;
   ElidedLabel* mpActiveWindowLabel;
};

#endif
