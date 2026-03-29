#include <stdio.h>
#include <pthread.h>

#define MAX 10000000

pthread_mutex_t mtx1;
pthread_mutex_t mtx2;

/*
 * PERBAIKAN: Deadlock terjadi karena task1 dan task2 mengunci mutex
 * dengan urutan yang BERBEDA (task1: mtx1->mtx2, task2: mtx2->mtx1).
 *
 * Solusi: Gunakan urutan penguncian yang KONSISTEN di kedua thread.
 * Selalu kunci mtx1 dulu, baru mtx2. Ini menghilangkan circular wait.
 *
 * Selain itu, sebenarnya untuk increment satu variabel cukup 1 mutex.
 * Namun untuk tetap mempertahankan struktur asli, kita perbaiki urutannya.
 */

void *task1(void *arg)
{
    int *cnt = (int *)arg;
    for (int i = 0; i < MAX; ++i)
    {
        // Kunci mtx1 dulu, baru mtx2 (urutan konsisten)
        pthread_mutex_lock(&mtx1);
        pthread_mutex_lock(&mtx2);
        (*cnt)++;
        pthread_mutex_unlock(&mtx2); // Unlock dalam urutan terbalik (LIFO)
        pthread_mutex_unlock(&mtx1);
    }
    return NULL;
}

void *task2(void *arg)
{
    int *cnt = (int *)arg;
    for (int i = 0; i < MAX; ++i)
    {
        // DIPERBAIKI: kunci mtx1 dulu, baru mtx2 (sama dengan task1)
        pthread_mutex_lock(&mtx1);  // sebelumnya: lock mtx2 dulu -> DEADLOCK
        pthread_mutex_lock(&mtx2);  // sebelumnya: lock mtx1 dulu -> DEADLOCK
        (*cnt)++;
        pthread_mutex_unlock(&mtx2);
        pthread_mutex_unlock(&mtx1);
    }
    return NULL;
}

int main()
{
    pthread_t thrd1, thrd2;
    unsigned int count = 0;

    pthread_mutex_init(&mtx1, NULL);
    pthread_mutex_init(&mtx2, NULL);

    pthread_create(&thrd1, NULL, task1, &count);
    pthread_create(&thrd2, NULL, task2, &count);

    pthread_join(thrd1, NULL);
    pthread_join(thrd2, NULL);

    pthread_mutex_destroy(&mtx1);
    pthread_mutex_destroy(&mtx2);

    printf("Expected count: %u\n", MAX * 2);
    printf("Final count:    %u\n", count);

    return 0;
}
