# Graph Report - HermesDB  (2026-11-19)

## Corpus Check
- 20 files · ~6,289 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 2 file(s) not represented in the graph (top: (none) 2)

## Summary
- 325 nodes · 578 edges · 15 communities (11 shown, 4 thin omitted)
- Extraction: 98% EXTRACTED · 2% INFERRED · 0% AMBIGUOUS · INFERRED: 13 edges (avg confidence: 0.85)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `4423a440`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- Core Data Types
- Repository Persistence
- Eviction Implementations
- Controller Error Handling
- test_main.cc
- Cache Operations
- Value Semantics and Tests
- Cache Service
- Documentation Concepts
- Build Configuration
- CI Workflow
- types.h

## God Nodes (most connected - your core abstractions)
1. `Operation` - 27 edges
2. `CacheService` - 20 edges
3. `OperationType` - 19 edges
4. `Cacheable` - 18 edges
5. `DROGON_TEST()` - 17 edges
6. `OperationResult` - 16 edges
7. `LfuRepository` - 16 edges
8. `TlruRepository` - 16 edges
9. `CacheController` - 16 edges
10. `Hash` - 15 edges

## Surprising Connections (you probably didn't know these)
- `CacheController` --references--> `CacheService`  [EXTRACTED]
  controllers/CacheController.h → models/cacheservice.h
- `CacheService` --references--> `CompressionMethod`  [EXTRACTED]
  models/cacheservice.h → repository/diskrepository.h
- `CacheService` --references--> `DiskRepository`  [EXTRACTED]
  models/cacheservice.h → repository/diskrepository.h
- `CacheService` --references--> `Repository`  [EXTRACTED]
  models/cacheservice.h → repository/repository.h
- `parseOperation()` --references--> `OperationType`  [EXTRACTED]
  query/syntaxmodule.cc → models/operation.h

## Import Cycles
- None detected.

## Communities (15 total, 4 thin omitted)

### Community 0 - "Core Data Types"
Cohesion: 0.07
Nodes (13): Cacheable, type, Hash, List, Set, String, _str, Type (+5 more)

### Community 1 - "Repository Persistence"
Cohesion: 0.12
Nodes (19): compress(), CompressionMethod, Gzip, Lz4, Snappy, decompress(), DiskRepository, _compressionMethod (+11 more)

### Community 2 - "Eviction Implementations"
Cohesion: 0.08
Nodes (14): TlruRepository::purgeExpired(), InMemoryRepository, _capacity, LfuRepository, _cache, _freqs, _minfreq, LruRepository (+6 more)

### Community 3 - "Controller Error Handling"
Cohesion: 0.05
Nodes (23): CacheController, CacheController::applyOperation(), _mapper, protected, _service, OperationType, DEC, DELETE (+15 more)

### Community 5 - "Cache Operations"
Cohesion: 0.15
Nodes (10): Cache, _obj, Operation, expiresAt, key, ttl, type, value (+2 more)

### Community 6 - "Value Semantics and Tests"
Cohesion: 0.11
Nodes (3): Int, _n, DROGON_TEST()

### Community 7 - "Cache Service"
Cohesion: 0.20
Nodes (7): CacheService, _compressionMethod, _isPersistent, _persistent, _persistentDir, _policy, _repos

### Community 9 - "Documentation Concepts"
Cohesion: 0.14
Nodes (16): Cache Eviction Policy, Record Compression Methods, GZIP Compression, HermesDB, HTTP API, Key-Value Data Types, LFU Cache Policy, LRU Cache Policy (+8 more)

### Community 10 - "Build Configuration"
Cohesion: 0.21
Nodes (13): Application Source Directories, cache_service Executable Target, C++23 Standard, Drogon Build Dependency, Drogon-Generated Views, Drogon Test Registration, hermes_tests Executable Target, LZ4 Build Dependency (+5 more)

### Community 11 - "CI Workflow"
Cohesion: 0.15
Nodes (12): runs-on, steps, env, BUILD_TYPE, jobs, build-and-test, name, on (+4 more)

### Community 12 - "types.h"
Cohesion: 0.15
Nodes (6): SemanticModule, jsonValueType(), parseOperation(), SyntaxModule, SyntaxModule::parse(), SyntaxModule::validate()

## Knowledge Gaps
- **66 isolated node(s):** `_repos`, `_persistent`, `_isPersistent`, `_persistentDir`, `_policy` (+61 more)
  These have ≤1 connection - possible missing edges. (Counts symbols only; 142 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **4 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `Operation` connect `Cache Operations` to `Core Data Types`, `Repository Persistence`, `Eviction Implementations`, `Controller Error Handling`, `types.h`?**
  _High betweenness centrality (0.141) - this node is a cross-community bridge._
- **What connects `_repos`, `_persistent`, `_isPersistent` to the rest of the system?**
  _66 weakly-connected nodes found - possible missing edges._
- **Should `Core Data Types` be split into smaller, more focused modules?**
  _Cohesion score 0.06894049346879536 - nodes in this community are weakly interconnected._
- **Why does `Type` connect `Core Data Types` to `Eviction Implementations`, `types.h`, `Cache Operations`, `Cache Service`?**
  _High betweenness centrality (0.120) - this node is a cross-community bridge._
- **Should `Repository Persistence` be split into smaller, more focused modules?**
  _Cohesion score 0.11612903225806452 - nodes in this community are weakly interconnected._
- **Why does `OperationType` connect `Controller Error Handling` to `types.h`, `Cache Operations`?**
  _High betweenness centrality (0.095) - this node is a cross-community bridge._
- **Should `Eviction Implementations` be split into smaller, more focused modules?**
  _Cohesion score 0.08181818181818182 - nodes in this community are weakly interconnected._