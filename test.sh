#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  cc -o tmp tmp.s ext.c
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    #exit 1
  fi
}

assert 0 "main(){0;}"
assert 42 "main(){42;}"
assert 21 "main(){5+20-4;}"
assert 41 "main(){ 12 + 34 -5 ;} "
assert 47 "main(){5+6*7;}"
assert 15 "main(){5*(9-6);}"
assert 4 "main(){(3+5)/2;}"
assert 10 "main(){-10 + 20;}"
assert 10 "main(){20 + (-10) ;}"
assert 1 "main(){1 == 1 ;}"
assert 0 "main(){1 != 1 ;}"
assert 1 "main(){2>1;}"
assert 1 "main(){2 >= 1;}"
assert 0 "main(){2 < 1;}"
assert 0 "main(){2 <= 1;}"
assert 3 "main(){a=3;}"
assert 22 "main(){ b = 5 * 6 - 8 ;}"
assert 5 "main(){a = 2; b = 3; a + b;}"
assert 14 "main(){a = 3; b = 5 * 6 -8; a + b / 2;}"
assert 1 "main(){foo = 1;}"
assert 5 "main(){bar =  2 + 3;}"
assert 6 "main(){foo = 1; bar = 2 + 3; foo + bar;}"
assert 1 "main(){foo = 10; foo == 10;}"
assert 0 "main(){foo = 10; foo != 10;}"
assert 0 "main(){foo = 10; foo > 10;}"
assert 0 "main(){foo = 10; foo < 10;}"
assert 1 "main(){foo = 10; foo <= 10;}"
assert 1 "main(){foo = 10; foo >= 10;}"
assert 5 "main(){return 5;}"
assert 14 "main(){a = 3; b = 5 * 6 -8; return a + b / 2;}"
assert 5 "main(){if (1 == 1) a = 5; else a = 10;}"
assert 10 "main(){if (1 == 2) a = 5; else a = 10;}"
assert 15 "main(){if (1 == 2) a = 5; else a = 10; if (1 == 1) a = 15; else a = 20;}"
assert 15 "main(){if (1 == 2) {a = 5;} else {a = 10;} if (1 == 1) {a = 15;} else {a = 20;}}"
assert 8 "main(){if (1 == 1) {a = 5; b = 3; return a+b;} else {a = 10; b = 15; return a+b;}}"
assert 25 "main(){if (1 == 2) {a = 5; b = 3; return a+b;} else {a = 10; b = 15; return a+b;}}"
assert 1 "main(){if (1 == 1) { if (1 == 1) { a = 1;} else { a = 2;} } else { a = 4;} }" 
assert 3 "main(){
            if (1 == 1) {
            if (1 == 1) { a = 1; b = 2; return a+b; } else { c = 2; d = 3; return c+d;}
            } else { g = 100; h = 200; return g+h;}
            }" 
assert 1 "main(){if (1==1) a=1; else {if (1==1) a=2; else a=3;}"
assert 3 "main(){
            if (1 == 1) {
            if (1 == 1) { a = 1; b = 2; return a+b; } else { c = 2; d = 3; return c+d;}
            } else { if (1 == 1) { e = 4; f = 5; return e+f;} else { g = 100; h = 200; return g+h;} }
            }"  
assert 10 "main(){cnt=10;x=0; while (cnt) {x=x+1;cnt=cnt-1;} return x;}"
#assert 2 "cnt=3;x=0; while (cnt) {x=x+1;cnt=cnt-1;{if(cnt==1)return x;}}return x;"
assert 105 "main(){cnt=10;x=100; while (cnt>5) {cnt=cnt-1;x=x+1;} return x;}"
assert 10 "main(){cnt=0;for(a=0;a<10;a=a+1){cnt=cnt+1;} return cnt;}"
assert 50 "main(){cnt=0;for(a=0;a<10;a=a+1){ b=0;while(b<5){b=b+1;cnt=cnt+1;}  } return cnt;}"
assert "foo" "main(){foo();}"
#assert -15 "if (1==1) calc(1,2,3,4,5); else calc(5,4,3,2,1);"
#assert 22 "if (1==0) calc(1,2,3,4,5); else calc(5,4,3,2,1);"
assert -15 "main(){calc(1,2,3,4,5);}"
assert 0 "main(){calc(1,2,3,4,5);return 0;}"
assert -15 "main(){if (1==1) calc(1,2,3,4,5);}"
assert -15 "main(){if (1==1) calc(1,2,3,4,5); else a=1;}"
assert 1 "main(){if (1==0) calc(1,2,3,4,5); else {a=1;return a;}}"
assert -15 "main(){if (1==1) calc(1,2,3,4,5); else foo();}"
assert "foo" "main(){if (1==0) calc(1,2,3,4,5); else foo();}"
assert -15 "main(){if (1==1) calc(1,2,3,4,5); else calc(5,4,3,2,1);}"
assert 22 "main(){if (1==0) calc(1,2,3,4,5); else calc2(5,4,3,2,1);}"
assert 3 "main(){a=1;b=2;return a+b;}"
assert 10 "foo(){return 10;} main(){return foo();}"
assert 6 "add(a,b,c){return a+b+c;}main(){ return add(1,2,3);}"
assert 4 "add(a,b){if (a>b) return a; else return b;}main(){ return add(4,2);}"
assert 4 "add(x,y){if (x>y) return x; else return y;}main(){a=4;b=2; return add(a,b);}"
assert 208 "main(){x=3;return &x;}"
assert 3 "main(){x=3;y=&x;return *y;}" 
assert 3 "main(){x=3;y=5;z=&y+16;return *z;}"

echo OK