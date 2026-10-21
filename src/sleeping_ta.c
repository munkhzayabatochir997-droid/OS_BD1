/*
 * The Sleeping Teaching Assistant  (Silberschatz, Synchronization Examples)
 * Pthreads + POSIX semaphores + mutex.  Бодит thread-үүдээс event log (CSV) гаргана;
 * UI (src/sleeping_ta.html) тэр log-оор animation хийнэ.
 *
 * Build: gcc -Wall -Wextra -O2 -pthread -o build/sleeping_ta src/sleeping_ta.c
 * Run:   ./build/sleeping_ta -c 3 -n 5 -p 1 -d 0.5 -r 3 -s 1 -o run.csv
 */
#define _GNU_SOURCE
#include <errno.h>
#include <getopt.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAXC 10
#define MAXN 20

static int    chairs = 3, nstud = 5, rounds = 3;
static double prog_s = 2.0, help_s = 1.0;
static unsigned seed = 1;
static int    quiet = 0;
static const char *outpath = NULL;

/* Хуваалцсан төлөв: бүгд mutex `m`-ээр хамгаалагдана */
static int queue[MAXC], qhead = 0, qcount = 0;   /* FIFO: хүлээх сандлууд */
static int ta_done = 0;                           /* бүх оюутан дууссан */
static pthread_mutex_t m  = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t lm = PTHREAD_MUTEX_INITIALIZER;   /* log-ийн mutex */
static sem_t students_sem;                        /* хүлээж буй оюутны тоо; 0 бол TA унтна */
static sem_t call_sem[MAXN + 1], fin_sem[MAXN + 1];      /* TA дуудсан / тусалж дууссан */

static FILE *logf = NULL;
static struct timespec t0;

static double now_ms(void) {
    struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t);
    return (t.tv_sec - t0.tv_sec) * 1000.0 + (t.tv_nsec - t0.tv_nsec) / 1e6;
}
static void sleep_s(double s) {
    struct timespec ts = { (time_t)s, (long)((s - (time_t)s) * 1e9) };
    while (nanosleep(&ts, &ts) == -1 && errno == EINTR) {}
}
/* event бүрийг хугацаатай нь (lm түгжээ дотор, тиймээс цаг монотон) бичнэ */
static void ev(const char *name, int s, int q) {
    pthread_mutex_lock(&lm);
    double t = now_ms();
    if (logf) { fprintf(logf, "%.3f,%s,%d,%d\n", t, name, s, q); fflush(logf); }
    if (!quiet) {
        if (s) printf("[%9.1f ms] %-10s студент %-2d  дараалал %d/%d\n", t, name, s, q, chairs);
        else   printf("[%9.1f ms] %-10s              дараалал %d/%d\n", t, name, q, chairs);
        fflush(stdout);
    }
    pthread_mutex_unlock(&lm);
}

static void *ta_thread(void *arg) {
    (void)arg;
    for (;;) {
        int slept = 0;
        pthread_mutex_lock(&m);
        if (qcount == 0 && !ta_done) { ev("TA_SLEEP", 0, 0); slept = 1; }
        pthread_mutex_unlock(&m);

        sem_wait(&students_sem);                 /* унтах / сэрэх цэг */

        pthread_mutex_lock(&m);
        if (qcount == 0 && ta_done) { pthread_mutex_unlock(&m); break; }
        if (slept) ev("TA_WAKE", 0, qcount);
        int s = queue[qhead]; qhead = (qhead + 1) % MAXC; qcount--;   /* FIFO pop */
        ev("HELP_START", s, qcount);
        pthread_mutex_unlock(&m);

        sem_post(&call_sem[s]);                  /* оюутныг дуудна */
        sleep_s(help_s);                         /* тусалж байна */
        pthread_mutex_lock(&m); ev("HELP_END", s, qcount); pthread_mutex_unlock(&m);
        sem_post(&fin_sem[s]);                   /* оюутан чөлөөлөгдөнө */
    }
    return NULL;
}

static void *student_thread(void *arg) {
    int id = (int)(intptr_t)arg;
    unsigned sd = seed * 1000003u + (unsigned)id * 7919u;
    for (int r = 0; r < rounds; r++) {
        sleep_s(prog_s * (0.5 + rand_r(&sd) / (double)RAND_MAX));   /* програмчилна */
        pthread_mutex_lock(&m);
        ev("ARRIVE", id, qcount);
        if (qcount < chairs) {
            queue[(qhead + qcount) % MAXC] = id; qcount++;
            ev("SIT", id, qcount);
            pthread_mutex_unlock(&m);
            sem_post(&students_sem);             /* TA-г сэрээх / дарааллыг мэдэгдэх */
            sem_wait(&call_sem[id]);             /* дуудагдахыг хүлээх */
            sem_wait(&fin_sem[id]);              /* тусламж дуустал хүлээх */
        } else {
            ev("FULL", id, qcount);              /* сандал дүүрэн: буцна */
            pthread_mutex_unlock(&m);
        }
    }
    return NULL;
}

static void usage(const char *p) {
    fprintf(stderr,
        "Хэрэглээ: %s [-c сандал 1-10] [-n оюутан 1-20] [-p програмчлах дундаж с 0.01-60]\n"
        "          [-d тусламжийн хугацаа с 0.01-60] [-r мөчлөгийн тоо 1-100] [-s seed]\n"
        "          [-o log.csv] [-q]\n", p);
}
static int pint(const char *s, long lo, long hi, const char *nm, long *out) {
    char *e; errno = 0; long v = strtol(s, &e, 10);
    if (errno || !*s || *e) { fprintf(stderr, "Алдаа: %s бүхэл тоо байх ёстой (өгсөн: '%s').\n", nm, s); return -1; }
    if (v < lo || v > hi)   { fprintf(stderr, "Алдаа: %s нь %ld-%ld хооронд байх ёстой (өгсөн: %ld).\n", nm, lo, hi, v); return -1; }
    *out = v; return 0;
}
static int pdbl(const char *s, double lo, double hi, const char *nm, double *out) {
    char *e; errno = 0; double v = strtod(s, &e);
    if (errno || !*s || *e || v != v) { fprintf(stderr, "Алдаа: %s тоо байх ёстой (өгсөн: '%s').\n", nm, s); return -1; }
    if (v < lo || v > hi) { fprintf(stderr, "Алдаа: %s нь %g-%g хооронд байх ёстой (өгсөн: %g).\n", nm, lo, hi, v); return -1; }
    *out = v; return 0;
}

int main(int argc, char **argv) {
    long li; double d; int c;
    opterr = 0;
    if (argc == 2 && !strcmp(argv[1], "--help")) { usage(argv[0]); return 0; }
    while ((c = getopt(argc, argv, "c:n:p:d:r:s:o:q")) != -1) {
        switch (c) {
        case 'c': if (pint(optarg, 1, MAXC, "chairs (-c)", &li)) return 2; chairs = (int)li; break;
        case 'n': if (pint(optarg, 1, MAXN, "students (-n)", &li)) return 2; nstud = (int)li; break;
        case 'r': if (pint(optarg, 1, 100, "rounds (-r)", &li)) return 2; rounds = (int)li; break;
        case 's': if (pint(optarg, 0, 1000000, "seed (-s)", &li)) return 2; seed = (unsigned)li; break;
        case 'p': if (pdbl(optarg, 0.01, 60, "prog (-p)", &d)) return 2; prog_s = d; break;
        case 'd': if (pdbl(optarg, 0.01, 60, "help (-d)", &d)) return 2; help_s = d; break;
        case 'o': outpath = optarg; break;
        case 'q': quiet = 1; break;
        default:
            if (optopt) fprintf(stderr, "Алдаа: -%c сонголт буруу эсвэл утга дутуу.\n", optopt);
            usage(argv[0]); return 2;
        }
    }
    if (optind < argc) { fprintf(stderr, "Алдаа: үл мэдэгдэх аргумент '%s'.\n", argv[optind]); usage(argv[0]); return 2; }
    if (outpath && !(logf = fopen(outpath, "w"))) { perror("log файл нээж чадсангүй"); return 1; }
    if (logf) fprintf(logf, "# chairs=%d students=%d prog=%g help=%g rounds=%d seed=%u\ntime_ms,event,student,queue_len\n",
                      chairs, nstud, prog_s, help_s, rounds, seed);

    sem_init(&students_sem, 0, 0);
    for (int i = 0; i <= MAXN; i++) { sem_init(&call_sem[i], 0, 0); sem_init(&fin_sem[i], 0, 0); }
    clock_gettime(CLOCK_MONOTONIC, &t0);

    pthread_t ta, st[MAXN + 1];
    pthread_create(&ta, NULL, ta_thread, NULL);
    for (int i = 1; i <= nstud; i++) pthread_create(&st[i], NULL, student_thread, (void *)(intptr_t)i);
    for (int i = 1; i <= nstud; i++) pthread_join(st[i], NULL);

    pthread_mutex_lock(&m); ta_done = 1; pthread_mutex_unlock(&m);
    sem_post(&students_sem);                     /* унтаж буй TA-г сэрээж гаргана */
    pthread_join(ta, NULL);
    ev("END", 0, 0);
    if (logf) fclose(logf);
    return 0;
}
