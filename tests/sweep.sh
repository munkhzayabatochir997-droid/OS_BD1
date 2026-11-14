#!/usr/bin/env bash
# Гүйцэтгэлийн sweep (БД2): сандлын тоо 1..10 үед буцсан оюутны хувь, дундаж хүлээлт, TA унтсан тоо.
# Тохиргоо тогтмол: -n 10 -p 0.5 -d 0.1 -r 5, seed 1..3-ийн дундаж. Гаралт: CSV (stdout).
cd "$(dirname "$0")/.." || exit 1
make -s || exit 1
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
echo "chairs,turned_away_pct,avg_wait_ms,ta_sleeps,check"
for c in 1 2 3 4 5 6 7 8 9 10; do
  for s in 1 2 3; do
    build/sleeping_ta -c "$c" -n 10 -p 0.5 -d 0.1 -r 5 -s "$s" -q -o "$T/l.csv" || exit 1
    build/check_log "$T/l.csv" >/dev/null && ck=PASS || ck=FAIL
    awk -F, -v ck=$ck '!/^#/ && NR>2 { if($2=="ARRIVE")a++; if($2=="FULL")f++; if($2=="TA_SLEEP")z++;
         if($2=="SIT")sit[$3]=$1; if($2=="HELP_START"){w+=$1-sit[$3]; h++} }
         END{ printf "%.4f %.4f %d %s\n", 100*f/a, (h?w/h:0), z, ck }' "$T/l.csv"
  done | awk -v c=$c '{p+=$1; w+=$2; z+=$3; if($4!="PASS")bad=1} END{printf "%d,%.1f,%.1f,%.1f,%s\n", c, p/NR, w/NR, z/NR, bad?"FAIL":"PASS"}'
done
