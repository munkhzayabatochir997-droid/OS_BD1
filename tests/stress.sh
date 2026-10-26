#!/usr/bin/env bash
# Stress тест: олон янзын тохиргоогоор N удаа ажиллуулж race/deadlock/дүрэм зөрчлийг хайна.
# Хэрэглээ: tests/stress.sh [N=30]     (timeout 60с хэтэрвэл DEADLOCK гэж үзнэ)
cd "$(dirname "$0")/.." || exit 1
make -s || exit 1
N=${1:-30}; fail=0; T=$(mktemp -d)
TMO=$(command -v timeout || command -v gtimeout || true)
tmo()  { if [ -n "$TMO" ]; then "$TMO" "$@"; else shift; "$@"; fi; }
for i in $(seq 1 "$N"); do
  c=$(( i % 10 + 1 )); n=$(( i * 7 % 20 + 1 ))
  out=$(tmo 60 ./build/sleeping_ta -c $c -n $n -p 0.05 -d 0.02 -r 3 -s "$i" -q -o "$T/l.csv" 2>&1)
  rc=$?
  if [ $rc -eq 124 ]; then echo "  ✘ run $i (c=$c n=$n): TIMEOUT / DEADLOCK"; fail=$((fail+1)); continue; fi
  if [ $rc -ne 0 ]; then echo "  ✘ run $i (c=$c n=$n): rc=$rc $out"; fail=$((fail+1)); continue; fi
  res=$(build/check_log "$T/l.csv") || { echo "  ✘ run $i (c=$c n=$n s=$i): $res"; fail=$((fail+1)); }
done
rm -rf "$T"; echo "== Stress: $N ажиллуулалт, $fail алдаатай"; [ $fail -eq 0 ]
