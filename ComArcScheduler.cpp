#include <iostream>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <algorithm>

using namespace std;

int main() {
    key_t key = ftok("/home/toor/dev/ComArcScheduler.cpp", 1);

    if (key == -1) {
        perror("ftok failed");
        exit(1);
    }

    const int NUM_INTS = 4;

    cout << "Creating shared memory segment" << endl;
    int shm_id = shmget(key, ((NUM_INTS + 1) * sizeof(int)), IPC_CREAT | 0644);

    if (shm_id == -1) {
        perror("shmget failed");
        exit(1);
    }

    cout << "Attaching to shared memory segment" << endl;
    int* shm_ptr = (int*)shmat(shm_id, NULL, 0);
    if (shm_ptr == (int*)-1) {
        perror("shmat failed");
        exit(1);
    }

    cout << endl;

    int randomNums[NUM_INTS];
    while (shm_ptr[NUM_INTS] != 1) {
        cout << "Waiting for data to be ready..." << endl;
        sleep(1);
    }

    cout << endl;

    for (int i = 0; i < NUM_INTS; i++) {
        randomNums[i] = shm_ptr[i];
        cout << "Reading " << shm_ptr[i] << " in scheduler from shared memory" << endl;
        cout << "Number " << shm_ptr[i] << " added to array" << endl;
        cout << endl;
    }

    cout << "Numbers in array before sorting: ";
    for (int i = 0; i < NUM_INTS; i++) {
        cout << randomNums[i] << " ";
    }

    cout << endl;
    cout << endl;

    cout << "Sorting in ascending order." << endl;
    sort(randomNums, randomNums + NUM_INTS);

    cout << endl;

    cout << "Array after sorting: ";
    cout << "[ ";
    for (int i = 0; i < NUM_INTS; i++) {
        cout << randomNums[i];
        if (i < NUM_INTS - 1) {
            cout << ", ";
        }
    }
    cout << " ]" << endl;

    cout << endl;

    cout << "Detaching from shared memory segment" << endl;
    if (shmdt(shm_ptr) == -1) {
        perror("shmdt failed");
        exit(1);
    }

    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        perror("shmctl failed");
        exit(1);
    }

    cout << "Shared memory deleted." << endl;

    getchar();

    return 0;
}