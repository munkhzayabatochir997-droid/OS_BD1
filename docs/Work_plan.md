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
| 1 <нэр> | <жишээ: C хэрэгжилт> | src/sleeping_ta.c |
| 2 <нэр> | <жишээ: UI ба log parser> | src/sleeping_ta.html |
| 3 <нэр> | <жишээ: тест, stress> | tests/ |
| 4 <нэр> | <жишээ: Manual, screenshot> | docs/ |
| 5 <нэр> | <жишээ: тайлан, Teams пост> | docs/Report.md |

| Тест | Оролт/нөхцөл | Хүлээгдсэн үр дүн |
|---|---|---|
| T1 | -c 3 -n 5 -p 0.3 -d 0.2 -r 3 | invariant OK, дараалал ≤ 3 |
| T2 | -c 1 -n 10 | олон оюутан буцна |
| E1 | -c 0 | алдааны мэдээлэл, exit code 2 |
