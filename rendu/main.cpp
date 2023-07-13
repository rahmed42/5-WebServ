/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abittel <abittel@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/07/16 15:41:42 by rahmed            #+#    #+#             */
/*   Updated: 2022/09/07 16:00:50 by abittel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"
#include "new_Server.hpp"
#include <pthread.h>

int main(int argc, char **argv, char** envp)
{
	if (argc == 2)
	{
		init_error_pages();	  // Init data to generate pages errors
		init_content_files(); // Init data to get the good Content-type for a file
		std::vector<struct server_config> cfg;
		try
		{	cfg = parse_file(argv[1]);	}
		catch (std::exception &e)
		{	return (1);					}
		for (std::vector<struct server_config>::iterator it = cfg.begin(); it != cfg.end(); ++it)
			it->envp = envp;
		new_Server server(cfg);
		try{
			server.run();
		} catch(std::exception& e){
			std::cout << e.what() << std::endl;
		}
		return (0);
	}
	else
		std::cout << "\033[1;31mPlease set a config file : ./webserv [config_file name]\033[0m\n";
}
