# Grid engine developer guide

This guide is for developers who change `smartmet-engine-grid` or write plugins that use
it. It covers the engine's code: startup and shutdown, how it builds the three grid
services, the API that plugins call, its background work, and the pitfalls.

Related documents:

* [doc/grid-engine.md](../doc/grid-engine.md): the **configuration reference** (every
  setting in `grid-engine.conf`, the mapping files, newbase compatibility). This guide
  does not repeat it.
* [FEATURES.md](../FEATURES.md): the feature inventory used for release notes.
* The grid-content [developer guide](https://github.com/fmidev/smartmet-library-grid-content/blob/master/docs/developer-guide.md)
  (Content, Data and Query Servers) and the grid-files
  [developer guide](https://github.com/fmidev/smartmet-library-grid-files/blob/master/docs/developer-guide.md)
  (file reading, identification, caches). Most of the engine's work is done in those
  libraries.

## Contents

1. [What the engine does](#1-what-the-engine-does)
2. [Building and testing](#2-building-and-testing)
3. [Source files](#3-source-files)
4. [Lifecycle](#4-lifecycle)
5. [How the services are built](#5-how-the-services-are-built)
6. [Plugin API](#6-plugin-api)
7. [Parameter and producer mappings](#7-parameter-and-producer-mappings)
8. [Background update thread](#8-background-update-thread)
9. [Runtime configuration changes](#9-runtime-configuration-changes)
10. [Admin interfaces](#10-admin-interfaces)
11. [Concurrency](#11-concurrency)
12. [Common tasks](#12-common-tasks)
13. [Known pitfalls](#13-known-pitfalls)

---

## 1. What the engine does

The grid engine (`grid.so`) is the SmartMet Server's single entry point to gridded
data. It:

* **initialises grid-files** once per process: the identification tables
  (`Identification::gridDef.init()`), topography, the memory mapper, and the decoded
  value cache;
* **builds the grid-content services** from the configuration: one or more Content
  Server sources, an in-memory cache or merge in front of them, a Data Server and a Query
  Server. Each can be local or a CORBA client;
* **offers a higher-level API** to plugins: `executeQuery()`, producer and parameter
  name resolution (`getProducerName`, `getParameterString`, `getParameterDetails`),
  metadata tables, producer content hashes, and vertical cross-sections;
* **maintains parameter mappings**: it loads the mapping files, generates the `*_auto`
  mapping files for new parameters found in the content, and resolves newbase-style
  producer and parameter names;
* **provides admin views**: the HTML browser used by `grid-admin`, and the reactor admin
  tables `gridgenerations`, `gridgenerationsqd`, `gridproducers` and `gridparameters`.

Its users are the timeseries (through its `GridInterface`), wms, wfs, edr, download,
cross_section, grid-gui and grid-admin plugins. The engine is optional: most plugins
check `isEnabled()` and fall back to querydata when grid support is off.

## 2. Building and testing

```bash
make               # builds grid.so
make install       # grid.so -> $(datadir)/smartmet/engines/, headers -> $(includedir)/smartmet/engines/grid/
make rpm           # smartmet-engine-grid, -devel, and smartmet-engine-grid-test
make configtest    # validates the configuration files with cfgvalidate
```

* **CORBA** is on by default (`CORBA = enabled` in the Makefile). The stub include paths
  point at the **installed** grid-content headers
  (`/usr/include/smartmet/grid-content/*/corba/stubs`), so a CORBA build needs
  `smartmet-library-grid-content-devel` installed, not just a sibling checkout.
* **There are no unit tests in this repository.** The `test` target does nothing. The
  engine is tested through the plugin test suites that run with grid support (timeseries,
  wms, edr, download, cross_section).
* **`testdata/` builds the `smartmet-engine-grid-test` package**, which is the grid
  fixture every plugin test uses:
  * `grid/engine/`: a test `grid-engine.conf`, mapping files, Lua files, aliases and a
    producer file;
  * `grid/library/`: a test `grid-files.conf` with its CSVs (installed as
    `/usr/share/smartmet/test/grid/library/`, which the grid-files tests read too);
  * `grid/redis/`: a Redis dump (`redis-server.rdb`) with the content registry of the
    GRIB files in `smartmet-test-data`, and a config template;
  * `smartmet-grid-test-config-creator`: fills a template with values from a
    configuration file and `-D name value` overrides. Plugin test Makefiles use it to
    write a `redis.conf` with a free port and a private directory, then start their own
    `redis-server`.

  The dump is the content registry the tests see. Grid files added to the test data
  are invisible to the tests until they are registered in it.

## 3. Source files

| File | Contents |
|------|----------|
| `grid/Engine.{h,cpp}` | The engine: configuration parsing, service wiring, plugin API, mapping maintenance, update thread, admin tables. |
| `grid/Browser.{h,cpp}` | The HTML admin browser: configuration and state pages, the mapping, alias and Lua files, the Content, Data and Query Server pages, content lists, and logs. Called through `Engine::browserRequest()` / `browserContent()`. |
| `grid/ParameterDetails.{h,cpp}` | The result of `getParameterDetails()`: the original and resolved producer and parameter, geometry, level, forecast type and number, and the matching mappings. |
| `grid/MappingDetails.{h,cpp}` | One `QueryServer::ParameterMapping` together with the times it is available (`mTimes`, analysis time → forecast times). |
| `grid/MetaData.h` | The result type of `getEngineMetadata()`: producer, generation, geometry, bounding box, levels, parameters and times per data set. |

`cfg/` is a complete example configuration (the one described in
`doc/grid-engine.md`). It is not what production runs; production configuration lives
in the deployment repositories.

## 4. Lifecycle

The server loads `grid.so` and calls `engine_class_creator(configfile)`, which runs the
**constructor**:

1. It reads `grid-engine.conf`. The attributes listed in `configAttribute[]` at the top
   of the constructor must exist. Construction fails with `Missing configuration
   attribute!` naming the first one that is absent. Then it reads the optional attributes, with the defaults set just above them.
2. It calls `Identification::gridDef.init(grid-files config)` and
   `Map::topography.init(...)`. These happen **in the constructor**, so every plugin
   sees an initialised grid-files, even before `init()` has run.

Then the reactor calls **`init()`** (in its own thread):

1. If `smartmet.engine.grid.enabled` is false, it creates disabled
   `ServiceInterface` objects (every call returns `SERVICE_DISABLED`) and returns.
2. `initMemoryMapper()` applies the `memoryMapper.*` settings.
3. `initContentSources()` → `initContentCache()` → `initDataServerImpl()` →
   `initQueryServerImpl()` → `initLogs()` (see [§5](#5-how-the-services-are-built)).
4. **It waits until the content server reports `isReady()`**: the cache has loaded the
   whole registry and, with local file caching, the files that must be cached first.
   With a large registry, this is most of the engine's startup time.
5. It loads the producer, generation and level lists and the mappings, and generates the
   auto mapping files (`updateMappings()`). It then initialises the producer-mapping and
   alias collections and the browser, registers the admin tables, and starts the update
   thread.

**`shutdown()`** stops the memory mapper's fault handler, sets `mShutdownRequested`,
joins the update thread, and shuts down the Query Server, the Data Server, the cache or
merge, and every content source, in that order. The destructor `abort()`s if the engine
is destroyed while enabled but not shut down, because the background threads of the
services would otherwise run on freed memory.

## 5. How the services are built

```
content-source[0] ─┐   (redis | postgresql | corba | http | file)
content-source[1] ─┼─> one source:  CacheImplementation ─┐
content-source[n] ─┘   several:     MergeImplementation ─┤   (or the source itself, if contentCache.enabled = false)
                                                         ▼
                                            mContentServerCache  ◄── getContentServer_sptr()
                                                         │
             DataServer::ServiceImplementation ──────────┤  or DataServer::Corba::ClientImplementation (remote + IOR)
             QueryServer::ServiceImplementation ─────────┘  or QueryServer::Corba::ClientImplementation (remote + IOR)
```

* **Content sources** (`initContentSources()`): each enabled
  `content-server.content-source` entry becomes one `ContentServer::ServiceInterface`:
  * `redis`: `RedisImplementation` (primary and optional secondary, table prefix,
    password, lock);
  * `postgresql`: `PostgresqlImplementation` (primary and secondary connection
    strings);
  * `corba` / `http`: a client for a remote content server;
  * `file`: a `MemoryImplementation` loaded from the CSV files in `memoryContentDir`.
* **Cache** (`initContentCache()`): with exactly one source, a `CacheImplementation`
  (options `contentSwapEnabled`, `contentUpdateInterval`, `requestForwardEnabled`, and
  the file-cache wait times). With several sources, a `MergeImplementation` over all of
  them. Its event thread is started immediately. The switch is
  `content-server.cache.enabled`. With the cache disabled, only the **first** source is
  used (by the plugins and by the Data and Query Servers alike), and a warning is
  printed if more are configured.
* **Data Server** (`initDataServerImpl()`): local unless `data-server.remote` is true.
  A remote server needs a valid IOR; an empty or too short one (50 characters or less)
  is a configuration error. The local server gets the grid directory,
  clean-up settings and the local file cache, and starts its event and cache threads.
  The same function initialises grid-files' `valueCache`.
  `smartmet.library.grid-files.cache.type = "filesys"` makes it file-backed in
  `…cache.directory`; any other value keeps it in memory.
* **Query Server** (`initQueryServerImpl()`): local unless `query-server.remote` is
  true, with the same IOR check. It receives all the configuration file lists (mappings, aliases, producers,
  producer mappings, Lua, unit and height conversions) and its two per-thread content
  caches.

## 6. Plugin API

Plugins get the engine with `reactor.getEngine<SmartMet::Engine::Grid::Engine>("grid")`
and call its methods directly. The main groups:

| Group | Methods | Notes |
|-------|---------|-------|
| Services | `getContentServer_sptr()`, `getDataServer_sptr()`, `getQueryServer_sptr()`, `getContentSourceServer_sptr(i)` | The content server is the cache or merge, not the raw source (with the cache disabled, it is source 0). `getContentSourceServer_sptr(i)` bypasses the cache. |
| Queries | `executeQuery(Query&)` → result code; `executeQuery(Query_sptr)` → the query, **throws** on error | When the engine is disabled or the reactor is shutting down, the first returns `SERVICE_DISABLED` and the second returns the query unexecuted. Both add `contour.threads` if the configuration sets it and the caller has not. |
| Name resolution | `getProducerName(alias)`, `getProducerNameList(mappingName, names)`, `getProducerAlias(name, levelId)`, `getParameterString(producer, parameter)`, `getParameterAlias()`, `getParameterDetails(...)`, `mapParameterDetails()`, `getParameterMappings(...)`, `isGridProducer()` | Used to turn newbase-style or aliased names into the names and parameter strings the Query Server understands ([§7](#7-parameter-and-producer-mappings)). |
| Content info | `getProducerList()`, `getProducerInfoByName/ById()`, `getGenerationInfoById()`, `getProducerParameterLevelList()`, `getProducerParameterLevelIdList()`, `getProducerLevelIdList()`, `getFmiParameterLevelId()`, `getAnalysisTimes()`, `getExtAnalysisTimes()` | Served from the engine's own copies of the lists, refreshed every 60 s (producers and generations) or 300 s (levels). |
| Metadata tables | `getProducerInfo()`, `getGenerationInfo()`, `getExtGenerationInfo()`, `getParameterInfo()`, `getEngineMetadata(producer)` | Spine tables for admin requests and plugin metadata endpoints. |
| Change detection | `getProducerHash(id | name)` | The Content Server's hash of a producer's content, cached for 120 s. WMS uses it in its ETags, so a producer's new data can take up to two minutes to change the ETag. |
| Special | `getVerticalGrid(...)` (cross_section), `setDem()` / `setLandCover()` (injected by the gis engine; forwarded to the Data and Query Servers) | |
| State | `isEnabled()`, `getCacheStats()`, `getStateAttributes()`, `getConfigurationFileName()`, `getProducerFileName()`, `getContentSources()` | |

## 7. Parameter and producer mappings

The engine keeps **its own copy** of the parameter mappings, separate from the Query
Server's. The Query Server uses its copy to execute queries. The engine uses its copy to
answer the name-resolution calls above and to build the `gridparameters` table.

### Auto-generated mapping files

Every 300 s, `updateMappings()`:

1. loads all `query-server.mappingFiles` into a new `ParamMappingFile_vec`, and the
   alias mappings from `mappingAliasFiles`;
2. for each file in `query-server.mappingUpdateFile.{fmi,newbase,netCdf}`, lists every
   producer's parameters from the content server
   (`getProducerParameterListByProducerId`). It writes a mapping line for each
   parameter that no existing mapping covers (`writeMappingLine()`, using the FMI
   parameter definition for the interpolation methods and conversions) into
   `<file>.tmp.<pid>`. If the result differs from the current file, the temp file is
   renamed over it;
3. swaps the new mapping vector, the alias mappings and the parameter table in under
   the write lock.

The auto files must also be listed in `mappingFiles`, or the Query Server never reads
them. The Query Server picks up the renamed file through its own modification-time
check. Several server processes that share one configuration directory all write the
same auto files. The temp-and-rename step keeps readers from seeing a half-written file.

### Newbase compatibility

Plugins such as timeseries accept newbase producer and parameter names (for example
`pal_skandinavia` / `Temperature`). `getParameterDetails(producer, parameter, …)`
resolves them in this order:

1. `producerMappingFiles` (`cfg/newbase/*.cfg`): a producer alias
   (`pal:pal_skandinavia`) is followed recursively.
2. `query-server.aliasFiles`: a parameter alias gives the official name
   (`fog:FogIntensity`).
3. The producer mapping for `producer;parameter` gives the Radon producer, geometry,
   level id, level, forecast type and number
   (`pal_skandinavia;FogIntensity:SMARTMET;1096;;;;;`).
4. `getParameterMappings()` finds the parameter mappings for that producer, parameter,
   geometry and level. `mapParameterDetails()` adds the available times.

`getParameterString(producer, parameter)` returns the result as the Query Server's
`name:producer:geometry:levelId:level:…` parameter string (see the grid-content guide,
§9).

## 8. Background update thread

`startUpdateProcessing()` starts one pthread that runs `updateProcessing()` once a
second until shutdown:

* **Content server restart detection.** If the server time in the last event is newer
  than before, the content server was restarted. The engine clears its producer,
  generation, geometry and level lists and the producer hash cache.
* **`updateMappings()`**: at most every 300 s ([§7](#7-parameter-and-producer-mappings)).
* **`updateProducerAndGenerationList()`**: producers, generations and geometries every
  60 s, levels every 300 s. It also copies the Query Server's producer list into
  `mProducerSearchList`.
* **`checkConfiguration()`**: every 30 s ([§9](#9-runtime-configuration-changes)).

Each step catches and prints its own exceptions, so one failing step does not stop the
others. The grid-content services run their own threads besides this one: cache event
processing, Data Server event, cleanup and file-cache processing, Query Server file
reloads, and the memory mapper's fault handlers.

## 9. Runtime configuration changes

The main configuration file is **not** read only at startup. Every 30 s,
`checkConfiguration()` checks its modification time. If it has changed, the engine
re-reads the file and applies **only** these settings:

* `smartmet.engine.grid.enabled`: the engine can be **disabled** at runtime (all
  services are switched off), but it cannot be enabled if it started disabled;
* the six processing and debug logs (`content-server`, `data-server`, `query-server`
  × `processing-log`, `debug-log`: `enabled`, `file`, `maxSize`, `truncateSize`),
  through `applyLogConfiguration()`;
* `browser.enabled` and `browser.flags`;
* `data-server.grid-storage.clean-up.age` and `.checkInterval`.

Everything else (content sources, caches, remote or local, file lists) needs a restart.
The *files* the lists point to (mappings, aliases, producers, Lua, unit conversions) are
hot-reloaded by the Query Server and by the engine's own collections.

## 10. Admin interfaces

* **Reactor admin tables** (registered in `init()`, public access):
  `what=gridgenerations`, `gridgenerationsqd` (the newbase view), `gridproducers` and
  `gridparameters`. They are built by `getGenerationInfo()`, `getExtGenerationInfo()`,
  `getProducerInfo()` and `getParameterInfo()`.
* **Browser** (`Browser.cpp`): HTML pages rendered into the grid-admin plugin through
  `browserContent()` / `browserRequest()`. Pages that modify things are gated by
  `browser.flags`: `contentModificationEnabled` (1) allows adding and deleting content,
  and `logModificationEnabled` (2) allows switching logs. The session user must be in
  the `grid-admin` group.

## 11. Concurrency

Plugin threads call the engine concurrently with the update thread. The engine's own
shared state is protected like this:

| State | Lock | Refreshed by |
|-------|------|--------------|
| `mProducerInfoList`, `mGenerationInfoList`, `mGeometryInfoList`, `mLevelInfoList`, `mProducerSearchList` | `mProducerInfoList_modificationLock` | `updateProducerAndGenerationList()` (write lock), restart detection |
| `mParameterMappingDefinitions` (a `shared_ptr` that is swapped), `mParameterAliasMappings`, `mParameterTable` | `mParameterMappingDefinitions_modificationLock` | `updateMappings()` |
| `mProducerHashMap` | `mProducerHashMap_modificationLock` | `getProducerHash()` on demand |
| `mProducerMappingDefinitions`, `mParameterAliasDefinitions` | internal to `AliasFileCollection` | `checkUpdates()` at the start of the lookups that use them |

Many of these members are `mutable` so that `const` API methods can refresh them. When
you add a cached list, give it its own lock, and do not call the content server while
holding a write lock that plugin threads need. Content server calls can take a long time
during a reload.

## 12. Common tasks

### 12.1 Adding a configuration setting

1. Add the member with a default in the constructor's initialisation block.
2. Read it with `configurationFile.getAttributeValue("smartmet.engine.grid....", member)`.
   Add it to `configAttribute[]` only if it is mandatory. A mandatory setting breaks
   every existing configuration file that lacks it.
3. If it should change at runtime, also read and apply it in `checkConfiguration()`.
4. Document it in `doc/grid-engine.md`, add it to `cfg/grid-engine.conf` and to
   `testdata/grid/engine/grid-engine.conf`, and add a line to `FEATURES.md`.

### 12.2 Adding a plugin-facing method

Add a **non-virtual** public method. Plugins resolve `SmartMet::Engine::` symbols when
the server loads them, so:

* a new non-virtual method is safe. Plugins built against the new header need the new
  engine, so bump the plugins' `Requires: smartmet-engine-grid >= …`;
* changing or removing an existing method's signature makes the plugins built against
  the old header fail to load (unresolved symbol) until they are rebuilt;
* adding, removing or reordering **virtual** methods, or changing the layout of members
  that inline functions touch (`getContentSources()` returns a member by reference),
  breaks already-built plugins **at runtime**, typically as a crash in an unrelated call.
  Release the engine and all grid-using plugins together if you must do this.

### 12.3 Adding a content source type

Add the backend in grid-content, then a branch in `initContentSources()`, the
configuration keys in the constructor (`ContentSource` struct), and documentation.

## 13. Known pitfalls

* **Startup blocks on the content cache.** `init()` waits for `isReady()`. A slow or
  unreachable Redis, or `fileCacheMaxFirstWaitTime` with many files to cache, delays
  the whole server start.
* **The value cache type is `"filesys"`.** Any other string, including `"filesystem"`
  (as a code comment says), gives the in-memory cache.
* **Producer hashes are cached for 120 s**, so ETags based on them (WMS) lag behind new
  data by up to two minutes.
* **The engine's and the Query Server's mappings can briefly disagree.** They are
  reloaded independently, so a name-resolution call may see a mapping that the
  following query does not yet have, or the other way round.
* **The auto mapping files are shared.** Every process that points at the same
  `mappingUpdateFile` rewrites it. If the directory is not writable, the update throws
  `Cannot open a mapping file for writing!`, which is printed every 300 s.
