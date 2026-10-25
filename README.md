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
git clone <repo-url> && cd F.CSM302_BD1_SleepingTA
make                                                       # build/sleeping_ta
./build/sleeping_ta -c 3 -n 5 -p 1 -d 0.5 -r 3 -s 1 -o run.csv
# src/sleeping_ta.html-г browser-т нээгээд "C-ийн log" талбараас run.csv-г сонгож ▶ Play дарна
make test                                                  # бүх автомат тест
make stress                                                # 30 янзын тохиргооны stress тест
```
Аргумент: `-c` сандал 1–10 · `-n` оюутан 1–20 · `-p` ирэх давтамж: оюутан дахин ирэх дундаж интервал (с) · `-d` тусламжийн хугацаа (с) · `-r` мөчлөгийн тоо · `-s` seed · `-o` log · `-q` чимээгүй.
Жишээ log-ууд: `examples/`.

**Windows:** C програмыг WSL (Ubuntu) дотор ажиллуулна, UI-г Windows-ийн browser-т нээнэ — `docs/Manual.md` §3. C-гүйгээр `examples/*.csv`-г UI-д ачаалж болно.

## Багийн гишүүд
| Гишүүн | GitHub | Хариуцсан ажил |
|---|---|---|
| <нэр> | <username> | <жишээ: C хэрэгжилт, тест> |
| <нэр> | <username> | <жишээ: UI, Manual> |

## Бүтэц
`src/` эх код · `tests/` тест (`run_tests.sh`, `check_log.c`, `test_ui_model.js`, `test_cases.md`) · `examples/` бодит log · `docs/` Manual, Report

## AI ашигласан тухай
Эх код, тест, баримтын эхний хувилбарыг Claude (Anthropic)-ийн тусламжтай бэлтгэсэн. Багийнхан кодыг уншиж ойлгож, ажиллуулан туршиж, дараахыг өөрсдөө хийсэн: <өөрсдийн хийсэн өөрчлөлт, нэмсэн тест, зассан алдаа>. Гадны сан ашиглаагүй.
