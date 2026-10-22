/* Sleeping Teaching Assistant - Pthreads + POSIX semaphores (reference impl.)
 * Build: gcc -Wall -pthread -o sleeping_ta sleeping_ta.c
 * Run:   ./sleeping_ta <chairs> <students> <prog_max_s> <help_s> [rounds]  */
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int chairs, nstud, progmax, helpt, rounds = 3, waiting = 0;
static volatile int done = 0;
static sem_t students, ta_ready;
static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

static void *ta(void *a) {
    (void)a;
    for (;;) {
        printf("[TA] унтаж байна\n");
        sem_wait(&students);                 /* унтах / сэрэх */
        if (done) break;
        pthread_mutex_lock(&m); waiting--; pthread_mutex_unlock(&m);
        sem_post(&ta_ready);                 /* оюутныг дуудах */
        printf("[TA] тусалж байна (%d с)\n", helpt);
        sleep(helpt);
    }
    return NULL;
}
static void *student(void *a) {
    long id = (long)a; unsigned seed = (unsigned)id * 7919u;
    for (int r = 0; r < rounds; r++) {
        sleep(1 + rand_r(&seed) % progmax);  /* програмчилна */
        pthread_mutex_lock(&m);
        if (waiting < chairs) {
            waiting++;
            printf("[S%ld] сандал дээр суув (%d/%d)\n", id, waiting, chairs);
            pthread_mutex_unlock(&m);
            sem_post(&students);             /* TA-г сэрээх */
            sem_wait(&ta_ready);             /* дуудагдахыг хүлээх */
            printf("[S%ld] тусламж авч байна\n", id);
        } else {
            pthread_mutex_unlock(&m);
            printf("[S%ld] сандал дүүрэн, буцав\n", id);
        }
    }
    return NULL;
}
int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IOLBF, 0);
    if (argc < 5 || argc > 6) { fprintf(stderr, "Хэрэглээ: %s chairs students prog_max help [rounds]\n", argv[0]); return 1; }
    chairs = atoi(argv[1]); nstud = atoi(argv[2]); progmax = atoi(argv[3]); helpt = atoi(argv[4]);
    if (argc == 6) rounds = atoi(argv[5]);
    if (chairs < 1 || chairs > 10 || nstud < 1 || nstud > 20 || progmax < 1 || helpt < 1 || rounds < 1) {
        fprintf(stderr, "Алдаа: chairs 1-10, students 1-20, бусад >= 1 байх ёстой.\n"); return 1; }
    sem_init(&students, 0, 0); sem_init(&ta_ready, 0, 0);
    pthread_t t, s[20];
    pthread_create(&t, NULL, ta, NULL);
    for (long i = 0; i < nstud; i++) pthread_create(&s[i], NULL, student, (void *)(i + 1));
    for (int i = 0; i < nstud; i++) pthread_join(s[i], NULL);
    done = 1; sem_post(&students); pthread_join(t, NULL);
    puts("Дууслаа."); return 0;
}
