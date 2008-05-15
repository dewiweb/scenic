#include <iostream>
#include <sstream>

#include "sipSingleton.h"
#include "sipPrivate.h"

SipSingleton* SipSingleton::s = 0;

const char* SipSingleton::rx_req(const char *data, unsigned int len) 
{
    std::cerr << "rx_request: " ;
    std::cerr.write(data, len);
    std::cerr << std::endl;

    if (!strncmp(data,"Hello",5))
    {
        return "Break Yourself!";
    }

    return "what?";
}



void SipSingleton::rx_res(const char *data, unsigned int len) 
{
    std::cerr << "rx_response:" ;
    std::cerr.write(data,len);
    std::cerr << std::endl;
}



SipSingleton* SipSingleton::Instance()
{
    if(s == 0)
        s = new SipSingleton();

    return s;
}



int main(int argc, char *argv[])
{

    if(sip_pass_args(argc,argv) < 0)
        return -1;

    sip_init();

    if (argc == 5)
    {
        send_request((char*)"Hello World");
    }


    for (;;)
    {
        static int eventCount;
        if (eventCount += sip_handle_events())
        {
            std::cout << "HANDLED " << eventCount << " EVENTS " << std::endl;
        }
    }

    return 0;
}

