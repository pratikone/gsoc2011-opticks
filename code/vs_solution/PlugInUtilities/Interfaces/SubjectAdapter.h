/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SUBJECTADAPTER_H
#define SUBJECTADAPTER_H

#include "Subject.h"
#include "SubjectImp.h"

#include <string>

/**
 *  Implementation of the interface for generalized registration of
 *  objects intereseted in mutations of the subclasses of this interface.
 */
class SubjectAdapter : public Subject, public SubjectImp SUBJECTADAPTEREXTENSION_CLASSES
{
public:
   /**
    *  Inherited obligation from TypeAwareObject. Allow observers to
    *  determine the type of mutating object notifying them.
    *
    *  @return  The name of the subclass of this interface.
    */
   virtual const std::string& getObjectType() const;

   /**
    *  Inherited obligation from TypeAwareObject. Allow observers to
    *  determine the type of mutating object notifying them.
    *
    *  @param   name
    *           Name of the class which is being inquired about.
    *
    *  @return  True if the subclass matches of this interface matches or
    *           is a parent of the class identified in the argument.
    */
   virtual bool isKindOf(const std::string& name) const;

   SUBJECTADAPTER_METHODS(SubjectImp)
};

#endif
