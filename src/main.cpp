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

#include <iostream>
#include <cstdlib>

#include <boost/asio.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/date_time.hpp>

#include "bridge.h"

#include "config.h"

void show_version(){
    boost::posix_time::ptime t = boost::posix_time::time_from_string(GIT_VERSION_DATE);
    std::locale locale;
    boost::posix_time::time_facet *facet = new boost::posix_time::time_facet("%d %B %Y");
    std::cout.imbue(std::locale(locale, facet));
    std::cout << APPLICATION_NAME << " " << APPLICATION_VERSION_STRING << " " << t << std::endl;
}

int main(int argc, char ** argv){

    boost::program_options::options_description description("MyTool Usage");
    description.add_options()
        ("help,h", "Display this help message")
        ("local,l", boost::program_options::value<std::string>(),"Local endpoint")
        ("ssl_ca,a", boost::program_options::value<std::string>(),"SSL CA")
        ("remote,r", boost::program_options::value<std::string>(),"Remote endpoint")
        ("ssl_cert,c", boost::program_options::value<std::string>(),"SSL Certificate")
        ("ssl_key,k", boost::program_options::value<std::string>(),"SSL Private Key")
        ("version,v", "Display the version number");

    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(description).run(), vm);
    boost::program_options::notify(vm);

    if(vm.count("help")){
        std::cout << description;
        return 0;
    }

    if(vm.count("version")){
        show_version();
        return 0;
    }


   const unsigned short local_port   = boost::lexical_cast<unsigned short>(vm["local"].as<std::string>().substr(vm["local"].as<std::string>().find(":") + 1));
   const unsigned short forward_port = boost::lexical_cast<unsigned short>(vm["remote"].as<std::string>().substr(vm["remote"].as<std::string>().find(":") + 1));
   const std::string local_host      = vm["local"].as<std::string>().substr(0, vm["local"].as<std::string>().find(":"));
   const std::string forward_host    = vm["remote"].as<std::string>().substr(0, vm["remote"].as<std::string>().find(":"));

   boost::asio::io_service ios;

    std::cout << "local  -> " << local_host << ":" << local_port << std::endl;
    std::cout << "remote -> " << forward_host << ":" << forward_port << std::endl;

   try
   {
      tcp_proxy::bridge::acceptor acceptor(ios,
                                           local_host, local_port,
                                           forward_host, forward_port);

      if(vm.count("ssl_ca")){
          acceptor.set_upstream_ssl(vm["ssl_ca"].as<std::string>());
      }

      if(vm.count("ssl_cert")){
          acceptor.set_downstream_ssl(vm["ssl_key"].as<std::string>(), vm["ssl_cert"].as<std::string>());
      }

      acceptor.accept_connections();

      ios.run();
   }
   catch(std::exception& e)
   {
      std::cerr << "Error: " << e.what() << std::endl;
      return 1;
   }

   return 0;
}


