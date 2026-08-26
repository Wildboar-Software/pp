# AGENTS.md

Project-level guidance for AI agents working on the **PP X.400 MTA** codebase.

PP is a 1990s X.400 Message Transfer Agent written in K&R C, resurrected to build
on modern Linux. See `README.md` for the full product background, the canonical
build/install steps, and the `examples/` configurations. The definitive operator
documentation is `doc/manual/volume1`.

## Cursor Cloud specific instructions

The environment for this repo is **Docker-based**, because PP depends on ISODE
(`libisode`, `libdsap`) and the pepy/posy/rosy/pepsy ASN.1 compilers, which are
only practical to obtain from the prebuilt base image
`ghcr.io/wildboar-software/quipu:v8.3.11`. Do not try to build PP directly on the
VM host — build/run it inside a container from that base image.

### Starting Docker (needed every session)

Docker CE is installed in the environment but the daemon is **not auto-started**.
Start it once per session (it uses the `fuse-overlayfs` storage driver and
`iptables-legacy`, both already configured in `/etc/docker/daemon.json` and via
`update-alternatives`):

```bash
sudo dockerd            # run in a tmux/background session; leave it running
sudo docker info        # confirm the daemon is up
```

The startup update script does a best-effort `docker pull` of the base image only
when the daemon is already up; on a fresh pod you normally just start `dockerd`
and let `docker build` pull the base image on demand.

### Build (canonical)

The repo's own `pp.dockerfile` is the source of truth for building. It stages
`config/linux.h` → `h/config.h` and `config/linux.make` → `Make.defs`, installs
`libgdbm-compat-dev`/`groff`/X11 dev headers, then runs `./make`, `./make dirs`,
`./make install` (plus `examples/INTERNET` config):

```bash
sudo docker build -f pp.dockerfile -t pp:latest .
```

Building directly in a mounted `/workspace` works too (base image + the apt deps
above, then `./make`), but it pollutes the git tree with `*.o`/`*.a` and generated
`Make.defs`/`h/config.h`. `.dockerignore` already ignores `*.o`/`*.a`; do **not**
commit build artifacts or the generated config files.

### Run

- `pptsapd` is the `pp.dockerfile` CMD — the X.400 (1988) P1 inbound TSAP daemon.
- `qmgr` is the queue manager; `submit` is the enqueue path; channels under
  `/usr/lib/pp/cmds/chans/` (e.g. `local`, `smtp`, `x400in88`) do delivery/relay.
- Run daemons as the `pp` user, e.g. `su pp -c /usr/lib/pp/cmds/qmgr`.
- Logs go to `/usr/spool/pp/logs/{norm,oper}`. `config/linux.h` sets
  `PP_DEBUG_ALL`, so logs and stdout/stderr are extremely noisy; filter with
  `grep -vE '^LOGGING|tai/sys_tai|tailor line|default=normlog|Adding tailor'`.

### KNOWN CAVEAT: `qmgr` (and ISODE ROS responders) crash on modern glibc

`qmgr` segfaults on startup. Root cause (confirmed via gdb): ISODE's
`RyDispatch` returns `NOTOK` for the `readmsginfo` operation (a pepsy/ISODE
ASN.1 operation-table ABI mismatch in the prebuilt libraries), and the legacy
error path `ros_adios` in `Src/qmgr/ryresponder.c` calls `longjmp(toplevel,...)`
on a `jmp_buf` that is only ever `setjmp`-ed inside `ros_work` — so modern
glibc's fortified `__longjmp_chk` aborts with SIGSEGV. This is a legacy
code / toolchain incompatibility, **not** an environment-setup problem, and it
blocks full end-to-end delivery (which needs `qmgr`). Fixing it requires code /
ISODE changes, which are out of scope for environment setup.

### Local-delivery tailoring (needed for message routing)

Routing/auth tables live in `/usr/lib/pp/tables` inside the container and must be
compiled into a dbm database with `dbmbuild` (produces `ppdbm.dir`/`ppdbm.pag`):

```bash
chown -R pp /usr/spool/pp /usr/lib/pp/tables
su pp -c 'cd /usr/lib/pp/tables && /usr/lib/pp/cmds/dbmbuild'
```

Gotchas discovered during setup:
- The `domain` table's `LOC-DOM-SITE`/`LOC-DOM-MTA` placeholders are **not**
  auto-substituted. Per `CHANGES`, add explicit entries for the local domain
  (e.g. `localdomain.us:local`) and rerun `dbmbuild`, or routing fails with
  `Unknown domain`.
- Register `qmgr` in ISODE `isoentities` (`/usr/local/etc/isode/isoentities`) as
  `mylocalhost "pp qmgr" 1.17.6.2.1 INTERNET=mylocalhost+18000`, and make the
  `qmgrhost` from `/usr/lib/pp/tailor` resolve (add it to `/etc/hosts`).

### Hello-world (message ingestion through the MTA)

`sendmail` is PP's BSD-sendmail forgery that hands mail to `submit`:

```bash
printf 'To: pp@localdomain.us\nFrom: pp@localdomain.us\nSubject: Hi\n\nbody\n' \
  | su pp -c '/usr/lib/pp/cmds/sendmail -oi pp@localdomain.us'
ls -R /usr/spool/pp/queues/msg.*/     # message spooled: base/hdr.822 + base/1.ia5
```

This exercises the core engine: RFC822 parse → X.400 O/R address conversion
(RFC 1148bis) → IA5 bodypart → `Received:`/msg-id generation → queue spooling.
`tools/ckadr -r <addr>` is a quick way to exercise the address/routing engine on
its own.

Line endings: the local submission path (`sendmail`/`submit`) expects **bare LF**
(Unix), not CRLF. `_getline()` in `Src/submit/rd_rfchdr.c` only treats `\n` as
end-of-line and leaves any `\r` in the buffer, so a CRLF message's blank
header/body separator is not recognized and submission fails with
`Unable to parse '<body> ' as key:field` (nothing gets spooled). CRLF is a
wire-protocol (SMTP / X.400 P1) concern handled by the channels, not by local
submission.

### Lint / tests

There is no modern lint/test harness. The Makefile `lint` target uses the legacy
`lint` binary, which is not available on modern Linux; treat gcc build warnings
from `./make` as the effective lint. `Tools/ckconfig` sanity-checks the installed
tailor/tables (it will hang if it tries to contact a non-running `qmgr`).
