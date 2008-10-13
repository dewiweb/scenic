/* parser.cpp
 * Copyright 2008 Koya Charles & Tristan Matthews
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

/** \file
 *
 *      Command parser functions used in ipcp protocol
 *
 */

#include <sstream>
#include <string.h>
#include <vector>
#include "parser.h"
#include "logWriter.h"
#include "mapMsg.h"

using namespace Parser;

typedef std::string::size_type POS;

std::string Parser::strEsq(const std::string& str)
{
    std::string out;

    for(unsigned int pos=0; pos < str.size(); ++pos)    //for each char in string
    {
        char c = str[pos];                              //copy current character
        if(c == '\\')                                   //if backslash found
            out.append("\\\\");                         //escape it with backslash
        else if(c == '\"')                              //if quotation mark
            out.append("\\\"");                         //escape it with backslash
        else
            out.append(1, c);                           //otherwise pass it through
    }

    return out;                                         //return copy char to output
}


static std::string strUnEsq(const std::string& str)
{
    std::string out;

    //for each char in string
    for(unsigned int pos=0; pos < str.size(); ++pos)
    {
        char c = str[pos];                              //copy first character
        char c2 = str[pos+1];                           //copy second character
        if(c == '\\')                                   //if first character is backslash
        {
            if(c2 == '\\')                              //and second character is
            {                                           //also backslash
                out.append("\\");                       //add one backslash
                ++pos;                                  //increment pos
            }
        }
        else
            out.append(1, c);                           //otherwise copy char to output
    }

    return out;
}


/// returns the position of the trailing quote in a string
/// ignores escaped version
static POS get_end_of_quoted_string(const std::string& str)
{
    //return error if string doesn't start with "
    if(str[0] != '\"'){
        LOG_WARNING("String must start with \".");
        return 0;
    }
    //for each char in string
    for(unsigned int pos=1; pos < str.size(); ++pos)
    {
        if(str[pos] == '\"')                            //if char is " and if
            if(str[pos-1] != '\\')                      //previous char is not escape char
                return pos;                           //return position following "
    }

    LOG_WARNING("String has no terminating \".");
    return 0;
}


bool Parser::stringify(MapMsg& cmd_map, std::string& rstr)
{
    std::stringstream sstr;
    rstr.clear(); 
    //locate "command" and output value to str
    sstr << std::string(cmd_map["command"]) << ":";
    const std::pair<const std::string,StrIntFloat>* it;
    //for each pair in the map
    for(it = cmd_map.begin();
        it != 0; it = cmd_map.next())
    {
        if(it->first != "command")                //Insure it's not the command
        {
            sstr << " " << it->first << "=";                      //Output the key to the str
            switch(it->second.type())                   //Output the value to the str
            {
                case 's':                               //If the value is a string
                                                        //delimit with "
                    sstr << "\"" << strEsq(it->second) << "\"";                //Escape the string contents
                    break;
                
                case 'i':                               //If the value is integer
                                                        //generate string from int
                    sstr << int(it->second);
                    break;
                 
                case 'f':                               //If the value is a float
                                                        //generate string from float
                    sstr << double(it->second);
                    break;

                case 'F':
                    {
                    const std::vector<double> &v = it->second;
                    sstr << v[0];
                    //TODO: parse and tokenize vectors
                    }
                    break;

                default:
                    THROW_ERROR("Command " << it->first << " has unknown type " << it->second.type());
            }       
        }
    }
    rstr = sstr.str();
    return true;
}

void erase_to_end_whitespace(std::string &in)          
{
    POS pos = in.find_first_of(' ');
    in.erase(0,pos);
    pos = in.find_first_not_of(' ');
    if(pos != std::string::npos)
        in.erase(0,pos);
}

bool Parser::tokenize(const std::string& str, MapMsg &cmd_map)
{
    std::string::size_type tok_end;
    std::string lstr = str;                                     //copy in str to local string
    cmd_map.clear();                                            //clear output map

    tok_end = strcspn(lstr.c_str(), ":");                       //search for ":"
    if(tok_end == lstr.size())                                  //if : not found return error
        THROW_ERROR("No command found.");

    cmd_map["command"] = lstr.substr(0, tok_end);               //insert command into map
    erase_to_end_whitespace(lstr);                              //set lstring beyond command
    if(lstr.empty())
        return true;


    //loop until break or return
    for(;;)
    {
        tok_end = lstr.find_first_of('=');                   //search of "="
        if(tok_end == std::string::npos)                              //if not found no more
            break;                                              //key=value pairs so break loop
        std::string key_str = lstr.substr(0,tok_end);
        ++tok_end;
        lstr.erase(0,tok_end);                                   //lstr points to value
        if(lstr[0] == '\"')                                     //if value looks like a string
        {
            POS end_quote = get_end_of_quoted_string(lstr);     //find end of "
            if(end_quote == 0)                                        //if value not string
                THROW_ERROR("No end of quote found.");

            std::string quote = lstr.substr(1, end_quote-1);    //strip begin and end quotes
            quote = strUnEsq(quote);                            //clean any escape backslashes
            LOG_DEBUG("KEY:" << key_str << "VAL:" << quote );

            cmd_map[key_str] = quote;                           //insert key,value into map
            lstr.erase(0,end_quote);
            erase_to_end_whitespace(lstr);                      //set lstring beyond command
        }
        else{
            std::vector<double> vd;
            std::vector<int> vi;
            for(;;)
            {
                POS pos = lstr.find_first_of(" =");
                if(lstr[pos] == '=')
                    break;
                std::stringstream stream(lstr.substr(0,pos));                     //string of value
                lstr.erase(0,pos);
                erase_to_end_whitespace(lstr);                      //set lstring beyond command
                if(stream.str().find_first_of('=') != std::string::npos)
                    break;
                if(!lstr.size())
                    break;
                
                //if value does not contain . it is an int
                if(stream.str().find_first_of('.') == std::string::npos)
                {
                    int temp_i;
                    stream >> temp_i;                               //convert str to int
                    vi.push_back(temp_i);                           //stor val
                }
                else    //value contains . thus it is a float
                {                                                   
                    float temp_f;
                    stream >> temp_f;                               //convert str to float
                    vd.push_back(temp_f);                           //stor val
                }
                if(lstr.find_first_not_of(' ') == std::string::npos) //Only whitespace remains
                    break;
            }
            if(vd.size() > 0 && vi.size() > 0)
                THROW_ERROR("Cannot mix types in arrays");
            if(vd.size() == 1)
                cmd_map[key_str] = vd[0];
            else if (vd.size() > 1)
                cmd_map[key_str] = vd[0];
            //TODO replace above with ->  cmd_map[key_str] = vd;
            if(vi.size() == 1)
                cmd_map[key_str] = vi[0];
            else if (vi.size() > 1)
                cmd_map[key_str] = vi[0];
            //TODO replace above with ->  cmd_map[key_str] = vi;
        }
        if(lstr.find_first_not_of(' ') == std::string::npos)    //Only whitespace remains
            break;                                              
            
    }

    return true;
}


