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
	
#ifndef _SDP_CODEC_H
#define _SDP_CODEC_H

#include <string>

class sdpCodec
{
    public:
    sdpCodec();
	sdpCodec( std::string type, std::string name, int payload );
	~sdpCodec();

    std::string getType( void ){ return _m_type; }
    std::string getName( void ){ return _name; }
    int getPayload( void ){ return _payload; }


    private:
	std::string _name;
    std::string _m_type;
	int _payload;
	//int _frequency;
};

#endif // _SDP_CODEC_H
