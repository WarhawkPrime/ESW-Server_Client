
/*
#include "TCP_Socket.h"

TCP_Socket::TCP_Socket() {

}

TCP_Socket::~TCP_Socket() {

}

void TCP_Socket::fill_serverInfo()
{
	memset(&hints, 0, sizeof((struct addrinfo) hints));

	hints.ai_family = AF_UNSPEC;			//ipv4 or ipv6
	hints.ai_socktype = SOCK_STREAM;		//stream (tcp) socket
	hints.ai_flags = AI_PASSIVE;			//bind to ip of the host the programm is running on
	hints.ai_protocol = IPPROTO_TCP;
	//bei spezifischer Adresse: AI_PASSIVE entfernen und bei getaddrinfo null durch die gew�nschte adresse ersetzen

	getaddrinfo(NULL, CPORT, &hints, &serverinfo);
	rp = serverinfo;

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(IPORT);		//stores numbers in memory in Network byte order -> most significant byte first (big endian)
	servaddr.sin_addr.s_addr = INADDR_ANY;

	memset(servaddr.sin_zero, 0, sizeof(servaddr.sin_zero));
	addrSize = sizeof(servaddr);
}




void TCP_Socket::fill_serverInfo(const char* port)
{

	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_UNSPEC;			//ipv4 or ipv6
	hints.ai_socktype = SOCK_STREAM;		//stream (tcp) socket
	hints.ai_flags = AI_PASSIVE;			//bind to ip of the host the programm is running on
	hints.ai_protocol = IPPROTO_TCP;
	//bei spezifischer Adresse: AI_PASSIVE entfernen und bei getaddrinfo null durch die gew�nschte adresse ersetzen

	if ((rv = getaddrinfo(NULL, port, &hints, &serverinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(EXIT_FAILURE);
	};

	rp = serverinfo;

	u_int16_t iport = atoi(port);

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(iport);		//stores numbers in memory in Network byte order -> most significant byte first (big endian)
	servaddr.sin_addr.s_addr = INADDR_ANY;

	memset(servaddr.sin_zero, 0, sizeof(servaddr.sin_zero));
	addrSize = sizeof(servaddr);

	int hostname;
	char host[256];
	char *IP;
	struct hostent *host_entry;

	hostname = gethostname(host, sizeof(host));
	check_host_name(hostname);
	host_entry = gethostbyname(host);
	check_host_entry(host_entry);
	IP = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));

	std::cout << "server connects to host: " << host << "on IP: " << IP << " on Port: " << port << std::endl;
}


void TCP_Socket::create_socket()
{
	//int socket(int domain, int type, int protocol); -> -1 on error
	sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);	//sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	//int option = 1;
	//setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

	if (sockfd < 0) {
		perror("Could not create socket");
		exit(EXIT_FAILURE);
	}

}

//binds assigns the address specified by addr to the socket. "assigning a name to a socket"
//int bind(int sockfd, const struct sockaddr * addr, socklen_t addrlen);
void TCP_Socket::bind_socket()
{
	if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1)
	{
			perror("Could not bind socket");
			exit(EXIT_FAILURE);
	}
}


void TCP_Socket::listen_socket()
{
	if (listen(sockfd, LISTEN_BACKLOG) == -1)
	{
		perror("Could not set listen");
		exit(EXIT_FAILURE);
	}
}

// int accept(int sockfd, struct sockaddr *restrict addr, socklen_t *restrict addrlen);
void TCP_Socket::accept_connection()
{
	peer_addr_size = sizeof(peer_addr);
	clientfd = accept(sockfd, (struct sockaddr *)&peer_addr, &peer_addr_size);

	if (clientfd == -1)
	{
			perror("Could not accept");
			exit(EXIT_FAILURE);
	}
}


void TCP_Socket::send_msg_to(const char* msg)
{
	size_t len;
	int bytes_sent = 0;

	len = strlen(msg);
	bytes_sent = send(sockfd, msg, strlen(msg), 0);

	if (bytes_sent < 0) {
		perror("Could not connect to socket");
		exit(EXIT_FAILURE);
	}
	else {
		std::cout << "bytes sent: " << bytes_sent << std::endl;
	}
}



std::string TCP_Socket::rec_msg_fr()
{
	char readBuffer[BUF_SIZE];
	int num_bytes_read = BUF_SIZE;
	int num_bytes_written = 0;

	std::string rec_message;
	memset(readBuffer, 0, BUF_SIZE);

	//TODO: read the recv as long as there are bytes to receive
	while (num_bytes_read == BUF_SIZE) {
		num_bytes_read = recv(sockfd, readBuffer, BUF_SIZE, 0);

		//std::cout << "buffer: " << readBuffer << std::endl;

		if (num_bytes_read < 0) {
			perror("Read");
		}

		rec_message += readBuffer;
	}


	//rec_message = readBuffer;
	const char* teminate = "\0";
	rec_message.append(teminate);

	//std::cout << rec_message << std::endl;

	return rec_message;
}

void TCP_Socket::close_socket()
{
	int c = 0;

	c = close(sockfd);

	if (c < 0) {
		perror("Could not close socket");
		exit(EXIT_FAILURE);
	}
}

//unn�tig f�r udp
void TCP_Socket::connect_socket()
{
	if (connect(sockfd, serverinfo->ai_addr, serverinfo->ai_addrlen) < 0) {
		perror("Could not connect to socket");
		exit(EXIT_FAILURE);
	}

	if (rp == NULL) {
		std::cerr << "Could not connect to address" << std::endl;
	}
}


void TCP_Socket::check_host_name(int hostname) { //This function returns host name for local computer
   if (hostname == -1) {
      perror("gethostname");
      exit(1);
   }
}

void TCP_Socket::check_host_entry(struct hostent * hostentry) { //find host info from host name
   if (hostentry == NULL){
      perror("gethostbyname");
      exit(1);
   }
}

void TCP_Socket::IP_formatter(char *IPbuffer) { //convert IP string to dotted decimal format
   if (NULL == IPbuffer) {
      perror("inet_ntoa");
      exit(1);
   }
}

void* TCP_Socket::get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}



void TCP_Socket::routine(std::string port)
{
		//get port
		const char* port = temp_string.c_str();

		//fill server_info:
		memset(&hints, 0, sizeof(hints)); //v


		hints.ai_family = AF_UNSPEC;			//allow for ipv4 or ipv6
		hints.ai_socktype = SOCK_STREAM;		//stream (tcp) socket
		hints.ai_flags = AI_PASSIVE;			//bind to ip of the host the programm is running on. wildcard ip address
		hints.ai_protocol = IPPROTO_TCP;

		rv = getaddrinfo(NULL, CPORT, &hints, &serverinfo);
		if(rv != 0)
		{
				fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
				exit(EXIT_FAILURE);
		}

		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(IPORT);		//stores numbers in memory in Network byte order -> most significant byte first (big endian)
		servaddr.sin_addr.s_addr = INADDR_ANY;

		memset(servaddr.sin_zero, 0, sizeof(servaddr.sin_zero));
		addrSize = sizeof(servaddr);


		//int socket(int domain, int type, int protocol); -> -1 on error
		sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);	//sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		//int option = 1;
		//setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

		if (sockfd < 0) {
			perror("Could not create socket");
			exit(EXIT_FAILURE);
		}





}
*/
