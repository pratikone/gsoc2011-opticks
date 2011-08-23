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
#include "median_noise_ui.h"

#include <QtGui/QDoubleSpinBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QPushButton>


using namespace std;


median_noise_ui::median_noise_ui(QWidget* pParent) : QDialog(pParent)
{
   
   setWindowTitle("Median denoising");

   
   ResearchValue=3;
   
   
   
   QGridLayout* pLayout = new QGridLayout(this);
   pLayout->setMargin(10);
   pLayout->setSpacing(5);
   

   
   QLabel* pLable1 = new QLabel("Noise deviation: ", this);
   pLable1->hide();
   pLayout->addWidget(pLable1, 0, 0);

   SigmaValueBox = new QDoubleSpinBox(this);
   SigmaValueBox->setRange(10, 30);
   SigmaValueBox->setSingleStep(1);
   SigmaValueBox->setValue(SigmaValue);
   SigmaValueBox->hide();
   pLayout->addWidget(SigmaValueBox, 0, 1, 1, 2);
   

   
   QLabel* pLable2 = new QLabel("Research window: ", this);
   pLayout->addWidget(pLable2, 1, 0);

   ResearchBox = new QDoubleSpinBox(this);
   ResearchBox->setRange(3, 11);
   ResearchBox->setSingleStep(2);
   ResearchBox->setValue(ResearchValue);
   pLayout->addWidget(ResearchBox, 1, 1, 1, 2);

   
   
   QLabel* pLable3 = new QLabel("Comparison window: ", this);
   pLable3->hide();
   pLayout->addWidget(pLable3, 2, 0);

   CompareBox = new QDoubleSpinBox(this);
   CompareBox->setRange(3, 11);
   CompareBox->setSingleStep(2);
   CompareBox->setValue(CompareValue);
   CompareBox->hide();
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


void median_noise_ui::setSigmaValue(double t)
{
	SigmaValue = t;
}

void median_noise_ui::setResearchWindow(double t)
{
	 ResearchValue= t;
}

void median_noise_ui::setCompareWindow(double t)
{
	CompareValue = t;
}

double median_noise_ui::getSigmaValue()
{
	return SigmaValue;
}

double median_noise_ui::getResearchValue()
{
	return ResearchValue;
}

double median_noise_ui::getCompareValue()
{
	return CompareValue;
}


