/*
 * Copyright (C) 2008 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is free software: you can redistribute it and*or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Sropulpof is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Sropulpof.  If not, see <http:*www.gnu.org*licenses*>.
 */

#ifndef _SIP_SESSION_H
#define _SIP_SESSION_H

/*
 * @file 	sipsession.h
 * @brief	The SIP implementation
 */

#include <string>

#include "session.h"
#include "useragent.h"
#include "sdp.h"

#define _APP_NAME   "miville"
#define _DEFAULT_SIP_PORT   5060

class SIPSession
    : public Session
{
public:
    /*
     * Create a new SIP session object
     *
     * @param port	The local protocol listening port.
     */
    SIPSession( int port );

    /*
     * Class destructor
     */
    ~SIPSession();

    /*
     * Establishes a SIP connection with a remote host.
     *
     * @param 	r_uri	The remote SIP address
     * @param 	r_port	The remote connection port (default SIP port: 5060)
     */
    int connect( std::string r_uri, int r_port );

    /*
     * Terminate a current SIP remote connection
     */
    int disconnect();

    int accept( void );

    int refuse( int reason );

    void build_sdp( void );

    void startMainloop();

private:

    /* The application SIP User Agent */
    UserAgent* _app_ua;

    /* A SDP (Session Description Protocol) instance to build the sdp body */
    /* Should be an attribute of the useragent class */
    Sdp* _sdp;
};

#endif // _SIP_SESSION_H
