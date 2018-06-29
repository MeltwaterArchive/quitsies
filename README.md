![Quitsies](quitsies_logo.png "Quitsies")

Quitsies
========

**"Quitsies": allows any opponent to stop the game without consequence. Players
can either have "quitsies" (able to quit) or "no quitsies".** _- Wikipedia_

A NoSQL key/value datastore that persists data to the disk using RocksDB.
Quitsies aims to deliver high read and write performance in a service that can
be quit (and restarted) without consequence.

Quitsies is accessible via a REST API and a subset of the memcached API.

To run:

`quitsies [OPTIONS]...`

To set a key with HTTP:

`curl http://<address>:<http_port>/quitsies/key/<key> -d "<data>"`

To get a key with HTTP:

`curl http://<address>:<http_port>/quitsies/key/<key>`

To set a key over TCP:

`echo -e "set <key> 0 0 <data_length>\r\n<data>\r\n" | nc <address> <tcp_port>`

To get a key over TCP:

`echo -e "get <key>\r\n" | nc <address> <tcp_port>`

## TTL

Quitsies cannot set TTLs per data item, but can set a global TTL for all data.
This works by suffixing all data with a timestamp of when they were last set.
When you run quitsies you can use the `--db_ttl` flag to set a TTL, but be
warned that this global value will apply retroactively to existing data.

TTLs are applied at compaction time.

## Memcached API

Quitsies implements a subset of the memcached API in order to be compatible with
existing libraries. However, since quitsies works differently to memcached under
the bonnet there are some unimplemented commands and unused parameters that are
worth understanding if you intend to use it.

Currently supported commands and their caveats:

### Storage commands: set, add

Storage commands expect the same format from clients as the memcached API:

```
<command name> <key> <flags> <exptime> <bytes> [noreply]\r\n
```

However, the `<flags>` and `<exptime>` parameters are ignored for now. The `cas`
check and set operation is also not implemented within quitsies.

### Retrieval commands: get, gets

These commands are supported and should have parity with memcached.

### Deletion command: delete

This command is supported and should have parity with memcached.

## Tuning Performance

Quitsies has numerous input flags for tuning performance. The most prolific
option is `--db_write_mode`, which optimises quitsies for writing at the cost of
more expensive reads, this option is useful for quickly running a backfill.

## Snapshots and Restoration

A running quitsies service can save snapshots into `<db_path>_backup`. You can
trigger a snapshot with the rest API:

`curl http://<address>:<http_port>/quitsies/backup_create -X POST`

You can list snapshots with:

`curl http://<address>:<http_port>/quitsies/backup_info`

And you can purge all but the latest snapshot with:

`curl http://<address>:<http_port>/quitsies/backup_clean -X POST`

### Restoring

To restore the latest snapshot in `<db_path>_backup` to `<db_path>` you can run 
quitsies with the `--db_restore_backup` flag. This will take the latest snapshot
and use it to replace `<db_path>` entirely, so do not restore if there is live
data that isn't backed up.

## Development

Build and test with [bazel](https://bazel.io):

```sh
bazel test ...
bazel build quitsies
```

Or, pull the deps manually:

- [Served 1.4+](https://github.com/meltwater/served)
- [RocksDB 5.4.6+](https://github.com/facebook/rocksdb/releases)
- [Boost 1.58+](http://www.boost.org/)

And use the makefile:

```sh
make build
make test
make install
```
