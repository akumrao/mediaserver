Version details of libuv

for i in *.c; do mv -- "$i" "${i%.c}.cpp"; done