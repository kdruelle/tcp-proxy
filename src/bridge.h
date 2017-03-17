/*******************************************************************************
 ** 
 ** (C) 2011 Kevin Druelle <kevin@druelle.info>
 **
 ** this software is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU Lesser General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 ** 
 ** This software is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 ** 
 ** You should have received a copy of the GNU General Public License
 ** along with this software.  If not, see <http://www.gnu.org/licenses/>.
 ** 
 ******************************************************************************/

#ifndef __BRIDGE_H__
#define __BRIDGE_H__

#include <fstream>

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/thread/mutex.hpp>


namespace tcp_proxy{

class bridge : public boost::enable_shared_from_this<bridge>{
public:
    typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_t;
    typedef boost::shared_ptr<bridge> ptr_t;

    class acceptor{
    public:
        acceptor(boost::asio::io_service& io_service, const std::string& local_host, unsigned short local_port, const std::string& upstream_host, unsigned short upstream_port);
        bool accept_connections();

        void set_upstream_ssl(const std::string &);
        void set_downstream_ssl(const std::string &, const std::string &);

    private:
        void handle_accept(const boost::system::error_code& error);

        boost::asio::io_service& io_service_;
        boost::asio::ip::address_v4 localhost_address;
        boost::asio::ip::tcp::acceptor acceptor_;
        ptr_t session_;
        unsigned short upstream_port_;
        std::string upstream_host_;

        std::string _ca_file;
        std::string _key_file;
        std::string _cert_file;
    };

    bridge(boost::asio::io_service& ios);
    socket_t & downstream_socket();
    socket_t & upstream_socket();

    void start(const std::string & upstream_host_, unsigned short upstream_port_);
    void handle_upstream_connect(const boost::system::error_code& error_);
    void init_upstream_ssl(const std::string &);
    void init_downstream_ssl(const std::string &,const std::string &, const std::string &);

private:
    void handle_downstream_write(const boost::system::error_code & error_);
    void handle_downstream_read(const boost::system::error_code & error_, const size_t & bytes_transferred_);
    void handle_upstream_write(const boost::system::error_code & error_);
    void handle_upstream_read(const boost::system::error_code & error_, const size_t & bytes_transferred_);
    void handle_downstream_handshake(boost::system::error_code const &e);
    void handle_upstream_handshake(boost::system::error_code const &e);
    void close();

    std::string _upstream_host;
    unsigned int _upstream_port;

    boost::asio::io_service &_ios;
    boost::asio::ssl::context _downstream_context;
    boost::shared_ptr<socket_t> _downstream_socket;
    boost::asio::ssl::context _upstream_context;
    boost::shared_ptr<socket_t> _upstream_socket;

    bool _upstream_ssl;
    bool _downstream_ssl;

    enum { max_data_length = 8192 }; //8KB
    unsigned char downstream_data_[max_data_length];
    unsigned char upstream_data_[max_data_length];

    boost::mutex mutex_;

};

}

#endif /* !__BRIDGE_H__ */

