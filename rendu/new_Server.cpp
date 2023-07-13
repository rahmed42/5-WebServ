/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   new_Server.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: me <me@student.42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/08/26 22:54:46 by abittel           #+#    #+#             */
/*   Updated: 2022/09/09 21:31:04 by me               ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"
#include "Response.hpp"
#define MAX_RETRY 5
#define MAX_CONNECTION 100
#include "new_Server.hpp"


new_Server::new_Server(std::vector<struct server_config> cfg) : _cfgs(cfg), events(256), change_events(256)
{
}
new_Server::~new_Server()
{
}

void new_Server::launch_socket(struct server_config cfg, int port, std::string hostname)
{
	int retry(0);
	/* Create Socket  IPv4, TCP */
	cfg.actual_hostname = hostname;
	cfg.actual_port = port;
	int socket_fd;
	while ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		if (retry < MAX_RETRY)
			retry++;
		else
		{
			close(socket_fd);
			throw std::runtime_error("Failed to create socket.");
			return;
		}
	}
	/*struct epoll_event accept_event;
	accept_event.data.fd = socket_fd;
	accept_event.events = EPOLLIN;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &accept_event) < 0)
	{
		close(socket_fd);
		throw std::runtime_error("Failed to add socket to epoll.");
		return;
	}*/
	EV_SET(&change_events[0], socket_fd, EVFILT_READ, EV_ADD, 0, 0, 0);
 	if (kevent(epoll_fd, &change_events[0], 1, NULL, 0, NULL) == -1)
    {
		throw std::runtime_error("Failed to add socket to kqueu.");
    }
	retry = 0;

	/* Set Socket Options */
	int enable = true;
	while (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1)
	{
		if (retry < MAX_RETRY)
			retry++;
		else
		{
			close(socket_fd);
			throw std::runtime_error("Failed to set socket options.");
			return;
		}
	}
	retry = 0;

	/* Listen to _port on _hostname */
	struct sockaddr_in sockaddress;
	std::memset(&sockaddress, 0, sizeof(sockaddress));
	sockaddress.sin_family = AF_INET; // internetwork: UDP, TCP, etc. (IPv4)
	sockaddress.sin_addr.s_addr = inet_addr(hostname.c_str());
	sockaddress.sin_port = htons(static_cast<uint16_t>(port));

	/* Bind socket to _port on _hostname */
	while (bind(socket_fd, (struct sockaddr *)(&sockaddress), sizeof(sockaddress)) == -1)
	{
		if (retry < MAX_RETRY)
			retry++;
		else
		{
			close(socket_fd);
			std::string error = "Failed to bind socket of " + cfg.name + " [" + hostname + ":" + to_string(port) + "]";
			throw std::runtime_error(error);
			return;
		}
	}
	retry = 0;

	/* Start listening on port with maximum _max_con in the queue before be refused */
	while (listen(socket_fd, MAX_CONNECTION) == -1)
	{
		if (retry < MAX_RETRY)
			retry++;
		else
		{
			close(socket_fd);
			throw std::runtime_error("Failed to listen on port.");
			return;
		}
	}

	std::cout << "\033[1;33mStarting server : \033[0m" + cfg.name + " [" + hostname + ":" + to_string(port) + "] \n";
	_cfgs_map[socket_fd] = cfg;
}

void new_Server::run()
{
	//Init epoll
	//if ((epoll_fd = epoll_create1(0)) < 0)
		//throw std::runtime_error("Failed to create epoll.");
	if ((epoll_fd = kqueue()) < 0)
		throw std::runtime_error("Failed to create epoll.");
	//CREATE ALL SOCKETS IN ALL SERVERS AND ALL PORTS
	for (std::vector<server_config>::iterator it = _cfgs.begin(); it != _cfgs.end(); it++)
	{
		std::vector<int>::iterator it_p = it->ports.begin();
		std::vector<std::string>::iterator it_h = it->hostname.begin();
		for (; it_p != it->ports.end(); it_p++, it_h++)
		{
			try
			{	launch_socket(*it, *it_p, *it_h);	}
			catch (std::runtime_error &e)
			{	continue;	}
		}
	}

	while (1)
	{
	//	int nb_events = epoll_wait(epoll_fd, &(events[0]), events.size(), 3);
		int nb_events = kevent(epoll_fd, NULL, 0, &events[0], 1, NULL); 
		for (int i = 0; i < nb_events; i++)
		{
			if ((events[i].filter & EV_EOF) == events[i].filter)
			{
				close(events[i].ident);
				EV_SET(&change_events[0], events[i].ident, EV_EOF, EV_ADD, 0, 0, 0);
			}
			else if (_cfgs_map.count(events[i].ident))	//SOCKET EVENT
			{
				struct sockaddr_storage cli_addr;
				std::memset(&cli_addr, 0, sizeof(cli_addr));
				socklen_t size_cli = sizeof(cli_addr);
				int new_sock = accept(events[i].ident, (struct sockaddr *)&cli_addr, &size_cli);
				if (new_sock == -1)
				{
					close(events[i].ident);
					throw std::runtime_error("Failed to accept connection new_sock.");
					return;
				}
				_client_map[new_sock] = _cfgs_map[events[i].ident];
				EV_SET(&change_events[0], new_sock, EVFILT_READ, EV_ADD, 0, 0, 0);
			 	if (kevent(epoll_fd, &change_events[0], 1, NULL, 0, NULL) == -1)
					throw std::runtime_error("Failed to add socket to kqueu.");
			}
			else if (_client_map.count(events[i].ident)) //IS IN NEW CONNECTION
			{
				if ((events[i].filter & EVFILT_READ) == EVFILT_READ)	//OPEN IN READ 
				{
					if (!_response_map.count(events[i].ident))
					{
						Response  inter(&(_client_map[events[i].ident]), _client_map[events[i].ident].actual_port, _client_map[events[i].ident].actual_hostname, "");
						_response_map[events[i].ident] = inter;
						if (_response_map[events[i].ident].read_request(events[i].ident) == -1)
							continue;
						if (_response_map[events[i].ident].process("") < 0)
							continue;
					}
					if (_response_map[events[i].ident]._state_code == STATE_READ_REQUEST)
					{
						int res;
						if ((res = _response_map[events[i].ident].read_request(events[i].ident)) == -1)
							continue;
						else if (res == 413)
							_response_map[events[i].ident]._state_code = STATE_WRITE_RESPONSE;
					}
					if (_response_map[events[i].ident]._state_code == STATE_PROCESS || _response_map[events[i].ident]._state_code == STATE_READ_CGI)
					{
						if (_response_map[events[i].ident].process("") < 0)
						{
							std::cout << "ERR" << std::endl;
							continue;
						}
					}
					EV_SET(&change_events[0], events[i].ident, EVFILT_READ, EV_DELETE, 0, 0, 0);
					EV_SET(&change_events[0], events[i].ident, EVFILT_WRITE, EV_ADD, 0, 0, 0);
				 	if (kevent(epoll_fd, &change_events[0], 1, NULL, 0, NULL) == -1)
						throw std::runtime_error("Failed to add socket to kqueu.");
				}
				else if ((events[i].filter & EVFILT_WRITE) == EVFILT_WRITE) //OPEN IN WRITE
				{
					if (_response_map[events[i].ident]._state_code == STATE_WRITE_RESPONSE)
					{
						size_t		nb_wrt = 0;
						std::string rep_str = _response_map[events[i].ident].get_rep();
						size_t		rep_str_len = rep_str.size();
						ssize_t	res = write(events[i].ident, (rep_str.substr(nb_wrt)).c_str(), (rep_str_len - nb_wrt));
						//ssize_t	res = send(events[i].ident, (rep_str.substr(nb_wrt)).c_str(), 4096, MSG_DONTWAIT );
						nb_wrt += static_cast<size_t>(res);
						//std::cout << fcntl(events[i].ident, F_GETFD) << "/" << nb_wrt << "/" << rep_str_len << std::endl;
						if (nb_wrt < rep_str_len)
							_response_map[events[i].ident].set_rep(rep_str.substr(nb_wrt));
						else {
							EV_SET(&change_events[0], events[i].ident, EVFILT_WRITE, EV_DELETE, 0, 0, 0);
							close(events[i].ident);
							_response_map.erase(events[i].ident);
							_client_map.erase(events[i].ident);
						}
					}
				}
			}
		}
	}
}