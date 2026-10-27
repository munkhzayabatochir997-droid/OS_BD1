# F.CSM302 БД1 — 10. The Sleeping Teaching Assistant

![CI](https://github.com/munkhzayabatochir997-droid/OS_BD1/actions/workflows/ci.yml/badge.svg)

| | |
|---|---|
| Хуваарилалтын код | 10 |
| Сурах бичгийн бүлэг | Synchronization Examples |
| Үндсэн дүрслэл | Queue animation |
| Хэл / сан | C (Pthreads, POSIX semaphore, mutex) · HTML/JavaScript (UI) |

**Зорилго.** Нэг TA, хязгаарлагдмал хүлээх сандал, олон оюутны thread-ийг mutex + semaphore-оор синхрончилж
(race condition, deadlock, lost wake-up үгүй), хэрэгжилтийг бодит event log-оор animation хэлбэрээр харуулж турших.

## Яаж ажилладаг вэ
```
 sleeping_ta.c  ──(thread-үүдийн бодит event log: run.csv)──▶  sleeping_ta.html  (animation + log шалгалт)
        │                                                       tests/check_log.c (бие даасан шалгагч)
        └─ stdout дээр мөн уншигдахуйц лог хэвлэнэ
```
UI нь C програмын **бодит** гаралтыг (`-o run.csv`) дахин тоглуулдаг тул харагдаж буй бүх төлөв яг thread-үүдийн ажиллагаанаас гарна.
C суулгаагүй үед туршихад зориулж UI-д ижил event форматтай **Demo** загвар бий (өөр, хоёр дахь эх сурвалж).

## Quick Start (Linux / macOS / WSL)
```bash
git clone https://github.com/munkhzayabatochir997-droid/OS_BD1.git && cd OS_BD1
make                                                       # build/sleeping_ta
./build/sleeping_ta -c 3 -n 5 -p 1 -d 0.5 -r 3 -s 1 -o run.csv
# src/sleeping_ta.html-г browser-т нээгээд "C-ийн log" талбараас run.csv-г сонгож ▶ Play дарна
make test                                                  # бүх автомат тест
make stress                                                # 30 янзын тохиргооны stress тест
```
Аргумент: `-c` сандал 1–10 · `-n` оюутан 1–20 · `-p` ирэх давтамж: оюутан дахин ирэх дундаж интервал (с) · `-d` тусламжийн хугацаа (с) · `-r` мөчлөгийн тоо · `-s` seed · `-o` log · `-q` чимээгүй · `--no-mutex` mutex-гүй ажиллуулж race condition-ийг зориуд үзүүлнэ.
Жишээ log-ууд: `examples/`.

**Race condition үзүүлэх (`--no-mutex`).** `waiting`/queue-г хамгаалах mutex-ийг унтрааж ажиллуулбал хоёр оюутан зэрэг "сандал чөлөөтэй" гэж шалгаад хоёулаа суух тул сандлаас хэтэрнэ. `check_log` үүнийг "сандлаас хэтэрсэн" гэж илрүүлнэ:
```bash
./build/sleeping_ta -c 1 -n 8 -p 0.05 -d 0.05 -r 3 -s 1 --no-mutex -q -o race.csv
./build/check_log race.csv        # FAIL: #9 SIT: сандлаас хэтэрсэн
```
Анхаарах: энэ горимд "шалгах → суух" хоорондын цонхыг 1 мс-ээр өргөсгөдөг (үгүйгээр race маш ховор гарна). Race магадлалтай тул заримдаа илрэхгүй, мөн дараалал эвдэрч оюутан гацвал 3 кодтойгоор watchdog гаргана. Туршилтад 20 seed-ийн 16-д нь илэрсэн. `make test` нь 8 удаа хүртэл оролддог.

**macOS:** `sem_init` macOS дээр дэмжигддэггүй тул `#ifdef __APPLE__` дотор нэртэй семафор (`sem_open` + шууд `sem_unlink`) ашигладаг. CI нь Ubuntu болон macOS дээр хоёуланд нь ажиллана. `timeout` команд macOS-д анхнаасаа байдаггүй тул скриптүүд `gtimeout` (`brew install coreutils`) эсвэл timeout-гүй горимд шилждэг.

**Windows:** C програмыг WSL (Ubuntu) дотор ажиллуулна, UI-г Windows-ийн browser-т нээнэ — `docs/Manual.md` §3. C-гүйгээр `examples/*.csv`-г UI-д ачаалж болно.

## Багийн гишүүд
| Гишүүн | GitHub | 
|---|---|---|
| <нэр> | <Б. Мөнхзаяа /B241940022/user-munkhzayabatochir997 -droid>  
| <нэр> | <Н. Сүх-Амар //user-SuhAmar>

## Бүтэц
`src/` эх код · `tests/` тест (`run_tests.sh`, `check_log.c`, `test_ui_model.js`, `test_cases.md`) · `examples/` бодит log · `docs/` Manual, Report

## AI ашигласан тухай
Тест, баримтын эхний хувилбарыг Claude (Anthropic)-ийн тусламжтай бэлтгэсэн. 
