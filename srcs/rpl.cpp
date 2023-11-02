#include "../inc/main.hpp"


Client&	retrieveClient(Server *server, int const client_fd)
{
	std::map<const int, Client>&		client_list = server->getClients();
	std::map<const int, Client>::iterator it_client = client_list.find(client_fd);
	
	Client &client = it_client->second;
	return (client);
}

void	addToClientBuffer(Server *server, int const client_fd, std::string reply)
{
	Client &client = retrieveClient(server, client_fd);

	client.setSendBuffer(reply);
}

void	sendServerRpl(int const client_fd, std::string client_buffer)
{
	std::istringstream	buf(client_buffer);
	std::string			reply;
	
	send(client_fd, client_buffer.c_str(), client_buffer.size(), 0);
	while (getline(buf, reply))
	{
		std::cout << "[Server] Message sent to client " << client_fd << " >> " << reply << std::endl;
	}
}