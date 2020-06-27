# study/9cc

## Development

Create a docker image:

```bash
docker build -t 9ccdev .
```

Launch a bash shell in a docker container:

```bash
docker run --rm -it -v $PWD:/9cc -w /9cc 9ccdev
```

### Test Function Call

```bash
./9cc 'foo(1);' > tmpmain.s
cat <<EOF > tmpfoo.c
#include <stdio.h>
int foo(int a) { printf("OK %d\n", a); }
EOF
cc -o tmp tmpfoo.c tmpmain.s
./tmp
```

## Ref

* https://www.sigbus.info/compilerbook
