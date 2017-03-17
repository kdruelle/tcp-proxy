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

#include <boost/bind.hpp>
#include <boost/make_shared.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>

#include "hexprint.h"
#include "bridge.h"


static std::string format_datetime(boost::posix_time::ptime pt)
{
    std::string s;
    std::ostringstream datetime_ss;
    boost::posix_time::time_facet * p_time_output = new boost::posix_time::time_facet;
    std::locale special_locale (std::locale(""), p_time_output);
    datetime_ss.imbue (special_locale);
    (*p_time_output).format("%Y%m%d%H%M%S.%f");
    datetime_ss << pt;
    s = datetime_ss.str().c_str();

    return s;
}


tcp_proxy::bridge::bridge(boost::asio::io_service & ios_)
    : _ios(ios_),
      _downstream_context(boost::asio::ssl::context::sslv23),
      _upstream_context(boost::asio::ssl::context::sslv23),
      _downstream_ssl(false),
      _upstream_ssl(false)
{
    this->_upstream_socket = boost::make_shared<socket_t>(boost::ref(this->_ios), boost::ref(this->_upstream_context));
    this->_downstream_socket = boost::make_shared<socket_t>(boost::ref(this->_ios), boost::ref(this->_downstream_context));
}

tcp_proxy::bridge::socket_t & tcp_proxy::bridge::downstream_socket(){
    return *this->_downstream_socket;
}

tcp_proxy::bridge::socket_t & tcp_proxy::bridge::upstream_socket(){
    return *this->_upstream_socket;
}

void tcp_proxy::bridge::init_upstream_ssl(const std::string &_ca){
    this->_upstream_context.load_verify_file(_ca);
    this->_upstream_context.set_options(boost::asio::ssl::context::default_workarounds
                               | boost::asio::ssl::context::no_sslv2
                               | boost::asio::ssl::context::no_sslv3);
    this->_upstream_ssl = true;
    this->_upstream_socket = boost::make_shared<socket_t>(boost::ref(this->_ios), boost::ref(this->_upstream_context));
    this->_upstream_socket->set_verify_mode(boost::asio::ssl::verify_peer);
}

void tcp_proxy::bridge::init_downstream_ssl(const std::string &_ca, const std::string & _key, const std::string & _cert){
    this->_downstream_context.set_options(boost::asio::ssl::context::default_workarounds);
    SSL_CTX_set_options(this->_downstream_context.native_handle(), 
          SSL_OP_NO_SSLv2
        | SSL_OP_TLS_ROLLBACK_BUG
        | SSL_OP_SINGLE_DH_USE
        | SSL_OP_CIPHER_SERVER_PREFERENCE
        );

    this->_downstream_context.use_certificate_chain_file(_ca);
    this->_downstream_context.use_certificate_file(_cert, boost::asio::ssl::context::pem);
    this->_downstream_context.use_private_key_file(_key, boost::asio::ssl::context::pem);
    //this->_context.use_tmp_dh_file(this->_map.find("ssl_dhparam")->second);

    SSL_CTX_set_cipher_list(this->_downstream_context.native_handle(), 
        "TLSv1.2:"
        "TLSv1:"
        "SSLv3"
        "!aPSK:"
        "!aDSS:"
        "!aECDSA"
        "!RC4:"
        "!aNULL:"
        "!eNULL"
    );

    EC_KEY * ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
    SSL_CTX_set_tmp_ecdh(this->_downstream_context.native_handle(), ecdh);
    EC_KEY_free(ecdh);
    this->_downstream_ssl = true;
    this->_downstream_socket = boost::make_shared<socket_t>(boost::ref(this->_ios), boost::ref(this->_downstream_context));
}

void tcp_proxy::bridge::start(const std::string& upstream_host, unsigned short upstream_port){
    this->_upstream_host = upstream_host;
    this->_upstream_port = upstream_port;
    if(this->_downstream_ssl){
        return this->_downstream_socket->async_handshake(boost::asio::ssl::stream_base::server,
            boost::bind(&tcp_proxy::bridge::handle_downstream_handshake, this->shared_from_this(), boost::asio::placeholders::error));
        
    }
    this->_upstream_socket->next_layer().async_connect(
        boost::asio::ip::tcp::endpoint(
            boost::asio::ip::address::from_string(upstream_host),
            upstream_port),
        boost::bind(&bridge::handle_upstream_connect,
            shared_from_this(),
            boost::asio::placeholders::error)
        );
}

void tcp_proxy::bridge::handle_upstream_connect(const boost::system::error_code& error)
{
    if (error){
        std::cout << "error tcp_proxy::bridge::handle_upstream_connect : " << error.message() << std::endl;
        return close();
    }
    std::string datetime = format_datetime(boost::posix_time::microsec_clock::universal_time());

    if(this->_upstream_ssl){
        return this->_upstream_socket->async_handshake(boost::asio::ssl::stream_base::client,
                boost::bind(&tcp_proxy::bridge::handle_upstream_handshake, this->shared_from_this(),
                boost::asio::placeholders::error));
    }

    this->_upstream_socket->next_layer().async_read_some(
                                           boost::asio::buffer(upstream_data_,max_data_length),
                                           boost::bind(&bridge::handle_upstream_read,
                                                       shared_from_this(),
                                                       boost::asio::placeholders::error,
                                                       boost::asio::placeholders::bytes_transferred));
    if(this->_downstream_ssl){
    this->_downstream_socket->async_read_some(
        boost::asio::buffer(downstream_data_,max_data_length),
            boost::bind(&bridge::handle_downstream_read,
                shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }else{
    this->_downstream_socket->next_layer().async_read_some(
        boost::asio::buffer(downstream_data_,max_data_length),
            boost::bind(&bridge::handle_downstream_read,
                shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }
}

void tcp_proxy::bridge::handle_downstream_handshake(const boost::system::error_code& error){
    if(error){
        std::cout << "error tcp_proxy::bridge::handle_downstream_handshake : " << error.message() << std::endl;
        int i = 0;
        const char * cipher = NULL;
        std::string cipher_list;
        while((cipher = SSL_get_cipher_list(this->_downstream_socket->native_handle(), i)) != NULL){
            cipher_list.append(cipher);
            cipher_list.append(":");
            i++;
        }
        std::cout << "SSL loaded ciphers : " << cipher_list << std::endl;
        return;
    }
    this->_upstream_socket->next_layer().async_connect(
        boost::asio::ip::tcp::endpoint(
            boost::asio::ip::address::from_string(this->_upstream_host),
            this->_upstream_port),
        boost::bind(&bridge::handle_upstream_connect,
            shared_from_this(),
            boost::asio::placeholders::error)
        );
}

void tcp_proxy::bridge::handle_upstream_handshake(const boost::system::error_code& error){
    if(error) return;
    this->_upstream_socket->async_read_some(
        boost::asio::buffer(upstream_data_,max_data_length),
            boost::bind(&bridge::handle_upstream_read,
                shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));

    if(this->_downstream_ssl){
        this->_downstream_socket->async_read_some(
            boost::asio::buffer(downstream_data_,max_data_length),
                boost::bind(&bridge::handle_downstream_read,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
    }else{
        this->_downstream_socket->next_layer().async_read_some(
            boost::asio::buffer(downstream_data_,max_data_length),
                boost::bind(&bridge::handle_downstream_read,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
    }
}

void tcp_proxy::bridge::handle_downstream_write(const boost::system::error_code& error)
{
    if (error) close();

    if(this->_upstream_ssl){
        this->_upstream_socket->async_read_some(
            boost::asio::buffer(upstream_data_,max_data_length),
                boost::bind(&bridge::handle_upstream_read,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
    }else{
        this->_upstream_socket->next_layer().async_read_some(
            boost::asio::buffer(upstream_data_,max_data_length),
                boost::bind(&bridge::handle_upstream_read,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
    }
}

void tcp_proxy::bridge::handle_downstream_read(const boost::system::error_code& error,
                            const size_t& bytes_transferred)
{
    if (error) return close();

    std::cout << std::endl << "Downstream -> Upstream" << std::endl;
    print_payload(reinterpret_cast<unsigned char*>(&downstream_data_[0]),static_cast<std::streamsize>(bytes_transferred));

    if(this->_upstream_ssl){
        boost::asio::async_write(*this->_upstream_socket,
            boost::asio::buffer(downstream_data_,bytes_transferred),
            boost::bind(&bridge::handle_upstream_write,
                shared_from_this(),
                boost::asio::placeholders::error));
    }else{
        boost::asio::async_write(this->_upstream_socket->next_layer(),
            boost::asio::buffer(downstream_data_,bytes_transferred),
            boost::bind(&bridge::handle_upstream_write,
                shared_from_this(),
                boost::asio::placeholders::error));
    }
}

void tcp_proxy::bridge::handle_upstream_write(const boost::system::error_code& error)
{
    if (error) return close();

    if(this->_downstream_ssl){
        this->_downstream_socket->async_read_some(
            boost::asio::buffer(downstream_data_,max_data_length),
            boost::bind(&bridge::handle_downstream_read,
                shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }else{
        this->_downstream_socket->next_layer().async_read_some(
            boost::asio::buffer(downstream_data_,max_data_length),
            boost::bind(&bridge::handle_downstream_read,
                shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }
}

void tcp_proxy::bridge::handle_upstream_read(const boost::system::error_code& error,
                          const size_t& bytes_transferred)
{
    if (error) return close();
    
    std::cout << std::endl << "Upstream -> Downstream" << std::endl;
    print_payload(reinterpret_cast<unsigned char*>(&upstream_data_[0]),static_cast<std::streamsize>(bytes_transferred));
    
    if(this->_downstream_ssl){
        boost::asio::async_write(*this->_downstream_socket,
            boost::asio::buffer(upstream_data_,bytes_transferred),
            boost::bind(&bridge::handle_downstream_write,
                shared_from_this(),
                boost::asio::placeholders::error));
    }else{
        boost::asio::async_write(this->_downstream_socket->next_layer(),
            boost::asio::buffer(upstream_data_,bytes_transferred),
            boost::bind(&bridge::handle_downstream_write,
                shared_from_this(),
                boost::asio::placeholders::error));
    }
}

void tcp_proxy::bridge::close()
{
    boost::mutex::scoped_lock lock(mutex_);

    if (this->_downstream_socket->next_layer().is_open())
    {
       this->_downstream_socket->next_layer().close();
    }

    if (this->_upstream_socket->next_layer().is_open())
    {
        this->_upstream_socket->next_layer().close();
    }

}








tcp_proxy::bridge::acceptor::acceptor(boost::asio::io_service& io_service,
                  const std::string& local_host, unsigned short local_port,
                  const std::string& upstream_host, unsigned short upstream_port)
         : io_service_(io_service),
           localhost_address(boost::asio::ip::address_v4::from_string(local_host)),
           acceptor_(io_service_,boost::asio::ip::tcp::endpoint(localhost_address,local_port)),
           upstream_port_(upstream_port),
           upstream_host_(upstream_host)
         {
         }


void tcp_proxy::bridge::acceptor::set_upstream_ssl(const std::string & _ca){
    this->_ca_file = _ca;
}

void tcp_proxy::bridge::acceptor::set_downstream_ssl(const std::string & _key, const std::string & _cert){
    this->_key_file = _key;
    this->_cert_file = _cert;
}

bool tcp_proxy::bridge::acceptor::accept_connections()
{
    try
    {

        this->session_ = boost::shared_ptr<bridge>(new bridge(io_service_));

        if(!this->_ca_file.empty()) 
            this->session_->init_upstream_ssl(this->_ca_file);

        if(!this->_key_file.empty())
            this->session_->init_downstream_ssl(this->_ca_file, this->_key_file, this->_cert_file);


        acceptor_.async_accept(session_->downstream_socket().next_layer(),
                               boost::bind(&acceptor::handle_accept,
                                           this,
                                           boost::asio::placeholders::error));
    }
    catch(std::exception& e)
    {
        std::cerr << "acceptor exception: " << e.what() << std::endl;
        return false;
    }

    return true;
}


void tcp_proxy::bridge::acceptor::handle_accept(const boost::system::error_code& error)
         {
            if (!error)
            {
               session_->start(upstream_host_,upstream_port_);

               if (!accept_connections())
               {
                  std::cerr << "Failure during call to accept." << std::endl;
               }
            }
            else
            {
               std::cerr << "Error accept: " << error.message() << std::endl;
            }
         }


















