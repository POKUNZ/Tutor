/*
 * cook.c - Farhan Kebab (Koki / Consumer)
 * Memasak pesanan dari Shared Memory
 * Harus dijalankan SETELAH order.c
 *
 * Compile: gcc -o cook cook.c
 * Run:     ./cook  (setelah ./order berjalan)
 *
 * Jika order di-kill, cook akan ikut ter-kill
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
#define MAX_ORDERS   35
#define TOTAL_ORDERS 100

typedef struct {
    int  jumlah_antrean;
    int  order_masuk;
    int  order_selesai;
    pid_t cook_pid;
} SharedData;

int shmid, semid;
SharedData *shm;

void handle_signal(int sig) {
    printf("\n[COOK] Menerima sinyal %d. Farhan Kebab tutup dapur!\n", sig);
    shmdt(shm);
    exit(0);
}

void sem_wait_op(int id, int num) {
    struct sembuf op = {num, -1, 0};
    semop(id, &op, 1);
}
void sem_signal_op(int id, int num) {
    struct sembuf op = {num, 1, 0};
    semop(id, &op, 1);
}

int main() {
    srand(time(NULL) + 1);
    signal(SIGINT,  handle_signal);
    signal(SIGTERM, handle_signal);

    // Attach ke shared memory yang dibuat order.c
    shmid = shmget(SHM_KEY, sizeof(SharedData), 0666);
    if (shmid < 0) {
        fprintf(stderr, "[COOK] ERROR: Jalankan order.c terlebih dahulu!\n");
        exit(1);
    }

    shm = (SharedData *)shmat(shmid, NULL, 0);
    if (shm == (void *)-1) { perror("shmat"); exit(1); }

    // Simpan PID cook ke shared memory agar order bisa kill cook
    shm->cook_pid = getpid();

    // Attach ke semaphore
    semid = semget(SEM_KEY, 3, 0666);
    if (semid < 0) { perror("semget"); exit(1); }

    printf("[COOK] Farhan Kebab (Koki) siap memasak! PID: %d\n\n", getpid());

    int masak_ke = 0;
    while (masak_ke < TOTAL_ORDERS) {
        // Tunggu ada pesanan (full > 0)
        sem_wait_op(semid, 2);   // tunggu ada pesanan (full--)
        sem_wait_op(semid, 0);   // lock mutex

        // Critical section: ambil pesanan
        shm->jumlah_antrean--;
        shm->order_selesai++;
        masak_ke++;

        printf("[COOK] Sedang Memasak pesanan ke-%d | Antrian tersisa: %d/%d\n",
               masak_ke, shm->jumlah_antrean, MAX_ORDERS);
        fflush(stdout);

        sem_signal_op(semid, 0); // unlock mutex
        sem_signal_op(semid, 1); // sinyal ada slot kosong lagi (empty++)

        // Simulasi waktu memasak (2-4 detik)
        sleep(rand() % 3 + 2);

        printf("[COOK] Pesanan ke-%d selesai! | Total selesai: %d\n",
               masak_ke, shm->order_selesai);
        fflush(stdout);
    }

    printf("\n[COOK] Semua pesanan selesai dimasak! Total: %d\n", masak_ke);
    shmdt(shm);
    return 0;
}
