/*
 * The information in this file is
 * Copyright (C) 2011, Pratik Anand <pratik@pratikanand.com>
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef nlmean_UI_H
#define nlmean_UI_H

//#include "ExecutableShell.h"
#include <QtGui/QDialog>


class QDoubleSpinBox;

class nlmean_ui : public QDialog
{   
	 Q_OBJECT
public:
   nlmean_ui(QWidget* pParent);
   
   QDoubleSpinBox* CompareBox;
   QDoubleSpinBox* ResearchBox;
   QDoubleSpinBox* SigmaValueBox;

   int getCompareValue();
   int getResearchValue();
   double getSigmaValue();



private slots:
   void setCompareWindow(int t=0);
   void setResearchWindow(int t=0);
   void setSigmaValue(double t=0.0);

private:
	int CompareValue;
    int ResearchValue;
	double SigmaValue;
	
};

#endif
