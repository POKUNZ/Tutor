/*
 * OSFinal_Soal2_mutex.c
 * Dining Philosophers Problem - Solusi dengan pthread_mutex_t
 *
 * Solusi deadlock prevention: filosof dengan ID GENAP ambil garpu KANAN dulu,
 * filosof dengan ID GANJIL ambil garpu KIRI dulu.
 * Ini memecah circular wait -> tidak ada deadlock.
 *
 * Compile: gcc -o phil_mutex OSFinal_Soal2_mutex.c -lpthread
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define NUM_PHILOSOPHERS 5
#define EATING_ROUNDS    3  // Setiap filosof makan sebanyak ini

pthread_mutex_t chopstick[NUM_PHILOSOPHERS]; // 1 mutex per garpu

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

        // Solusi anti-deadlock: filosof genap ambil kanan dulu, ganjil ambil kiri dulu
        // Ini memecah circular wait
        if (id % 2 == 0) {
            pthread_mutex_lock(&chopstick[right]); // ambil kanan
            printf("[%s] mengambil garpu kanan (%d)\n", names[id], right);
            pthread_mutex_lock(&chopstick[left]);  // ambil kiri
            printf("[%s] mengambil garpu kiri  (%d)\n", names[id], left);
        } else {
            pthread_mutex_lock(&chopstick[left]);  // ambil kiri
            printf("[%s] mengambil garpu kiri  (%d)\n", names[id], left);
            pthread_mutex_lock(&chopstick[right]); // ambil kanan
            printf("[%s] mengambil garpu kanan (%d)\n", names[id], right);
        }

        eat(id);

        // Letakkan kembali kedua garpu
        pthread_mutex_unlock(&chopstick[left]);
        pthread_mutex_unlock(&chopstick[right]);
        printf("[%s] meletakkan garpu dan kembali berpikir.\n", names[id]);
    }

    printf("[%s] selesai makan %d ronde.\n", names[id], EATING_ROUNDS);
    return NULL;
}

int main() {
    srand(time(NULL));
    pthread_t threads[NUM_PHILOSOPHERS];

    // Inisialisasi semua mutex (garpu)
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_mutex_init(&chopstick[i], NULL);
    }

    printf("=== Dining Philosophers (MUTEX) ===\n\n");

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

    // Destroy semua mutex
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_mutex_destroy(&chopstick[i]);
    }

    printf("\n=== Semua filosof selesai makan. ===\n");
    return 0;
}
