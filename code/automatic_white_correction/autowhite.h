/*
 * The information in this file is
 * Copyright (C) 2011, Pratik Anand <pratik@pratikanand.com>
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef AUTOWHITE_H
#define AUTOWHITE_H

#include "ExecutableShell.h"

class autowhite : public ExecutableShell
{
public:
   autowhite();
   virtual ~autowhite();

   virtual bool getInputSpecification(PlugInArgList*& pInArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pOutArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

protected:
	bool maxBandValue(RasterElement *pRaster,double result[]);
	bool copyImage(RasterElement *pRaster,RasterElement *dRaster,int i,Progress* pProgress,double correct);
};

#endif
