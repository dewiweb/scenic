/* GTHREAD-QUEUE-PAIR - Library of Thread Queue Routines for GLIB
 * Copyright 2008  Koya Charles & Tristan Matthews
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <glib.h>
#include <iostream>
#include "gutil/baseModule.h"
#include "logWriter.h"
#include "gutil/optionArgs.h"

class DModule
    : public BaseModule
{
    bool pass;
   public:
    DModule()
        : BaseModule(),pass(false){}
    bool run();
};


bool DModule::run()
{
    if(!pass)
        while(1)
        {
            LOG("x ");
        }
    return true;
}


int main (int argc, char** argv)
{
    DModule m;


    m.run();
    return 0;
}


