/*
 * order.c - Si Imut (Pelayan / Producer)
 * Mencatat pesanan ke Shared Memory
 * Harus dijalankan SEBELUM cook.c
 *
 * Compile: gcc -o order order.c
 * Run:     ./order
 *
 * Jika order di-kill, cook ikut ter-kill via SIGTERM
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define SHM_KEY      0xABCD
#define SEM_KEY      0xEF01
#define MAX_ORDERS   35       // Kapasitas "Kitab Catatan Digital"
#define TOTAL_ORDERS 100      // Total pesanan yang akan masuk

typedef struct {
    int  jumlah_antrean;   // Jumlah pesanan saat ini
    int  order_masuk;      // Total pesanan yang sudah masuk
    int  order_selesai;    // Total pesanan yang sudah dimasak
    pid_t cook_pid;        // PID cook.c untuk di-kill saat order di-kill
} SharedData;

int shmid, semid;
SharedData *shm;

// Signal handler: jika order di-kill, kirim SIGTERM ke cook juga
void handle_signal(int sig) {
    printf("\n[ORDER] Menerima sinyal %d. Menghentikan cook...\n", sig);
    if (shm && shm->cook_pid > 0) {
        kill(shm->cook_pid, SIGTERM);
    }
    shmdt(shm);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);
    exit(0);
}

// Semaphore operations
void sem_wait_op(int id, int num) {
    struct sembuf op = {num, -1, 0};
    semop(id, &op, 1);
}
void sem_signal_op(int id, int num) {
    struct sembuf op = {num, 1, 0};
    semop(id, &op, 1);
}

int main() {
    srand(time(NULL));
    signal(SIGINT,  handle_signal);
    signal(SIGTERM, handle_signal);

    // Buat shared memory
    shmid = shmget(SHM_KEY, sizeof(SharedData), IPC_CREAT | 0666);
    if (shmid < 0) { perror("shmget"); exit(1); }

    shm = (SharedData *)shmat(shmid, NULL, 0);
    if (shm == (void *)-1) { perror("shmat"); exit(1); }

    // Inisialisasi
    shm->jumlah_antrean = 0;
    shm->order_masuk    = 0;
    shm->order_selesai  = 0;
    shm->cook_pid       = 0;

    /*
     * Buat 3 semaphore:
     * [0] = mutex (binary, init=1) untuk mutual exclusion
     * [1] = empty  (counting, init=MAX_ORDERS) - slot kosong tersedia
     * [2] = full   (counting, init=0) - slot terisi (pesanan tersedia)
     */
    semid = semget(SEM_KEY, 3, IPC_CREAT | 0666);
    if (semid < 0) { perror("semget"); exit(1); }

    semctl(semid, 0, SETVAL, 1);           // mutex
    semctl(semid, 1, SETVAL, MAX_ORDERS);  // empty slots
    semctl(semid, 2, SETVAL, 0);           // full slots

    printf("[ORDER] Kitab Catatan Digital siap. Kapasitas: %d pesanan.\n", MAX_ORDERS);
    printf("[ORDER] Si Imut (Pelayan) mulai menerima pesanan...\n\n");

    for (int i = 1; i <= TOTAL_ORDERS; i++) {
        // Tunggu jika buffer penuh
        sem_wait_op(semid, 1);   // tunggu ada slot kosong (empty--)
        sem_wait_op(semid, 0);   // lock mutex

        // Critical section: tambah pesanan
        shm->jumlah_antrean++;
        shm->order_masuk++;

        printf("[ORDER] Pesanan #%d masuk! | Antrian sekarang: %d/%d\n",
               i, shm->jumlah_antrean, MAX_ORDERS);
        fflush(stdout);

        sem_signal_op(semid, 0); // unlock mutex
        sem_signal_op(semid, 2); // sinyal ada pesanan baru (full++)

        // Simulasi waktu mencatat pesanan (1-3 detik)
        sleep(rand() % 3 + 1);
    }

    printf("\n[ORDER] Semua %d pesanan sudah dicatat. Menunggu koki selesai...\n",
           TOTAL_ORDERS);

    // Tunggu semua pesanan selesai dimasak
    while (shm->order_selesai < TOTAL_ORDERS) {
        sleep(1);
    }

    printf("[ORDER] Semua pesanan selesai! Total masuk: %d, Total masak: %d\n",
           shm->order_masuk, shm->order_selesai);

    // Cleanup
    shmdt(shm);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);
    printf("[ORDER] Sistem dimatikan. Sampai jumpa!\n");

    return 0;
}
