/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abittel <abittel@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/08/14 00:14:01 by abittel           #+#    #+#             */
/*   Updated: 2022/08/27 23:02:16 by abittel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"

class Request
{
	public:
		std::string							_req;
		std::string							_req_meth;
		std::string							_req_http_type;
		std::string							_req_path;
		std::string							_req_accept_content;
		unsigned int						_req_content_length;
		std::string							_req_content_type;
		std::map<std::string, std::string>	_req_var;
		bool								_is_upload_req;
		std::string							_req_upload_token;
		bool								_download_file;
		bool								_is_chunked_req;

		bool 								_finish_read;

		Request(std::string req = "");
		~Request();
		Request(Request const &src);
		Request const& operator=(Request const &rhs);
		void			set_request(std::string val);
		int				parse_request();
		void			parse_variables();
		int 			read_request(int	fd_client);
	private:
		std::string	get_next_line(int fd_client, bool get_buffer = false);
		std::string gnl_line;
		long nb_rd;
};