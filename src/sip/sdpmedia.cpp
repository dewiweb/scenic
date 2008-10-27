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

#include "sdpmedia.h"
#include <string.h>
#include <sstream>
#include <iostream>

static const char* streamDirectionStr[] =
{
    "sendrecv",
    "sendonly",
    "recvonly",
    "inactive"
};

static const char* mediaTypeStr[] =
{
    "audio",
    "video",
    "application",
    "text",
    "image",
    "message"
};

sdpMedia::sdpMedia( int type )
    : _media_type( (mediaType)type ), _codec_list(0), _port( 0 ), _stream_type( SEND_RECEIVE ){}


sdpMedia::sdpMedia( std::string type, int port, std::string dir)
    : _media_type( (mediaType)-1), _codec_list(0), _port(port),
    _stream_type((streamDirection)-1){
    unsigned int i;
    const char* tmp;

    for( i=0 ; i<MEDIA_COUNT ; i++){
        tmp = mediaTypeStr[i];
        if( strcmp(type.c_str(), tmp) == 0 ){
            _media_type = (mediaType)i;
            break;
        }
    }

    if( strcmp( dir.c_str(), "default") == 0 )
        dir = DEFAULT_STREAM_DIRECTION;
    for( i=0; i<DIR_COUNT; i++ ){
        tmp = streamDirectionStr[i];
        if( strcmp(dir.c_str(), tmp) == 0){
            _stream_type = (streamDirection)i;
            break;
        }
    }
}


sdpMedia::~sdpMedia()
{
    int i;
    for(i=0; i<(int)_codec_list.size(); i++)
        delete _codec_list[i];
}


std::string sdpMedia::get_media_type_str( void ){
    std::string value;

    // Test the range to be sure we know the media
    if( _media_type >= 0 && _media_type < MEDIA_COUNT )
        value = mediaTypeStr[ _media_type ];
    else
        value = "unknown";
    return value;
}

void sdpMedia::add_codec( sdpCodec *codec ){
    _codec_list.push_back(codec);
}

void sdpMedia::add_codec( std::string codecName ){
    // We have to build the codec from its encoding name
    _codec_list.push_back( new sdpCodec( _media_type, codecName ));
}

void sdpMedia::add_codec( int payload ){
    _codec_list.push_back( new sdpCodec( payload) );
}

void sdpMedia::remove_codec( std::string codecName )
{
    // Look for the codec by its encoding name
    int i;
    int size;
    std::string enc_name;
    std::vector<sdpCodec*>::iterator iter;

    size = _codec_list.size();
    std::cout << "vector size: " << size << std::endl;

    for( i=0 ; i<size ; i++ ){
        std::cout << _codec_list[i]->_name.c_str() << std::endl;
        if( strcmp(_codec_list[i]->_name.c_str(), codecName.c_str()) == 0 ){
            std::cout << "erase " <<_codec_list[i]->_name << std::endl;
            iter = _codec_list.begin()+i;
            _codec_list.erase(iter);
            break;
        }
    }
}


void sdpMedia::clear_codec_list( void ) {
    // Erase every codecs from the list
    _codec_list.clear();
}


std::string sdpMedia::get_stream_direction_str( void ) {
    std::string value;

    // Test the range of the value
    if( _stream_type >= 0 && _stream_type < DIR_COUNT )
        value = streamDirectionStr[ _stream_type ];
    else
        value = "unknown";
    return value;
}


std::string sdpMedia::to_string( void ){
    std::ostringstream display;
    int size, i;

    size = _codec_list.size();

    display << get_media_type_str();
    display << ":" << get_port();
    display << ":";
    for(i=0; i<size; i++){
        display << _codec_list[i]->_name << "/";
    }

    display << ":" << get_stream_direction_str() << std::endl;

    return display.str();
}


