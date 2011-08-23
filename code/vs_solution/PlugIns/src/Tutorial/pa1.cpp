/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "Progress.h"
#include "pa1.h"

REGISTER_PLUGIN_BASIC(OpticksTutorial, pa1);

pa1::pa1()
{
   setDescriptorId("{7b4beca0-5ec0-11e0-80e3-0800200c9a66}");
   setName("Pa1 1");
   setDescription("Creating your first plug-in.");
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setProductionStatus(false);
   setType("Sample");
   setMenuLocation("[Tutorial]/Pa1");
   setAbortSupported(false);
}

pa1::~pa1()
{
}

bool pa1::getInputSpecification(PlugInArgList*& pInArgList)
{
   VERIFY(pInArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pInArgList->addArg<Progress>(Executable::ProgressArg(), NULL, "Progress reporter");
   return true;
}

bool pa1::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   pOutArgList = NULL;
   return true;
}

bool pa1::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   if (pInArgList == NULL)
   {
      return false;
   }
   Progress* pProgress = pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg());
   if (pProgress != NULL)
   {
      pProgress->updateProgress("This demonstrates display of a warning.", 0, WARNING);
      pProgress->updateProgress("This demonstrates display of an error.", 0, ERRORS);
      pProgress->updateProgress("Hello World!", 100, NORMAL);
   }
   return true;
}
