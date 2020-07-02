/**
 * @brief Socketcan_IO_mgr
 * @author Nicholas Schloss <nicholas.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2020 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD license. See LICENSE for details
*/
#pragma once 
#include "hadoucan_util/lin_can_helper.hpp"
#include <linux/can.h>
#include <linux/can/raw.h>

#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/socket.h>

#include <chrono>
#include <condition_variable>

#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>


class Socketcan_IO_mgr
{
	public:
		Socketcan_IO_mgr();
		~Socketcan_IO_mgr();

		bool setup_socket(std::string socket_name);
		bool setup_socket(int filehandle);
		bool send_packet(canfd_frame frame, size_t mtu);
		bool get_packet(std::chrono::milliseconds max_wait, Linux_can *obj_frame);
		void begin_read_thread_func();
		void end_read_thread_func();
		std::chrono::milliseconds timeout;
	protected:
		void read_thread_func();
		void write_thread_func();
		bool read_queue_not_empty() const;

		std::deque<std::shared_ptr<Linux_can>> in_packet_queue;

		boost::thread read_thread;
		std::mutex in_packet_queue_mutex;
		std::condition_variable in_packet_queue_condvar;

		int socket_fh;
		ifreq ifr;
		sockaddr_can addr;
		int set_canFD;
};
