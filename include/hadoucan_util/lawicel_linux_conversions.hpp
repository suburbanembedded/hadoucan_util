/**
 * @brief lawicel_linux_conversions
 * @author Nicholas Schloss <nicholas.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2020 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD license. See LICENSE for details
*/
#include "common_util/Byte_util.hpp"

#include <cstring>
#include <linux/can.h>
#include <linux/can/raw.h>

bool len_to_dlc_str(int can_dlc, std::string* const out_str);


int dlc_to_len(int dlc);


bool linux_to_lawicel(canfd_frame const * const fd_frame_ptr, const size_t mtu,
    std::string* const out_str);


bool lawicel_to_linux(const std::string& str, canfd_frame* const out_frame,
    size_t* const out_mtu);

std::string hex_to_ascii(std::string hex_str);
