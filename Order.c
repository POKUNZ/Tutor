#include <stdio.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>

struct Data {
    int menu_id;
};

int main() {
    
    key_t key = ftok("resto", 65);
    int shmid = shmget(key, sizeof(struct Data), 0666|IPC_CREAT);
    struct Data meja = (struct Data) shmat(shmid, (void*)0, 0);

   
    sem_t *sem_koki = sem_open("/sem_resto", O_CREAT, 0666, 0);

    printf("--- PELANGGAN ---\nMasukkan ID Menu: ");
    scanf("%d", &meja->menu_id);

    printf("Pesanan ditaruh di meja. Membunyikan bel!\n");
    sem_post(sem_koki); 

    shmdt(meja);
    sem_close(sem_koki);
    return 0;
}

