/* Event log-ийн бие даасан шалгагч (invariant checker) — C хэлээр.
 * Build: gcc -O2 -o build/check_log tests/check_log.c
 * Run:   build/check_log run.csv [--expect-full | --expect-no-full]
 * Exit: 0 = PASS, 1 = FAIL, 2 = файл/хэрэглээний алдаа */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXE 200000
#define MAXS 64
typedef struct { double t; char e[16]; int s, q; } Ev;
static Ev ev[MAXE];
static int nev = 0, nerr = 0;

static void bad(int i, const char *fmt, ...) {
    if (nerr < 10) {
        va_list a; va_start(a, fmt);
        if (nerr == 0) printf("FAIL:\n");
        printf("  #%d %s: ", i + 1, i < nev ? ev[i].e : "-"); vprintf(fmt, a); printf("\n");
        va_end(a);
    }
    nerr++;
}

int main(int argc, char **argv) {
    const char *path = NULL; int expfull = 0, expnofull = 0;
    double chairs = 0, nstud = 0, rounds = 0, helpv = 0;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--expect-full")) expfull = 1;
        else if (!strcmp(argv[i], "--expect-no-full")) expnofull = 1;
        else path = argv[i];
    }
    if (!path) { fprintf(stderr, "Хэрэглээ: %s log.csv [--expect-full|--expect-no-full]\n", argv[0]); return 2; }
    FILE *f = fopen(path, "r");
    if (!f) { perror("log нээж чадсангүй"); return 2; }

    char line[256];
    while (fgets(line, sizeof line, f)) {
        size_t n = strlen(line);
        while (n && (line[n-1] == '\n' || line[n-1] == '\r' || line[n-1] == ' ')) line[--n] = 0;
        if (!n || !strncmp(line, "time_ms", 7)) continue;
        if (line[0] == '#') {
            for (char *tok = strtok(line + 1, " "); tok; tok = strtok(NULL, " ")) {
                char k[32]; double v;
                if (sscanf(tok, "%31[^=]=%lf", k, &v) == 2) {
                    if (!strcmp(k, "chairs")) chairs = v; else if (!strcmp(k, "students")) nstud = v;
                    else if (!strcmp(k, "rounds")) rounds = v; else if (!strcmp(k, "help")) helpv = v;
                }
            }
            continue;
        }
        Ev x;
        if (sscanf(line, "%lf,%15[^,],%d,%d", &x.t, x.e, &x.s, &x.q) != 4) { printf("FAIL: буруу мөр: %s\n", line); return 1; }
        if (nev >= MAXE) { printf("FAIL: log хэт том\n"); return 1; }
        ev[nev++] = x;
    }
    fclose(f);
    if (chairs < 1 || nstud < 1 || rounds < 1 || helpv <= 0) { printf("FAIL: header дутуу (# chairs= students= help= rounds=)\n"); return 1; }

    int q[MAXS], qn = 0, ta = 0 /*0 сэрүүн,1 унтсан,2 тусалж байна*/, cur = -1;
    int st[MAXS] = {0} /*0 прог,1 ирсэн,2 хүлээж,3 тусламж*/, arr[MAXS] = {0};
    int helped = 0, full = 0, sleeps = 0, maxq = 0;
    double last = -1, hstart = 0;
    for (int i = 0; i < nev; i++) {
        Ev *x = &ev[i]; int s = x->s;
        if (s < 0 || s >= MAXS) { bad(i, "студентийн дугаар буруу"); continue; }
        if (x->t < last) bad(i, "цаг буурсан");
        last = x->t;
        int sv = st[s];
        if (!strcmp(x->e, "ARRIVE")) {
            if (sv != 0) bad(i, "студент %d буруу төлөвт ирсэн", s);
            if (x->q != qn) bad(i, "queue_len=%d, бодит=%d", x->q, qn);
            st[s] = 1; arr[s]++;
        } else if (!strcmp(x->e, "SIT")) {
            if (sv != 1) bad(i, "ARRIVE-гүй суусан");
            if (qn >= chairs) bad(i, "сандлаас хэтэрсэн");
            if (qn < MAXS) q[qn++] = s;
            if (qn > maxq) maxq = qn;
            if (x->q != qn) bad(i, "queue_len=%d, бодит=%d", x->q, qn);
            st[s] = 2;
        } else if (!strcmp(x->e, "FULL")) {
            if (sv != 1) bad(i, "ARRIVE-гүй буцсан");
            if (qn != chairs) bad(i, "сандал дүүрэн биш (%d/%d) атал буцсан", qn, (int)chairs);
            full++; st[s] = 0;
        } else if (!strcmp(x->e, "TA_SLEEP")) {
            if (ta != 0 || qn) bad(i, "дараалал хоосон биш/завгүй үед унтсан");
            ta = 1; sleeps++;
        } else if (!strcmp(x->e, "TA_WAKE")) {
            if (ta != 1 || !qn) bad(i, "унтаагүй эсвэл дараалал хоосон үед сэрсэн");
            ta = 0;
        } else if (!strcmp(x->e, "HELP_START")) {
            if (ta != 0) bad(i, "TA төлөв %d үед тусалж эхэлсэн", ta);
            if (!qn || q[0] != s) bad(i, "FIFO зөрчигдсөн (head=%d)", qn ? q[0] : -1);
            else { memmove(q, q + 1, (size_t)(qn - 1) * sizeof(int)); qn--; }
            if (x->q != qn) bad(i, "queue_len=%d, бодит=%d", x->q, qn);
            if (sv != 2) bad(i, "хүлээгээгүй оюутанд тусалсан");
            st[s] = 3; ta = 2; cur = s; hstart = x->t;
        } else if (!strcmp(x->e, "HELP_END")) {
            if (ta != 2 || cur != s) bad(i, "тохирох HELP_START байхгүй");
            if (x->t - hstart < helpv * 1000 * 0.9) bad(i, "тусламж хэт богино");
            if (x->q != qn) bad(i, "queue_len=%d, бодит=%d", x->q, qn);
            st[s] = 0; ta = 0; cur = -1; helped++;
        } else if (!strcmp(x->e, "END")) {
            if (i != nev - 1) bad(i, "END сүүлийн event биш");
            if (qn || ta == 2) bad(i, "END үед дараалал/TA дуусаагүй");
        } else bad(i, "үл мэдэгдэх event");
        if (qn > chairs) bad(i, "дараалал сандлаас их");
    }
    if (!nev || strcmp(ev[nev-1].e, "END")) bad(nev, "END event байхгүй");
    for (int s = 1; s <= (int)nstud && s < MAXS; s++)
        if (arr[s] != (int)rounds) bad(nev, "студент %d: ирэлт %d != %d", s, arr[s], (int)rounds);
    int total = (int)(nstud * rounds);
    if (helped + full != total) bad(nev, "helped(%d)+full(%d) != %d", helped, full, total);
    if (expfull && full == 0) bad(nev, "FULL гарах ёстой байсан");
    if (expnofull && full > 0) bad(nev, "FULL гарах ёсгүй байсан");
    if (nerr) { printf("  (нийт %d зөрчил)\n", nerr); return 1; }
    printf("PASS: events=%d, arrivals=%d, helped=%d, turned_away=%d, max_queue=%d, ta_sleeps=%d\n", nev, total, helped, full, maxq, sleeps);
    return 0;
}
