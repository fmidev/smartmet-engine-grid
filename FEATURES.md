# smartmet-engine-grid — Feature List

A structured inventory of capabilities provided by the grid engine.
Use as a checklist when drafting release notes. When new functionality
is added, append the new entry under the matching section (and bump the
*Last updated* line at the bottom).

`smartmet-engine-grid` is a SmartMet Server engine (`grid.so`) that gives
plugins a single high-level API for querying grid-based meteorological
data. It embeds three service layers from `smartmet-library-grid-content`
(Content / Data / Query Server), wires them to a chosen backend, and adds
producer/parameter mapping, Lua function loading, query execution, and a
web-based admin browser.

---

## 1. Embedded servers

Three service layers, all configured from `cfg/grid-engine.conf` and
exposed to plugins via `Engine::getContentServer_sptr()`,
`getDataServer_sptr()`, `getQueryServer_sptr()`.

- **Content Server** — metadata registry (producers, generations,
  geometries, files, content records).
- **Data Server** — grid value access from files.
- **Query Server** — high-level query execution combining metadata
  lookups with value fetches.

## 2. Content-source backends

Each content source declared in `cfg/grid-engine.conf` can pick a
different backend. Multiple sources can coexist and are merged behind a
single ContentServer interface.

- **Redis** — primary/secondary instances with per-source table prefix
  (`mType == "redis"`).
- **CORBA** — remote ContentServer over CORBA (`mType == "corba"`).
- **HTTP** — remote ContentServer over JSON-over-HTTP (`mType == "http"`).
- **File** — scan a filesystem tree for grid files
  (`mType == "file"`).
- **Multi-source merging** — when more than one source is declared, the
  engine builds a `MergeImplementation` that combines them.

## 3. In-process content cache

- **Cache wrapper** (`contentCacheEnabled`) — fronts the master content
  source with a `CacheImplementation`, giving plugins lock-free reads.
- **Swap mode** (`contentSwapEnabled`) — periodically build a fresh search
  structure and swap atomically.
- **Incremental mode** — apply pending deltas to the existing structure.
- **Cached-file readiness gating** — defer swap until locally cached
  files are downloaded; configurable first-time and steady-state max
  wait.
- **Hash-based change detection** — only rebuild when producer/generation/
  geometry/file/content hashes actually change.

## 4. Data Server modes

- **Local** — DataServer impl runs in-process and memory-maps grid files
  through `smartmet-library-grid-files`.
- **CORBA client** — proxy a remote DataServer process.
- **Grid value cache** — uncompressed-grid LRU shared by both modes;
  configurable size, memory- or filesystem-backed.

## 5. Query Server modes

- **Local** — in-process Query Server with Lua functions, parameter
  mappings, aliases, unit conversions, query cache.
- **CORBA client** — proxy a remote Query Server.
- **Query cache** — result memoisation across requests.

## 6. Engine plugin API

The `SmartMet::Engine::Grid::Engine` class is the surface that plugins
talk to. Notable methods (~100+ total):

### Server access
- `getContentServer_sptr()` / `getContentSourceServer_sptr(idx)`
- `getDataServer_sptr()`
- `getQueryServer_sptr()`
- `getContentSources()` — enumerate configured content sources.

### Query execution
- `executeQuery(Query&)` — run a fully-formed `QueryServer::Query`.
- `executeQuery(shared_ptr<Query>)` — async-friendly variant returning a
  shared result.

### Metadata
- `getProducerList`, `getProducerNameList`, `getProducerInfoByName`,
  `getProducerInfoById`, `getProducerHash`.
- `getGenerationInfoById`, `getAnalysisTimes`, `getExtAnalysisTimes`.
- `getEngineMetadata(producerName)` — list `MetaData` for a producer.
- `getProducerInfo`, `getGenerationInfo`, `getExtGenerationInfo`,
  `getParameterInfo` — table-shaped reports (used by admin pages and
  external admin plugins).
- `getProducerParameterLevelList`, `getProducerParameterLevelIdList`,
  `getProducerLevelIdList`, `getVerticalGrid`.
- `isGridProducer(name)`, `isEnabled()`.

### Parameter & alias resolution
- `getFmiParameterLevelId` — resolve a level-id from a parameter string.
- `getProducerName(alias)` — resolve an alias to a real producer.
- `getProducerAlias(...)` — reverse lookup.
- `getParameterString(...)`, `getParameterAlias(...)`.
- `getParameterDetails(...)` — multiple overloads returning
  `ParameterDetails_vec` with producer/geometry/level/forecast info.
- `mapParameterDetails(...)` — apply mappings/aliases to a detail vector.
- `getParameterMappings(...)` — multiple overloads to query the loaded
  mapping tables.

### Operational
- `getCacheStats()` — `Fmi::Cache::CacheStatistics` snapshot for all
  caches.
- `getStateAttributes(parent)` — attribute tree for admin views.
- `updateProcessing()` — kick periodic maintenance.
- `setDem(...)` / `setLandCover(...)` — inject DEM / land-cover data for
  Lua functions and interpolation.

## 7. Parameter mapping & aliasing

- **Mapping files** under `cfg/parameter/`:
  - `mapping_fmi.csv`, `mapping_newbase.csv`, `mapping_netCdf.csv`
  - Auto-generated `*_auto.csv` companions for inferred mappings.
- **Mapping types** — FMI ↔ Newbase, FMI ↔ NetCDF name, FMI ↔ GRIB.
- **Aliases** under `cfg/alias/` (e.g. `alias_demo.cfg`,
  `alias_newbase_extension.cfg`).
- **Producer aliasing** — producer-name resolution from `producers.csv`
  with the producer file location returned by `getProducerFileName()`.
- **Newbase compatibility** — `cfg/newbase/` mapping configs.
- **Auto-generated mapping** — engine can extend `_auto.csv` files at
  runtime (`writeMappingLine`).
- **Unit conversions** — `cfg/unitConversions.csv`,
  `cfg/height_conversions.csv`.

## 8. Lua function library

Hot-reloadable Lua scripts under `cfg/lua/`, loaded via the grid-content
`LuaFileCollection`:

- `function_basic.lua` — arithmetic / utility helpers.
- `function_conversion.lua` — unit conversions.
- `function_interpolation.lua` — custom interpolation routines.
- `function_grid.lua` — grid-aware operations.
- `function_ensemble.lua` — ensemble statistics.
- `function_aviation.lua` — aviation-specific derivations.
- `function_newbase.lua` — newbase-compatibility helpers.
- `function_demo.lua` — examples.

## 9. Web-based admin browser

Exposed via the `grid-admin` plugin. Pages handled by
`Browser::requestHandler`:

- **Engine overview**
  - `page_start` — engine landing page.
  - `page_configuration` — current configuration summary.
  - `page_configurationFile` — raw config file view.
  - `page_stateInformation` — runtime state, including attribute tree.
- **Content inspection**
  - `page_contentInformation` — content registry summary.
  - `page_producers` — producer list.
  - `page_generations` — generation list per producer.
  - `page_files` — file list per generation.
  - `page_contentList` — content-record listing with sort/filter.
- **Per-server views**
  - `page_contentServer`, `page_dataServer`, `page_queryServer` — per-
    server status pages.
- **Per-server logs**
  - Three processing logs and three debug logs
    (`page_contentServer_processingLog`,
    `page_contentServer_debugLog`, etc.).
  - Logs can be enabled/disabled/cleared from the page (gated by the
    `logModificationEnabled` flag and the user's admin group).
  - Processing logs show newest-first; debug logs show chronological.
- **Configuration file editors / viewers**
  - `page_producerFile`, `page_parameterMappingFile(s)`,
    `page_parameterAliasFile(s)`, `page_producerMappingFile(s)`,
    `page_luaFile(s)`.
- **Capability flags** (`Browser::Flags`)
  - `contentModificationEnabled` — allow add/delete via the browser.
  - `logModificationEnabled` — allow toggling/clearing logs.

## 10. Logging

- **Per-server processing logs and debug logs** — six total (3 servers
  × 2 levels).
- **Per-log configuration** (under `smartmet.engine.grid.<server>.{
  processing-log, debug-log }`):
  - `enabled` — turn on/off.
  - `file` — log path.
  - `maxSize` — rotation threshold.
  - `truncateSize` — truncate target after rotation.
- **Hot-reload** of log configuration when the config file changes.
- **Engine-level applyLogConfiguration helper** — applies one log
  configuration block to a chosen Log object via a caller-supplied
  binder lambda.

## 11. Cluster & remote modes

- **Remote Content Server** via CORBA or HTTP (one source per
  configured endpoint).
- **Remote Data Server** via CORBA.
- **Remote Query Server** via CORBA.
- **Mix and match** — local Content + remote Data, or any combination,
  driven entirely by configuration.

## 12. Memory mapping

- **Memory-mapper integration** (`initMemoryMapper`) — configures the
  `grid-files` `MemoryMapper` with userfaultfd-based lazy paging,
  including HTTP/S3 credentials and queue tuning.

## 13. DEM & land-cover support

- **DEM injection** — plugins call `setDem()` so Lua functions and the
  engine's `feelsLike`/topography pipelines can read elevation.
- **Land-cover injection** — `setLandCover()` for land-type-aware Lua
  functions.

## 14. Configuration

- **libconfig** format with SmartMet extensions
  (`@include`, `@ifdef`, `$(VAR)`, `%(DIR)`).
- **Environment-driven endpoints** — `$REDIS_CONTENT_SERVER_PRIMARY_ADDRESS`
  etc.
- **Hot-reloadable**: parameter mapping files, parameter alias files,
  producer mapping files, Lua scripts, log configuration.
- **Static-init**: main config (`grid-engine.conf`), content-source
  declarations, browser flags. Restart required.
- **Config validation** via `make configtest` (`cfgvalidate`).

## 15. Build & integration

- **Output**: `grid.so` (engine shared library).
- **Loads at**: `$(prefix)/share/smartmet/engines/grid.so`.
- **Build**: `make` (release) / `make debug`.
- **Skip CORBA**: edit Makefile `CORBA = disabled`.
- **Install**: `make install`.
- **RPM**: `make rpm`.
- **Doxygen HTML**: `make doc`.
- **CI**: CircleCI on RHEL 8 and RHEL 10 via `ci-build deps` / `ci-build
  rpm` in the `fmidev/smartmet-cibase-{8,10}` Docker images. No test
  suite in this repo; integration is validated by downstream plugins.

---

*Last updated: 2026-06-01.*
