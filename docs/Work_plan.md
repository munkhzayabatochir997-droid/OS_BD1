# Хавсралт A. Ажлын төлөвлөгөө

| Талбар | Мэдээлэл |
|---|---|
| Хуваарилалтын код | V-1-10 |
| Төсөл | The Sleeping Teaching Assistant |
| Repository | https://github.com/munkhzayabatochir997-droid/OS_BD1 |
| Хэл/сан/орчин | C (Pthreads, POSIX semaphore, mutex), HTML/JavaScript; Linux, macOS, WSL |
| Үндсэн дүрслэл | Queue animation (сандал, дараалал, TA төлөв) |

| Гишүүн | Хариуцах ажил | Commit/file |
|---|---|---|
| Б.Мөнхзаяа /B241940022/ | C хэрэгжилт (pthreads, semaphore, mutex, `--no-mutex`, macOS), log-оор animation UI, `check_log` шалгагч, тест, Makefile, CI, README, жишээ log | `src/`, `tests/`, `examples/`, `Makefile`, `.github/`, `README.md` (13 commit) |
| Н.Сүх-Амар /B231940125/ | Manual-ийг өөр машин (Debian 13) дээр дагаж Build/Run/Test хийж нотолгоо цуглуулах; UI screenshot U1–U6; Manual, Report-ийн үр дүн шинэчлэх; Pull Request #1 | `docs/Manual.md`, `docs/Report.md`, `docs/results/`, `docs/images/`, `examples/debian_3chairs_5students_seed1.csv` (4 commit, `suhamar-manual` branch) |

| Тест | Оролт/нөхцөл | Хүлээгдсэн үр дүн |
|---|---|---|
| T1 | `-c 3 -n 5 -p 0.3 -d 0.2 -r 3 -s 1` | invariant OK, дараалал ≤ 3 |
| T2 | `-c 1 -n 10 -p 0.1 -d 0.3 -r 2 -s 2` | Олон оюутан буцна (FULL гарна) |
| T3 | `-c 10 -n 3 -p 0.3 -d 0.1 -r 3 -s 3` | FULL гарахгүй, TA олон удаа унтана |
| E1 | `-c 0` | Алдааны мэдээлэл, exit code 2 |
| N1 | `-c 1 -n 8 -p 0.05 -d 0.05 -r 3 -s K --no-mutex` | `check_log` FAIL (сандлаас хэтэрсэн) эсвэл watchdog exit 3 |
| ST1 | `make stress` (30 янзын c, n, seed) | Бүгд invariant OK, timeout үгүй |
| S1 | `make tsan` | Data race мэдээлэлгүй, log PASS |