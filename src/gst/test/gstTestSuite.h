
// gstTestSuite.h
// Copyright 2008 Koya Charles & Tristan Matthews
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

// Declares and registers a series of unit tests. Note that currently, these tests are quite simple.
// A more comprehensive/detailed testsuite would have to cover specific
// usecases and look more like client code. The current set of tests just does a quick probe of our
// gstreamer interface.
// Several of the tests (specifically those sending over udp) require two instances of the Tester
// to be running, one with an argument of 0 and the other with an argument of 1.

#ifndef _GST_TEST_SUITE_H_
#define _GST_TEST_SUITE_H_

#define BLOCKING 1

#if BLOCKING
#define BLOCK() GstTestSuite::block(__FILE__, __FUNCTION__, __LINE__)
#else
#define BLOCK()
#endif

#include <cpptest.h>

class GstTestSuite
    : public Test::Suite
{
    public:

        GstTestSuite()
            : id_(0), testLength_(10000000), audioFilename_("test_signal8.wav"), videoFilename_("/usr/share/example-content/Experience ubuntu.ogg")
        {}

        void set_id(int id);
        static bool areValidArgs(int argc, char **argv);
        const static int A_PORT;
        const static int V_PORT;
        
    protected:

        void block(const char *filename, const char *function, long lineNumber);
        virtual void setup();       // setup resources common to all tests
        virtual void tear_down();   // destroy common resources

        int id_;
        int testLength_;
//        const static int NUM_CHANNELS;
        static int killMainLoop(void *data);
        
        std::string audioFilename_;
        std::string videoFilename_;
};

#endif // _GST_TEST_SUITE_H_
