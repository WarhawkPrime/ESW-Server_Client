#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>

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

#include "TCP_Socket.h"

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

TCP_Socket tcp;



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

				//===== socket =====
				std::string temp_string = argv[1];
				const char* port = temp_string.c_str();

				tcp.fill_serverInfo(port);
				tcp.create_socket();
				tcp.bind_socket();
				tcp.listen_socket();
				//===== socket =====/

				long mean_time = 0;
				int counter = 0;

				while(true) {

					//wait for incoming connections
					std::cout << "waiting for connection request:" << std::endl;
					tcp.accept_connection();

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
						std::cout << "Message received: " << received_time << " ns"<< std::endl;
						std::cout << "Send time: " << duration << " ns"<< std::endl;
						std::cout << "GyroX: " << mmsg->data.PackedData.motion.gyro.x << std::endl;
						std::cout << "GyroY: " << mmsg->data.PackedData.motion.gyro.y << std::endl;
						std::cout << "GyroZ: " << mmsg->data.PackedData.motion.gyro.z << std::endl;
						std::cout << "AccX: " << mmsg->data.PackedData.motion.acc.x << std::endl;
						std::cout << "AccY: " << mmsg->data.PackedData.motion.acc.y << std::endl;
						std::cout << "AccZ: " << mmsg->data.PackedData.motion.acc.z << std::endl;

						//===== socket =====
						std::string message = "NR: " + mmsg->data.PackedData.id;
						message += "ST: " + send_time;
						message += "RT: " + received_time;
						message += "DT: " + duration;
						message += "GX: " + std::to_string(mmsg->data.PackedData.motion.gyro.x);
						message += "GY: " + std::to_string(mmsg->data.PackedData.motion.gyro.y);
						message += "GZ: " + std::to_string(mmsg->data.PackedData.motion.gyro.z);
						message += "AX: " + std::to_string(mmsg->data.PackedData.motion.acc.x);
						message += "AY: " + std::to_string(mmsg->data.PackedData.motion.acc.y);
						message += "AZ: " + std::to_string(mmsg->data.PackedData.motion.acc.z);

						const char* msg = message.c_str();
						tcp.send_msg_to(msg);
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
		tcp.close_socket();
		//===== socket =====/
		shm_unlink(SHM_NAME);
    return 0;
}
