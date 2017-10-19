#pragma once

#include <iostream>
#include <boost\asio.hpp>
#include <boost\bind.hpp>
#include <vector>
#include <thread>

#define BYTES_NULL 0
#define MAX_CLIENTS SOMAXCONN

typedef boost::asio::ip::tcp::socket   tcp_socket;
typedef boost::asio::ip::tcp::endpoint tcp_endpoint;
typedef boost::asio::ip::tcp::acceptor tcp_acceptor;

typedef boost::system::error_code err_code;

using namespace boost::asio;

class UserSession;
class Server;

void user_session(UserSession &u_ession);

//Functions signatures


char* convertToChar(std::string message);

struct Message
{
	char *message;
	int size_message;

	Message(const Message &_message)
	{
		message = _message.message;
		size_message = _message.size_message;
	}

	Message(char *_message, int _size_message) : message(_message), size_message(_size_message) {};
	Message(char *_message) : message(_message), size_message(0) {};
};

struct User
{
	tcp_socket *socket;
	int id_user;

	User(tcp_socket *_socket, int id) : socket(_socket), id_user(id) {};

	void sendToUser(Message *message);
	void close();

	bool read(char *buff, int size_buffer);
	bool write(Message *msg);

};

class PollMessages
{
private:
	//static int new_messages;
public:

	static std::vector<Message*> messages;

	static void addMessage(char *message,int size_message);
	static void addMessage(std::string message);

	static int newMessagesCount();
};

class UserSession
{
private:

	User *_user;

public:

	UserSession(User *user) : _user(user){};

	void disconnect();
	void session();
};

class Server
{
private:

	int count_users;

	io_service service;
	
	tcp_acceptor *acceptor;

	void handler(tcp_socket *,err_code );
	void start_accept();

public:

	static std::vector<User*> users;

	Server(short port, std::string hostAddress);
	~Server();
	
	static void send_messages();
	static void disconnect_user(int user_id);
};
