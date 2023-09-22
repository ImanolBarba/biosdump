# biosdump
Simple BIOS dumper for x86 class machines

## Compatibility

BIOSDUMP works with original Intel 8088 hardware and DOS 2.0+.

## How to build

This project is built using OpenWatcom 1.9/2.0. The Makefile provided works with OpenWatcom's `wmake`.

To build the regular build:
```
C:\SOURCE\BIOSDUMP\> wmake
```

To build the debug build:
```
C:\SOURCE\BIOSDUMP\> wmake DEBUG=1
```


# Usage
```
BIOSDUMP.EXE [-o OFFSET] [-s SIZE] OUTPUT

	-o Memory offset to BIOS (hex, absolute address)
	-s BIOS size (hex)
```

# Examples
By default, dumps 64KiB from F000:0000 to BIOS.BIN (standard x86 memory map)
```
C:\> BIOSDUMP.EXE
```

Dump 100 bytes from DEAD:000B to FILE.LOL
```
C:\> BIOSDUMP.EXE -o DEADB -s 64 FILE.LOL
``` 
