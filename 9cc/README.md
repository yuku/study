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

## Ref

* https://www.sigbus.info/compilerbook
