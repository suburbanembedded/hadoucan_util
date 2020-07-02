/**
 * @brief lawicel_linux_conversions
 * @author Nicholas Schloss <nicholas.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2020 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD license. See LICENSE for details
*/

#include "hadoucan_util/lawicel_linux_conversions.hpp"
#include "common_util/Byte_util.hpp"

#include <iostream>

bool len_to_dlc_str(int can_dlc, std::string* const out_str){

    if(can_dlc < 0)
    {
        return false;
    }

    if(can_dlc < 9)
    {
    	out_str->push_back(Byte_util::nibble_to_hex(can_dlc));
    }
    else
    {
        switch(can_dlc)
        {
            case 12:
                out_str->push_back('9');
                break;
            case 16:
                out_str->push_back('A');
                break;
            case 20:
                out_str->push_back('B');
                break;
            case 24:
                out_str->push_back('C');
                break;
            case 32:
                out_str->push_back('D');
                break;
            case 48:
                out_str->push_back('E');
                break;
            case 64:
                out_str->push_back('F');
                break;
            default:
                return false;
        }
    }
    return true;
}


int dlc_to_len(int dlc){
    if(dlc > -1 && dlc < 9)
    {
        return dlc;
    }
    switch(dlc)
    {
        case 9:
            return 12;
        case 10:
            return 16;
        case 11:
            return 20;
        case 12:
            return 24;
        case 13:
            return 32;
        case 14:
            return 48;
        case 15:
            return 64;
    }
    return -1;
}

bool linux_to_lawicel(canfd_frame const * const fd_frame_ptr, const size_t mtu, std::string* const out_str)
{
    char cmd = '\0';
    char const * fmt_string = "";
    canid_t id_mask = CAN_SFF_MASK;
    out_str->clear();
    if(fd_frame_ptr->can_id & CAN_ERR_FLAG)
    {
        return false;
    }

    if(mtu == sizeof(struct canfd_frame))
    {
        if(fd_frame_ptr->can_id & CAN_RTR_FLAG)
        {
            return false;
        }
        if(fd_frame_ptr->flags & CANFD_ESI)
        {
            printf("flags CANFD_ESI is set\n");
            return false;
        }
        if(fd_frame_ptr->flags & CANFD_BRS)
        {
            cmd = 'b';
        }
        else
        {
            cmd = 'd';
        }

    }
    else{
        if(fd_frame_ptr->can_id & CAN_RTR_FLAG)
        {
            cmd = 'r';
        }
        else{
            cmd = 't';
        }
    }

    if(fd_frame_ptr->can_id & CAN_EFF_FLAG){
        id_mask = CAN_EFF_MASK;
        // Extended Frame
        cmd = std::toupper(cmd);
        fmt_string = "%08X";
    }
    else{
     fmt_string = "%03X";   
    }

    std::array<char, 10> temp_ID;
    temp_ID[0] = cmd;
    snprintf(temp_ID.data()+1, temp_ID.size()-1, fmt_string, fd_frame_ptr->can_id & id_mask);
    out_str->append(temp_ID.data());

    if(!len_to_dlc_str(fd_frame_ptr->len, out_str))
    {
    	return false;
    }

    if(fd_frame_ptr->len > 0)
    {
        uint8_t const * const data_ptr = (mtu == CANFD_MTU) ? (fd_frame_ptr->data) : (reinterpret_cast<can_frame const * const>(fd_frame_ptr)->data);
        for(int i = 0; i < fd_frame_ptr->len; i++)
        {
            std::array<char, 3> temp_data;
            snprintf(temp_data.data(), temp_data.size(), "%02X", data_ptr[i]);
            out_str->append(temp_data.data());
        }
    }
    
    out_str->push_back('\r');
    return true;
}

bool lawicel_to_linux(const std::string& str, canfd_frame* const out_frame, size_t* const out_mtu)
{
    memset(out_frame, 0, sizeof(*out_frame));
    switch(std::toupper(str[0]))
    {
        case 'B':
            out_frame->flags |= CANFD_BRS;
            *out_mtu = CANFD_MTU;
            break;
        case 'D':
            *out_mtu = CANFD_MTU;
            break;
        case 'R':
            out_frame->can_id |= CAN_RTR_FLAG;
            *out_mtu = CAN_MTU;
            break;
        case 'T':
            *out_mtu = CAN_MTU;
            break;       
        default: 
            //Handles case of incorrect Frame type
        	printf("Incorrect frame type specifier\n");
            return false;
    }

    char const * str_data_start = nullptr;
    char const * str_dlc_start = nullptr;
    uint8_t* out_data_start = nullptr;
    if(std::isupper(str[0]))
    {
        unsigned int temp_id = 0;
        int ret = sscanf(str.data()+1, "%8X", &temp_id);
        if(ret != 1)
        {
        	printf("Failed scanning extended id\n");
            return false;
        }
        out_frame->can_id |= temp_id & CAN_EFF_MASK;
        out_frame->can_id |= CAN_EFF_FLAG;

        str_data_start = str.data() + 1 + 8 + 1;
        str_dlc_start = str.data() + 1 + 8;
        out_data_start = out_frame->data;
    }
    else{
        unsigned int temp_id = 0;
        int ret = sscanf(str.data()+1, "%3X", &temp_id);
        if(ret != 1)
        {
        	printf("Failed scanning standard length id\n");
            return false;
        }
        out_frame->can_id |= temp_id & CAN_SFF_MASK;

        str_data_start = str.data() + 1 + 3 + 1;
        str_dlc_start = str.data() + 1 + 3;
        out_data_start = reinterpret_cast<can_frame* const>(out_frame)->data;
    }

    int data_len = 0;
    int ret = sscanf(str_dlc_start, "%1X", &data_len);
    if(ret != 1)
    {
    	printf("Failed scanning dlc specifier\n");
        return false;
    }

    if(dlc_to_len(data_len) != -1)
    {
        out_frame->len = dlc_to_len(data_len);
    }
    else{
        // Handles case of bad payload size
        printf("Failed due to incorrect data length\n");
        return false;
    }

    for(int i = 0; i < out_frame->len; i++)
    {
        if(!Byte_util::hex_to_byte(str_data_start + i*2, out_data_start + i))
        {
            std::cout << str.data() << "\n";
        	printf("hex to byte failed\n");
            return false;
        }
    }

    return true;
}
