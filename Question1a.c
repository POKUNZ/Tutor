#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

/*
 * Process Hierarchy:
 *        A
 *       / \
 *      B   C
 *      |  / \
 *      D E   F
 */

int main() {
    pid_t pidB, pidC, pidD, pidE, pidF;

    // ============================================================
    // Process A (root / original process)
    // ============================================================
    printf("[A] Process A started (PID: %d)\n", getpid());

    // --- Fork B ---
    pidB = fork();
    if (pidB < 0) { perror("fork B"); exit(1); }

    if (pidB == 0) {
        // ============================================================
        // PROCESS B
        // ============================================================
        printf("[B] Process B started (PID: %d, PPID: %d)\n", getpid(), getppid());

        pidD = fork();
        if (pidD < 0) { perror("fork D"); exit(1); }

        if (pidD == 0) {
            // ============================================================
            // PROCESS D (leaf node under B)
            // ============================================================
            printf("[D] Process D started (PID: %d, PPID: %d)\n", getpid(), getppid());
            printf("[D] Process D finished\n");
            exit(0);
        }

        waitpid(pidD, NULL, 0); // B reaps D to prevent zombie
        printf("[B] Process B finished\n");
        exit(0);
    }

    // Wait for B to complete before forking C
    // (ensures ordered output B->D before C->E,F)
    waitpid(pidB, NULL, 0);

    // --- Fork C ---
    pidC = fork();
    if (pidC < 0) { perror("fork C"); exit(1); }

    if (pidC == 0) {
        // ============================================================
        // PROCESS C
        // ============================================================
        printf("[C] Process C started (PID: %d, PPID: %d)\n", getpid(), getppid());

        pidE = fork();
        if (pidE < 0) { perror("fork E"); exit(1); }

        if (pidE == 0) {
            // ============================================================
            // PROCESS E (leaf node under C)
            // ============================================================
            printf("[E] Process E started (PID: %d, PPID: %d)\n", getpid(), getppid());
            printf("[E] Process E finished\n");
            exit(0);
        }

        pidF = fork();
        if (pidF < 0) { perror("fork F"); exit(1); }

        if (pidF == 0) {
            // ============================================================
            // PROCESS F (leaf node under C)
            // ============================================================
            printf("[F] Process F started (PID: %d, PPID: %d)\n", getpid(), getppid());
            printf("[F] Process F finished\n");
            exit(0);
        }

        waitpid(pidE, NULL, 0); // C reaps E to prevent zombie
        waitpid(pidF, NULL, 0); // C reaps F to prevent zombie
        printf("[C] Process C finished\n");
        exit(0);
    }

    // A waits for C (B already waited above)
    waitpid(pidC, NULL, 0);
    printf("[A] Process A finished\n");

    return 0;
}
