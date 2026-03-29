#include <stdio.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>

struct Data {
    int menu_id;
};

int main() {
    // 1. Akses Shared Memory yang sama
    key_t key = ftok("resto", 65);
    int shmid = shmget(key, sizeof(struct Data), 0666|IPC_CREAT);
    struct Data *meja = (struct Data*) shmat(shmid, (void*)0, 0);

    //Semaphore
    sem_t *sem_koki = sem_open("/sem_resto", O_CREAT, 0666, 0);

    printf("--- KOKI ---\nMenunggu bel berbunyi...\n");

    while(1) {
        sem_wait(sem_koki); // Menunggu (P operation). Jika 0, dia akan berhenti di sini.
        
        printf("[KOKI]: Bel bunyi! Mengambil pesanan ID %d dari meja...\n", meja->menu_id);
        sleep(2);
        printf("[KOKI]: Pesanan %d selesai!\n", meja->menu_id);
    }

    shmdt(meja);
    shmctl(shmid, IPC_RMID, NULL);
    sem_close(sem_koki);
    sem_unlink("/sem_resto");
    return 0;
}