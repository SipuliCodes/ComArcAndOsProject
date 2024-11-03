#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <sys/shm.h>
#include <sys/ipc.h>

using namespace std;

bool getIntInRange(int& value, int min, int max) {
	while (true) {
		cout << "Enter an integer between " << min << " and " << max << ": ";
		
		
		cin >> value;
		
		if (cin.fail()) {
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			cout << "Invalid input. Please enter a valid integer.\n";
		}
		else if (value < min || value > max) {
			cout << "Input out of range. Please enter a value between " << min << " and " << max;
		}
		else {
			return true;
		}
	}
}

int main() {

	int pipeFD[2];
	const int NUM_INTS = 4;
	
	int randomNums[NUM_INTS];
	
	if (pipe(pipeFD) == -1) {
		cerr << "Pipe creation failed." << endl;
		exit(1);
	}
	
	cout << "Parent pid: " << getpid() << endl;
	cout << endl;
	
	for(int i = 0; i < NUM_INTS; i++) {
		pid_t pid = fork();
	
		if (pid == -1) {
			perror("fork");
			exit(1);
		}
		
		if (pid == 0) {
			int c_pid = getpid();
			cout << "Created child process: " << c_pid << endl;
			
			int number;
			srand(time(NULL) + c_pid);
			number = rand() % 20;
			//To put own numbers, comment the 2 lines above this and uncomment line below
			//getIntInRange(number, 0, 19);
			
			cout << "Random number in child: " << number << endl;
			write(pipeFD[1], &number, sizeof(int));
			cout << "Child " << c_pid << " wrote number " << number << " in pipe." << endl;
		
			cout << "Exited child process " << c_pid << endl;
			exit(0);
		}
		if(pid > 0) {
			int receivedNumber;
			read(pipeFD[0], &receivedNumber, sizeof(int));
			pid_t pid = wait(NULL);
			cout << "Init reads number " << receivedNumber << " from pipe " << endl;
			randomNums[i] = receivedNumber;
			cout << "Added " << receivedNumber << " to list." << endl;
			cout << endl;
		}
	}
	
	close(pipeFD[1]);
	
	cout << endl;
	
	close(pipeFD[0]);
	
	key_t key = ftok("/home/toor/dev/ComArcScheduler.cpp", 1);
	
	if(key == -1) {
		perror("ftok failed");
		exit(1);
	}
	
	cout << "Getting shared memory segment id" << endl;
	int shm_id = shmget(key, (NUM_INTS + 1) * sizeof(int), 0644);
	
	if (shm_id == -1) {
		perror("shmget failed");
		exit(1);
	}
	
	cout << "Attaching to shared memory segment." << endl;
	int* shm_ptr = (int*) shmat(shm_id, NULL, 0);
	if (shm_ptr == (int*) -1) {
		perror("shmat failed");
		exit(1);
	}
	
	cout << endl;
	
	cout << "Sending array to shared memory segment from init" << endl;
	for(int i = 0; i < NUM_INTS; i++) {
		cout << randomNums[i] << " set to shared memory." << endl;
		shm_ptr[i] = randomNums[i];
	}
	
	cout << endl;
	
	shm_ptr[NUM_INTS] = 1;
	cout << "Array is in shared memory segment and ready flag changed to 1" << endl;
	
	cout << endl;
	
	cout << "Detaching from shared memory segment" << endl;
	if(shmdt(shm_ptr) == -1) {
		perror("shmdt failed");
		exit(1);
	}
	
	getchar();
	
	return 0;
}