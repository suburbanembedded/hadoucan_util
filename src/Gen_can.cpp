/**
 * @brief gen_rand_can
 * @author Nicholas Schloss <nicholas.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2020 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD license. See LICENSE for details
*/

/*
	Plan to add functionality of options specifying frame type and data length in the future,
	will probably make this modular as that will be easy to implement
*/ 

#include "hadoucan_util/Gen_can.hpp"
#include "hadoucan_util/lawicel_linux_conversions.hpp"
#include "common_util/Byte_util.hpp"


#include <sstream>


bool Gen_can::decimal_to_hex(int hex_length, unsigned int value, std::string* const can_str)
{
    char temp[10];
    int ret = snprintf(temp, 10, "%0.*X", hex_length, value);
    if(ret < 0)
    {
    	return false;
    }
    std::string hex_num(temp);
    can_str->append(hex_num);
    return true;
}

bool Gen_can::gen_rand_can(std::string* const can_str)
{
	can_str->clear();
	bool FD_frame = false;
	int id_length = 0;
	size_t dlc_max = 0;
	unsigned int rand_num = rand_type(rng);

	switch(rand_num)
	{
		case 0 : //we'll call this t
			FD_frame = false;
			id_length = 3;
			dlc_max = 8;
			can_str->push_back('t');
			break;
		case 1 : // T
			FD_frame = false;
			id_length = 8;
			dlc_max = 8;
			can_str->push_back('T');
			break;
		case 2 : // d
			FD_frame = true;
			id_length = 3;
			dlc_max = 15;
			can_str->push_back('d');
			break;
		case 3 : // D
			FD_frame = true;
			id_length = 8;
			dlc_max = 15;
			can_str->push_back('D');
			break;
	}

	if(id_length == 8)
	{
		//generate rand_num 0 to 536870911
		rand_num = rand_ext(rng); // 00000000-1FFFFFFF
	}
	if(id_length == 3)
	{
		//generate rand_num 0 to 2047
		rand_num = rand_std(rng); // 000-7FF
	}
	if(!decimal_to_hex(id_length, rand_num, can_str)) // append ID
	{
		return false;
	}

	unsigned int dlc = std::uniform_int_distribution<int>(0,dlc_max)(rng);
	unsigned int num_data_bytes = dlc_to_len(dlc);
	can_str->push_back(Byte_util::nibble_to_hex(dlc)); //specifies data length
	for(unsigned int i = 0; i < num_data_bytes; i++)
	{
		const uint8_t data = rand_data(rng);
		
		can_str->push_back(Byte_util::nibble_to_hex(data >> 4));
		can_str->push_back(Byte_util::nibble_to_hex(data & 0x0F));
	}

	can_str->push_back('\r');
	return true;
}
