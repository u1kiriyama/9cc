#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 "0;"
assert 42 "42;"
assert 21 "5+20-4;"
assert 41 " 12 + 34 -5 ; "
assert 47 "5+6*7;"
assert 15 "5*(9-6);"
assert 4 "(3+5)/2;"
assert 10 "-10 + 20;"
assert 10 "20 + (-10) ;"
assert 1 "1 == 1 ;"
assert 0 "1 != 1 ;"
assert 1 "2>1;"
assert 1 "2 >= 1;"
assert 0 "2 < 1;"
assert 0 "2 <= 1;"
assert 3 "a=3;"
assert 22 " b = 5 * 6 - 8 ;"
assert 5 "a = 2; b = 3; a + b;"
assert 14 "a = 3; b = 5 * 6 -8; a + b / 2;"
assert 1 "foo = 1;"
assert 5 "bar =  2 + 3;"
assert 6 "foo = 1; bar = 2 + 3; foo + bar;"
assert 1 "foo = 10; foo == 10;"
assert 0 "foo = 10; foo != 10;"
assert 0 "foo = 10; foo > 10;"
assert 0 "foo = 10; foo < 10;"
assert 1 "foo = 10; foo <= 10;"
assert 1 "foo = 10; foo >= 10;"
assert 5 "return 5;"
assert 14 "a = 3; b = 5 * 6 -8; return a + b / 2;"
assert 5 "if (1 == 1) a = 5; else a = 10;"
assert 10 "if (1 == 2) a = 5; else a = 10;"

echo OK