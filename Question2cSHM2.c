/*
 * Question2cSHM2.c
 * Program 2 dari 2 - menggunakan Shared Memory + Semaphore
 * Program ini adalah "task2" equivalent yang menginkrementasi
 * shared counter sebanyak MAX kali.
 *
 * Jalankan SHM1 dulu, lalu SHM2 di terminal lain.
 *
 * Compile: gcc -o shm2 Question2cSHM2.c
 * Run:     ./shm2  (jalankan setelah shm1 berjalan)
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>

#define MAX       10000000
#define SHM_KEY   0x1234    // Harus sama dengan SHM1
#define SEM_KEY   0x5678    // Harus sama dengan SHM1
#define SHM_SIZE  sizeof(SharedData)

typedef struct {
    int  counter;
    int  prog1_done;
    int  prog2_done;
} SharedData;

// Operasi semaphore: lock (P operation)
void sem_lock(int semid) {
    struct sembuf op = {0, -1, 0};
    semop(semid, &op, 1);
}

// Operasi semaphore: unlock (V operation)
void sem_unlock(int semid) {
    struct sembuf op = {0, 1, 0};
    semop(semid, &op, 1);
}

int main() {
    // --- Attach ke shared memory yang sudah dibuat SHM1 ---
    int shmid = shmget(SHM_KEY, SHM_SIZE, 0666);
    if (shmid < 0) {
        perror("shmget - Pastikan SHM1 sudah berjalan terlebih dahulu!");
        exit(1);
    }

    SharedData *shm = (SharedData *)shmat(shmid, NULL, 0);
    if (shm == (void *)-1) { perror("shmat"); exit(1); }

    // --- Attach ke semaphore yang sudah dibuat SHM1 ---
    int semid = semget(SEM_KEY, 1, 0666);
    if (semid < 0) { perror("semget"); exit(1); }

    printf("[SHM2] Attached to shared memory and semaphore.\n");
    printf("[SHM2] Starting increment (MAX = %d)...\n", MAX);

    // --- Increment counter sebanyak MAX kali ---
    for (int i = 0; i < MAX; i++) {
        sem_lock(semid);    // Lock sebelum akses shared variable
        shm->counter++;     // Critical section
        sem_unlock(semid);  // Unlock setelah selesai
    }

    // Tandai program 2 sudah selesai
    shm->prog2_done = 1;
    printf("[SHM2] Done incrementing.\n");

    // Detach dari shared memory (tidak menghapus, SHM1 yang hapus)
    shmdt(shm);
    printf("[SHM2] Detached from shared memory. Exiting.\n");

    return 0;
}
