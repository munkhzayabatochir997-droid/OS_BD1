# F.CSM302 БД1 — 10. The Sleeping Teaching Assistant

- **Код:** 10  **Бүлэг:** Synchronization Examples  **Дүрслэл:** Queue animation
- **Зорилго:** Sleeping TA бодлогыг semaphore + mutex-ээр хэрэгжүүлж, сандал/оюутан/хугацааг тохируулан дүрсэлж турших.

## Quick Start
1. **UI:** `src/sleeping_ta.html`-г браузерт нээнэ (суулгах зүйлгүй) → параметр оруулаад **Run**.
2. **C (Pthreads, Linux/macOS/WSL):**
   ```
   gcc -Wall -pthread -o sleeping_ta src/sleeping_ta.c
   ./sleeping_ta 3 6 4 2 3
   ```
   Аргумент: `chairs students prog_max_s help_s [rounds]`

## Бүтэц
`src/` эх код · `tests/` тестийн хүснэгт · `docs/Manual.md` гарын авлага

## Тэмдэглэл
UI дахь симуляци нь C хувилбартай ижил логик (унтах/сэрээх, FIFO сандал, дүүрэн үед буцах)-г JavaScript-ээр дискрет хугацаанд загварчилсан. AI (Claude) ашиглан код, баримт бэлтгэсэн.