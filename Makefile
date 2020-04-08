tzx2tap: tzx2tap.c
	zcc +zxn -v -startup=30 -clib=sdcc_iy -SO3 --opt-code-size --max-allocs-per-node200000 tzx2tap.c -o tzx2tap -subtype=dotn -create-app
