//
// connection.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2018 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "connection.hpp"
#include "connection_manager.hpp"
#include <vector>
#include <boost/bind.hpp>
#include <iostream>

using boost::asio::ip::tcp;

/// constructor
connection::connection(boost::asio::io_service& io_service,
    connection_manager& manager)
  : socket_(io_service),
    connection_manager_(manager),
    socket_srv(io_service),
    resolver_(io_service)
{
}

/// get socket
boost::asio::ip::tcp::socket& connection::socket()
{
  return socket_;
}

/// start connection and read from client
void connection::start()
{
  socket_.async_read_some(boost::asio::buffer(buffer_),
      boost::bind(&connection::handle_read_client, shared_from_this(),
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
}

/// stop connection
void connection::stop()
{
  socket_.close();
}

/// handle read from client
void connection::handle_read_client(const boost::system::error_code& e,
    std::size_t bytes_transferred)
{
  if (!e){
    std::string client_data(buffer_.begin(), buffer_.end());
    parse_http_request(client_data);
    if(addr.length > 0 && port.length > 0){
      tcp::resolver::query query(addr, port);
      resolver_.async_resolve(query,
        boost::bind(&connection::handle_resolve, shared_from_this(),
          boost::asio::placeholders::error,
          boost::asio::placeholders::iterator));
    }
    else{
      boost::asio::async_write(socket_, 
        boost::asio::buffer(reply_.bad_request),
        boost::bind(&connection::handle_write_client,
        shared_from_this(),
        boost::asio::placeholders::error));
    }
  }
  if (e != boost::asio::error::operation_aborted)
  {
    connection_manager_.stop(shared_from_this());
  }
}

/// handle query resolution
void connection::handle_resolve(const boost::system::error_code& err,
      tcp::resolver::iterator endpoint_iterator)
{
  if (!err){
    boost::asio::async_connect(socket_srv, endpoint_iterator,
      boost::bind(&connection::handle_connect, shared_from_this(),
      boost::asio::placeholders::error));
  }
  else{
    std::cout << "Error: " << err.message() << "\n";
  }
}

/// handle connection
void connection::handle_connect(const boost::system::error_code& err)
{
  if (!err)
  {
    boost::asio::async_write(socket_srv, boost::asio::buffer(buffer_),
      boost::bind(&connection::handle_write_server, 
      shared_from_this(),
      boost::asio::placeholders::error));
  }
  else
  {
    std::cout << "Error: " << err.message() << "\n";
  }
}

/// handle writing to server
void connection::handle_write_server(const boost::system::error_code& e)
{
  if (!e){
    /// start reading content from server
    boost::asio::async_read(socket_srv, response_buffer,
      boost::asio::transfer_at_least(1),
      boost::bind(&connection::handle_read_server, shared_from_this(),
        boost::asio::placeholders::error));
  }
  else{
    std::cout << "Error: " << e.message() << std::endl;
  }
}

/// handle reading from server
void connection::handle_read_server(const boost::system::error_code& e)
{
  if (!e){
    /// continue reading until eof
  }
}

/// handle writing to client
void connection::handle_write_client(const boost::system::error_code& e)
{
  if (!e)
  {
    // Initiate graceful connection closure.
    boost::system::error_code ignored_ec;
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
  }

  if (e != boost::asio::error::operation_aborted)
  {
    connection_manager_.stop(shared_from_this());
  }
}

void connection::parse_http_request(std::string& request)
{
  // find positions of different delimiter
  int pos1 = request.find("://");
  int pos2 = request.find(":", pos1);
  int pos3 = request.find("/", pos1);
  
  if (pos1 != std::string::npos && pos3 != std::string::npos){
    if (pos2 != std::string::npos){
      addr = request.substr(pos1+3, pos2-(pos1+3));
      port = request.substr(pos2+1, pos3-(pos2+1));
    }
    else{
      addr = request.substr(pos1+3, pos3-(pos1+3));
      port = "80";
    }
  }
}
