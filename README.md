# mpb

Mainframe Package Bureau.

Welcome. Pull up a chair. You're among friends here.

The mainframe community has been sharing code longer than most programming communities have existed. The CBT Tape has been circulating since 1975, back when sharing software meant Arnold Casinghino physically mailing magnetic tapes to people who needed them. Before GitHub, before SourceForge, before the phrase "open source" was even coined, mainframe programmers were distributing assembler macros, ISPF dialogs, and COBOL copybooks to each other on reels of tape through the postal service. That is the tradition this project stands on.

MPB is a package manager for mainframe languages. COBOL, HLASM, REXX, PL/I, JCL, and Fortran. You type `mpb install` and it fetches the code from wherever the author keeps it. That's the whole idea. No ceremony, no accounts, no cloud dashboards. Just code arriving where you need it, which is more or less what Arnold was doing fifty years ago except now the tapes are git repositories and the postage is free.

We would like to thank everyone who has contributed packages and tools over the years. Lionel Dyck, whose decades of ISPF and REXX tooling have quietly made life better for mainframe developers everywhere. Don Higgins, who created z390 and proved that a portable mainframe assembler could exist outside of IBM hardware. Abe Kornelis, who has kept z390 alive and thriving as a community project with tireless dedication. And Arnold Casinghino, who started the CBT Tape in 1975 and built the culture of sharing that all of this rests on.

If you have something useful sitting on your system that other people might benefit from, we would genuinely love to hear from you.

## Quick Start

```bash
make
./mpb search
./mpb install cobol-dateutil
```

The copybooks are now in `mpb_packages/cobol-dateutil/src/` and you can COPY them into your program.

## Commands

```
mpb install <pkg>       install a package from the registry
mpb remove <pkg>        uninstall a package
mpb update <pkg>        pull the latest version
mpb search <query>      search packages by name or description
mpb search              list everything in the registry
mpb info <pkg>          show package details, license, and repo
mpb list                show what you have installed locally
mpb init <name> <lang>  create a package.json for a new package
```

## What Happens When You Install

```
$ mpb install cobol-dateutil
mpb: cobol-dateutil v0.1.0 (cobol) by Zane Hambly
mpb: Date arithmetic for COBOL. Business days, day count conventions, ISO weeks.
mpb: license: Apache-2.0 — permissive (safe for commercial use)
mpb: cloning from https://github.com/Zaneham/cobol-dateutil
mpb: installed cobol-dateutil v0.1.0
```

For copyleft packages it will let you know before proceeding, because introducing a GPL dependency into a proprietary codebase is the sort of thing that goes better when it involves a conversation with someone in legal rather than a surprise six months later:

```
$ mpb install zigi
mpb: zigi v4.0.0 (rexx) by Lionel B. Dyck
mpb: The git interface for the rest of us. ISPF dialog for z/OS git operations.
mpb: license: GPL-2.0 — COPYLEFT (derivative works must be open source)
mpb: proceed? [y/N]
```

## Available Packages

```
$ mpb search

  zblas                   1.0.0  [hlasm]  Apache-2.0  z/OS native BLAS with z13+ vector SIMD
  cobol-dateutil          0.1.0  [cobol]  Apache-2.0  Date arithmetic, business days, day count conventions
  cobol-ebcdic            0.1.0  [cobol]  Apache-2.0  EBCDIC/ASCII conversion, CP037 and CP500
  zigi                    4.0.0  [rexx ]  GPL-2.0     The git interface for the rest of us
  CBTView                 1.0.0  [rexx ]  GPL-2.0     Access to the CBTTape from ISPF
  racfadm                 1.0.0  [rexx ]  GPL-2.0     RACF administration and reporting
  pdsegen                 1.0.0  [rexx ]  GPL-2.0     PDSE Version 2 member generation
  xmitip                  1.0.0  [rexx ]  GPL-2.0     z/OS email client
  ftpb                    1.0.0  [rexx ]  GPL-2.0     FTP from ISPF
  whoson                  1.0.0  [rexx ]  GPL-2.0     TSO users across the sysplex
  omvscmds                1.0.0  [rexx ]  GPL-2.0     OMVS shell commands from ISPF
```

Three languages, two contributors, and growing. If you have something to share we would love to have it.

## License Checking

Every package carries an SPDX license identifier. MPB classifies them before installation so there are no surprises:

| Classification | Licenses | What happens |
|---|---|---|
| Permissive | Apache-2.0, MIT, BSD, ISC | Installs with a note |
| Weak copyleft | LGPL, MPL-2.0, EPL-2.0 | Installs with a note |
| Copyleft | GPL-2.0, GPL-3.0, AGPL-3.0 | Warns and asks for confirmation |
| Proprietary | PROPRIETARY, UNLICENSED | Warns and asks for confirmation |
| Unknown | Missing or unrecognised | Warns and asks for confirmation |

This is not legal advice and never will be. It is a friendly tap on the shoulder before you do something that might require a less friendly conversation later.

## How the Registry Works

The registry lives in its own repository at [mpb-registry](https://github.com/Zaneham/mpb-registry). It contains an `index.json` and one small `package.json` per package. Each package.json points to the author's own repository where the actual code lives.

Nobody uploads their code to the registry. The registry is an index, not a warehouse. Your code stays in your repo, under your control, with your license. MPB just tells people where to find it.

A package entry looks like this:

```json
{
  "name": "cobol-dateutil",
  "version": "0.1.0",
  "description": "Date arithmetic for COBOL.",
  "author": "Zane Hambly",
  "license": "Apache-2.0",
  "language": "cobol",
  "repository": "https://github.com/Zaneham/cobol-dateutil"
}
```

## Publishing a Package

Fork [mpb-registry](https://github.com/Zaneham/mpb-registry), add a directory under `packages/` with your `package.json`, update `index.json`, and submit a pull request. Same model as Homebrew and the Notepad++ plugin list. If pull requests aren't your thing, open an issue with a link to your repo and we will sort it out.

## Supported Languages

| Code | Language | Typical Files |
|---|---|---|
| cobol | COBOL | .cpy, .cob, .cbl |
| hlasm | HLASM | .asm, .mac |
| rexx | REXX | .rexx, .exec |
| pli | PL/I | .pli |
| jcl | JCL | .jcl |
| fortran | Fortran | .f, .f77, .f90 |

If your favourite mainframe language isn't here, let us know. The architecture supports adding new ones with a single line of code.

## Building

```bash
make
```

Requires `gcc`, `curl`, and `git`. Nothing else.

## Contributing

Contributions are welcome and genuinely appreciated, whether that's a new package for the registry, a bug fix in the CLI, or just a note saying you tried it and something didn't work. The mainframe community has always been generous with its knowledge and we would like MPB to reflect that.

If you are one of the people who has been quietly maintaining REXX execs and COBOL copybooks and HLASM macros for years without much recognition, thank you. Seriously. This project exists because of the work you've already done.

## License

Apache 2.0.
