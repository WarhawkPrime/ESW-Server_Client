#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>

#include <netinet/in.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <new>

#include <chrono>
#include <ctime>

#include "CCommQueue.h"
#include "SensorTag.h"

//#include "TCP_Socket.h"

using namespace std;

#define SHM_NAME        "/estSHM"
#define QUEUE_SIZE      1
#define NUM_MESSAGES    10000

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

//TCP_Socket tcp;

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



int main(int argc, char* argv[])
{
		signal(SIGPIPE, SIG_IGN);
		//std::cout << "argc" << argc << " : " << argv[1] << std::endl;

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

    //cout << "Creating a child process ..." << endl;
    pid_t pid = fork();

		/* ========== CHILD ========== */
    if (0 == pid)
    {
        // Child process - Gets data from ST and sends it to the parent
				/* ========== ========== ========== */

				int counter = 0;
				while(true) {

					if(counter < NUM_MESSAGES) {
						MostMessage mmsg = create_Message(counter);
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
				}

				/* ========== ========== ========== */
				printf("from child: pid=%d, parent_pid=%d\n",(int)getpid(), (int)getppid());
				exit(42);
    }
		/* ========== PARENT ========== */
		/*
		Sends data from the que to the client
		*/
    else if (pid > 0)
    {

				//===== ===== socket ===== =====

				//attributes
			  int sockfd, clientfd;
			  struct sockaddr_in servaddr, destaddr;
			  socklen_t size;
				//struct sockaddr_storage serverStorage;

			  std::string temp_string = argv[1];
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
				servaddr.sin_addr.s_addr = INADDR_ANY;

			  if((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr))) < 0) {
			    perror("bind error");
			    exit(EXIT_FAILURE);
			  }

			  if((listen(sockfd, BACKLOG)) < 0) {
			    perror("listen error");
			    exit(EXIT_FAILURE);
			  }


			  struct  sockaddr_storage    serverStorage;
			  socklen_t                   addr_size   = sizeof serverStorage;
			  int newSocket = accept(sockfd, (struct sockaddr*)&serverStorage, &addr_size);
			  if (newSocket < 0) {
					perror("accept error");
			    exit(EXIT_FAILURE);
				}


				long mean_time = 0;
				int counter = 0;

				while(true) {

					std::cout << "start of while: " << std::endl;
					//std::cout << "waiting for access on child .... " << std::endl;

					binary_semaphore->take();  // wait for signal

					CMessage msg;
					//std::cout << "getMessage on child .... " << std::endl;
					if(queue->getMessage(msg)){

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
						std::cout << "Message send: " << send_time  << " ns"<< std::endl;
						//std::cout << "Message received: " << received_time << " ns"<< std::endl;
						//std::cout << "Send time: " << duration << " ns"<< std::endl;
						std::cout << "GyroX: " << mmsg->data.PackedData.motion.gyro.x << std::endl;
						std::cout << "GyroY: " << mmsg->data.PackedData.motion.gyro.y << std::endl;
						std::cout << "GyroZ: " << mmsg->data.PackedData.motion.gyro.z << std::endl;
						std::cout << "AccX: " << mmsg->data.PackedData.motion.acc.x << std::endl;
						std::cout << "AccY: " << mmsg->data.PackedData.motion.acc.y << std::endl;
						std::cout << "AccZ: " << mmsg->data.PackedData.motion.acc.z << std::endl;



						//===== socket =====

						std::string message = std::to_string(mmsg->data.PackedData.id);
						message += ";";
						message += std::to_string(send_time);
						message += ";";
						message += std::to_string(mmsg->data.PackedData.motion.gyro.x);
						message += ";";
						message += std::to_string(mmsg->data.PackedData.motion.gyro.y);
						message += ";";
						message += std::to_string(mmsg->data.PackedData.motion.gyro.z);
						message += ";";
						message += std::to_string(mmsg->data.PackedData.motion.acc.x);
						message += ";";
						message += std::to_string(mmsg->data.PackedData.motion.acc.y);
						message += ";";
						message += std::to_string(mmsg->data.PackedData.motion.acc.z);
						message += ";";
						const char* msg = message.c_str();
						//const char* msg = "Test message";
						std::cout << "writing ..." << std::endl;
						write(newSocket, msg, strlen(msg));
						std::cout << "write length: " << strlen(msg) << std::endl;

						//empfangsbestätigung
						char        buffer[BUF_SIZE];
					  int get = BUF_SIZE;

					  std::string ack_string = "";

						std::cout << "reading ..." << std::endl;
				    get = read(newSocket, buffer, BUF_SIZE - 1);

				    buffer[get] = '\0';
				    ack_string += buffer;
				    std::cout << "bytes read: " << get << std::endl;


						//===== socket =====/
						binary_semaphore->give();

						if(counter == NUM_MESSAGES)
							break;
					}
				}

				std::cout << std::endl;
				mean_time = mean_time / NUM_MESSAGES;
				std::cout << "Mean Time: " << mean_time << " ns" << std::endl;
				//std::cout << "finished: " << *global_finished << std::endl;

				//tcp.close_socket();
				shutdown(sockfd, SHUT_WR);
				close(newSocket);
  			close(sockfd);

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

		//===== socket =====
		//tcp.close_socket();
		//===== socket =====/
		shm_unlink(SHM_NAME);
    return 0;
}
