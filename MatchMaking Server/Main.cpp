	#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <list>

using namespace std;
#define MAXPENDINGCON 8
#define RCVBUFSIZE 1024
vector<SOCKET> clients;

typedef struct playerinfo
{
	SOCKET client;
	int id;
}PlayerInfo;

typedef struct sessioninfo
{
	int id;
	string name;
	string serverip;
	int serverport;
}SessionInfo;

list<PlayerInfo> players;
list<SessionInfo> sessions;

int playerCount = 0;
int sessionCount = 0;

void InterpreteMessage(char* buffer, PlayerInfo pInfo)
{
	string temp;
	vector <string> params;
	char cmd = 0;
	for (int i = 0; buffer[i] != '#'; i++)
	{
		if ((buffer[i] == '|') && (cmd == 0))
		{
			cmd = temp[0];
			temp = "";
		}
		else if ((buffer[i] == '|') && (cmd != 0))
		{
			params.push_back(temp);
			temp = "";
		}
		else
		{
			temp = temp + buffer[i];
		}
	}
	if (cmd == 'g')
	{
	
		if (sessions.size() > 0)
		{
			for (list<SessionInfo>::iterator it = sessions.begin();it != sessions.end(); it++)
			{
				string message = "s|" +to_string( it->id) + "|" +
					it->name + "|" + it->serverip +	"|" +
					to_string(it->serverport) + "|";

				if (send(pInfo.client, message.c_str(),
					message.length(), 0) == SOCKET_ERROR)
				{
					cout << "send failed!" << endl;
				}

			}
		}
		else
		{
			string message = "s|null";

			if (send(pInfo.client, message.c_str(),
				message.length(), 0) == SOCKET_ERROR)
			{
				cout << "send failed!" << endl;
			}
		}
	}
	else if (cmd == 'h')
	{
		SessionInfo session;
		session.id = sessionCount++;
		session.name = params.at(0);
		session.serverip = params.at(1);
		session.serverport = stoi(params.at(2));
		sessions.push_back(session);

		string message = "o|" + params.at(2) + "|";
		if (send(pInfo.client, message.c_str(),
			message.length(), 0) == SOCKET_ERROR)
		{
			cout << "send failed!" << endl;
		}
	}
	else
	{
		cout << "Unknown message: " << buffer << endl;	
	}
}

//g|#
//h|SESSION_NAME|SESSION_IP|SESSION_PORT|#

//s|SESSION_ID|SESSION_NAME|SESSION_IP|SESSION_PORT|
//o|SESSION_PORT|

void HandleClientThread(PlayerInfo pInfo)
{
	char buffer[RCVBUFSIZE];
	string clientmsg;


	while (recv(pInfo.client, buffer, sizeof(buffer), 0) > 0)
	{
		cout << buffer << endl;
		InterpreteMessage(buffer, pInfo);
		memset(buffer, 0, sizeof(buffer));
	}

	if (closesocket(pInfo.client) == SOCKET_ERROR)
	{
		cout << "closesocket() failed" << endl;
	}

}


int main() {
	SOCKET server;
	SOCKADDR_IN server_addr, client_addr;
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != NO_ERROR) {
		cout << "WSAStartup failed" << endl;
		exit(EXIT_FAILURE);
	}

	if ((server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
		cout << "socket() failed" << endl;
		exit(EXIT_FAILURE);
	}
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(8804);

	if (::bind(server, (struct sockaddr*) & server_addr, sizeof(server_addr)) == SOCKET_ERROR)
	{
		cout << "bind failed" << endl;
		exit(EXIT_FAILURE);
	}

	if (listen(server, MAXPENDINGCON) == SOCKET_ERROR) {
		cout << "listen failed" << endl;
		exit(EXIT_FAILURE);
	}
	cout << "Server Survived" << endl;

	while (true)
	{
		SOCKET client;
		int clientlen = sizeof(client_addr);

		if ((client = accept(server, (struct sockaddr*) & client_addr, &clientlen)) == INVALID_SOCKET)
		{
			cout << "accept() failed" << endl;
		}
		else {
			char addrstr[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &(client_addr.sin_addr), addrstr, INET_ADDRSTRLEN);
			cout << "Connection from " << addrstr << endl;

			PlayerInfo pInfo;
			pInfo.client = client;
			pInfo.id = playerCount++;
			players.push_back(pInfo);



			//clients.push_back(client);
			thread* c = new std::thread(HandleClientThread, pInfo);
		}

	}

}