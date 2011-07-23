/*
 * The information in this file is
 * Copyright (C) 2011, Pratik Anand <pratik@pratikanand.com>
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppAssert.h"
#include "AppVerify.h"
#include "nlmean_ui.h"

#include <QtGui/QDoubleSpinBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QPushButton>


using namespace std;


nlmean_ui::nlmean_ui(QWidget* pParent) : QDialog(pParent)
{
   
   setWindowTitle("Non-local mean denoising");

   SigmaValue=15;
   ResearchValue=17;
   CompareValue=3;
   
   
   QGridLayout* pLayout = new QGridLayout(this);
   pLayout->setMargin(10);
   pLayout->setSpacing(5);

   QLabel* pLable1 = new QLabel("Noise deviation: ", this);
   pLayout->addWidget(pLable1, 0, 0);

   SigmaValueBox = new QDoubleSpinBox(this);
   SigmaValueBox->setRange(10, 30);
   SigmaValueBox->setSingleStep(1);
   SigmaValueBox->setValue(SigmaValue);
   pLayout->addWidget(SigmaValueBox, 0, 1, 1, 2);


   QLabel* pLable2 = new QLabel("Research window: ", this);
   pLayout->addWidget(pLable2, 1, 0);

   ResearchBox = new QDoubleSpinBox(this);
   ResearchBox->setRange(17, 25);
   ResearchBox->setSingleStep(2);
   ResearchBox->setValue(ResearchValue);
   pLayout->addWidget(ResearchBox, 1, 1, 1, 2);

   QLabel* pLable3 = new QLabel("Comparison window: ", this);
   pLayout->addWidget(pLable3, 2, 0);

   CompareBox = new QDoubleSpinBox(this);
   CompareBox->setRange(3, 11);
   CompareBox->setSingleStep(2);
   CompareBox->setValue(CompareValue);
   pLayout->addWidget(CompareBox, 2, 1, 1, 2);

   QHBoxLayout* pRespLayout = new QHBoxLayout;
   pLayout->addLayout(pRespLayout, 3, 0, 1, 3);

   QPushButton* pAccept = new QPushButton("OK", this);
   pRespLayout->addStretch();
   pRespLayout->addWidget(pAccept);

   QPushButton* pReject = new QPushButton("Cancel", this);
   pRespLayout->addWidget(pReject);
   
   connect(pAccept, SIGNAL(clicked()), this, SLOT(accept()));
   connect(pReject, SIGNAL(clicked()), this, SLOT(reject()));

   connect(SigmaValueBox, SIGNAL(valueChanged(double)), this, SLOT(setSigmaValue(double)));
   connect(ResearchBox, SIGNAL(valueChanged(double)), this, SLOT(setResearchWindow(double)));
   connect(CompareBox, SIGNAL(valueChanged(double)), this, SLOT(setCompareWindow(double)));
   
}


void nlmean_ui::setSigmaValue(double t)
{
	SigmaValue = t;
}

void nlmean_ui::setResearchWindow(int t)
{
	 ResearchValue= t;
}

void nlmean_ui::setCompareWindow(int t)
{
	CompareValue = t;
}

double nlmean_ui::getSigmaValue()
{
	return SigmaValue;
}

int nlmean_ui::getResearchValue()
{
	return ResearchValue;
}

int nlmean_ui::getCompareValue()
{
	return CompareValue;
}


