/*
 * The information in this file is
 * Copyright (C) 2011, Pratik Anand <pratik@pratikanand.com>
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef nlmean_H
#define nlmean_H

#include "ExecutableShell.h"


class nlmean : public ExecutableShell
{
public:
   nlmean();
   virtual ~nlmean();

   virtual bool getInputSpecification(PlugInArgList*& pInArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pOutArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

protected:
	bool standard_deviation(RasterElement *pRaster, RasterElement *dRaster, int i,const double SIGMA,const int size,const int csize, Progress *pProgress);
};

#endif
