# Хавсралт A. Ажлын төлөвлөгөө
| Талбар | Мэдээлэл |
|---|---|
| Хуваарилалтын код | 10 |
| Төсөл | The Sleeping Teaching Assistant |
| Repository | https://github.com/munkhzayabatochir997-droid/OS_BD1 |
| Хэл/сан/орчин | C (Pthreads, POSIX semaphore) · HTML/JS · Linux/WSL |
| Үндсэн дүрслэл | Queue animation (сандал, дараалал, TA төлөв) |

| Гишүүн | Хариуцах ажил | Commit/file |
|---|---|---|
| Б.Мөнхзаяа /B241940022/ | C хэрэгжилт (pthreads, semaphore, mutex, `--no-mutex`, macOS), log-оор animation UI, `check_log` шалгагч, тест, Makefile, CI, README, жишээ log | `src/`, `tests/`, `examples/`, `Makefile`, `.github/`, `README.md` (13 commit) |
| Н.Сүх-Амар /B231940125/ | Manual-ийг өөр машин (Debian 13) дээр дагаж Build/Run/Test хийж нотолгоо цуглуулах; UI screenshot U1–U6; Manual, Report-ийн үр дүн шинэчлэх; Pull Request #1 | `docs/Manual.md`, `docs/Report.md`, `docs/results/`, `docs/images/`, `examples/debian_3chairs_5students_seed1.csv` (4 commit, `suhamar-manual` branch) |

| Тест | Оролт/нөхцөл | Хүлээгдсэн үр дүн |
|---|---|---|
| T1 | -c 3 -n 5 -p 0.3 -d 0.2 -r 3 | invariant OK, дараалал ≤ 3 |
| T2 | -c 1 -n 10 | олон оюутан буцна |
| E1 | -c 0 | алдааны мэдээлэл, exit code 2 |
