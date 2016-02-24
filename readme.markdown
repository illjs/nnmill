# libmill + nanomsg experiment
supported operating systems:
- [x] linux
- [x] osx
- [ ] iOS

makefile pulls down and builds libmill/nanomsg into a new opt prefix dir:

```sh
.
├── AUTHORS
├── example.c
├── Makefile
├── nnmill.c
├── nnmill.h
├── opt
│   ├── bin
│   ├── include
│   └── lib
├── readme.markdown
└── test
    ├── inproc.c
    ├── ipc.c
    └── tcp.c
```

clone and run
```sh
git clone https://github.com/reqshark/nnmill
cd nnmill
make

# compile/exec example.c
make run
```

## contributors

<table><tbody>
<tr><th align="left">Bent Cardan</th><td><a href="https://github.com/reqshark/">GitHub/reqshark</a></td><td><a href="http://twitter.com/rekshark">Twitter/@rekshark</a></td></tr>
<tr><th align="left">Fatih Kaya</th><td><a href="https://github.com/fatihky">GitHub/fatihky</a></td><td><a href="http://twitter.com/webci274">Twitter/@webci274</a></td></tr>
<tr><th align="left">Martin Sustrik</th><td><a href="https://github.com/sustrik/">GitHub/sustrik</a></td><td><a href="http://twitter.com/sustrik">Twitter/@sustrik</a></td></tr>
</tbody></table>

# MIT
