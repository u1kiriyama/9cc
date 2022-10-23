# 『低レイヤを知りたい人のためのCコンパイラ作成入門』on M1mac

https://www.sigbus.info/compilerbook

M1 mac で実行。
_はじめに_ で触れられているようにmacOSは対象外だが、途中までならなんとかなるだろうと思い勉強を始めた。
進めるにつれてwebサイトの通りに書いてもコンパイルできなかったり、適宜コードを追加しないといけない。

## 環境
```gcc --version```
```
Apple clang version 13.1.6 (clang-1316.0.21.2.5)
Target: arm64-apple-darwin21.1.0
Thread model: posix
InstalledDir: /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin
```
```uname -m```
```
arm64
```
```lldb -version```
```
lldb-1316.0.9.46
Apple Swift version 5.6.1 (swiftlang-5.6.0.323.66 clang-1316.0.20.12)
```
よく使うコマンド
+ lldb 9cc
+ b main (アセンブリで実行する際は_mainで止まる)
+ b 関数名
+ r "1+1;"
+ reg read
+ reg read x0
+ reg read -f b cpsr
+ mem read 0x000000016fdff3bc
+ p 変数
+ p tok->len


## 機械語とアセンブラ
x86-64からarm64への読替えは下記を参考にした。
+ 手元でCからアセンブリを生成
+ [COMPILER EXPLORER](https://godbolt.org)(armv8-a clang 9.0.0, オプションなし)
+ [Jun's Homepage](https://www.mztn.org/dragon/arm6400idx.html#toc)

レジスタはlldb の reg read によれば、x0〜x32, cpsr がある。x29 = fp, x30 = lr, x31 = sp, x32 = pc に割り当てられている。fp, spはよく使う。cpsrは[ステータスレジスタ](https://www.mztn.org/slasm/arm02.html)で割り算で使用。

### レジスタの読み替え
+ rax -> x8
+ rdi -> x9
+ rbp -> fp
+ rsp -> sp
#### その他
+ `ret` : x86-64は`rax`の値を返すが、arm64は`x0`のようなので、`ret`前に `mov x0, x8`が必要。
+ 即値を直接スタックに積む命令がよくわからなかったので`x10`に入れた上で、`str`している。

### Cとそれに対応するアセンブラ
```test1.c
int main() {
    return 42;
}
```

```cc -o test1 test1.c ```

```test1.s
	.section	__TEXT,__text,regular,pure_instructions
	.build_version macos, 12, 0	sdk_version 12, 3
	.globl	_main                           ; -- Begin function main
	.p2align	2
_main:                                  ; @main
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #16
	.cfi_def_cfa_offset 16
	str	wzr, [sp, #12]
	mov	w0, #42
	add	sp, sp, #16
	ret
	.cfi_endproc
                                        ; -- End function
.subsections_via_symbols
```
不要そうな箇所をコメントアウト。当初`.p2align 2`をコメントアウトしており、
`ld: warning: arm64 function not 4-byte aligned: _main from ...`
が出ていたが、実行は問題なかったのでこのコードを生成せず進めていた。
途中で`.p2align 2`を入れるとwarningが出ないことに気づき、それ以降このコードを生成するようにした。
```
test2.s
;	.section	__TEXT,__text,regular,pure_instructions
;	.build_version macos, 12, 0	sdk_version 12, 3
	.globl	_main                           ; -- Begin function main
	.p2align	2
_main:                                  ; @main
;	.cfi_startproc
; %bb.0:
;	sub	sp, sp, #16
;	.cfi_def_cfa_offset 16
;	str	wzr, [sp, #12]
	mov	w0, #42
;	add	sp, sp, #16
	ret
;	.cfi_endproc
                                        ; -- End function
;.subsections_via_symbols
```
```cc -o test2 test2.s
./test2
echo $?
42
```
## 細かいメモなど
### ステップ１
`-static`をつけよ、とあるが下記のようなエラーが出る。
```
ld: library not found for -lcrt0.o
clang: error: linker command failed with exit code 1 (use -v to see invocation)
```
ネット記事によると mac(Apple clang?)の場合 -staticは使えないらしい。
`-static`なしで実行できるので行けるところまで行く。
### スタックマシン
arm64には`push`, `pop`がない(?)ので、それに対応するコードを compiler explorer で生成した。
```入力
int main() {
    int a = 65537;
    int b = 65536;
    int c = a + b;
    return c;
}
```
```出力
main:                                   // @main
        sub     sp, sp, #16             // =16
        mov     w8, #65537
        mov     w9, #65536
        str     wzr, [sp, #12]
        str     w8, [sp, #8]
        str     w9, [sp, #4]
        ldr     w8, [sp, #8]
        ldr     w9, [sp, #4]
        add     w8, w8, w9
        str     w8, [sp]
        ldr     w0, [sp]
        add     sp, sp, #16             // =16
        ret
```
生成されたコードでは必要なスタックを一気に確保して、`[sp, #4]`などでアクセスしているが _「早すぎる最適化は全ての悪の元凶」_ とのことなので、必要になり次第、都度、`sub sp, sp, #4`で確保していくことにした。
その際、`sp`を操作して直接アクセスすると`bus error`で落ちる。`mov x10, sp`などとしてダミーspを作ると動くのでこれで進める。

### ステップ7: 比較演算子
#### ステータスレジスタ

これが参考になる。https://www.mztn.org/slasm/arm02.htm

|bit|フラグ|意味          |備考                      |
|--:|-----:|------------|---------------------------|
|31 |     N|     ネガティブ|        ２の補数表現で負なら１|
|30 |     Z|          ゼロ|                  ゼロなら１|
|29 |     C|キャリー／ボロー|   無符号加減算の桁あふれの時１|
|28 |     V|  オーバーフロー|   符号付加減算の桁あふれの時１|
|27 |     Q|  オーバーフロー|オーバーフロー、飽和(DSP)の時１|

動作確認([0]は以降全てゼロを表す。) Cの挙動がよくわからないがこの結果を参考にして実装。

+ 初期状態 00110[0]
+ 4-3 00100[0]
+ 4-4 01100[0]
+ 3-4 10000[0]

### ステップ9：1文字のローカル変数
_パーサの変更_ の最後の方で `consume_ident()` を導入している。
`consume_ident()` の実装がないので自分で考えないといけないわけだが、
まずは、`TK_IDENT` でないならば、`NULL`を返せばよさそう。
```
node->offset = (tok->str[0] - 'a' + 1) * 8;
```
となっているので、このトークンを返しつつ、`next`に送らないといけない。

### ステップ12:制御構文
今までは式文ごとにnodeをreturnしていたが、`gen`関数の`ND_IF`の中に式文が含まれる形になるため、これまでのように単純に書けない。そこで、専用の`Node *ifstatement_node`を用意した。次のステップでも似たようなことをやろうとしているみたいだったのでちょうどよかった。エレガントさはなくなるが、その他多数メンバーを追加。アセンブリを読んで`sp`やレジスタの値を追いかけながらデバッグした。

### ステップ14:関数呼び出し
```call_foo.c
#include <stdio.h>

int main() {
    extern int foo();
    foo();
}
```
```foo.c
#include <stdio.h>
int foo() {
    printf("OKfoo\n");
    return 0;
}
```
`cc -S call_foo.c`で`call_foo.s`を生成し、中身を確認する。
```call_foo.s
.section	__TEXT,__text,regular,pure_instructions
	.build_version macos, 12, 0	sdk_version 12, 3
	.globl	_main                           ; -- Begin function main
	.p2align	2
_main:                                  ; @main
	.cfi_startproc
; %bb.0:
	stp	x29, x30, [sp, #-16]!           ; 16-byte Folded Spill
	mov	x29, sp
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	bl	_foo
	mov	w0, #0
	ldp	x29, x30, [sp], #16             ; 16-byte Folded Reload
	ret
	.cfi_endproc
                                        ; -- End function
.subsections_via_symbols
```
`x29`->`fp`, `x30`->`lr`と置き換え、いらなさそうな箇所をコメントアウトして動作確認をすると下記まで削れる。
```call_func_my.s
;x29 fp
;x30 lr
	;.section	__TEXT,__text,regular,pure_instructions
	;.build_version macos, 12, 0	sdk_version 12, 3
	.globl	_main                           ; -- Begin function main
	.p2align	2
_main:                                  ; @main
	;.cfi_startproc
; %bb.0:
	stp	fp, lr, [sp, #-16]!           ; 16-byte Folded Spill
	mov	fp, sp
	;.cfi_def_cfa w29, 16
	;.cfi_offset w30, -8
	;.cfi_offset w29, -16
	bl	_foo
	;mov	w0, #0
	ldp	fp, lr, [sp], #16             ; 16-byte Folded Reload
	ret
	;.cfi_endproc
                                        ; -- End function
;.subsections_via_symbols
```
これをもとに`codegen.c`を修正。



