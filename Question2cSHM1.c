/*
 * Question2cSHM1.c
 * Program 1 dari 2 - menggunakan Shared Memory + Semaphore
 * Program ini adalah "task1" equivalent yang menginkrementasi
 * shared counter sebanyak MAX kali.
 *
 * Jalankan SHM1 dulu, lalu SHM2 di terminal lain.
 * Setelah keduanya selesai, SHM1 akan mencetak hasil akhir.
 *
 * Compile: gcc -o shm1 Question2cSHM1.c
 * Run:     ./shm1
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>

#define MAX       10000000
#define SHM_KEY   0x1234    // Key untuk shared memory
#define SEM_KEY   0x5678    // Key untuk semaphore
#define SHM_SIZE  sizeof(SharedData)

// Struct yang disimpan di shared memory
typedef struct {
    int  counter;        // Counter yang diincrement bersama
    int  prog1_done;     // Flag: program 1 sudah selesai?
    int  prog2_done;     // Flag: program 2 sudah selesai?
} SharedData;

// Operasi semaphore: lock (P operation - decrement)
void sem_lock(int semid) {
    struct sembuf op = {0, -1, 0};
    semop(semid, &op, 1);
}

// Operasi semaphore: unlock (V operation - increment)
void sem_unlock(int semid) {
    struct sembuf op = {0, 1, 0};
    semop(semid, &op, 1);
}

int main() {
    // --- Buat shared memory ---
    int shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid < 0) { perror("shmget"); exit(1); }

    // Attach shared memory ke address space proses ini
    SharedData *shm = (SharedData *)shmat(shmid, NULL, 0);
    if (shm == (void *)-1) { perror("shmat"); exit(1); }

    // Inisialisasi shared data (hanya dilakukan oleh program 1)
    shm->counter   = 0;
    shm->prog1_done = 0;
    shm->prog2_done = 0;

    // --- Buat semaphore ---
    int semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    if (semid < 0) { perror("semget"); exit(1); }

    // Inisialisasi semaphore ke 1 (binary semaphore / mutex)
    semctl(semid, 0, SETVAL, 1);

    printf("[SHM1] Shared memory & semaphore initialized.\n");
    printf("[SHM1] Starting increment (MAX = %d)...\n", MAX);

    // --- Increment counter sebanyak MAX kali ---
    for (int i = 0; i < MAX; i++) {
        sem_lock(semid);    // Lock sebelum akses shared variable
        shm->counter++;     // Critical section
        sem_unlock(semid);  // Unlock setelah selesai
    }

    // Tandai program 1 sudah selesai
    shm->prog1_done = 1;
    printf("[SHM1] Done incrementing. Waiting for SHM2 to finish...\n");

    // Tunggu program 2 selesai
    while (!shm->prog2_done) {
        sleep(1);
    }

    // Cetak hasil akhir
    printf("[SHM1] Expected count : %d\n", MAX * 2);
    printf("[SHM1] Final count    : %d\n", shm->counter);

    // --- Cleanup: detach, hapus shared memory dan semaphore ---
    shmdt(shm);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);

    printf("[SHM1] Shared memory and semaphore cleaned up. Exiting.\n");
    return 0;
}
