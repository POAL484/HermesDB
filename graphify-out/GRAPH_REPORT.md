# Graph Report - HermesDB  (2026-11-22)

## Corpus Check
- 20 files · ~7,404 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 2 file(s) not represented in the graph (top: (none) 2)

## Summary
- 321 nodes · 533 edges · 30 communities (10 shown, 20 thin omitted)
- Extraction: 97% EXTRACTED · 3% INFERRED · 0% AMBIGUOUS · INFERRED: 16 edges (avg confidence: 0.85)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `f77e2926`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- Type
- Repository Persistence
- Eviction Implementations
- Controller Error Handling
- Operation
- Value Semantics and Tests
- CacheService
- Documentation Concepts
- Build Configuration
- CI Workflow
- OperationType

## God Nodes (most connected - your core abstractions)
1. `Operation` - 27 edges
2. `Value` - 25 edges
3. `DROGON_TEST()` - 23 edges
4. `Cache` - 21 edges
5. `CacheService` - 20 edges
6. `OperationType` - 20 edges
7. `LfuRepository` - 16 edges
8. `TlruRepository` - 16 edges
9. `LruRepository` - 13 edges
10. `CacheController` - 12 edges

## Surprising Connections (you probably didn't know these)
- `CacheController::applyOperation()` --references--> `Error`  [INFERRED]
  controllers/CacheController.cc → models/operation.h
- `CacheController::applyOperation()` --references--> `SemanticError`  [INFERRED]
  controllers/CacheController.cc → query/semanticmodule.h
- `CacheController::applyOperation()` --references--> `SyntaxError`  [INFERRED]
  controllers/CacheController.cc → query/syntaxmodule.h
- `CacheController` --references--> `CacheService`  [EXTRACTED]
  controllers/CacheController.h → models/cacheservice.h
- `LfuRepository::insert()` --references--> `Cache`  [EXTRACTED]
  repository/inmemoryrepository.cc → models/cache.h

## Import Cycles
- None detected.

## Communities (30 total, 20 thin omitted)

### Community 0 - "Type"
Cohesion: 0.16
Nodes (10): Type, HASH, INT, LIST, OTHER, SET, STRING, oneOf() (+2 more)

### Community 1 - "Repository Persistence"
Cohesion: 0.21
Nodes (13): compress(), decompress(), DiskRepository::load(), DiskRepository::save(), escapeRecord(), fromHex(), gzipCompress(), gzipDecompress() (+5 more)

### Community 2 - "Eviction Implementations"
Cohesion: 0.08
Nodes (29): OperationResult, message, InMemoryRepository, _capacity, LfuRepository, LfuRepository::apply(), _cache, LfuRepository::deleteKey() (+21 more)

### Community 3 - "Controller Error Handling"
Cohesion: 0.08
Nodes (7): CacheController, CacheController::applyOperation(), protected, _service, Error, SemanticError, SyntaxError

### Community 5 - "Operation"
Cohesion: 0.13
Nodes (12): Cache, _value, Operation, expiresAt, key, ttl, type, value (+4 more)

### Community 7 - "CacheService"
Cohesion: 0.09
Nodes (17): CacheService, _compressionMethod, _isPersistent, _persistent, _persistentDir, _policy, _repos, CompressionMethod (+9 more)

### Community 9 - "Documentation Concepts"
Cohesion: 0.14
Nodes (16): Cache Eviction Policy, Record Compression Methods, GZIP Compression, HermesDB, HTTP API, Key-Value Data Types, LFU Cache Policy, LRU Cache Policy (+8 more)

### Community 10 - "Build Configuration"
Cohesion: 0.21
Nodes (13): Application Source Directories, cache_service Executable Target, C++23 Standard, Drogon Build Dependency, Drogon-Generated Views, Drogon Test Registration, hermes_tests Executable Target, LZ4 Build Dependency (+5 more)

### Community 11 - "CI Workflow"
Cohesion: 0.15
Nodes (12): runs-on, steps, env, BUILD_TYPE, jobs, build-and-test, name, on (+4 more)

### Community 15 - "OperationType"
Cohesion: 0.08
Nodes (22): OperationType, DEC, DEL, DELETE, GET, GETITEM, HDEL, HSET (+14 more)

## Knowledge Gaps
- **68 isolated node(s):** `protected`, `_service`, `HASH`, `INT`, `LIST` (+63 more)
  These have ≤1 connection - possible missing edges. (Counts symbols only; 153 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **20 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `Operation` connect `Operation` to `Type`, `Repository Persistence`, `Eviction Implementations`, `test_main.cc`, `CacheService`, `OperationType`?**
  _High betweenness centrality (0.144) - this node is a cross-community bridge._
- **Are the 3 inferred relationships involving `DROGON_TEST()` (e.g. with `.apply()` and `.errorCode()`) actually correct?**
  _`DROGON_TEST()` has 3 INFERRED edges - model-reasoned connections that need verification._
- **What connects `protected`, `_service`, `HASH` to the rest of the system?**
  _68 weakly-connected nodes found - possible missing edges._
- **Should `Eviction Implementations` be split into smaller, more focused modules?**
  _Cohesion score 0.07568027210884354 - nodes in this community are weakly interconnected._
- **Why does `DROGON_TEST()` connect `Value Semantics and Tests` to `Controller Error Handling`, `test_main.cc`, `Operation`?**
  _High betweenness centrality (0.101) - this node is a cross-community bridge._
- **Should `Controller Error Handling` be split into smaller, more focused modules?**
  _Cohesion score 0.08021390374331551 - nodes in this community are weakly interconnected._
- **Why does `CacheService` connect `CacheService` to `Eviction Implementations`, `Controller Error Handling`, `test_main.cc`?**
  _High betweenness centrality (0.099) - this node is a cross-community bridge._