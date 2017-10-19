#include "Server.h"

//Class Server
std::vector<User*> Server::users = std::vector<User*>(0);
std::vector<Message*> PollMessages::messages = std::vector<Message*>(0);

Server::Server(short port, std::string hostAddress)
{	
	count_users = 0;

	tcp_endpoint endpoint(ip::address_v4::from_string(hostAddress), port);
	acceptor = new tcp_acceptor(service, endpoint);
	
	std::cout << "Start server.Host address : " << hostAddress << ", port : " << port << std::endl;
	std::cout << "Server listening..." << std::endl;

	start_accept();

	service.run();
}

Server::~Server()
{

}

void Server::start_accept()	
{	
	tcp_socket *socket = new tcp_socket(service);
		acceptor->async_accept(*socket,boost::bind(&Server::handler,this,socket,boost::asio::placeholders::error));
}

void Server::handler(tcp_socket *socket,err_code er)
{
	count_users++;
	
	static int count_users = 0;

	if (!er)
	{	
		count_users++;

		User *user = new User(socket,count_users);

		UserSession session(user);
		
		Server::users.push_back(user);

		std::thread t(user_session,session);
		t.detach();

		std::cout << "New user connection!" << std::endl;
	}

	start_accept();
}

void Server::send_messages()
{	

	int users_size = Server::users.size();

	for (int i = 0; i < users_size; i++)
	{
		users[i]->write(PollMessages::messages[PollMessages::messages.size() - 1]);
	}

	
	int all_messages = PollMessages::messages.size();
	int count_messages = all_messages - PollMessages::newMessagesCount();
	
	for (int i = 0; i < users.size(); i++)
	{
		for (int j = count_messages; j < all_messages; j++)
		{
			users[i]->sendToUser(PollMessages::messages[j]);
		}
	}
	
}

void Server::disconnect_user(int user_id)
{	
	
	int users_size = users.size();

	for (int i = 0; i < users_size; i++)
	{
		if (users[i]->id_user == user_id)
		{
			users.erase(users.begin() + i);
		}
	}
	
}

//Class UserSession
void UserSession::session()
{	
	char buff[100];

	err_code er;

	while (1)
	{
		if (!_user->read(buff, 100))
		{
			std::cout << "User disconnected!" << std::endl;

			disconnect();
			return;
		}
		PollMessages::addMessage(buff, 100);
	}
}

void UserSession::disconnect()
{	
	Server::disconnect_user(_user->id_user);

	_user->close();
	delete _user;
}

//User session func

void user_session(UserSession &u_session)
{
	u_session.session();
}

//Class PollMessages

int PollMessages::newMessagesCount()
{
	return 0;
}

void PollMessages::addMessage(char *message,int size_message)
{
	Message* msg = new Message(message, size_message);
	messages.push_back(msg);

	Server::send_messages();
}

void PollMessages::addMessage(std::string message)
{
	Message* msg = new Message(convertToChar(message), message.size());
	messages.push_back(msg);

	Server::send_messages();
}


char* convertToChar(std::string message)
{
	char *buf = (char*)message.c_str();

	return buf;
}

//Struct user

void User::sendToUser(Message *message)
{
	socket->write_some(buffer(message->message, message->size_message));
}

void User::close()
{
	socket->close();
}

bool User::read(char *buff, int size_buffer)
{
	err_code er;
	int bytes_read = socket->read_some(buffer(buff, size_buffer), er);

	if (er == error::eof || bytes_read == 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool User::write(Message *msg)
{
	err_code er;

	socket->write_some(buffer(msg->message, msg->size_message), er);

	if (er)	return false;
	else	return true;
}