#!/usr/bin/env bash
# Автомат тест: Build → олон тохиргоогоор ажиллуулах → log-ийг check_log-оор шалгах → буруу оролтын тест
cd "$(dirname "$0")/.." || exit 1
make -s || { echo "BUILD FAIL"; exit 1; }
BIN=build/sleeping_ta; T=$(mktemp -d); pass=0; fail=0
# macOS дээр `timeout` анхнаасаа байдаггүй (brew install coreutils -> gtimeout). Байхгүй бол timeout-гүй ажиллуулна.
TMO=$(command -v timeout || command -v gtimeout || true)
tmo()  { if [ -n "$TMO" ]; then "$TMO" "$@"; else shift; "$@"; fi; }
ok()   { echo "  ✔ $1"; pass=$((pass+1)); }
no()   { echo "  ✘ $1"; fail=$((fail+1)); }
run()  { name=$1; shift; flag=$1; shift
         tmo 60 $BIN "$@" -q -o "$T/l.csv" && out=$(build/check_log "$T/l.csv" $flag) && ok "$name — $out" || no "$name — ${out:-ажиллаагүй}"; }
err()  { name=$1; shift; $BIN "$@" >/dev/null 2>"$T/e"; rc=$?
         [ $rc -eq 2 ] && [ -s "$T/e" ] && ok "$name (rc=2, мэдээлэл: $(head -1 "$T/e"))" || no "$name (rc=$rc)"; }
echo "== Үндсэн ба захын тест"
run "T1 үндсэн 3 сандал/5 оюутан"       ""                -c 3 -n 5  -p 0.3 -d 0.2  -r 3 -s 1
run "T2 1 сандал, олон оюутан (FULL)"   --expect-full     -c 1 -n 10 -p 0.1 -d 0.3  -r 2 -s 2
run "T3 10 сандал, 3 оюутан (FULL үгүй)" --expect-no-full -c 10 -n 3 -p 0.3 -d 0.1  -r 3 -s 3
run "T4 1 сандал, 1 оюутан"              --expect-no-full -c 1 -n 1  -p 0.1 -d 0.1  -r 5 -s 4
run "T5 хамгийн их 10 сандал/20 оюутан" ""                -c 10 -n 20 -p 0.1 -d 0.05 -r 3 -s 5
run "T6 өөр seed"                        ""                -c 2 -n 6  -p 0.2 -d 0.15 -r 4 -s 99
echo "== Буруу оролт"
err "E1 chairs=0"       -c 0;      err "E2 chairs=11"   -c 11
err "E3 students=0"     -n 0;      err "E4 students=21" -n 21
err "E5 prog<0"         -p -1;     err "E6 help=0"      -d 0
err "E7 rounds=0"       -r 0;      err "E8 chairs=abc"  -c abc
err "E9 үл мэдэгдэх -x" -x;        err "E10 илүү аргумент" foo
err "E11 chairs=3.5 (бүхэл биш)" -c 3.5;  err "E12 seed=-1" -s -1
$BIN -q -o /nonexistent_dir/x.csv >/dev/null 2>&1; [ $? -eq 1 ] && ok "E13 log файл нээгдэхгүй (rc=1)" || no "E13 log файл нээгдэхгүй"
echo "== Race condition (--no-mutex)"
# Mutex-гүй үед race нь магадлалтай тул хүртэл 8 удаа оролдоно. Нэг удаа ч check_log FAIL өгвөл (эсвэл watchdog rc=3) илэрсэн гэж үзнэ.
detected=0; tries=0
for k in 1 2 3 4 5 6 7 8; do
  tries=$k
  tmo 60 $BIN -c 1 -n 8 -p 0.05 -d 0.05 -r 3 -s $k --no-mutex -q -o "$T/nm.csv" >/dev/null 2>&1; rc=$?
  if [ $rc -eq 3 ] || ! build/check_log "$T/nm.csv" >"$T/nm.out" 2>&1; then detected=1; break; fi
done
[ $detected -eq 1 ] && ok "N1 --no-mutex: race илэрсэн ($tries дахь оролдлого; $(sed -n '2p' "$T/nm.out" 2>/dev/null | sed 's/^ *//'))" || no "N1 --no-mutex: 8 оролдлогод race илрээгүй"
tmo 60 $BIN -c 3 -n 5 -p 0.3 -d 0.2 -r 3 -s 1 --no-mutex -q -o "$T/nm2.csv" >/dev/null 2>&1; grep -q 'mutex=0' "$T/nm2.csv" && ok "N2 log-ийн header-т mutex=0 бичигдсэн" || no "N2 header mutex=0"
$BIN --no-mutex --bogus >/dev/null 2>"$T/e"; [ $? -eq 2 ] && [ -s "$T/e" ] && ok "E14 үл мэдэгдэх урт сонголт (rc=2)" || no "E14 үл мэдэгдэх урт сонголт"
rm -rf "$T"; echo "== Дүн: $pass амжилттай, $fail бүтэлгүйтсэн"; [ $fail -eq 0 ]
