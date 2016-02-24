# libmill + nanomsg experiment
use a linux for this.

makefile pulls down and builds libmill/nanomsg into a new opt prefix dir:

```sh
.
├── inproccoroutines.c
├── Makefile
├── opt
│   ├── bin
│   ├── include
│   └── lib
└── readme.markdown
```

clone and run
```sh
git clone https://github.com/reqshark/nnmill
cd nnmill
make

# compile/exec example.c
make run
```
