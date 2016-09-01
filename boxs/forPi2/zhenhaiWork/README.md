#Build codes in Raspberry Pi 2

## Install utilities

```bash
$ sudo apt-get install git-core
$ sudo apt-get install build-essential
```

## Install dependent libraries

```bash
$ sudo apt-get install libi2c-dev
$ sudo apt-get install libcurl14-openssl-dev

$ git clone git://git.drogon.net/wiringPi
$ cd wiringPi
$ ./build
```


## Build

```bash
$ gcc main.c standerInput.c eeprom.c ftp.c lock.c lcd.c m3050/m3050_2.c -o m3050/m3050 -lm -lpthread -lwiringPi -lcurl -Wall
```

or run Make

```bash
$ make
```

> Before running make, you should:
> 1. Edit the `main.c` and uncomment the machine type which you want to build (in line 19 ~ 57)
> 2. modify the `Makefile` to enable/disable `LOCALTEST` and choose which the machine type what you want to build.
 