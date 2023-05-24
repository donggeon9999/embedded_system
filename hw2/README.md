# 과제2 
# TIMER DEVICE DRIVER

## 이름 : 이동건
## 학번 : 20181664
## 학과 : 컴퓨터공학과

## 디바이스 정보
device driver name : /dev/dev_driver
device driver major number : 242

## 실행방법
1. (On host side, in app folder) compile app

: app 폴더에서 make 를 실행한다.
```
  $ make
```
: app 이름의 실행 파일 생성 

: app/app


2. (On host side, in module folder) comile module

: module 폴더에서 make를 실행한다.
```
  $ make
```
: dev_driver.ko 이름의 모듈 파일 생성 

: module/dev_driver.ko


3. (On host side)minicom 실행

: 보드와 통신을 위해 minicom을 실행한다.

: 새로운 터미널 창을 실행하고 

: $ sudo minicom 명령어로 minicom 실행

4. (On host side, in app folder) app 실행 파일 push

: app 폴더에서 app 실행파일을 보드로 보내기 위해 adb push 를 진행한다.

```
$ adb push app /data/local/tmp
```

5. (On host side, in module folder) dev_driver.ko 파일 push

: app 폴더에서 dev_driver.ko 파일을 보드로 보내기 위해 adb push 를 진행한다.

```
$ adb push dev_driver.ko /data/local/tmp
```

6. (On the target board-side,in the Minicom) module insertion

```
  $ echo "7 6 1 7" > /proc/sys/kernel/printk // log level 변경

  $ cd /data/local/tmp //경로 변경

  $ insmod dev_driver.ko // 모듈 삽입

  $ mknod dev_driver c 242 0 //device driver file 생성
```
7.  (On the target board-side,in the Minicom) app 실행

```
  $ ./app TIMER_INTERVAL[1-100] TIMER_CNT[1-100] TIMER_INIT[0001-8000]
```

: TIMER_INTERVAL: HZ 값 1~100 (0.1~10 초)

: TIMER_CNT: 디바이스 출력 변경 횟수 (1~100)

: TIMER_INIT: FND 에 출력되는 초기 문양과 위치 (0001~8000)

: 단 4자리 중 한 자리만 1~8 숫자가 입력되도록 설정한다. 예를 들어,0040이라면 왼쪽에서 세번째 자 리에 "4"를 출력한다.

## 주의사항
1. 학번과 이름은 고정되어 있으므로, 만약 해당 정보 수정시 driver.h 파일에 있는 학번과 이름의 길에 해당하는 매크로

```
  #define st_id_size 8
  #define name_size 13
```

값을 학번길이와 이름 길이에 맞게 수정해 주어야 한다.

2. ./app 파일 실행시 넘겨주는 argument는 위에 명세한 조건에 반드시 맞도록 하여야 한다.
