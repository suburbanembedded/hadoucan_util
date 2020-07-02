/**
 * @brief Socketcan_IO_mgr
 * @author Nicholas Schloss <nicholas.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2020 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD license. See LICENSE for details
*/

#include "hadoucan_util/Socketcan_IO_mgr.hpp"
#include "hadoucan_util/error_handling.hpp"
#include "hadoucan_util/lawicel_linux_conversions.hpp"
#include "hadoucan_util/lin_can_helper.hpp"

#include <functional>
#include <vector>

#include <boost/date_time.hpp>
/*----------------------------- SocketCAN socket management class ------------------------*/
Socketcan_IO_mgr::Socketcan_IO_mgr()
{
  socket_fh = -1;
  set_canFD = -1;
  timeout = std::chrono::milliseconds(10);
}

Socketcan_IO_mgr::~Socketcan_IO_mgr()
{
  end_read_thread_func();
  close(socket_fh);
}




bool Socketcan_IO_mgr::setup_socket(std::string socket_name)
{
  socket_fh = socket(PF_CAN, SOCK_RAW, CAN_RAW);
  if(socket_fh < 0)
  {
    printf("Error opening socket\n");
    return false;
  }
  memset(&ifr, 0, sizeof(ifr));;
  strcpy(ifr.ifr_name, socket_name.data());
  ioctl(socket_fh, SIOCGIFINDEX, &ifr);

  memset(&addr, 0, sizeof(addr));
  addr.can_family = AF_CAN;
  addr.can_ifindex = ifr.ifr_ifindex;
  set_canFD = 1;
  if(setsockopt(socket_fh, SOL_CAN_RAW, CAN_RAW_FD_FRAMES,
  &set_canFD, sizeof(set_canFD)))
  {
    printf("Error, setting socket options failed\n");
    return false;
  }

  if(ioctl(socket_fh, SIOCGIFMTU, &ifr) < 0)
  {
    printf("Error, ioctl failed\n");
    return false;
  }
  int ret = bind(socket_fh, (struct sockaddr *)&addr, sizeof(addr));
  if(ret < 0)
  {
    printf("Error, bind failed\n");
    return false;
  }
  return true;
}

bool Socketcan_IO_mgr::setup_socket(int filehandle)
{
  if(filehandle < 0)
  {
    printf("Error, invalid filehandle\n");
    return false;
  }
  socket_fh = filehandle;
  return true;
}




bool Socketcan_IO_mgr::send_packet(canfd_frame frame, size_t mtu)
{
  ssize_t ret = write(socket_fh, &frame, mtu);
  if(ret == -1)
  {
    abort();
    printf("Error, write to socket failed\n");
    return false;
  }

  if(ret != mtu)
  {
    abort();
    printf("Error, write size not equal to frame size\n");
    return false;
  }
  return true;
}



bool Socketcan_IO_mgr::read_queue_not_empty() const
{
  return !in_packet_queue.empty();
}

void Socketcan_IO_mgr::begin_read_thread_func()
{
  read_thread = boost::thread(std::bind(&Socketcan_IO_mgr::read_thread_func, this));
}

void Socketcan_IO_mgr::end_read_thread_func()
{
  read_thread.interrupt();
  read_thread.join();
}



bool Socketcan_IO_mgr::get_packet(std::chrono::milliseconds max_wait, Linux_can *obj_frame)
{
  std::unique_lock<std::mutex> lock(in_packet_queue_mutex);
  if(!in_packet_queue_condvar.wait_for(lock, max_wait, std::bind(&Socketcan_IO_mgr::read_queue_not_empty, this)))
  {
    return false;
  }
  obj_frame->fd_frame = in_packet_queue.front()->fd_frame;
  obj_frame->mtu = in_packet_queue.front()->mtu;
  obj_frame->time_Read = in_packet_queue.front()->time_Read;
  in_packet_queue.pop_front();
  return true;
}


void Socketcan_IO_mgr::read_thread_func()
{
  while(true)
  {
    boost::this_thread::interruption_point();
    std::shared_ptr<Linux_can> obj_frame = std::make_shared<Linux_can>();
    int nbytes = read(socket_fh, &obj_frame->fd_frame, sizeof(canfd_frame));
    std::string time_of_read = to_simple_string(boost::posix_time::microsec_clock::local_time());
    if(nbytes == -1)
    {
      if(errno != EINTR)
      {
        printf("Error reading from socket\n");
        return;
      }
      continue;
    }
    if(nbytes > sizeof(canfd_frame))
    {
      continue;
    }
    switch(nbytes)
    {
      case sizeof(canfd_frame) :
        obj_frame->mtu = nbytes;
      case sizeof(can_frame) :
        obj_frame->mtu = nbytes;
        obj_frame->time_Read = time_of_read;
        {
          std::lock_guard<std::mutex> lock(in_packet_queue_mutex);
          in_packet_queue.push_back(obj_frame);
        }
        in_packet_queue_condvar.notify_one();
        break;

      default :
        break;
    }
  }

}