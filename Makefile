CC = clang
CFLAGS = -g -std=c89

all : lemon lemon++
clean:
	$(RM) lemon lemon++

lemon++ : lemon.c
	$(CC) $(CFLAGS) -DLEMONPLUSPLUS=1 $< -o $@

lemon : lemon.c
	$(CC) $(CFLAGS) $< -o $@