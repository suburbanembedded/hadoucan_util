/**
 * @brief Hadoucan_IO_mgr
 * @author Nicholas Schloss <nicholas.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2020 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD license. See LICENSE for details
*/

#include "hadoucan_util/Hadoucan_IO_mgr.hpp"
#include "hadoucan_util/lawicel_linux_conversions.hpp"
#include "hadoucan_util/lin_can_helper.hpp"
#include "hadoucan_util/error_handling.hpp"

#include <fcntl.h>
#include <iostream>

#include <functional>
#include <vector>
#include <deque>
#include <fstream>
#include <boost/date_time.hpp>


/*----------------------------- HADOUCAN serial port management class ------------------------*/
Hadoucan_IO_mgr::Hadoucan_IO_mgr(bool set_timestamp)
{
	serial_fh = -1;
	timeout = std::chrono::milliseconds(10);
	max_str_length = 140;
	timestamp = set_timestamp;
	if(timestamp)
	{
		max_str_length = 144;
	}

}

Hadoucan_IO_mgr::~Hadoucan_IO_mgr()
{
	end_read_thread_func();
	close_filehandle();
}



bool Hadoucan_IO_mgr::set_termios_attr()
{
	cfmakeraw(&t);
	t.c_lflag &= ~ICANON;
	t.c_cc[VMIN] = 0;
	t.c_cc[VTIME] = 1;
	if(tcsetattr(serial_fh, TCSAFLUSH, &t) != 0)
	{
		printf("Failed to set termios attributes\n");
		return false;
	}
	return true;
}


bool Hadoucan_IO_mgr::set_filehandle(int filehandle){
	if(filehandle < 0)
	{
		printf("Error, invalid filehandle\n");
		return false;
	}
	serial_fh = filehandle;
	if(!set_termios_attr())
	{
		return false;
	}
	read_thread = boost::thread(std::bind(&Hadoucan_IO_mgr::read_thread_func, this));
	return true;
}


bool Hadoucan_IO_mgr::open_filehandle(std::string file_path){
	serial_fh = open(file_path.data(), O_RDWR | O_NOCTTY);
	if(serial_fh < 0)
	{
		printf("Error opening device %s\n", file_path.data());
		std::cout << get_errno_msg() << "\n";
		return false;
	}
	if(!set_termios_attr())
	{
		return false;
	}
	return true;
}


bool Hadoucan_IO_mgr::close_filehandle(){
	int ret = close(serial_fh);
	if(ret < 0)
	{
		printf("Error closing serial port\n");
		return false;
	}
	return true;
}



bool Hadoucan_IO_mgr::send_packet(const std::string& msg)
{
	bool success = true;
	size_t bytes_written = 0;

	do{
		ssize_t ret = write(serial_fh, msg.data(), msg.size());
		if(ret == -1)
		{
			if(errno != EINTR)
			{
				success = false;
				break;
			}
			continue;
		}
		bytes_written += ret;
	}while(bytes_written < msg.size());

	tcflush(serial_fh, TCOFLUSH);
	return success;	
}


bool Hadoucan_IO_mgr::send_packet(canfd_frame frame, size_t mtu)
{
	std::string packet;

	if(!linux_to_lawicel(&frame, mtu, &packet))
	{
		printf("string to CAN frame conversion failed\n");
		return false;
	}

	bool success = true;
	size_t bytes_written = 0;

	do{
		ssize_t ret = write(serial_fh, packet.data(), packet.size());
		if(ret == -1)
		{
			if(errno != EINTR)
			{
				success = false;
				break;
			}
			continue;
		}
		bytes_written += ret;
	}while(bytes_written < packet.size());

	tcflush(serial_fh, TCOFLUSH);
	return success;
}


bool Hadoucan_IO_mgr::read_queue_not_empty() const
{
	return !in_packet_queue.empty();
}


bool Hadoucan_IO_mgr::get_packet(std::chrono::milliseconds max_wait, Linux_can *obj_frame)
{

	std::unique_lock<std::mutex> lock(in_packet_queue_mutex);
	if(!in_packet_queue_condvar.wait_for(lock, max_wait, std::bind(&Hadoucan_IO_mgr::read_queue_not_empty, this)))
	{
		return false;
	}
	obj_frame->fd_frame = in_packet_queue.front()->fd_frame;
	obj_frame->mtu = in_packet_queue.front()->mtu;
	obj_frame->time_Read = in_packet_queue.front()->time_Read;
	in_packet_queue.pop_front();
	return true;
}


void Hadoucan_IO_mgr::begin_read_thread_func()
{
	read_thread = boost::thread(std::bind(&Hadoucan_IO_mgr::read_thread_func, this));
}

void Hadoucan_IO_mgr::read_thread_func()
{

	std::vector<char> buffer;
	buffer.resize(2048);
	while(true)
	{
		boost::this_thread::interruption_point();
		const int nbytes = read(serial_fh, buffer.data(), buffer.size());
		const std::string time_of_read = to_simple_string(boost::posix_time::microsec_clock::local_time());
		if(nbytes == -1)
		{
			if(errno != EINTR)
			{
				printf("Failed to read from serial buffer\n");
				return;
			}
			continue;
		}
		in_byte_queue.insert(in_byte_queue.end(), buffer.data(), buffer.data()+nbytes);

		std::string packet;
		packet.reserve(256);
		while(!in_byte_queue.empty())
		{
			switch(std::toupper(in_byte_queue.front()))
			{
				case 'T':
				case 'D':
				case 'B':
				case 'R':
				{
					break;
				}
				default :
				{
					in_byte_queue.pop_front();
					continue;
				}
			}

			std::deque<char>::iterator iter = std::find(in_byte_queue.begin(), in_byte_queue.end(), '\r');
			if(std::distance(in_byte_queue.begin(), iter) > max_str_length)
			{
				in_byte_queue.pop_front();
				break;
			}
			if(iter == in_byte_queue.end())
			{
				break;
			}

			std::shared_ptr<Linux_can> obj_frame = std::make_shared<Linux_can>();
			if(timestamp)
			{
				packet.assign(in_byte_queue.begin(), iter - 4);
				for(size_t i = 4; i > 0; i--)
				{
					obj_frame->time_Read.assign(iter-4, iter);
				}
			}
			if(!timestamp)
			{
				packet.assign(in_byte_queue.begin(), iter);
				obj_frame->time_Read = time_of_read;
			}

			if(lawicel_to_linux(packet, &(obj_frame->fd_frame), &(obj_frame->mtu)))
			{
				{
					std::lock_guard<std::mutex> lock(in_packet_queue_mutex);
					in_packet_queue.push_back(obj_frame);
				}

				in_packet_queue_condvar.notify_one();
			}
			else
			{
				printf("FAILED LAWICEL\n");
			}
			in_byte_queue.erase(in_byte_queue.begin(), iter);
		} // while in_byte_queue not empty

	} // while no interrupt
}


void Hadoucan_IO_mgr::end_read_thread_func()
{
	read_thread.interrupt();
	read_thread.join();
}


bool Hadoucan_IO_mgr::timestamp_on()
{
	return timestamp;
}