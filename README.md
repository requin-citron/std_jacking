# std_jacking

std_jacking is a tool to hijack stdin and stderr from other process

## requirements

you need to have gdb to attach process


```apt install gdb```

## compilation

```
git clone https://github.com/requin-citron/std_jacking.git
cd std_jacking
make
```

## usage

```
./std_jacking -p pid -eo
```

If you want to hijack shell process you should add
with line in rc file.
```
eval "$(resize)"
alias ls='ls -C  --color=yes'
```
