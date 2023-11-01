#include "main.hpp"

std::map<const int, Client>&	Server::getClients()		{ return (clients); }
std::string 					Server::getDatetime() const { return (datetime); }

static int acceptSocket(int listenSocket)
{
	sockaddr_in client;
	socklen_t addr_size = sizeof(sockaddr_in);
	return (accept(listenSocket, (sockaddr *)&client, &addr_size));
}

void    Server::addClient(int client_socket, std::vector<pollfd> &poll_fds)
{
    pollfd  client_pollfd;
    Client  new_client(client_socket);

    client_pollfd.fd = client_socket;
    client_pollfd.events = POLLIN | POLLOUT;
    poll_fds.push_back(client_pollfd);

    clients.insert(std::pair<int, Client>(client_socket, new_client));
    std::cout << "[SERVER]: CLIENT #" << client_socket << " ADDED" << std::endl;
}

int	Server::createClientConnexion(std::vector<pollfd>& poll_fds, std::vector<pollfd>& new_pollfds)
{
	int client_sock = acceptSocket(server_socket_fd); // Accepts the socket and returns a dedicated fd for this new Client-Server connexion
	if (client_sock == -1)
	{
		std::cerr << "[Server] Accept() failed" << std::endl;
		return (3);
	}
	addClient(client_sock, new_pollfds); // Beware, here we push the new client_socket in NEW_pollfds
	return (SUCCESS);
}

int	Server::handlePolloutEvent(std::vector<pollfd>& poll_fds, std::vector<pollfd>::iterator &it, const int current_fd)
{
	Client *client = getClient(this, current_fd);
	if (!client)
		std::cout << "[Server] Did not found connexion to client sorry" << std::endl;
	else
	{
		sendServerRpl(current_fd, client->getSendBuffer());
		client->getSendBuffer().clear();
		if (client->getDeconnexionStatus() == true)
		{
			delClient(poll_fds, it, current_fd);
			return (BREAK);
		}
	}
	return (SUCCESS);
}

int Server::serverLoop()
{
    std::vector<pollfd> poll_fds;
    pollfd              server_poll_fd;

    server_poll_fd.fd = server_socket_fd;
    server_poll_fd.events = POLLIN;

    poll_fds.push_back(server_poll_fd);
    while (server_shutdown == false)
    {
        std::vector<pollfd> new_pollfds;
        if (poll((pollfd *)&poll_fds[0], (unsigned int)poll_fds.size(), -1) <= SUCCESS)
        {
            if (errno == EINTR)
                break ;
            std::cerr << "[SERVER]: POLL ERROR" << std::endl;
            throw ;
        }
        std::vector<pollfd>::iterator it = poll_fds.begin();
        while (it != poll_fds.end())
        {
            if (it->revents & POLLIN)
            {
                if (it->fd == server_socket_fd)
                {
                    std::cout << " test test" << std::endl;
                    if (this->createClientConnexion(poll_fds, new_pollfds) == CONTINUE)
                        continue;
                }
                else
                {
                    std::cout << " ytrtes" << std::endl;
                    if (this->handleExistingConnexion(poll_fds, it) == BREAK)
                        break ;
                }
            }
            else if (it->revents & POLLOUT)
            {
                if (handlePolloutEvent(poll_fds, it, it->fd) == BREAK)
                    break ;
            }
            it++;
        }
        poll_fds.insert(poll_fds.end(), new_pollfds.begin(), new_pollfds.end());
    }
    return (SUCCESS);
}

static void print(std::string type, int client_socket, char *message)
{
	if (message)
		std::cout << std::endl << type << client_socket << " << " << message;
}

int Server::handleExistingConnexion(std::vector<pollfd>& poll_fds, std::vector<pollfd>::iterator &it)
{
    Client *client;
    client = getClient(this, it->fd);
    char message[BUF_SIZE_MSG];
    int read_count;

    memset(message, 0, sizeof(message));
    read_count = recv(it->fd, message, BUF_SIZE_MSG, 0);

    std::cout << "readcount = " << read_count << std::endl;
    if (read_count <= -1)
    {
        std::cerr << "[SERVER]: recv() FAILED" << std::endl;
        delClient(poll_fds, it, it->fd);
        return (BREAK);
    }
    else if (read_count == 0)
    {
        std::cout << "[SERVER]: A CLIENT JUST DISCONNECTED\n";
        delClient(poll_fds, it ,it->fd);
        return (BREAK);
    }
    else
    {
       print("[CLIENT]: MESSAGE RECEIVED FROM CLIENT ", it->fd, message);
       client->setReadBuffer(message);

       if (client->getReadBuffer().find("\r\n") != std::string::npos)
       {
            try
            {
                test(it->fd, client->getReadBuffer());
                if (client->getReadBuffer().find("\r\n"))
                    client->getReadBuffer().clear();
            }
            catch(const std::exception& e)
            {
                std::cout << "[SERVER]: EXCEPTION: ";
                std::cerr << e.what() << std::endl;
                if (client->isRegistrationDone() == true)
                    client->setDeconnexionStatus(true);
                return (BREAK);
            }
            
       }
    }
    return (SUCCESS);
}

static void splitMessage(std::vector<std::string> &cmds, std::string msg)
{
	int pos = 0;
	std::string delimiter = "\n";
	std::string substr;

	while ((pos = msg.find(delimiter)) != static_cast<int>(std::string::npos))
	{
		substr = msg.substr(0, pos);
		cmds.push_back(substr);
		msg.erase(0, pos + delimiter.length());
	}
}

void	registerClient(Server *server, int const client_fd, std::map<const int, Client>::iterator &it)
{
	addToClientBuffer(server, client_fd, RPL_WELCOME(user_id(it->second.getNickname(), it->second.getUsername()), it->second.getNickname()));
	addToClientBuffer(server, client_fd, RPL_YOURHOST(it->second.getNickname(), "42_Ftirc", "1.1"));
	addToClientBuffer(server, client_fd, RPL_CREATED(it->second.getNickname(), server->getDatetime()));
	addToClientBuffer(server, client_fd, RPL_MYINFO(it->second.getNickname(), "localhost", "1.1", "io", "kost", "k"));
	addToClientBuffer(server, client_fd, RPL_ISUPPORT(it->second.getNickname(), "CHANNELLEN=32 NICKLEN=9 TOPICLEN=307"));
}

void Server::test(int const client_fd, std::string message)
{
    std::vector<std::string>    cmds;
    std::map<const int, Client>::iterator it = clients.find(client_fd);

    splitMessage(cmds, message);

    for(size_t i = 0; i!= cmds.size(); i++)
    {
        if (it->second.isRegistrationDone() == false)
        {
            if (it->second.reg() == false)
            {
                registerClient(this, client_fd, it);
                it->second.reg() = true;
            }
        }
    }
}

void Server::delClient(std::vector<pollfd> &poll_fds, std::vector<pollfd>::iterator &it, int current_fd)
{
    std::cout << "[SERVER] CLIENT #" << current_fd << " disconnected" << std::endl;

    int key = current_fd;

    close(current_fd);
    clients.erase(key);
    poll_fds.erase(it);

    std::cout << "[SERVER]: Client deleted. TOTAL CLIENT IS NOW: " << (unsigned int)(poll_fds.size() - 1) << std::endl;
}

Client* getClient(Server *server, int const client_fd)
{
    std::map<const int, Client>&            client_list = server->getClients();
    std::map<const int, Client>::iterator   it_client = client_list.find(client_fd);

    if (it_client == client_list.end())
        return (NULL);
    return (&it_client->second);
}   