/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: me <me@student.42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/08/26 22:54:46 by abittel           #+#    #+#             */
/*   Updated: 2022/09/10 16:30:42 by me               ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"
#include "Response.hpp"
#define MAX_RETRY 5
#define MAX_CONNECTION 100
#define TIMEOUT_LIMIT 10
#include "Server.hpp"


new_Server::new_Server(std::vector<struct server_config> cfg) : _cfgs(cfg), events(2560), expire_time_pair() , inserted_fd() 
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
	struct epoll_event accept_event;
	accept_event.data.fd = socket_fd;
	accept_event.events = EPOLLIN;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &accept_event) < 0)
	{
		close(socket_fd);
		throw std::runtime_error("Failed to add socket to epoll.");
		return;
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

void new_Server::add_event_time(int fd, std::time_t time)
{
	if (expire_time_pair.insert(std::pair<int, std::time_t>(fd, time)).second == false)
	{
		expire_time_pair[fd] = time;
	}
	else
	{
		inserted_fd.push_back(fd);			
	}
}
void new_Server::delete_event_time(int fd)
{
	//DELETE IN INSERTED_FD
	for(std::vector<int>::iterator it = inserted_fd.begin(); it < inserted_fd.end(); it++)
	{
		if (*it == fd)
		{
			inserted_fd.erase(it);			
			break;
		}
	}
	if (expire_time_pair.count(fd))
		expire_time_pair.erase(expire_time_pair.at(fd));
}

void new_Server::run()
{
	//Init epoll
	if ((epoll_fd = epoll_create1(0)) < 0)
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
		int nb_events = epoll_wait(epoll_fd, &(events[0]), events.size(), 1);
		// HERE IS THE TIMEOUT
		for (size_t i = 0; i < inserted_fd.size() ; i++)
		{
			std::time_t curr_time = time(0);
			std::time_t event_time;
			if (expire_time_pair.count(inserted_fd[i]))
			{
				event_time = expire_time_pair.at(inserted_fd[i]);
				if ((curr_time - event_time) > TIMEOUT_LIMIT)
				{
					std::cout << inserted_fd[i] << "/" << curr_time << "/" << event_time << std::endl;
					close(inserted_fd[i]);
					if (_client_map.count(inserted_fd[i]))
						_client_map.erase(inserted_fd[i]);
					if (_response_map.erase(inserted_fd[i]))
						_response_map.erase(inserted_fd[i]);
					delete_event_time(inserted_fd[i]);
				}
			}
		}
		for (int i = 0; i < 256 &&  i < nb_events; i++)
		{
			if (_cfgs_map.count(events[i].data.fd))	//SOCKET EVENT
			{
				struct sockaddr_storage cli_addr;
				std::memset(&cli_addr, 0, sizeof(cli_addr));
				socklen_t size_cli = sizeof(cli_addr);
				int new_sock = accept(events[i].data.fd, (struct sockaddr *)&cli_addr, &size_cli);
				if (new_sock == -1)
				{
					close(events[i].data.fd);
					throw std::runtime_error("Failed to accept connection new_sock.");
					return;
				}
				_client_map[new_sock] = _cfgs_map[events[i].data.fd];
				struct epoll_event accept_event;
				accept_event.data.fd = new_sock;
				accept_event.events = EPOLLIN;
				if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_sock, &accept_event) < 0)
					close(new_sock);
				delete_event_time(new_sock);
				add_event_time(new_sock, time(0));
			}
			else if (_client_map.count(events[i].data.fd)) //IS IN NEW CONNECTION
			{
				if (events[i].events & EPOLLIN)	//OPEN IN READ 
				{
					if (!_response_map.count(events[i].data.fd))
					{
						Response  inter(&(_client_map[events[i].data.fd]), _client_map[events[i].data.fd].actual_port, _client_map[events[i].data.fd].actual_hostname, "");
						_response_map[events[i].data.fd] = inter;
						if (_response_map[events[i].data.fd].read_request(events[i].data.fd) == -1)
							continue;
						if (_response_map[events[i].data.fd].process("") < 0)
							continue;
					}
					if (_response_map[events[i].data.fd]._state_code == STATE_READ_REQUEST)
					{
						if (_response_map[events[i].data.fd].read_request(events[i].data.fd) == -1)
							continue;
					}
					if (_response_map[events[i].data.fd]._state_code == STATE_PROCESS
					|| _response_map[events[i].data.fd]._state_code == STATE_READ_CGI)
					{
						if (_response_map[events[i].data.fd]._state_code == STATE_READ_CGI && _response_map[events[i].data.fd].process("") < 0)
						{
							struct epoll_event accept_event;
		  					accept_event.data.fd = events[i].data.fd;
							accept_event.events = EPOLLOUT;
							if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, events[i].data.fd, &accept_event) < 0)
								close (events[i].data.fd);
							continue;
						}
						else if (_response_map[events[i].data.fd]._state_code != STATE_READ_CGI && _response_map[events[i].data.fd].process("") < 0)
							continue;
					}
					delete_event_time(events[i].data.fd);
					struct epoll_event accept_event;
  					accept_event.data.fd = events[i].data.fd;
					accept_event.events = EPOLLOUT;
					if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, events[i].data.fd, &accept_event) < 0)
						close (events[i].data.fd);
					add_event_time(events[i].data.fd, time(0));
				}
				if (events[i].events & EPOLLOUT) //OPEN IN WRITE
				{
					if (_response_map[events[i].data.fd]._state_code == STATE_READ_CGI)
					{
						if (_response_map[events[i].data.fd].process("") < 0)
							continue;
					}
					if (_response_map[events[i].data.fd]._state_code == STATE_WRITE_RESPONSE)
					{
						size_t		nb_wrt = 0;
						std::string rep_str = _response_map[events[i].data.fd].get_rep();
						size_t		rep_str_len = rep_str.size();
						while (nb_wrt < rep_str_len) // Write reponse to client FD i
						{
							ssize_t	res = write(events[i].data.fd, (rep_str.substr(nb_wrt)).c_str(), (rep_str_len - nb_wrt));
							if (res == -1)
								continue;
							nb_wrt += static_cast<size_t>(res);
						}
						epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
						close(events[i].data.fd);
						_response_map.erase(events[i].data.fd);
						_client_map.erase(events[i].data.fd);
						delete_event_time(events[i].data.fd);
					}
				}
				
			}
		}
	}
}
