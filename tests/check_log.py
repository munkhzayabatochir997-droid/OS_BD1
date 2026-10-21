#!/usr/bin/env python3
"""C програмын event log-ийг бие даан шалгагч (invariant checker).
Хэрэглээ: check_log.py run.csv [--expect-full | --expect-no-full]"""
import sys

def load(path):
    meta, ev = {}, []
    for line in open(path, encoding="utf-8"):
        line = line.strip()
        if not line or line.startswith("time_ms"):
            continue
        if line.startswith("#"):
            for kv in line[1:].split():
                k, v = kv.split("="); meta[k] = float(v)
            continue
        t, e, s, q = line.split(","); ev.append((float(t), e, int(s), int(q)))
    return meta, ev

def check(meta, ev):
    err = []; chairs = int(meta["chairs"]); n = int(meta["students"]); rounds = int(meta["rounds"])
    q, ta, cur, last, hstart = [], "idle", None, -1.0, 0.0
    st = {}; arr = {}; helped = full = sleeps = maxq = 0
    def bad(i, msg): err.append(f"#{i+1} {ev[i][1]}: {msg}")
    for i, (t, e, s, ql) in enumerate(ev):
        if t < last: bad(i, "цаг буурсан")
        last = t; sv = st.get(s, "prog")
        if e == "ARRIVE":
            if sv != "prog": bad(i, f"студент {s} төлөв {sv} үед ирсэн")
            if ql != len(q): bad(i, f"queue_len={ql}, бодит={len(q)}")
            st[s] = "arrived"; arr[s] = arr.get(s, 0) + 1
        elif e == "SIT":
            if sv != "arrived": bad(i, "ARRIVE-гүй суусан")
            if len(q) >= chairs: bad(i, "сандлаас хэтэрсэн")
            q.append(s); maxq = max(maxq, len(q))
            if ql != len(q): bad(i, f"queue_len={ql}, бодит={len(q)}")
            st[s] = "waiting"
        elif e == "FULL":
            if sv != "arrived": bad(i, "ARRIVE-гүй буцсан")
            if len(q) != chairs: bad(i, f"сандал дүүрэн биш ({len(q)}/{chairs}) атал буцсан")
            full += 1; st[s] = "prog"
        elif e == "TA_SLEEP":
            if ta != "idle" or q: bad(i, "дараалал хоосон биш/завгүй үед унтсан")
            ta = "sleep"; sleeps += 1
        elif e == "TA_WAKE":
            if ta != "sleep" or not q: bad(i, "унтаагүй эсвэл дараалал хоосон үед сэрсэн")
            ta = "idle"
        elif e == "HELP_START":
            if ta != "idle": bad(i, f"TA төлөв {ta} үед тусалж эхэлсэн")
            if not q or q[0] != s: bad(i, f"FIFO зөрчигдсөн (head={q[0] if q else None})")
            else: q.pop(0)
            if ql != len(q): bad(i, f"queue_len={ql}, бодит={len(q)}")
            if sv != "waiting": bad(i, "хүлээгээгүй оюутанд тусалсан")
            st[s] = "helping"; ta = "help"; cur = s; hstart = t
        elif e == "HELP_END":
            if ta != "help" or cur != s: bad(i, "тохирох HELP_START байхгүй")
            if t - hstart < meta["help"] * 1000 * 0.9: bad(i, "тусламж хэт богино")
            if ql != len(q): bad(i, f"queue_len={ql}, бодит={len(q)}")
            st[s] = "prog"; ta = "idle"; cur = None; helped += 1
        elif e == "END":
            if i != len(ev) - 1: bad(i, "END сүүлийн event биш")
            if q or ta == "help": bad(i, "END үед дараалал/TA дуусаагүй")
        else: bad(i, "үл мэдэгдэх event")
        if len(q) > chairs: bad(i, "дараалал сандлаас их")
    if not ev or ev[-1][1] != "END": err.append("END event байхгүй")
    for s in range(1, n + 1):
        if arr.get(s, 0) != rounds: err.append(f"студент {s}: ирэлт {arr.get(s,0)} != {rounds}")
    if helped + full != n * rounds: err.append(f"helped({helped})+full({full}) != {n*rounds}")
    return err, dict(events=len(ev), arrivals=n*rounds, helped=helped, turned_away=full, max_queue=maxq, ta_sleeps=sleeps)

if __name__ == "__main__":
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    meta, ev = load(args[0]); err, s = check(meta, ev)
    if "--expect-full" in sys.argv and s["turned_away"] == 0: err.append("FULL гарах ёстой байсан")
    if "--expect-no-full" in sys.argv and s["turned_away"] > 0: err.append("FULL гарах ёсгүй байсан")
    if err:
        print("FAIL:", *err[:10], sep="\n  "); sys.exit(1)
    print("PASS:", ", ".join(f"{k}={v}" for k, v in s.items()))
