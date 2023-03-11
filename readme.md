# Dynamic Linked-Data Index

Source code repository of the Dynamic Linked-Data Index (DLDI), a storage backend for Linked Data. 

## Purpose

There is a need for a persistable and queryable linked-data format which supports performant updates. HDT satisfies this, except it does not support performant data removals. This software should satisfy this niche. 

## Features

 - Generating a DLDI from plain-text linked-data files.
 - Composing a DLDI by combining multiple existing DLDIs, as removals or additions. 
 - Term prefix queries in a given triple-term position.
 - Term prefix queries for any combination of triple-term positions.
 - Triple pattern queries. 

## Development status

 - Needs code quality improvements / cleanups. 
 - Needs extensive testing, through unit tests or otherwise.  
 - Needs benchmarking and comparison against other linked data indexes/storage formats. 

## Building

```bash
./bin/build
```

## Usage 

```bash
./Release/dldi --help
```

## Testing

```bash
./bin/test
./tests/merge-test-1/run.sh
```

## Emulating the CI locally

TODO set up actual CI. 

```bash
DOCKER_BUILDKIT=1 docker build --target ci .
```
