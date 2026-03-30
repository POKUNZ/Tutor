/*
 * pipe_simple.c
 * Contoh penggunaan pipe sederhana antara parent dan child process
 *
 * Parent menulis pesan ke pipe, child membaca dan mencetaknya.
 *
 * Compile: gcc -o pipe_simple pipe_simple.c
 * Run:     ./pipe_simple
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main() {
    int fd[2];       // fd[0] = read end, fd[1] = write end
    pid_t pid;
    char message[] = "Halo dari Parent Process!";
    char buffer[100];

    // Buat pipe
    if (pipe(fd) < 0) {
        perror("pipe failed");
        exit(1);
    }

    pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(1);
    }

    if (pid == 0) {
        // ==============================
        // CHILD PROCESS - membaca pipe
        // ==============================
        close(fd[1]); // Tutup write end (child tidak perlu)

        read(fd[0], buffer, sizeof(buffer));
        printf("[CHILD]  Menerima pesan: \"%s\"\n", buffer);

        close(fd[0]);
        exit(0);

    } else {
        // ==============================
        // PARENT PROCESS - menulis pipe
        // ==============================
        close(fd[0]); // Tutup read end (parent tidak perlu)

        printf("[PARENT] Mengirim pesan: \"%s\"\n", message);
        write(fd[1], message, strlen(message) + 1);

        close(fd[1]);
        wait(NULL); // Tunggu child selesai
        printf("[PARENT] Child selesai membaca.\n");
    }

    return 0;
}
