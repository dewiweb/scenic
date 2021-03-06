
/*
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of Scenic.
 *
 * Scenic is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Scenic is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Scenic.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "milhouse_logger.h"
#include <boost/bind.hpp>
#include <iostream>


MilhouseLogger::MilhouseLogger(const std::string &logLevel) :
    printQueue_(),
    printThread_(boost::bind<void>(&MilhouseLogger::printMessages, this)),
    gstDebug_(false),
    level_(argToLogLevel(logLevel))
{
}


LogLevel MilhouseLogger::argToLogLevel(const std::string &level)
{
    std::string upperCase(level);
    std::transform(upperCase.begin(), upperCase.end(), upperCase.begin(), toupper);
    std::map<std::string, LogLevel> strings;
    strings["CRITICAL"] = strings["1"] = CRITICAL;
    strings["ERROR"] = strings["2"] = ERROR;
    strings["WARNING"] = strings["3"] =  WARNING;
    strings["INFO"] = strings["4"] = INFO;
    strings["DEBUG"] = strings["5"] = DEBUG;

    if (upperCase == "GST-DEBUG" or upperCase == "6")
    {
        gstDebug_ = true;   // special case
        return DEBUG;
    }
    else
    {
        // make sure the argument wasn't junk
        if (strings.find(upperCase) != strings.end())
            return strings[upperCase];
        else // default to DEBUG, this user probably needs it
            return DEBUG;
    }
}


MilhouseLogger::~MilhouseLogger()
{
    // end our print thread
    printQueue_.push("QUIT_SENTINEL:");
    printThread_.join(); // wait for print thread to go out before main thread does
}

/// This is called in the main thread
void MilhouseLogger::operator()(LogLevel& level, std::string& msg)
{
    if (level_ <= level)    // only push to print queue if the loglevel of this msg exceeds
        printQueue_.push(msg);  // the logger's loglevel
}

/// This is executed in printThread_. The only shared data is printQueue_, which is thread-safe
void MilhouseLogger::printMessages()
{
    bool done = false;
    while (!done)
    {
        std::string msg;
        printQueue_.wait_and_pop(msg);

        /// quit when msg starts with quit:
        if (msg != "QUIT_SENTINEL:")
                std::cout << msg;
        else  // got a sentinel
            done = true;
        // since we're not busy waiting, we don't really need this sleep
        // boost::this_thread::sleep(boost::posix_time::milliseconds(1));
    }
}

