ZMAKEBAS=./zmakebas/zmakebas

all: tzx2tap tzxconv.bas
tzx2tap: tzx2tap.c
	zcc +zxn -v -startup=30 -clib=sdcc_iy -SO3 --opt-code-size tzx2tap.c -o tzx2tap -subtype=dotn -create-app -pragma-define:CLIB_OPT_SCANF=0 -pragma-define:CLIB_OPT_PRINTF=0x5605
tzxconv.bas: tzxconv.txt
	$(ZMAKEBAS) -3 -o ./tzxconv.bas ./tzxconv.txt
