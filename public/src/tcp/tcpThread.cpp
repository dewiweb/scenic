/* GTHREAD-QUEUE-PAIR - Library of TcpThread Queue Routines for GLIB
 * Copyright 2008  Koya Charles & Tristan Matthews
 *
 * This library is free software; you can redisttribute it and/or
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
#include <iostream>
#include "tcpThread.h"
#include "logWriter.h"
#include "parser.h"
#include <errno.h>
#include <string.h>

class TcpLogFunctor
    : public LogFunctor
{
    public:
        TcpLogFunctor(TcpThread& tcp)
            : tcp_(tcp){}
        TcpThread& tcp_;
        void operator()(LogLevel&, std::string& msg);
};


void TcpLogFunctor::operator()(LogLevel& level, std::string& msg)
{
    MapMsg m("log");
    m["level"] = level;
    m["msg"] = msg;
    tcp_.send(m);
}


static std::string get_line(std::string& msg)
{
    std::string ret;
    std::string::size_type pos = msg.find_first_of("\n\r");
    if(pos != std::string::npos)
    {
        ret = msg.substr(0, pos+2);
        msg.erase(0, pos+2);
    }
    else{
        ret = msg;
        msg.clear();
    }
    return ret;
}


int TcpThread::main()
{
    bool quit = false;
    std::string msg;
    TcpLogFunctor lf_(*this);

    try
    {
        while(!quit)
        {
            serv_.socket_bind_listen();
            while(!serv_.accept())
            {
                if((quit = gotQuit()))
                    return 0;
                usleep(10000);
            }
            try
            {
                LOG_INFO("Got Connection.");
                if(logFlag_)
                    LOG::register_cb(&lf_);
                while(serv_.connected())
                {
                    if((quit = gotQuit()))
                    {
                        break;
                    }
                    if(serv_.recv(msg))
                    {
                        std::string line = get_line(msg);
                        do
                        {
                            MapMsg mapMsg;
                            if(Parser::tokenize(line, mapMsg))
                                queue_.push(mapMsg);
                            else
                                LOG_WARNING("Bad Msg Received.");
                            line = get_line(msg);
                        }
                        while(!line.empty());
                    }
                    else
                        usleep(1000);
                }
            }
            catch(Except e)
            {
                LOG_DEBUG( "CAUGHT " << e.msg_);
            }
            if(logFlag_)
                LOG::unregister_cb();
            if(!quit)
                LOG_WARNING("Disconnected from Core.");
            usleep(1000);
            serv_.close();
        }
    }
    catch(Except e)
    {
        LOG_DEBUG("Passing exception to other Thread" << e.msg_);

        MapMsg mapMsg("exception");
        mapMsg["exception"] = CriticalExcept(e.msg_, e.errno_);
        queue_.push(mapMsg);
    }
    return 0;
}


bool TcpThread::gotQuit()
{
    MapMsg f = queue_.timed_pop(1);
    std::string command;
    if(f["command"].type() == 'n')
        return false;
    if(f["command"].get(command)&& command == "quit")
        return true;
    else
        send(f);
    return false;
}


bool TcpThread::send(MapMsg& msg)
{
    std::string msg_str;
    bool ret;
    LOG::hold_cb();           // to insure no recursive calls due to log message calling send
    try
    {
        Parser::stringify(msg, msg_str);
        ret = serv_.send(msg_str);
        LOG::release_cb();
    }
    catch(ErrorExcept e)
    {
        LOG_DEBUG(std::string(msg["command"]) << " Error at Send. Cancelled. " <<
                  strerror(e.errno_));
        LOG::release_cb();
        if(e.errno_ == EBADF) //Bad File Descriptor
            throw (e);
    }


    return ret;
}


bool TcpThread::socket_connect_send(const std::string& addr, MapMsg& msg)
{
    std::string msg_str;
    Parser::stringify(msg, msg_str);
    return serv_.socket_connect_send(addr, msg_str);
}


