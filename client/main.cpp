#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <arpa/inet.h>

#include "CCommQueue.h"
#include "SensorTag.h"
//#include "TCP_Socket.h"

#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <new>
#include <sstream>

#include <chrono>
#include <ctime>

using namespace std;

#define SHM_NAME        "/estSHM"
#define QUEUE_SIZE      1
#define NUM_MESSAGES    10

struct PackedData {
	UInt16 id;
	Motion_t motion;
	UInt64 time;
};
typedef struct PackedData PackedData_t;

/* ========== own ========== */
#define BUF_SIZE 1024
void *addr;
Int8 *que;

#define BACKLOG 10

//static int *global_finished;

CBinarySemaphore *binary_semaphore;
CCommQueue *queue;


PackedData_t create_Sensordata(int id){
	SensorTag st;
	st.initRead();
	st.writeMovementConfig();

	Motion_t motion = st.getMotion();


	auto now = std::chrono::system_clock::now();
	//time_t rt = std::chrono::system_clock::to_time_t (updated);
	auto now_ms = std::chrono::time_point_cast<std::chrono::nanoseconds>(now);
	auto value = now_ms.time_since_epoch();
	long duration = value.count();



	PackedData_t pck;
	pck.id = id;
	pck.motion = motion;
	pck.time = duration;
	return pck;

}


MostMessage create_Message(int id){

	create_Sensordata(id);

	PackedData_t pck = create_Sensordata(id);

	//CMessage msg;
	MostMessage msg;
	msg.data.PackedData.id = pck.id;
	msg.data.PackedData.motion = pck.motion;
	msg.data.PackedData.time = pck.time;

	return msg;
}

/* ========== ========== ========== */

MostMessage deserializeMessage(std::string msg) {

	std::stringstream ss(msg);
	std::string result;
	MostMessage mmsg;
	PackedData_t pck;
	Motion_t motion;

	//std::cout << "Received Message: " << msg << std::endl;
	std::cout << "--------------------------------------" << std::endl;

	int counter = 0;
	while (std::getline(ss, result, ';')) {

		switch (counter) {

		case 0: {
			pck.id = std::stoi(result);

			//std::cout << "Counter: " << counter << " Result: " << result << std::endl;
			counter++;
			break;
		}
		case 1: {
			pck.time = std::stold(result);
			//std::cout << "Counter: " << counter << " Result: " << result << std::endl;
			counter++;
			break;
		}
		case 2: {
			motion.gyro.x = std::stof(result);
			//std::cout << "Counter: " << counter << " Result: " << result << std::endl;
			counter++;
			break;
		}
		case 3: {
			motion.gyro.y = std::stof(result);
			//std::cout << "Counter: " << counter << " Result: " << result << std::endl;
			counter++;
			break;
		}
		case 4: {
			motion.gyro.z = std::stof(result);
			//std::cout << "Counter: " << counter << " Result: " << result << std::endl;
			counter++;
			break;
		}
		case 5: {
			motion.acc.x = std::stof(result);
			//std::cout << "Counter: " << counter << " Result: " << result << std::endl;
			counter++;
			break;
		}
		case 6: {
			motion.acc.y = std::stof(result);
			//std::cout << "Counter: " << counter << " Result: " << result << std::endl;
			counter++;
			break;
		}
		case 7: {
			motion.acc.z = std::stof(result);
			//std::cout << "Counter: " << counter << " Result: " << result << std::endl;
			counter++;
			break;
		}
		default: {
			//std::cout << "Error in deserializeMessage()" << std::endl;
			break;
		}
		}
	}

	pck.motion = motion;

	mmsg.data.PackedData.id = pck.id;
	mmsg.data.PackedData.motion = pck.motion;
	mmsg.data.PackedData.time = pck.time;

	return mmsg;
}


int main(int argc, char** argv)
{
		if (argc < 3 || argc > 3) {

			std::cout << "Usage: termin5 [server ip] [port]" << std::endl;
			return 0;
		}

		// Variables for TCP socket
		const char* address = argv[1];
		const char* port = argv[2];
		std::cout << address << " / " << port << std::endl;



		shm_unlink(SHM_NAME);
		int fd = shm_open(SHM_NAME, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
		if (fd == -1){
			perror("shm_open");
		}

		int size = CCommQueue::getNumOfBytesNeeded(QUEUE_SIZE) + sizeof(CBinarySemaphore) + sizeof(CBinarySemaphore)+ sizeof(long);

		if (int res = ftruncate(fd, size)) {
			perror("ftruncate");
		}

		addr = mmap(NULL, BUF_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if(addr == MAP_FAILED) {
			perror("mmap");
		}

		binary_semaphore = new(addr) CBinarySemaphore();
		CBinarySemaphore c_sem;
		queue = new(addr + sizeof(c_sem)) CCommQueue(QUEUE_SIZE, c_sem);

		//global_finished = new(addr + sizeof(c_sem) + sizeof(queue)) int;

		//*global_finished = 1;
		//std::cout << "finished: " << *global_finished << std::endl;

    cout << "Creating a child process ..." << endl;
    pid_t pid = fork();

		/* ========== CHILD ========== */
    if (0 == pid)
    {
        // Child process - Reads all Messages from the Queue and outputs them with auxiliary data.
		/* ========== ========== ========== */

		std::string received_data;

		// Set server info and create TCP socket

		//attributes
    int sockfd, clientfd;
    struct sockaddr_in servaddr, destaddr;
    socklen_t size;
  	//struct sockaddr_storage serverStorage;
		
    std::string temp_string = argv[2];
    const char* port = temp_string.c_str();

		sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sockfd < 0) {
      perror("socket error");
      exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&destaddr, 0, sizeof(destaddr));

  	u_int16_t iport = atoi(port);

    servaddr.sin_family = AF_INET;
  	servaddr.sin_port = htons(iport);		//stores numbers in memory in Network byte order -> most significant byte first (big endian)
  	servaddr.sin_addr.s_addr = inet_addr(argv[1]);

		if ((connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) < 0) {
      perror("socket error");
      exit(EXIT_FAILURE);
    }

		//--------------//


		int counter = 0;
		while (true) {


			if (counter < NUM_MESSAGES) {

				char        buffer[BUF_SIZE];
			  int get = BUF_SIZE;

			  std::string message = "";
			  while(get != 79) {
					std::cout << "reading... " << std::endl;
			    get = read(sockfd, buffer, BUF_SIZE - 1);

			    buffer[get] = '\0';
			    message += buffer;
			    std::cout << "bytes read: " << get << std::endl;
			  }

				std::cout << "sending ack: " << std::endl;
				//lesebestätigung
				const char *ack = "1";
				write(sockfd, ack, strlen(ack));


				//std::cout << "Received data: " << message << std::endl;
				//std::cout << std::endl;
				MostMessage mmsg = deserializeMessage(message);
				const CMessage c_msg(mmsg);

				queue->add(c_msg);
				binary_semaphore->give();
				counter++;
				//std::cout << "message created and send: " << counter << std::endl;
				binary_semaphore->give();
			}
			else {
				break;
			}

			/*
			else if(counter == NUM_MESSAGES + (2*QUEUE_SIZE)) {
				break;
			}
			else {
				binary_semaphore->give();
				counter++;
			}
			*/
		}

		close(sockfd);
		/* ========== ========== ========== */
		printf("from child: pid=%d, parent_pid=%d\n",(int)getpid(), (int)getppid());
		exit(42);
    }
		/* ========== PARENT ========== */
    else if (pid > 0)
    {

		int counter = 0;
		long mean_time = 0;

		while (true) {

			//std::cout << "waiting for access on child .... " << std::endl;

			binary_semaphore->take();  // wait for signal

			CMessage msg;
			//std::cout << "getMessage on child .... " << std::endl;
			if (queue->getMessage(msg)) {

				counter++;
				const MostMessage* mmsg = msg.getMostMessage();

				auto now = std::chrono::system_clock::now();
				auto now_ms = std::chrono::time_point_cast<std::chrono::nanoseconds>(now);
				auto value = now_ms.time_since_epoch();
				long received_time = value.count();

				UInt64 send_time = mmsg->data.PackedData.time;
				long duration = received_time - send_time;
				mean_time += duration;

				//Ausgaben:
				std::cout << std::endl;
				//std::cout << "a message exists" << std::endl;
				std::cout << "Message Number: " << mmsg->data.PackedData.id << std::endl;
				std::cout << "Message send: " << send_time << " ns" << std::endl;
				std::cout << "Message received: " << received_time << " ns" << std::endl;
				std::cout << "Send time: " << duration << " ns" << std::endl;
				std::cout << "GyroX: " << mmsg->data.PackedData.motion.gyro.x << std::endl;
				std::cout << "GyroY: " << mmsg->data.PackedData.motion.gyro.y << std::endl;
				std::cout << "GyroZ: " << mmsg->data.PackedData.motion.gyro.z << std::endl;
				std::cout << "AccX: " << mmsg->data.PackedData.motion.acc.x << std::endl;
				std::cout << "AccY: " << mmsg->data.PackedData.motion.acc.y << std::endl;
				std::cout << "AccZ: " << mmsg->data.PackedData.motion.acc.z << std::endl;

				binary_semaphore->give();

				if (counter == NUM_MESSAGES)
					break;
			}
		}

		mean_time = mean_time / NUM_MESSAGES;
		std::cout << "Mean time: " << mean_time << "ns" << std::endl;


		printf("from parent: pid=%d child_pid=%d\n",(int)getpid(), (int)pid);
		/* ========== ========== ========== */
		int status;
		pid_t waited_pid = waitpid(pid, &status, 0);

		if (waited_pid < 0) {
		perror("waitpid() failed");
		exit(EXIT_FAILURE);
		}
		else if (waited_pid == pid) {
			if (WIFEXITED(status)) {
			/* WIFEXITED(status) returns true if the child has terminated
				* normally. In this case WEXITSTATUS(status) returns child's
				* exit code.
				*/
			printf("from parent: child exited with code %d\n",WEXITSTATUS(status));
			}
		}
    }
    else
    {
        // Error
				perror("fork() failed");
			  exit(EXIT_FAILURE);
    }


		shm_unlink(SHM_NAME);
    return 0;
}
