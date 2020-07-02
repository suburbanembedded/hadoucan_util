/**
 * @brief Hadoucan_IO_mgr
 * @author Nicholas Schloss <nicholas.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2020 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD license. See LICENSE for details
*/
#pragma once 

#include "hadoucan_util/lin_can_helper.hpp"

#include <linux/can.h>
#include <linux/can/raw.h>

#include <chrono>
#include <condition_variable>
#include <termios.h>

#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>


class Hadoucan_IO_mgr
{
	public:
		Hadoucan_IO_mgr(bool set_timestamp = false);
		~Hadoucan_IO_mgr();

		bool open_filehandle(std::string file_path);
		bool set_filehandle(int filehandle);
		bool close_filehandle();
		bool send_packet(const std::string& msg);
		bool send_packet(canfd_frame frame, size_t mtu);
		bool get_packet(std::chrono::milliseconds max_wait, Linux_can *obj_frame);
		void begin_read_thread_func();
		void end_read_thread_func();
		bool timestamp_on();
		std::chrono::milliseconds timeout;
	protected:
		void read_thread_func();
		bool read_queue_not_empty() const;
		bool set_termios_attr();

		std::deque<char> in_byte_queue;
		std::deque<std::shared_ptr<Linux_can>> in_packet_queue;

		boost::thread read_thread;
		std::mutex in_packet_queue_mutex;
		std::condition_variable in_packet_queue_condvar;

		int serial_fh;
		termios t;

		bool timestamp;
		unsigned int max_str_length;
};
