
// busMsgHandler.h
// Copyright (C) 2008-2009 Société des arts technologiques (SAT)
// http://www.sat.qc.ca
// All rights reserved.
//
// This file is part of [propulse]ART.
//
// [propulse]ART is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// [propulse]ART is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
//
// Abstract interface that defines one method, handleBusMsg

#ifndef _BUS_MSG_HANDLER_H_
#define _BUS_MSG_HANDLER_H_

class _GstMessage;

/** 
* Abstract interface which requires its implementors to provide 
* functionality to handle messages posted on the bus. Variation on
* the Observer and Chain of Responsibility patterns.
*/

class BusMsgHandler
{
    public:
    /// This method is called by the GstBus listener when it has a new msg. 
    virtual bool handleBusMsg(_GstMessage *msg) = 0;
    /** 
     * Destructor */
    virtual ~BusMsgHandler() {};
};

#endif // _BUS_MSG_HANDLER_H_ 

