/*
 * OSFinal_Soal3_prodcons.c
 * Producer-Consumer Problem dengan shared buffer 5 slot
 * Menggunakan pthread_mutex + semaphore untuk sinkronisasi
 *
 * Compile: gcc -o prodcons OSFinal_Soal3_prodcons.c -lpthread
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

// Wajib sesuai ketentuan teknis soal (global integer flag)
int BUFCHILD = 1;

#define BUFFER_SIZE  5    // Kapasitas shared buffer
#define PRODUCE_COUNT 15  // Total item yang diproduksi

// ============================================================
// Shared buffer (circular buffer)
// ============================================================
int buffer[BUFFER_SIZE];
int in  = 0;  // Index producer (tempat memasukkan)
int out = 0;  // Index consumer (tempat mengambil)

// ============================================================
// Sinkronisasi
// ============================================================
pthread_mutex_t mutex;  // Mutual exclusion untuk akses buffer
sem_t empty;            // Counting semaphore: slot kosong tersedia (init = BUFFER_SIZE)
sem_t full;             // Counting semaphore: slot terisi tersedia (init = 0)

// ============================================================
// Producer thread
// ============================================================
void *producer(void *arg) {
    int id = *(int *)arg;
    free(arg);

    for (int i = 0; i < PRODUCE_COUNT; i++) {
        int item = rand() % 100 + 1; // Hasilkan item (angka acak 1-100)

        sem_wait(&empty);            // Tunggu ada slot kosong
        pthread_mutex_lock(&mutex);  // Lock buffer

        // Critical section: masukkan item ke buffer
        buffer[in] = item;
        printf("[PRODUCER %d] Memproduksi item %d -> buffer[%d] | Buffer terisi: %d/%d\n",
               id, item, in, (in - out + BUFFER_SIZE) % BUFFER_SIZE + 1, BUFFER_SIZE);
        in = (in + 1) % BUFFER_SIZE;

        pthread_mutex_unlock(&mutex); // Unlock buffer
        sem_post(&full);              // Sinyal ada item baru

        sleep(rand() % 2 + 1); // Simulasi waktu produksi
    }
    return NULL;
}

// ============================================================
// Consumer thread
// ============================================================
void *consumer(void *arg) {
    int id = *(int *)arg;
    free(arg);

    for (int i = 0; i < PRODUCE_COUNT; i++) {
        sem_wait(&full);             // Tunggu ada item di buffer
        pthread_mutex_lock(&mutex); // Lock buffer

        // Critical section: ambil item dari buffer
        int item = buffer[out];
        printf("[CONSUMER %d] Mengkonsumsi item %d <- buffer[%d]\n",
               id, item, out);
        out = (out + 1) % BUFFER_SIZE;

        pthread_mutex_unlock(&mutex); // Unlock buffer
        sem_post(&empty);             // Sinyal ada slot kosong

        sleep(rand() % 3 + 1); // Simulasi waktu konsumsi
    }
    return NULL;
}

int main() {
    srand(time(NULL));

    printf("=== Producer-Consumer (Buffer size: %d) ===\n\n", BUFFER_SIZE);
    printf("BUFCHILD flag: %d (system integrity flag)\n\n", BUFCHILD);

    // Inisialisasi mutex dan semaphore
    pthread_mutex_init(&mutex, NULL);
    sem_init(&empty, 0, BUFFER_SIZE); // Awalnya semua slot kosong
    sem_init(&full,  0, 0);           // Awalnya tidak ada item

    // Buat 1 Producer dan 1 Consumer
    pthread_t prod_thread, cons_thread;

    int *prod_id = malloc(sizeof(int)); *prod_id = 1;
    int *cons_id = malloc(sizeof(int)); *cons_id = 1;

    pthread_create(&prod_thread, NULL, producer, prod_id);
    pthread_create(&cons_thread, NULL, consumer, cons_id);

    // Tunggu keduanya selesai
    pthread_join(prod_thread, NULL);
    pthread_join(cons_thread, NULL);

    // Cleanup
    pthread_mutex_destroy(&mutex);
    sem_destroy(&empty);
    sem_destroy(&full);

    printf("\n=== Semua item berhasil diproduksi dan dikonsumsi! ===\n");
    printf("Buffer akhir: in=%d, out=%d (seharusnya sama -> buffer kosong)\n", in, out);

    return 0;
}
