/*
 * OSFinal_Soal2_semaphore.c
 * Dining Philosophers Problem - Solusi dengan POSIX Semaphore
 *
 * Menggunakan sem_t (POSIX semaphore) sebagai pengganti mutex.
 * Solusi anti-deadlock: batasi maksimum 4 filosof yang boleh
 * mencoba mengambil garpu secara bersamaan (room semaphore).
 * Dengan hanya 4 dari 5 yang bisa mencoba, paling tidak 1 filosof
 * pasti bisa mendapat kedua garpunya -> tidak ada deadlock.
 *
 * Compile: gcc -o phil_sem OSFinal_Soal2_semaphore.c -lpthread
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define NUM_PHILOSOPHERS 5
#define EATING_ROUNDS    3

sem_t chopstick[NUM_PHILOSOPHERS]; // 1 semaphore per garpu (binary: 0 atau 1)
sem_t room;                        // Batasi max 4 filosof di "ruang makan"

const char *names[] = {"Socrates", "Plato", "Aristotle", "Descartes", "Kant"};

void think(int id) {
    printf("[%s] sedang berpikir...\n", names[id]);
    sleep(rand() % 3 + 1);
}

void eat(int id) {
    printf("[%s] sedang MAKAN!\n", names[id]);
    sleep(rand() % 2 + 1);
}

void *philosopher(void *arg) {
    int id = *(int *)arg;
    free(arg);

    int left  = id;
    int right = (id + 1) % NUM_PHILOSOPHERS;

    for (int round = 0; round < EATING_ROUNDS; round++) {
        think(id);

        // Masuk "ruang makan" - max 4 filosof (mencegah deadlock circular wait)
        sem_wait(&room);

        // Ambil garpu kiri
        sem_wait(&chopstick[left]);
        printf("[%s] mengambil garpu kiri  (%d)\n", names[id], left);

        // Ambil garpu kanan
        sem_wait(&chopstick[right]);
        printf("[%s] mengambil garpu kanan (%d)\n", names[id], right);

        eat(id);

        // Letakkan garpu kembali
        sem_post(&chopstick[right]);
        sem_post(&chopstick[left]);
        printf("[%s] meletakkan garpu dan kembali berpikir.\n", names[id]);

        // Keluar dari ruang makan
        sem_post(&room);
    }

    printf("[%s] selesai makan %d ronde.\n", names[id], EATING_ROUNDS);
    return NULL;
}

int main() {
    srand(time(NULL));
    pthread_t threads[NUM_PHILOSOPHERS];

    // Inisialisasi semaphore garpu: nilai awal 1 (tersedia)
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        sem_init(&chopstick[i], 0, 1);
    }

    // Inisialisasi room semaphore: nilai awal NUM_PHILOSOPHERS - 1 = 4
    sem_init(&room, 0, NUM_PHILOSOPHERS - 1);

    printf("=== Dining Philosophers (SEMAPHORE) ===\n\n");

    // Buat thread untuk setiap filosof
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        int *id = malloc(sizeof(int));
        *id = i;
        pthread_create(&threads[i], NULL, philosopher, id);
    }

    // Tunggu semua filosof selesai
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Destroy semua semaphore
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        sem_destroy(&chopstick[i]);
    }
    sem_destroy(&room);

    printf("\n=== Semua filosof selesai makan. ===\n");
    return 0;
}
