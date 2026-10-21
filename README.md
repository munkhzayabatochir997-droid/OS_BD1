# F.CSM302 БД1 — 10. The Sleeping Teaching Assistant

| | |
|---|---|
| Хуваарилалтын код | 10 |
| Сурах бичгийн бүлэг | Synchronization Examples |
| Үндсэн дүрслэл | Queue animation |
| Хэл / сан | C (Pthreads, POSIX semaphore, mutex) · HTML/JavaScript (UI) · Python 3 (тест) |

**Зорилго.** Нэг TA, хязгаарлагдмал хүлээх сандал, олон оюутны thread-ийг mutex + semaphore-оор синхрончилж
(race condition, deadlock, lost wake-up үгүй), хэрэгжилтийг бодит event log-оор animation хэлбэрээр харуулж турших.

## Яаж ажилладаг вэ
```
 sleeping_ta.c  ──(thread-үүдийн бодит event log: run.csv)──▶  sleeping_ta.html  (animation + log шалгалт)
        │                                                       tests/check_log.py (бие даасан шалгагч)
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
```
Аргумент: `-c` сандал 1–10 · `-n` оюутан 1–20 · `-p` програмчлах дундаж (с) · `-d` тусламжийн хугацаа (с) · `-r` мөчлөгийн тоо · `-s` seed · `-o` log · `-q` чимээгүй.
Жишээ log-ууд: `examples/`.

## Бүтэц
`src/` эх код · `tests/` тест (`run_tests.sh`, `check_log.py`, `test_ui_model.js`, `test_cases.md`) · `examples/` бодит log · `docs/` Manual, Report

## Тэмдэглэл (AI / бэлэн код)
Эх код, тест, баримтын эхний хувилбарыг Claude (Anthropic) туслахтайгаар бэлтгэсэн; багийнхан шалгаж, туршиж, өөрчилсөн. Гадны сан ашиглаагүй (UI-ийн тестэд зөвхөн Node.js). Үндсэн тайландаа энэ тухай заавал дурдана.
