/*
 * Soal 2 - Alur Komando Jend. Rusdi
 * Hierarki: A -> B, C
 *           B -> D, E
 *
 * Urutan output: A => B => D => E => C
 *
 * - Process A: menerima input kalimat dari keyboard
 * - Process B: meneruskan ke D dan E
 * - Process D: mencetak kalimat apa adanya
 * - Process E: mencetak kalimat apa adanya
 * - Process C: mencetak kalimat dalam HURUF KAPITAL
 *
 * Compile: gcc -o soal2 Soal2_hierarchy.c
 * Run:     ./soal2
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>

#define MSG_SIZE 256

// Fungsi mengubah string menjadi huruf kapital semua
void to_uppercase(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = toupper((unsigned char)str[i]);
    }
}

int main() {
    char message[MSG_SIZE];
    pid_t pidB, pidC, pidD, pidE;

    // ============================================================
    // PROCESS A - menerima input dari keyboard
    // ============================================================
    printf("[A] Jendral Rusdi (Process A) - PID: %d\n", getpid());
    printf("[A] Masukkan pesan: ");
    fflush(stdout);
    fgets(message, MSG_SIZE, stdin);
    // Hapus newline di akhir
    message[strcspn(message, "\n")] = '\0';

    printf("[A] Pesan diterima: \"%s\"\n", message);
    printf("[A] Menurunkan perintah ke B dan C...\n\n");
    fflush(stdout);

    // --- Fork B ---
    pidB = fork();
    if (pidB < 0) { perror("fork B"); exit(1); }

    if (pidB == 0) {
        // ============================================================
        // PROCESS B - Mayor Reza Kecap
        // ============================================================
        printf("[B] Mayor Reza Kecap (Process B) - PID: %d, PPID: %d\n",
               getpid(), getppid());
        printf("[B] Pesan diterima: \"%s\"\n", message);
        printf("[B] Membagi tugas ke Sersan D dan E...\n\n");
        fflush(stdout);

        // Fork D
        pidD = fork();
        if (pidD < 0) { perror("fork D"); exit(1); }

        if (pidD == 0) {
            // ============================================================
            // PROCESS D - Sersan Ambatukam
            // ============================================================
            printf("[D] Sersan Ambatukam (Process D) - PID: %d, PPID: %d\n",
                   getpid(), getppid());
            printf("[D] Pesan: \"%s\"\n\n", message);
            fflush(stdout);
            exit(0);
        }

        waitpid(pidD, NULL, 0); // B tunggu D selesai

        // Fork E
        pidE = fork();
        if (pidE < 0) { perror("fork E"); exit(1); }

        if (pidE == 0) {
            // ============================================================
            // PROCESS E - Sersan Ambatublow
            // ============================================================
            printf("[E] Sersan Ambatublow (Process E) - PID: %d, PPID: %d\n",
                   getpid(), getppid());
            printf("[E] Pesan: \"%s\"\n\n", message);
            fflush(stdout);
            exit(0);
        }

        waitpid(pidE, NULL, 0); // B tunggu E selesai
        printf("[B] Semua tugas selesai.\n\n");
        fflush(stdout);
        exit(0);
    }

    waitpid(pidB, NULL, 0); // A tunggu B selesai sebelum fork C

    // --- Fork C ---
    pidC = fork();
    if (pidC < 0) { perror("fork C"); exit(1); }

    if (pidC == 0) {
        // ============================================================
        // PROCESS C - Mayor Epstein (ubah ke HURUF KAPITAL)
        // ============================================================
        char upper_msg[MSG_SIZE];
        strncpy(upper_msg, message, MSG_SIZE);
        to_uppercase(upper_msg);

        printf("[C] Mayor Epstein (Process C) - PID: %d, PPID: %d\n",
               getpid(), getppid());
        printf("[C] Pesan rahasia (KAPITAL): \"%s\"\n\n", upper_msg);
        fflush(stdout);
        exit(0);
    }

    waitpid(pidC, NULL, 0); // A tunggu C selesai

    printf("[A] Semua komando telah dieksekusi. Program selesai.\n");
    return 0;
}
