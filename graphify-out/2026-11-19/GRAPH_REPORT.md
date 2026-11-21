# Graph Report - HermesDB  (2026-11-18)

## Corpus Check
- Corpus is ~5,651 words - fits in a single context window. You may not need a graph.

## Summary
- 299 nodes · 530 edges · 13 communities (12 shown, 1 thin omitted)
- Extraction: 98% EXTRACTED · 2% INFERRED · 0% AMBIGUOUS · INFERRED: 13 edges (avg confidence: 0.85)
- Token cost: 0 input · 0 output

## Community Hubs (Navigation)
- Core Data Types
- Repository Persistence
- Eviction Implementations
- Controller Error Handling
- Cache Operations
- Value Semantics and Tests
- Cache Service
- Operation Commands
- Documentation Concepts
- Build Configuration
- CI Workflow
- Syntax Parsing

## God Nodes (most connected - your core abstractions)
1. `Operation` - 23 edges
2. `CacheService` - 20 edges
3. `OperationType` - 19 edges
4. `Cacheable` - 18 edges
5. `CacheController` - 16 edges
6. `LfuRepository` - 16 edges
7. `Cache` - 15 edges
8. `Hash` - 15 edges
9. `OperationResult` - 14 edges
10. `DROGON_TEST()` - 14 edges

## Surprising Connections (you probably didn't know these)
- `CacheController::applyOperation()` --references--> `SemanticError`  [INFERRED]
  controllers/CacheController.cc → query/semanticmodule.h
- `CacheController::applyOperation()` --references--> `SyntaxError`  [INFERRED]
  controllers/CacheController.cc → query/syntaxmodule.h
- `CacheController` --references--> `CacheService`  [EXTRACTED]
  controllers/CacheController.h → models/cacheservice.h
- `CacheController` --references--> `OperationType`  [EXTRACTED]
  controllers/CacheController.h → models/operation.h
- `CacheService` --references--> `CompressionMethod`  [EXTRACTED]
  models/cacheservice.h → repository/diskrepository.h

## Import Cycles
- None detected.

## Communities (13 total, 1 thin omitted)

### Community 0 - "Core Data Types"
Cohesion: 0.07
Nodes (13): Cacheable, type, Hash, List, Set, String, _str, Type (+5 more)

### Community 1 - "Repository Persistence"
Cohesion: 0.09
Nodes (18): compress(), CompressionMethod, Gzip, Lz4, Snappy, decompress(), DiskRepository, _compressionMethod (+10 more)

### Community 2 - "Eviction Implementations"
Cohesion: 0.11
Nodes (9): InMemoryRepository, _capacity, LfuRepository, _cache, _freqs, _minfreq, LruRepository, _hashtable (+1 more)

### Community 3 - "Controller Error Handling"
Cohesion: 0.10
Nodes (7): CacheController, CacheController::applyOperation(), _mapper, protected, _service, SemanticError, SyntaxError

### Community 5 - "Cache Operations"
Cohesion: 0.15
Nodes (9): Cache, _obj, Operation, key, type, value, OperationResult, message (+1 more)

### Community 6 - "Value Semantics and Tests"
Cohesion: 0.12
Nodes (3): Int, _n, DROGON_TEST()

### Community 7 - "Cache Service"
Cohesion: 0.17
Nodes (7): CacheService, _compressionMethod, _isPersistent, _persistent, _persistentDir, _policy, _repos

### Community 8 - "Operation Commands"
Cohesion: 0.12
Nodes (16): OperationType, DEC, DELETE, GET, GETITEM, HDEL, HSET, INC (+8 more)

### Community 9 - "Documentation Concepts"
Cohesion: 0.14
Nodes (16): Cache Eviction Policy, Record Compression Methods, GZIP Compression, HermesDB, HTTP API, Key-Value Data Types, LFU Cache Policy, LRU Cache Policy (+8 more)

### Community 10 - "Build Configuration"
Cohesion: 0.21
Nodes (13): Application Source Directories, cache_service Executable Target, C++23 Standard, Drogon Build Dependency, Drogon-Generated Views, Drogon Test Registration, hermes_tests Executable Target, LZ4 Build Dependency (+5 more)

### Community 11 - "CI Workflow"
Cohesion: 0.15
Nodes (12): runs-on, steps, env, BUILD_TYPE, jobs, build-and-test, name, on (+4 more)

### Community 12 - "Syntax Parsing"
Cohesion: 0.39
Nodes (3): jsonValueType(), parseOperation(), SyntaxModule

## Knowledge Gaps
- **62 isolated node(s):** `protected`, `_service`, `_mapper`, `_obj`, `_repos` (+57 more)
  These have ≤1 connection - possible missing edges. (Counts symbols only; 129 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **1 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `Operation` connect `Cache Operations` to `Core Data Types`, `Repository Persistence`, `Eviction Implementations`, `Application Entry Points`, `Cache Service`, `Operation Commands`, `Syntax Parsing`?**
  _High betweenness centrality (0.140) - this node is a cross-community bridge._
- **What connects `protected`, `_service`, `_mapper` to the rest of the system?**
  _62 weakly-connected nodes found - possible missing edges._
- **Should `Core Data Types` be split into smaller, more focused modules?**
  _Cohesion score 0.07088989441930618 - nodes in this community are weakly interconnected._
- **Why does `Type` connect `Core Data Types` to `Eviction Implementations`, `Application Entry Points`, `Cache Operations`, `Cache Service`, `Syntax Parsing`?**
  _High betweenness centrality (0.122) - this node is a cross-community bridge._
- **Should `Repository Persistence` be split into smaller, more focused modules?**
  _Cohesion score 0.0945945945945946 - nodes in this community are weakly interconnected._
- **Why does `OperationType` connect `Operation Commands` to `Syntax Parsing`, `Controller Error Handling`, `Application Entry Points`, `Cache Operations`?**
  _High betweenness centrality (0.100) - this node is a cross-community bridge._
- **Should `Eviction Implementations` be split into smaller, more focused modules?**
  _Cohesion score 0.10837438423645321 - nodes in this community are weakly interconnected._