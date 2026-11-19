# F.CSM302 БД1, БД2 — 10. The Sleeping Teaching Assistant

[![CI](https://github.com/munkhzayabatochir997-droid/OS_BD1/actions/workflows/ci.yml/badge.svg)](https://github.com/munkhzayabatochir997-droid/OS_BD1/actions/workflows/ci.yml)

| | |
|---|---|
| Хуваарилалтын код | V-1-10 |
| Сурах бичгийн бүлэг | Synchronization Examples |
| Үндсэн дүрслэл | Queue animation |
| Хэл / сан | C (Pthreads, POSIX semaphore, mutex) · HTML/JavaScript (UI) |

**Зорилго.** Нэг TA, хязгаарлагдмал хүлээх сандал, олон оюутны thread-ийг mutex + semaphore-оор синхрончилж (race condition, deadlock, lost wake-up үгүй), хэрэгжилтийг бодит event log-оор animation хэлбэрээр харуулж турших.

**Final release:** [v2.0](https://github.com/munkhzayabatochir997-droid/OS_BD1/releases/tag/v2.0) (БД2). БД1-ийн хувилбар: [v1.0](https://github.com/munkhzayabatochir997-droid/OS_BD1/releases/tag/v1.0).

## БД2: материал ба сайжруулалт

| Материал | Файл |
|---|---|
| Постер | [`docs/V-1-10_Poster.pdf`](docs/V-1-10_Poster.pdf) |
| Final Report | [`docs/V-1-10_Final_Report.pdf`](docs/V-1-10_Final_Report.pdf) |
| Manual | [`docs/V-1-10_Manual.pdf`](docs/V-1-10_Manual.pdf), [`docs/Manual.md`](docs/Manual.md) |

Шүүмжийн дагуу хийсэн гол сайжруулалт (дэлгэрэнгүй: Final Report, 4-р хэсэг):
1. **Шалгагч чангарсан (R1–R3).** C (`check_log`) ба UI (`validate()`) шалгагч ижил дүрэмтэй болж, зориуд эвдсэн 5 log-ийг 5/5 илрүүлдэг. Тест 22-оос 27, UI тест 19-өөс 24 болсон.
2. **Mutex-гүй дүрслэл (R4, R6–R8).** Хэтэрсэн оюутан улаан хүрээтэй сандалд, график тасрахгүй; log ачаалахад талбар header-ээс бөглөгдөж, `mutex=0` анхааруулга гарна.
3. **Найдвартай race команд (R5).** 1–20 seed-ийн 20/20-д race илэрдэг.
4. **Гүйцэтгэлийн туршилт (R11).** `make sweep`: сандлын тоо ба буцалт, дундаж хүлээлтийн хамаарал.

## Дэлгэцийн зураг

| Mutex-тэй: 3/3 сандал дүүрэн, TA 5-р оюутанд тусалж байна | Mutex-гүй: 1 сандалтай атал 3 оюутан суусан |
|---|---|
| ![Mutex-тэй ажиллагаа](docs/images/ui_02_queue_full.png) | ![Mutex-гүй горим](docs/images/ui_06_no_mutex.png) |

## Яаж ажилладаг вэ

```
 sleeping_ta.c  ──(thread-үүдийн бодит event log: run.csv)──▶  sleeping_ta.html  (animation + log шалгалт)
        │                                                       tests/check_log.c (бие даасан шалгагч)
        └─ stdout дээр мөн уншигдахуйц лог хэвлэнэ
```

UI нь C програмын **бодит** гаралтыг (`-o run.csv`) дахин тоглуулдаг тул харагдаж буй бүх төлөв яг thread-үүдийн ажиллагаанаас гарна. C суулгаагүй үед туршихад зориулж UI-д ижил event форматтай **Demo** загвар бий.

## Quick Start (Linux / macOS / WSL)

```
git clone https://github.com/munkhzayabatochir997-droid/OS_BD1.git && cd OS_BD1
make                                                       # build/sleeping_ta
./build/sleeping_ta -c 3 -n 5 -p 1 -d 0.5 -r 3 -s 1 -o run.csv
# src/sleeping_ta.html-г browser-т нээгээд "C-ийн log" талбараас run.csv-г сонгож Play дарна
make test        # C шалгагч + үндсэн, захын, буруу log-ийн тест (27 тест)
make test-ui     # UI-ийн parser, шалгагч, Demo, төлөв сэргээх тест (24 тест)
make stress      # 30 янзын тохиргооны stress тест
make tsan        # ThreadSanitizer (data race илрүүлэгч)
make sweep       # сандал 1–10: буцалт ба дундаж хүлээлтийн туршилт
```

**Туршсан орчин (dependency version):** Debian 13 x86_64, gcc 14.2.0, GNU Make 4.4.1, Node.js 20.19 (зөвхөн `make test-ui`), Chrome. Гадны сан шаардлагагүй (libc, pthread). CI: Ubuntu ба macOS.

Аргумент: `-c` сандал 1–10 · `-n` оюутан 1–20 · `-p` ирэх давтамж: оюутан дахин ирэх дундаж интервал (0.01–60 с) · `-d` тусламжийн хугацаа (0.01–60 с) · `-r` мөчлөгийн тоо (1–100) · `-s` seed · `-o` log · `-q` чимээгүй · `--no-mutex` mutex-гүй ажиллуулж race condition-ийг зориуд үзүүлнэ. Туршилтын өгөгдөл: `examples/` (C-ийн бодит log-ууд, `race_demo.csv` орно), `tests/fixtures/` (зориуд эвдсэн 5 log), `docs/results_bd2/` (туршилтын гаралт).

## Race condition үзүүлэх (`--no-mutex`)

`waiting`/queue-г хамгаалах mutex-ийг унтрааж ажиллуулбал хоёр оюутан зэрэг "сандал чөлөөтэй" гэж шалгаад хоёулаа суух тул сандлаас хэтэрнэ. `check_log` үүнийг "сандлаас хэтэрсэн" гэж илрүүлнэ:

```
./build/sleeping_ta -c 1 -n 10 -p 0.01 -d 0.01 -r 5 -s 1 --no-mutex -q -o race.csv
./build/check_log race.csv        # FAIL: SIT: сандлаас хэтэрсэн
```

Анхаарах: энэ горимд "шалгах" ба "суух" хоорондын цонхыг 1 мс-ээр өргөсгөдөг (үгүйгээр race маш ховор гарна). Race магадлалтай үзэгдэл боловч дээрх команд 1–20 seed-ийн 20/20-д нь race илрүүлсэн. Дараалал эвдэрч оюутан гацвал 3 кодтойгоор watchdog гаргана. `make test` нь 8 удаа хүртэл оролддог. UI-д `race.csv`-г ачаалбал хэтэрсэн оюутнууд улаан сандалд харагдаж, `mutex=0` анхааруулга гарна.

## Шалгагч ба тест

Хоёр бие даасан шалгагч ижил дүрмээр log-ийг шалгана: `tests/check_log.c` (C) ба UI доторх `validate()` (JavaScript). Дүрэм нь event бүрийн `student`, `queue_len` талбар, FIFO дараалал, ирэлт = тусламж + буцалт зэргийг хамарна.

`tests/fixtures/` дотор зөв log-ийн нэг талбарыг зориуд эвдсэн 5 буруу log байдаг бөгөөд хоёр шалгагч бүгдийг FAIL гэж илрүүлэх ёстой:

| Файл | Эвдсэн зүйл |
|---|---|
| `bad_end_queue.csv` | `END` мөрийн `queue_len` = 2 |
| `bad_ta_sleep_student.csv` | `TA_SLEEP` мөрөнд оюутан 3 |
| `bad_ta_wake_student.csv` | `TA_WAKE` мөрөнд оюутан 2 |
| `bad_full_queue.csv` | `FULL` мөрийн `queue_len` = 0 |
| `bad_student_range.csv` | оюутан 7 (N = 5) |

## Сандлын тооны туршилт

`make sweep` нь `tests/sweep.sh`-ийг ажиллуулж 10 оюутан, ирэх интервал 0.5 с, тусламж 0.1 с, 5 мөчлөгтэй үед сандлыг 1–10 болгон 3 seed-ээр дундажлана. Үр дүн: сандал нэмэх тусам буцсан оюутны хувь 44%-иас 0% хүртэл буурч, дундаж хүлээлт 53 мс-ээс 382 мс хүртэл өснө.

## Платформ

**macOS:** `sem_init` macOS дээр дэмжигддэггүй тул `#ifdef __APPLE__` дотор нэртэй семафор (`sem_open` + шууд `sem_unlink`) ашигладаг. CI нь Ubuntu болон macOS дээр хоёуланд нь ажиллана. `timeout` команд macOS-д анхнаасаа байдаггүй тул скриптүүд `gtimeout` (`brew install coreutils`) эсвэл timeout-гүй горимд шилждэг.

**Windows:** C програмыг WSL (Ubuntu) дотор ажиллуулна, UI-г Windows-ийн browser-т нээнэ — `docs/Manual.md` §3. C-гүйгээр `examples/*.csv`-г UI-д ачаалж болно.

## Багийн гишүүд

| Гишүүн | ID | GitHub |
|---|---|---|
| Б. Мөнхзаяа | B241940022 | [munkhzayabatochir997-droid](https://github.com/munkhzayabatochir997-droid) |
| Н. Сүх-Амар | B231940125 | [SuhAmar](https://github.com/SuhAmar) |

## Бүтэц

```
src/        sleeping_ta.c, sleeping_ta.html
tests/      run_tests.sh, check_log.c, test_ui_model.js, test_cases.md,
            sweep.sh, fixtures/ (5 буруу log)
examples/   бодит log-ууд (race_demo.csv орно)
docs/       V-1-10_Poster.pdf, V-1-10_Final_Report.pdf, V-1-10_Manual.pdf, Manual.md,
            images/ (UI-ийн зураг), results_bd2/ (туршилтын гаралт), 10_BD1_Report.pdf (БД1)
.github/    workflows/ci.yml
```

## AI ашигласан тухай

Тест, баримтын эхний хувилбарыг Claude (Anthropic)-ийн тусламжтай бэлтгэсэн. БД2-ийн тайлан, Manual-ын LaTeX, постерийн бэлтгэл, UI-ийн дэлгэцийн зураг авахад мөн Claude ашигласан.
