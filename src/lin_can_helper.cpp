/**
 * @brief lin_can_helper
 * @author Nicholas Schloss <nicholas.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2020 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD license. See LICENSE for details
*/
#include "hadoucan_util/lin_can_helper.hpp"

#include <linux/can.h>
#include <cstring>

can_frame* Linux_can::to_can()
{
	return (can_frame*)&fd_frame;
}
canfd_frame* Linux_can::to_canfd()
{
	return &fd_frame;
}

can_frame const * Linux_can::to_can() const 
{
	return (can_frame* const)&fd_frame;
}
canfd_frame const * Linux_can::to_canfd() const 
{
	return &fd_frame;
}

bool Linux_can::is_can() const
{
	return mtu == CAN_MTU;
}
bool Linux_can::is_canfd() const
{
	return mtu == CANFD_MTU;
}

void Linux_can::set(const can_frame& in_frame)
{
	mtu = CAN_MTU;
	memcpy(&fd_frame, &in_frame, mtu);
}

void Linux_can::set(const canfd_frame& in_frame)
{
	mtu = CANFD_MTU;
	fd_frame = in_frame;
}

bool Linux_can::set(const canfd_frame& in_frame, size_t in_mtu)
{
	mtu = in_mtu;
	if(mtu == CAN_MTU)
	{
		memcpy(&fd_frame, &in_frame, mtu);
	}
	else if(mtu == CANFD_MTU)
	{
		fd_frame = in_frame;
	}
	else
	{
		return false;
	}
	return true;
}
