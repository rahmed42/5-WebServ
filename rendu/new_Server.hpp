/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   new_Server.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abittel <abittel@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/08/27 15:16:47 by abittel           #+#    #+#             */
/*   Updated: 2022/08/30 15:03:23 by abittel          ###   ########.fr       */
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
		std::vector<struct kevent>	events;
		std::vector<struct kevent>	change_events;

		void launch_socket(struct server_config cfg, int _port, std::string _hostname);
	public:
		new_Server(std::vector<struct server_config> cfg);
		~new_Server();
		void								run();	
};