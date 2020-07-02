/**
 * @brief lin_can_helper
 * @author Nicholas Schloss <nicholas.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2020 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD license. See LICENSE for details
*/
#pragma once

#include <linux/can.h>
#include <cstdlib>
#include <string>

class Linux_can
{
	public:

	can_frame* to_can();
	canfd_frame* to_canfd();

	can_frame const * to_can() const;
	canfd_frame const * to_canfd() const;

	bool is_can() const;
	bool is_canfd() const;

	void set(const can_frame& in_frame);
	void set(const canfd_frame& in_frame);
	bool set(const canfd_frame& in_frame, size_t in_mtu);

	canfd_frame fd_frame;
	size_t mtu;
	std::string time_Read;
};
