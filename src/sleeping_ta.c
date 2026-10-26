/*
 * The Sleeping Teaching Assistant  (Silberschatz, Synchronization Examples)
 * Pthreads + POSIX semaphores + mutex.  Бодит thread-үүдээс event log (CSV) гаргана;
 * UI (src/sleeping_ta.html) тэр log-оор animation хийнэ.
 *
 * Build: gcc -Wall -Wextra -O2 -pthread -o build/sleeping_ta src/sleeping_ta.c
 * Run:   ./build/sleeping_ta -c 3 -n 5 -p 1 -d 0.5 -r 3 -s 1 -o run.csv
 *        ./build/sleeping_ta -c 1 -n 8 -p 0.05 -d 0.05 -r 3 --no-mutex -o race.csv   (race condition үзүүлэх)
 *
 * Платформ: Linux (sem_init) ба macOS (sem_init дэмжигддэггүй тул нэртэй sem_open).
 */
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
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
static int    use_mutex = 1;          /* --no-mutex үед 0: waiting/queue-г хамгаалахгүй (race үзүүлэх) */
static const char *outpath = NULL;

/* Хуваалцсан төлөв: бүгд mutex `m`-ээр хамгаалагдана */
static int queue[MAXC], qhead = 0, qcount = 0;   /* FIFO: хүлээх сандлууд */
static int ta_done = 0;
static int n_helped = 0, n_turned = 0, n_sleeps = 0;   /* дүнгийн тоолуурууд */
static pthread_mutex_t m  = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t lm = PTHREAD_MUTEX_INITIALIZER;
static sem_t *students_sem;                       /* хүлээж буй оюутны тоо; 0 бол TA унтна */
static sem_t *call_sem[MAXN + 1], *fin_sem[MAXN + 1];    /* TA дуудсан / тусалж дууссан */
#define NSEM (1 + 2 * (MAXN + 1))
#ifndef __APPLE__
static sem_t sem_store[NSEM];
#endif

/* `m`-ийг зөвхөн use_mutex үед түгжинэ. (log-ийн `lm` үргэлж ажиллана: файл эвдрэхгүй.) */
#define M_LOCK()   do { if (use_mutex) pthread_mutex_lock(&m); } while (0)
#define M_UNLOCK() do { if (use_mutex) pthread_mutex_unlock(&m); } while (0)

/* Семафор үүсгэх: Linux дээр sem_init, macOS дээр нэргүй семафор байдаггүй тул sem_open. */
static int sem_create(sem_t **out, int idx) {
#ifdef __APPLE__
    char name[32];
    snprintf(name, sizeof name, "/sta%d_%d", (int)getpid(), idx);
    sem_unlink(name);
    sem_t *s = sem_open(name, O_CREAT | O_EXCL, 0600, 0);
    if (s == SEM_FAILED) return -1;
    sem_unlink(name);                             /* нэрийг шууд устгана: програм унавал ч үлдэхгүй */
    *out = s;
#else
    if (sem_init(&sem_store[idx], 0, 0) != 0) return -1;
    *out = &sem_store[idx];
#endif
    return 0;
}
static void sem_release(sem_t *s) {
#ifdef __APPLE__
    sem_close(s);
#else
    sem_destroy(s);
#endif
}

/* --no-mutex үед "шалгах -> суух" хоорондын цонхыг 1 мс болгон өргөсгөнө. Ингэхгүй бол race
 * маш ховор гарч үзүүлэхэд хэцүү. Mutex-тэй горимд ажиллахгүй. */
static void race_window(void) { if (!use_mutex) { struct timespec ts = {0, 1000000L}; nanosleep(&ts, NULL); } }

/* --no-mutex үед race нь оюутныг хэзээ ч дуудагдахгүй болгож гацаах боломжтой (lost entry).
 * Watchdog нь log-ийг хэвээр үлдээгээд гарна; check_log END байхгүй гэж FAIL өгнө. */
static void on_alarm(int sig) {
    (void)sig;
    static const char msg[] = "WATCHDOG: --no-mutex горимд race-ээс болж гацсан (log хэвээр үлдсэн).\n";
    ssize_t r = write(2, msg, sizeof msg - 1); (void)r;
    _exit(3);
}

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
        M_LOCK();
        if (qcount == 0 && !ta_done) { ev("TA_SLEEP", 0, 0); slept = 1; n_sleeps++; }
        M_UNLOCK();

        sem_wait(students_sem);                 /* унтах / сэрэх цэг */

        M_LOCK();
        if (qcount == 0 && ta_done) { M_UNLOCK(); break; }
        if (slept) ev("TA_WAKE", 0, qcount);
        int s = queue[((qhead % MAXC) + MAXC) % MAXC]; qhead = (qhead + 1) % MAXC; qcount--;
        ev("HELP_START", s, qcount);
        M_UNLOCK();

        sem_post(call_sem[s]);
        sleep_s(help_s);
        M_LOCK(); ev("HELP_END", s, qcount); n_helped++; M_UNLOCK();
        sem_post(fin_sem[s]);
    }
    return NULL;
}

static void *student_thread(void *arg) {
    int id = (int)(intptr_t)arg;
    unsigned sd = seed * 1000003u + (unsigned)id * 7919u;
    for (int r = 0; r < rounds; r++) {
        sleep_s(prog_s * (0.5 + rand_r(&sd) / (double)RAND_MAX));
        M_LOCK();
        ev("ARRIVE", id, qcount);
        if (qcount < chairs) {
            race_window();
            queue[(((qhead + qcount) % MAXC) + MAXC) % MAXC] = id; qcount++;
            ev("SIT", id, qcount);
            M_UNLOCK();
            sem_post(students_sem);             /* TA-г сэрээх / дарааллыг мэдэгдэх */
            sem_wait(call_sem[id]);             /* дуудагдахыг хүлээх */
            sem_wait(fin_sem[id]);              /* тусламж дуустал хүлээх */
        } else {
            ev("FULL", id, qcount);
            n_turned++;
            M_UNLOCK();
        }
    }
    return NULL;
}

static void usage(const char *p) {
    fprintf(stderr,
        "Хэрэглээ: %s [-c сандал 1-10] [-n оюутан 1-20] [-p оюутны ирэх давтамж: дундаж интервал с 0.01-60]\n"
        "          [-d тусламжийн хугацаа с 0.01-60] [-r мөчлөгийн тоо 1-100] [-s seed]\n"
        "          [-o log.csv] [-q] [--no-mutex]\n"
        "  --no-mutex  mutex-гүй ажиллуулна (race condition-ийг зориуд үзүүлэх; зөв биш үр дүн гарна)\n", p);
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
    static const struct option lopts[] = {
        {"no-mutex", no_argument, NULL, 'M'}, {"help", no_argument, NULL, 'h'}, {0, 0, 0, 0}
    };
    while ((c = getopt_long(argc, argv, "c:n:p:d:r:s:o:q", lopts, NULL)) != -1) {
        switch (c) {
        case 'h': usage(argv[0]); return 0;
        case 'M': use_mutex = 0; break;
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
            else        fprintf(stderr, "Алдаа: үл мэдэгдэх сонголт '%s'.\n", argv[optind - 1]);
            usage(argv[0]); return 2;
        }
    }
    if (optind < argc) { fprintf(stderr, "Алдаа: үл мэдэгдэх аргумент '%s'.\n", argv[optind]); usage(argv[0]); return 2; }
    if (outpath && !(logf = fopen(outpath, "w"))) { perror("log файл нээж чадсангүй"); return 1; }
    if (logf) fprintf(logf, "# chairs=%d students=%d prog=%g help=%g rounds=%d seed=%u mutex=%d\ntime_ms,event,student,queue_len\n",
                      chairs, nstud, prog_s, help_s, rounds, seed, use_mutex);

    if (sem_create(&students_sem, 0) != 0) { perror("семафор үүсгэж чадсангүй"); return 1; }
    for (int i = 0; i <= MAXN; i++)
        if (sem_create(&call_sem[i], 1 + i) != 0 || sem_create(&fin_sem[i], 1 + (MAXN + 1) + i) != 0) {
            perror("семафор үүсгэж чадсангүй"); return 1;
        }
    if (!use_mutex) {
        fprintf(stderr, "АНХААР: --no-mutex горим. waiting/queue хамгаалагдаагүй тул race condition гарна.\n");
        signal(SIGALRM, on_alarm);
        alarm((unsigned)(rounds * (1.5 * prog_s + nstud * help_s) + 5));
    }
    clock_gettime(CLOCK_MONOTONIC, &t0);

    pthread_t ta, st[MAXN + 1];
    pthread_create(&ta, NULL, ta_thread, NULL);
    for (int i = 1; i <= nstud; i++) pthread_create(&st[i], NULL, student_thread, (void *)(intptr_t)i);
    for (int i = 1; i <= nstud; i++) pthread_join(st[i], NULL);

    M_LOCK(); ta_done = 1; M_UNLOCK();
    sem_post(students_sem);                     /* унтаж буй TA-г сэрээж гаргана */
    pthread_join(ta, NULL);
    ev("END", 0, 0);
    alarm(0);
    if (logf) fclose(logf);
    sem_release(students_sem);
    for (int i = 0; i <= MAXN; i++) { sem_release(call_sem[i]); sem_release(fin_sem[i]); }
    if (!quiet) printf("Дүн: нийт ирэлт=%d, туслагдсан=%d, буцсан=%d, TA унтсан=%d\n", nstud * rounds, n_helped, n_turned, n_sleeps);
    return 0;
}
