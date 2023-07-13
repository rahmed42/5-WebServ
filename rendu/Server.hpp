/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: me <me@student.42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/08/27 15:16:47 by abittel           #+#    #+#             */
/*   Updated: 2022/09/10 15:38:35 by me               ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "WebServ.hpp"
#include "Response.hpp"

class new_Server
{
	private:
		int epoll_fd;
		std::vector<struct server_config>	_cfgs;
		std::map<int, struct server_config>	_cfgs_map;
		std::map<int, struct server_config>	_client_map;
		std::map<int, Response>	_response_map;

		void launch_socket(struct server_config cfg, int _port, std::string _hostname);
	public:
		new_Server(std::vector<struct server_config> cfg);
		~new_Server();
		void								run();	
		void add_event_time(int fd, std::time_t time);
		void delete_event_time(int fd);
		std::vector<struct epoll_event> events;
		std::map<int, std::time_t> expire_time_pair;
		std::vector<int> inserted_fd;

};