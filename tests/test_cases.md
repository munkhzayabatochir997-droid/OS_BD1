# Тестийн хүснэгт
Автомат: `make test` (C тест + UI model тест). ThreadSanitizer: `make tsan`.
Цаг хугацаатай (sleep) програм тул log яг ижил байхгүй; тиймээс **invariant** шалгана (check_log.c):
queue_len ≤ chairs · FIFO · TA нэг дор нэг оюутанд тусална · TA зөвхөн дараалал хоосон үед унтана ·
FULL зөвхөн сандал дүүрэн үед · оюутан бүр яг `rounds` удаа ирсэн · helped + turned_away = нийт ирэлт · END үед дараалал хоосон.

## A. C програм (`tests/run_tests.sh`)
| # | Төрөл | Оролт | Хүлээгдсэн | Бодит (хийсэн ажиллуулалт) |
|---|---|---|---|---|
| T1 | Үндсэн | -c 3 -n 5 -p 0.3 -d 0.2 -r 3 -s 1 | invariant OK | PASS: helped=13, turned_away=2, max_queue=3 |
| T2 | Захын | -c 1 -n 10 -p 0.1 -d 0.3 -r 2 -s 2 | FULL гарна | PASS: helped=4, turned_away=16 |
| T3 | Захын | -c 10 -n 3 -p 0.3 -d 0.1 -r 3 -s 3 | FULL гарахгүй | PASS: helped=9, turned_away=0 |
| T4 | Захын | -c 1 -n 1 -p 0.1 -d 0.1 -r 5 -s 4 | FULL гарахгүй | PASS: helped=5, ta_sleeps=6 |
| T5 | Ачаалал | -c 10 -n 20 -p 0.1 -d 0.05 -r 3 -s 5 | invariant OK | PASS: max_queue=10, 192 event |
| T6 | Өөр seed | -c 2 -n 6 -p 0.2 -d 0.15 -r 4 -s 99 | invariant OK | PASS |
| E1–E12 | Буруу оролт | chairs 0/11/3.5/"abc", students 0/21, prog<0, help=0, rounds=0, seed=-1, үл мэдэгдэх сонголт, илүү аргумент | exit code 2 + ойлгомжтой мэдээлэл | бүгд зөв |
| E13 | Файл | `-o /nonexistent_dir/x.csv` | exit code 1 | зөв |
| ST1 | Stress / deadlock | `make stress` (30 янзын c, n, seed; timeout 60 с) | бүгд invariant OK, timeout үгүй | 30/30 |
| S1 | Race шалгалт | `make tsan` (ThreadSanitizer) | data race мэдээлэлгүй | PASS, анхааруулгагүй |

## B. UI model (сонголтоор: `make test-ui`, Node.js шаардана)
Demo generator-ын 5 тохиргоо, seed давтагдах чанар, `examples/*.csv` (C-ийн бодит log) parse + validate,
Windows CRLF log, Demo→CSV→parse round-trip, хамгаалалт: FIFO зөрчсөн / сандлаас хэтэрсэн / END дутуу log-ийг илрүүлэх, parser-ийн алдаа (header дутуу, буруу event, хоосон).

## C. UI гараар (screenshot авч Report-д оруулна)
| # | Алхам | Хүлээгдсэн |
|---|---|---|
| U1 | `examples/overload_1chair_8students.csv` ачаалах, ▶ | Лог шалгалт ✔, 1 сандал, олон удаа улаан "буцсан" |
| U2 | Slider-ыг дундуур нь чирэх | Дараалал, TA төлөв, тоолуур event-тэй нийцнэ |
| U3 | ⏭/⏮ алхам | Нэг event бүрт нэг өөрчлөлт, log-ийн дээд мөр шинэчлэгдэнэ |
| U4 | Хурд 5x, Reset | Хурдан тоглоно, Reset 0-ээс эхэлнэ |
| U5 | Demo-д chairs=0 | Улаан алдааны мэдээлэл, симуляци өөрчлөгдөхгүй |
| U6 | Зориуд эвдсэн log (нэг мөрийг устгах) | "Лог шалгалт ✖ ... зөрчил" гарна |
