/*
 * The information in this file is
 * Copyright (C) 2011, Pratik Anand <pratik@pratikanand.com>
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef adaptive_median_H
#define adaptive_median_H

#include "ExecutableShell.h"

class adaptive_median : public ExecutableShell
{
public:
   adaptive_median();
   virtual ~adaptive_median();

   virtual bool getInputSpecification(PlugInArgList*& pInArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pOutArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
protected:
	bool copyImage4(RasterElement *pRaster,RasterElement *dRaster,int i,Progress* pProgress);
};

#endif
