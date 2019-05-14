//
// connection.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2018 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_CONNECTION_HPP
#define HTTP_CONNECTION_HPP

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include "reply.hpp";
#include <boost/enable_shared_from_this.hpp>
#include <string>
#include <vector>

class connection_manager;

/// Represents a single connection from a client.
class connection
  : public boost::enable_shared_from_this<connection>,
    private boost::noncopyable
{
public:
  /// Construct a connection with the given io_service.
  explicit connection(boost::asio::io_service& io_service, connection_manager& manager);

  /// Get the socket associated with the connection.
  boost::asio::ip::tcp::socket& socket();

  /// Start the first asynchronous operation for the connection.
  void start();

  /// Stop all asynchronous operations associated with the connection.
  void stop();

private:
  /// Handle completion of a read operation.
  void handle_read_client(const boost::system::error_code& e,
      std::size_t bytes_transferred);

  /// Handle completion of a query resolve operation
  void handle_resolve(const boost::system::error_code& err,
      tcp::resolver::iterator endpoint_iterator);

  /// Handle completion of a connect operation
  void handle_connect(const boost::system::error_code& e);

  /// Handle completion of a write operation.
  void handle_write_server(const boost::system::error_code& e);

  void handle_read_server(const boost::system::error_code& e);

  /// Handle completion of a write operation.
  void handle_write_client(const boost::system::error_code& e);

  ///-----------------------------------------------------------

  /// parse incoming client request;
  /// modifies class variable address and port.
  void parse_http_request(std::string& request);

  boost::asio::ip::tcp::socket socket_;
  connection_manager& connection_manager_;
  boost::array<char, 8192> buffer_;
  reply reply_;
  std::string addr, port;
  std::string response;
  boost::asio::ip::tcp::socket socket_srv;
  boost::asio::ip::tcp::resolver resolver_;
  boost::asio::streambuf response_buffer;
  
};

typedef boost::shared_ptr<connection> connection_ptr;

#endif // HTTP_CONNECTION_HPP
