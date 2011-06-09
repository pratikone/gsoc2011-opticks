/*
 * The information in this file is
 * Copyright (C) 2011, Pratik Anand <pratik@pratikanand.com>
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef median_noise_H
#define median_noise_H

#include "ExecutableShell.h"

class median_noise : public ExecutableShell
{
public:
   median_noise();
   virtual ~median_noise();

   virtual bool getInputSpecification(PlugInArgList*& pInArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pOutArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
protected:
	bool copyImage3(RasterElement *pRaster,RasterElement *dRaster,int i,Progress* pProgress);
};

#endif
