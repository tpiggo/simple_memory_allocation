CC=gcc
ORIGINAL_TEST=a3_test.exe
EXTRA_TEST=test_extra.exe
ET_C=teste.c
REALLOC=test_realloc.exe
RE_C=test_realloc.c


a3_test:
	$(CC) -o $(ORIGINAL_TEST) a3_test.c sma.c

test_e:
	$(CC) -o $(EXTRA_TEST) $(ET_C) sma.c

test_realloc:
	$(CC) -o $(REALLOC) $(RE_C) sma.c


clean:
	rm -f $(ORIGINAL_TEST)

clean_all:
	rm -rf *.exe