# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

The **Grid Engine** (`smartmet-engine-grid`) is a SmartMet Server engine that provides plugins access to grid-based meteorological data (GRIB1/GRIB2/NetCDF/QueryData). It embeds three server components — Content Server, Data Server, and Query Server — which can also run remotely via CORBA or HTTP.

## Build commands

```bash
make                # Build grid.so shared library
make clean          # Clean build artifacts
make format         # Run clang-format (NOTE: .clang-format has DisableFormat: true)
make install        # Install to $(PREFIX)/share/smartmet/engines/ and headers to $(includedir)/smartmet/engines/grid/
make rpm            # Build RPM package
make configtest     # Validate test config with cfgvalidate
```

There is no test suite in this repo (the `test/` Makefile target is a no-op). The `testdata/` directory builds a helper tool `smartmet-grid-test-config-creator`.

CORBA support can be disabled: edit the Makefile and set `CORBA = disabled`.

## Architecture

### Three embedded servers

The engine wraps three service layers from the `grid-content` library, all configured via `cfg/grid-engine.conf`:

- **Content Server** — tracks what grid data exists (producers, generations, files, content records). Primary storage is Redis; content is cached locally in memory with periodic swap for lock-free reads (`contentSwapEnabled`). Supports multiple content sources with different Redis table prefixes.
- **Data Server** — fetches actual grid values from files. Can be local (memory-mapped files) or remote (CORBA). Has its own uncompressed grid cache (memory or filesystem-backed).
- **Query Server** — executes data queries using content metadata + data server. Supports Lua scripting for custom parameter functions, parameter mappings/aliases, unit conversions, and a query cache.

### Source files

All source is in `grid/`:

| File | Role |
|------|------|
| `Engine.h/cpp` (~4700 LOC) | Main engine class — initialization, configuration parsing, server wiring, parameter mapping management, query execution, cache management |
| `Browser.h/cpp` (~4100 LOC) | Web-based admin browser (used by grid-admin plugin) for inspecting content/data/query server state |
| `MetaData.h` | Structs for producer/generation/geometry/parameter metadata exposed to plugins |
| `ParameterDetails.h/cpp` | Parameter detail resolution (producer, geometry, level, forecast type) |
| `MappingDetails.h/cpp` | Wraps `QueryServer::ParameterMapping` with time information |

### Key dependencies

- `grid-files` — low-level grid file I/O, coordinate conversions, memory mapping, value cache
- `grid-content` — Content/Data/Query Server implementations (Redis, CORBA, HTTP, PostgreSQL, cache, memory)
- `spine` — SmartMet Server framework (engine base class, HTTP, reactor, config)
- `macgyver`, `gis` — utilities, DEM/land cover

### Configuration system

The main config file (`cfg/grid-engine.conf`) uses libconfig JSON-like syntax with SmartMet extensions:

- `@include` / `@ifdef` for external config files and environment variables
- `%(DIR)` expands to the config file's directory
- `$(VAR)` expands environment variables (e.g., `$SMARTMET_ENV_FILE`, `$REDIS_CONTENT_SERVER_PRIMARY_ADDRESS`)

Configuration subdirectories under `cfg/`:
- `lua/` — Lua functions for parameter computation (basic, interpolation, conversion, ensemble, newbase)
- `parameter/` — CSV mapping files (FMI, newbase, NetCDF parameter name mappings)
- `newbase/` — producer mapping configs for newbase compatibility
- `alias/` — parameter name/function aliases

Many config files are hot-reloadable at runtime without server restart. The main config file is read only at startup.

## CI

CircleCI builds RPMs on RHEL 8 and RHEL 10 using `ci-build deps` and `ci-build rpm` in `fmidev/smartmet-cibase-{8,10}` Docker images.
