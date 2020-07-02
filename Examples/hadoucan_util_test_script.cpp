/**
 * @brief hadoucan_IO_mgr_test_script
 * @author Nicholas Schloss <nicholas.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2020 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD license. See LICENSE for details
*/
#include "hadoucan_util/lawicel_linux_conversions.hpp"
#include "hadoucan_util/Hadoucan_IO_mgr.hpp"
#include "hadoucan_util/Socketcan_IO_mgr.hpp"
#include "hadoucan_util/lin_can_helper.hpp"
#include "hadoucan_util/error_handling.hpp"
#include "hadoucan_util/Gen_can.hpp"

#include <linux/can.h>
#include <linux/can/raw.h>

#include <iostream>

class Test
{
    public:
       void Create_Test_Frames();
        std::vector<std::string> test_strings;
        int num_packets;
};

Hadoucan_IO_mgr *serial_mgr;
Socketcan_IO_mgr *sock_mgr;
Test *thisTest;

void Test::Create_Test_Frames()
{
    Gen_can generate_can;
    std::string packet;
    for(unsigned int i = 0; i < num_packets; i++)
    {
        generate_can.gen_rand_can(&packet);
        thisTest->test_strings.push_back(packet);
    }
}


void socket_dump()
{
    bool ret = true;
    Linux_can frame_obj;
    std::string packet;
    frame_obj.mtu = 0;
    while(true)
    {
        ret = sock_mgr->get_packet(sock_mgr->timeout, &frame_obj);
        if(ret == false)
        {
            continue;
        }
        linux_to_lawicel(&frame_obj.fd_frame, frame_obj.mtu, &packet);
        std::cout <<  frame_obj.time_Read.data() << "\t";
        std::cout << packet.data() << "\n";
    }   

}


int write_to_socket()
{
    Linux_can frame_obj;
    int suc_pack_sent = 0;
    for(int i = 0; i < thisTest->num_packets; i++)
    {
        if(!lawicel_to_linux(thisTest->test_strings[i], &frame_obj.fd_frame, &frame_obj.mtu)){
            printf("Lawicel conversion failed\n");
            return -1;
        }
        if(sock_mgr->send_packet(frame_obj.fd_frame, frame_obj.mtu))
        {
            suc_pack_sent++;
        }
        usleep(250);
    }
    return suc_pack_sent;
}


void hadoucan_dump(int packets_to_recv)
{
    Linux_can frame_obj;
    frame_obj.mtu = 0;
    std::string packet;
    bool ret = true;
    int packet_count = 0;
    int count = 0;
    while(true)
    {
        ret = serial_mgr->get_packet(serial_mgr->timeout, &frame_obj);
        count++;
        if(ret == false)
        {
            if(count > packets_to_recv+10)
            {
                break;
            }
            continue;
        }
        packet_count++;
        linux_to_lawicel(&frame_obj.fd_frame, frame_obj.mtu, &packet);
        for(unsigned int i = 0; i < thisTest->test_strings.size(); i++)
        {
            if(packet == thisTest->test_strings[i])
            {
                thisTest->test_strings.erase(thisTest->test_strings.begin() + i);
            }
        }
        std::cout << frame_obj.time_Read.data() << "\t";
        std::cout << packet.data() << "\n";
    } 
    std::cout << packet_count << std::endl;
}


int write_to_hadoucan()
{
    int suc_pack_sent = 0;
    for(int i = 0; i < thisTest->num_packets; i++)
    {
        if(serial_mgr->send_packet(thisTest->test_strings[i]))
        {
            suc_pack_sent++;
        }
        usleep(250);
    }
    return suc_pack_sent;
}



int main(int argc, char** argv){

// Set up Hadoucan management class
    serial_mgr = new  Hadoucan_IO_mgr(true);
    if(!serial_mgr->open_filehandle("/dev/ttyACM0"))
    {
        return -1;
    }

// Set up Socketcan management class
    sock_mgr = new Socketcan_IO_mgr();
    if(!sock_mgr->setup_socket("can0"))
    {
        return -1;
    }

// Create test packets using Gen_can
    thisTest = new Test();
    thisTest->num_packets = 10000;
    thisTest->Create_Test_Frames();



/*--------------- Send from HadouCAN ----------------------*/
/*    sock_mgr->begin_read_thread_func();
    int suc_pack_sent = write_to_hadoucan(packets_to_send);
    socket_dump();*/
/*---------------------------------------------------------*/


/*--------------- Send from SocketCAN ----------------------*/   
    serial_mgr->begin_read_thread_func();
    int suc_pack_sent = write_to_socket();
    if(suc_pack_sent == -1)
    {
        return -1;
    }
/*---------------------------------------------------------*/

    hadoucan_dump(suc_pack_sent);
    for(unsigned int i = 0; i < thisTest->test_strings.size(); i++)
    {
        std::cout << "Failed: " << thisTest->test_strings[i].data() << "\n";
    }

    delete serial_mgr;
    delete sock_mgr;
    delete thisTest;
    return 1;
}
