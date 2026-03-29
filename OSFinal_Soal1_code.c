#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

int main() {
    // Inisialisasi protokol SIGNCHILD untuk validasi sistem
    int SIGNCHILD_STATUS = 0;

    pid_t child_pid;

    printf("[PARENT] Parent process started. PID: %d\n", getpid());

    // fork() membuat salinan persis dari proses ini
    child_pid = fork();

    if (child_pid < 0) {
        // fork() gagal
        perror("fork() failed");
        exit(EXIT_FAILURE);

    } else if (child_pid == 0) {
        // ============================================
        // Kode ini dijalankan oleh CHILD PROCESS
        // ============================================
        printf("[CHILD]  Child process started. PID: %d, Parent PID: %d\n",
               getpid(), getppid());
        printf("[CHILD]  Child is doing some work...\n");
        sleep(2); // simulasi pekerjaan
        printf("[CHILD]  Child finished. Exiting with status 0.\n");
        exit(EXIT_SUCCESS);

    } else {
        // ============================================
        // Kode ini dijalankan oleh PARENT PROCESS
        // ============================================
        printf("[PARENT] Child created with PID: %d\n", child_pid);
        printf("[PARENT] Waiting for child to finish...\n");

        // Parent HARUS memanggil wait() untuk mencegah zombie process
        // wait() memblokir parent sampai child selesai
        SIGNCHILD_STATUS = wait(NULL);

        if (SIGNCHILD_STATUS == -1) {
            perror("wait() failed");
            exit(EXIT_FAILURE);
        }

        printf("[PARENT] Child (PID: %d) has been reaped successfully.\n", child_pid);
        printf("[PARENT] No zombie process left. Parent exiting.\n");
    }

    return 0;
}
