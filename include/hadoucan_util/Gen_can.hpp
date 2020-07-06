/**
 * @brief gen_rand_can
 * @author Nicholas Schloss <nicholas.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2020 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD license. See LICENSE for details
*/
#pragma once

#include <linux/can.h>
#include <linux/can/raw.h>

#include <string>
#include <random>
#include <chrono>

class Gen_can
{
	public:
		Gen_can() : rand_type(0, 3), rand_std(0,2047), rand_ext(0,536870911), rand_data(0,255), rng(7985) {}
		bool gen_rand_can(std::string* const can_str);
		bool decimal_to_hex(int hex_length, unsigned int value, std::string* const can_str);

		std::mt19937 rng;
		std::uniform_int_distribution<int> rand_type;
		std::uniform_int_distribution<int> rand_std;
		std::uniform_int_distribution<int> rand_ext;
		std::uniform_int_distribution<int> rand_data;
};

//rng(std::chrono::system_clock::now().time_since_epoch().count())
