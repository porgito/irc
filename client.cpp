#include "main.hpp"

std::string&	Client::getSendBuffer()  	{ return (sendbuf); }
std::string&	Client::getReadBuffer()  	{ return (readbuf); }
std::string&	Client::getNickname()  		{ return (nickname); }
std::string 	Client::getUsername() const { return (username); }
bool&			Client::getConnexionPassword()	{ return (connexion_password); }
bool&			Client::isRegistrationDone() 	{ return (registrationDone); }
bool&			Client::reg()			{ return (welcomeSent); }
bool&			Client::getDeconnexionStatus()	{ return (to_deconnect); }
int				Client::getNbInfo() const 		{ return (nbInfo); }

void	Client::setReadBuffer(std::string const &buf)
{
	readbuf += buf;
}

void	Client::setSendBuffer(std::string const &buf)
{
	sendbuf += buf;
}

void	Client::setDeconnexionStatus(bool status)
{
	to_deconnect = status;
}

Client::Client(int client_fd)
: client_fd(client_fd)
{}

Client::~Client() {}