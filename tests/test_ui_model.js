// UI-ийн логик (Demo generator, log parser, validator, state replay)-ийг Node дээр шалгана.
const fs = require('fs'), path = require('path'), assert = require('assert');
const html = fs.readFileSync(path.join(__dirname, '../src/sleeping_ta.html'), 'utf8');
const code = html.match(/<script>([\s\S]*)<\/script>/)[1];
const mod = { exports: {} }; new Function('module', code)(mod);
const { genDemo, parseLog, validate, stateAt } = mod.exports;
let pass = 0, fail = 0;
const t = (name, f) => { try { f(); console.log('  ✔', name); pass++; } catch (e) { console.log('  ✘', name, '-', e.message); fail++; } };

console.log('== UI model');
for (const [c, n, p, h, r, s] of [[3,6,2,1,3,1],[1,20,0.5,3,5,7],[10,3,5,0.5,4,3],[1,1,1,1,5,0],[10,20,0.1,0.1,20,42]])
  t(`Demo c=${c} n=${n} p=${p} h=${h} r=${r} seed=${s} → invariant OK`, () => {
    const d = genDemo({ chairs: c, students: n, prog: p, help: h, rounds: r, seed: s });
    assert.deepStrictEqual(validate(d.meta, d.ev), []);
    const st = stateAt(d.ev, d.ev.length);
    assert.strictEqual(st.helped + st.full, n * r); assert.strictEqual(st.q.length, 0);
  });
t('Demo: ижил seed → ижил event', () => {
  const a = genDemo({ chairs: 3, students: 5, prog: 1, help: 1, rounds: 3, seed: 5 }), b = genDemo({ chairs: 3, students: 5, prog: 1, help: 1, rounds: 3, seed: 5 });
  assert.deepStrictEqual(a.ev, b.ev); });
t('1 сандал, олон оюутан → FULL гарна', () => {
  const d = genDemo({ chairs: 1, students: 20, prog: 0.5, help: 3, rounds: 5, seed: 7 });
  assert(d.ev.some(x => x.e === 'FULL')); });

const exDir = path.join(__dirname, '../examples');
for (const f of fs.existsSync(exDir) ? fs.readdirSync(exDir).filter(x => x.endsWith('.csv')) : [])
  t(`C-ийн бодит log ${f} → parse + validate`, () => {
    const d = parseLog(fs.readFileSync(path.join(exDir, f), 'utf8'));
    assert.deepStrictEqual(validate(d.meta, d.ev), []); });

console.log('== Буруу log илрүүлэлт');
const base = () => genDemo({ chairs: 3, students: 6, prog: 0.3, help: 2, rounds: 3, seed: 3 });
t('FIFO зөрчил илрэнэ', () => { const d = base();
  const k = d.ev.findIndex(x => x.e === 'HELP_START' && x.q > 0); assert(k >= 0, 'тест өгөгдөлд дараалал үүсээгүй');
  assert.deepStrictEqual(validate(d.meta, d.ev), []);
  d.ev[k].s = d.ev.slice(0, k).filter(x => x.e === 'SIT').map(x => x.s).filter(s => s !== d.ev[k].s).pop();
  assert(validate(d.meta, d.ev).length > 0); });
t('Сандлаас хэтэрсэн log илрэнэ', () => { const d = base(); assert(d.ev.some(x => x.q > 1), 'queue>1 байх ёстой');
  d.meta.chairs = 1; assert(validate(d.meta, d.ev).length > 0); });
t('END устгавал илрэнэ', () => { const d = base(); d.ev.pop(); assert(validate(d.meta, d.ev).length > 0); });
t('Parser: header дутуу → алдаа', () => assert.throws(() => parseLog('time_ms,event,student,queue_len\n1,SIT,1,1\n')));
t('Parser: буруу event → алдаа', () => assert.throws(() => parseLog('# chairs=1 students=1 help=1 rounds=1\n1,FOO,1,1\n')));
t('Parser: хоосон log → алдаа', () => assert.throws(() => parseLog('# chairs=1 students=1 help=1 rounds=1\n')));
console.log(`== Дүн: ${pass} амжилттай, ${fail} бүтэлгүйтсэн`); process.exit(fail ? 1 : 0);
