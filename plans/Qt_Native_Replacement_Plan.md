# Phased Plan: Complete Qt Native Migration

**Status**: Phase 1 Complete (TGL Layer Elimination) ✅
**Date Created**: 2026-01-16
**Project**: openSYDE Codebase Simplification

---

## Executive Summary

This document outlines a phased approach to complete the migration from custom wrapper classes to Qt native implementations. Phase 1 (TGL layer elimination) has been successfully completed. The remaining phases focus on eliminating the SCL (STW Component Library) wrapper classes that add unnecessary abstraction over standard C++ and Qt functionality.

---

## Phase 1: TGL (Target Glue Layer) Elimination ✅ COMPLETE

### Status: Complete
**Files Modified**: 410+
**Files Deleted**: 22
**Impact**: Core infrastructure modernization

### What Was Accomplished

#### 1.1 TGL Platform Abstraction Removed
- **TglFile** → `QFile`, `QFileInfo`, `QDir`
- **TglUtils** → Qt file/path utilities
- **TglTime** → `QDateTime`, `QElapsedTimer`
- **TglTasks (C_TglCriticalSection)** → `QRecursiveMutex`, `QMutexLocker`

#### 1.2 Additional Custom Classes Eliminated
- **C_SclDateTime** → `QDateTime`
- **tinyxml2** → `QDomDocument`/`QDomElement`
- **C_SclIniFile** → Reimplemented using `QSettings` internally

#### 1.3 Platform Support Changes
- Removed Linux-specific implementations (now Windows-only with Qt)
- Deleted `can_dispatcher/target_linux_socket_can/`
- Deleted `ip_dispatcher/target_linux_sock/`

### Key Achievements
✅ Complete TGL layer removal (291 function calls replaced)
✅ Thread synchronization migrated to Qt (`QRecursiveMutex`)
✅ XML parsing completely on Qt DOM
✅ Date/time operations standardized on `QDateTime`
✅ Build system updated for Qt6 Core and XML modules

---

## Phase 2: C_SclString Migration to QString

### Priority: HIGH
**Estimated Impact**: ~4,940 occurrences across codebase
**Complexity**: High (pervasive throughout entire codebase)
**Risk**: Medium (well-understood migration pattern)

### Current State Analysis

#### C_SclString Overview
- **Purpose**: Borland AnsiString compatibility wrapper around `std::string`
- **Current Usage**: 4,940+ occurrences
- **Location**: `opensyde_core/scl/C_SclString.{cpp,hpp}`

#### Key Functionality to Replace
| C_SclString Method | QString Equivalent |
|-------------------|-------------------|
| `.c_str()` | `.toStdString().c_str()` or `.toLatin1().constData()` |
| `.AsStdString()` | `.toStdString()` |
| `.Length()` | `.length()` or `.size()` |
| `.SubString(pos, len)` | `.mid(pos-1, len)` (note: 0-based vs 1-based) |
| `.Pos(search)` | `.indexOf(search) + 1` (note: 0-based vs 1-based) |
| `.UpperCase()` | `.toUpper()` |
| `.LowerCase()` | `.toLower()` |
| `.Trim()` | `.trimmed()` |
| `.StringPrintFormatted()` | `QString::asprintf()` or `QString::arg()` |
| `.IntToStr()` | `QString::number()` |
| `.StringToInt()` | `.toInt()` |
| `.Delete(pos, len)` | `.remove(pos-1, len)` |
| `.Insert(str, pos)` | `.insert(pos-1, str)` |

### Migration Strategy

#### Option A: Gradual Module-by-Module Migration (RECOMMENDED)
1. **Start with leaf modules** (minimal dependencies)
2. **Update module signatures** to accept/return `QString`
3. **Convert internal usage** within module
4. **Add temporary conversion** at module boundaries
5. **Propagate upward** through dependency tree

**Advantages**:
- Lower risk (isolated changes)
- Incremental testing possible
- Can pause/resume work easily
- Easier code review

**Disadvantages**:
- Temporary conversion overhead at boundaries
- Longer overall timeline

#### Option B: Big-Bang Automated Replacement
1. **Create comprehensive regex replacement** script
2. **Run across entire codebase** in single operation
3. **Fix compilation errors** in batch
4. **Extensive testing** required

**Advantages**:
- Faster completion
- No temporary boundary conversions

**Disadvantages**:
- Higher risk
- Difficult to test incrementally
- Large changeset for code review
- Risk of subtle bugs from automated conversion

### Recommended Approach: Option A (Module-by-Module)

#### 2.1 Phase 2A: Data Model Layer
**Target**: Core data structures with minimal external dependencies

Files to migrate (example):
- `project/system/node/C_OscNode.cpp/hpp`
- `project/system/C_OscSystemDefinition.cpp/hpp`
- `data_dealer/C_OscDataDealer.cpp/hpp`

**Strategy**:
- Update public API signatures to `QString`
- Convert internal `C_SclString` member variables to `QString`
- Update all string operations to Qt equivalents

#### 2.2 Phase 2B: Protocol Drivers
**Target**: Communication protocol implementations

Files to migrate (example):
- `protocol_drivers/C_OscProtocolDriverOsy.cpp/hpp`
- `protocol_drivers/C_OscDiagProtocolOsy.cpp/hpp`
- `kefex_diaglib/` subdirectories

#### 2.3 Phase 2C: File I/O and Parsing
**Target**: Import/export and file handling modules

Files to migrate (example):
- `imports/C_OscImport*.cpp/hpp`
- `exports/C_OscExport*.cpp/hpp`
- `xml_parser/C_OscXmlParser.cpp/hpp`

#### 2.4 Phase 2D: Utility and Support Modules
**Target**: Helper classes and utilities

Files to migrate:
- `C_OscUtils.cpp/hpp`
- `C_OscLoggingHandler.cpp/hpp`
- `scl/C_SclChecksums.cpp/hpp`

#### 2.5 Phase 2E: GUI Layer
**Target**: opensyde_tool GUI components (after core library complete)

Files to migrate:
- `opensyde_tool/src/` entire tree
- Update all UI-related string handling

### Critical Considerations

#### Index Differences (CRITICAL)
- **C_SclString**: 1-based indexing (Borland compatibility)
- **QString**: 0-based indexing (standard C++)
- **Action Required**: Carefully audit all `.SubString()`, `.Pos()`, `.Insert()`, `.Delete()` calls

#### Character Encoding
- **C_SclString**: ASCII/Latin-1 (wraps `std::string`)
- **QString**: UTF-16 Unicode
- **Impact**: Generally transparent, but verify special characters

#### Performance
- **QString** is more efficient for UI operations (native Qt type)
- **QString** has better Unicode support
- **QString** integrates seamlessly with Qt APIs

### Testing Strategy
1. **Unit tests**: Create for each migrated module
2. **Integration tests**: Verify module interactions
3. **String index tests**: Specific tests for 1-based → 0-based conversion
4. **Unicode tests**: Verify special characters handled correctly
5. **Performance tests**: Ensure no regression in critical paths

---

## Phase 3: C_SclStringList Migration to QStringList

### Priority: MEDIUM
**Estimated Impact**: ~214 occurrences
**Complexity**: Medium
**Risk**: Low (straightforward mapping)
**Dependency**: Should follow Phase 2 (C_SclString migration)

### Current State Analysis

#### C_SclStringList Overview
- **Purpose**: Borland TStringList compatibility wrapper
- **Current Usage**: 214 occurrences
- **Location**: `opensyde_core/scl/C_SclStringList.{cpp,hpp}`
- **Internal Storage**: `C_SclDynamicArray<C_SclString>`

### Migration Mapping

| C_SclStringList Method | QStringList Equivalent |
|------------------------|------------------------|
| `.Add(str)` | `.append(str)` |
| `.Append(str)` | `.append(str)` |
| `.Clear()` | `.clear()` |
| `.Delete(idx)` | `.removeAt(idx)` |
| `.Exchange(idx1, idx2)` | `.swapItemsAt(idx1, idx2)` (Qt 5.13+) |
| `.Insert(idx, str)` | `.insert(idx, str)` |
| `.IndexOf(str)` | `.indexOf(str)` |
| `.GetText(sep)` | `.join(sep)` |
| `.GetCount()` | `.count()` or `.size()` |
| `.LoadFromFile(path)` | Manual: `QFile` read + `.split('\n')` |
| `.SaveToFile(path)` | Manual: `QFile` write + `.join('\n')` |
| `.IndexOfName(key)` | Custom helper or iterate |
| `.ValueFromIndex(idx)` | Custom helper |
| `.Values(key)` | Custom helper |
| `.AddStrings(list)` | `.append(*list)` |
| `.Sort()` | `.sort()` |
| `.Strings[idx]` | `[idx]` (direct indexing) |

### Migration Strategy

#### 3.1 Replace Direct Usage
Simple replacements where `C_SclStringList` is used locally:
```cpp
// Before:
C_SclStringList c_List;
c_List.Add("item");
c_List.SaveToFile("output.txt");

// After:
QStringList c_List;
c_List.append("item");
QFile file("output.txt");
file.open(QIODevice::WriteOnly);
file.write(c_List.join("\n").toUtf8());
file.close();
```

#### 3.2 Key=Value Functionality
C_SclStringList has special key=value pair functionality (like INI files).
**Options**:
- **Option A**: Create helper functions for key=value operations
- **Option B**: Replace with `QMap<QString, QString>` where key=value semantics are needed
- **Option C**: Use `QSettings` for configuration-style usage

**Recommendation**: Option B (QMap) for clarity and type safety

#### 3.3 File I/O Patterns
For `.LoadFromFile()` and `.SaveToFile()`:
```cpp
// Helper function to maintain compatibility
QStringList LoadStringListFromFile(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QStringList();
    }
    QTextStream in(&file);
    QStringList result;
    while (!in.atEnd()) {
        result.append(in.readLine());
    }
    return result;
}

void SaveStringListToFile(const QStringList& list, const QString& path) {
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << list.join("\n");
    }
}
```

### Execution Plan

1. **Create helper utilities** for LoadFromFile/SaveToFile patterns
2. **Migrate low-usage files first** (1-2 occurrences)
3. **Update core modules** with heavy usage
4. **Remove C_SclStringList class** after all migrations complete

---

## Phase 4: C_SclDynamicArray Migration to QList/QVector

### Priority: LOW-MEDIUM
**Estimated Impact**: ~137 occurrences
**Complexity**: Low-Medium
**Risk**: Low (already wraps QList internally!)

### Current State Analysis

#### C_SclDynamicArray Overview
- **Purpose**: Borland DynamicArray compatibility wrapper
- **Current Implementation**: **Already wraps QList internally!** (see line 34 of C_SclDynamicArray.hpp)
- **Current Usage**: 137 occurrences
- **Location**: `opensyde_core/scl/C_SclDynamicArray.hpp` (header-only template)

### Key Insight
**The wrapper already uses QList internally!** This is purely a compatibility layer for Borland-style indexing and API.

### Migration Mapping

| C_SclDynamicArray Method | QList/QVector Equivalent |
|--------------------------|--------------------------|
| `[idx]` (0-based) | `[idx]` or `.at(idx)` |
| `.Delete(idx)` | `.removeAt(idx)` |
| `.Insert(idx, item)` | `.insert(idx, item)` |
| `.GetLength()` | `.size()` or `.count()` |
| `.GetHigh()` | `.size() - 1` |
| `.SetLength(len)` | `.resize(len)` |
| `.IncLength(by)` | `.resize(size() + by)` |
| `.AsQList()` | Direct use (no conversion needed) |

### Migration Strategy

#### 4.1 QList vs QVector Choice
**Use QList** in most cases:
- Qt's recommended general-purpose container (Qt 6+)
- Optimized for typical use cases
- Better for non-POD types

**Use QVector** only if:
- Strict contiguous memory required
- Cache locality critical for performance
- Interfacing with C APIs requiring contiguous arrays

**Recommendation**: Default to `QList<T>` unless specific reason for `QVector<T>`

#### 4.2 Automated Replacement
Since the API is simple and mapping is 1:1, this is a good candidate for automated replacement:

```python
# replacement_patterns.py
replacements = {
    r'C_SclDynamicArray<(.+?)>': r'QList<\1>',
    r'\.GetLength\(\)': r'.size()',
    r'\.GetHigh\(\)': r'.size() - 1',
    r'\.SetLength\((.+?)\)': r'.resize(\1)',
    r'\.IncLength\((.+?)\)': r'.resize(size() + \1)',
    r'\.Delete\((.+?)\)': r'.removeAt(\1)',
    # Insert and indexing remain the same
}
```

#### 4.3 Manual Verification Required
- **GetHigh()** usage in loops: Verify correct behavior with `.size() - 1`
- **IncLength()** with default parameter: May need explicit `size() + 1`
- **Nested templates**: `C_SclDynamicArray<C_SclDynamicArray<T>>` → `QList<QList<T>>`

### Execution Plan

1. **Create automated replacement script**
2. **Test on small subset** of files (5-10 files)
3. **Verify compilation and basic functionality**
4. **Run on all remaining files**
5. **Manual review** of complex cases (nested templates, GetHigh in loops)
6. **Remove C_SclDynamicArray.hpp** after completion

---

## Phase 5: C_SclIniFile API Migration (Optional)

### Priority: LOW
**Estimated Impact**: Minimal (already uses QSettings internally)
**Complexity**: Low
**Risk**: Low

### Current State

C_SclIniFile **already uses QSettings internally** after your Phase 1 changes. The class now provides a compatibility API over Qt's native INI handling.

### Options

#### Option A: Keep C_SclIniFile (RECOMMENDED)
**Rationale**:
- Already modernized (uses QSettings internally)
- Provides stable API for existing code
- No significant benefit from further migration
- Maintains API compatibility for external code

#### Option B: Migrate to Direct QSettings Usage
**Only if**:
- You want to expose advanced QSettings features
- You want to reduce wrapper count to absolute minimum
- You have time for comprehensive testing of all INI operations

### Recommendation
**Keep C_SclIniFile as-is.** It's already modernized and provides value as a stable API.

---

## Phase 6: C_SclChecksums and C_SclResourceStrings (Optional)

### Priority: VERY LOW
**Impact**: Minimal
**Complexity**: Low

### C_SclChecksums
**Current Purpose**: Checksum calculation utilities
**Qt Alternative**: QCryptographicHash
**Recommendation**: Migrate only if needed for consistency

### C_SclResourceStrings
**Current Purpose**: String resource management
**Qt Alternative**: Qt Linguist / translation system
**Recommendation**: Evaluate if used; may be legacy

---

## Execution Order and Timeline

### Recommended Sequence

1. **Phase 2: C_SclString → QString** (HIGHEST PRIORITY)
   - Start with Phase 2A (Data Model Layer)
   - Continue through 2B-2E incrementally
   - Estimate: 4-8 weeks depending on thoroughness

2. **Phase 3: C_SclStringList → QStringList** (AFTER Phase 2)
   - Depends on Phase 2 completion
   - Estimate: 1-2 weeks

3. **Phase 4: C_SclDynamicArray → QList** (CAN RUN PARALLEL to Phase 3)
   - Independent of string migration
   - Good candidate for automation
   - Estimate: 1 week

4. **Phases 5-6: Optional cleanup** (AS NEEDED)
   - Only if pursuing absolute minimal wrapper count
   - Estimate: 1 week

### Total Estimated Timeline
- **Conservative**: 6-11 weeks
- **Aggressive**: 4-6 weeks (with automation and parallel work)

---

## Risk Mitigation

### Key Risks

1. **Index Off-by-One Errors** (C_SclString migration)
   - **Mitigation**: Comprehensive unit tests, manual audit of SubString/Pos calls
   - **Severity**: HIGH

2. **Unicode/Encoding Issues** (C_SclString migration)
   - **Mitigation**: Test with special characters, international text
   - **Severity**: MEDIUM

3. **Performance Regression**
   - **Mitigation**: Benchmark critical paths before/after
   - **Severity**: LOW (QString generally faster)

4. **API Breaking Changes**
   - **Mitigation**: Update all consumers in same changeset
   - **Severity**: MEDIUM (internal codebase only)

### Testing Requirements

For each phase:
- ✅ Unit tests for migrated modules
- ✅ Integration tests across module boundaries
- ✅ Manual testing of critical features
- ✅ Performance benchmarking of hot paths
- ✅ Unicode/special character testing (Phase 2)
- ✅ Index boundary testing (Phase 2)

---

## Success Metrics

### Phase Completion Criteria

Each phase is complete when:
1. ✅ All target files migrated
2. ✅ All tests passing
3. ✅ No compiler warnings introduced
4. ✅ Code review approved
5. ✅ Legacy wrapper classes removed from build
6. ✅ Documentation updated

### Overall Project Success

Project is successful when:
1. ✅ All SCL wrapper classes eliminated (or justified exceptions documented)
2. ✅ Codebase uses Qt native types throughout
3. ✅ No performance regressions
4. ✅ All functionality maintained
5. ✅ Build system simplified (fewer custom dependencies)

---

## Appendix: Migration Tools

### Automated Migration Scripts

#### A.1 C_SclString → QString Helper Script
```python
#!/usr/bin/env python3
"""
Script to assist with C_SclString to QString migration
"""

import re
import sys

def convert_file(filepath):
    replacements = [
        # Member access patterns
        (r'\.Length\(\)', r'.length()'),
        (r'\.UpperCase\(\)', r'.toUpper()'),
        (r'\.LowerCase\(\)', r'.toLower()'),
        (r'\.Trim\(\)', r'.trimmed()'),

        # Note: SubString and Pos require manual review due to indexing!
        # (r'\.SubString\((\d+), (\d+)\)', r'.mid(\1-1, \2)'),

        # Static methods
        (r'C_SclString::IntToStr\(', r'QString::number('),

        # Type declarations
        (r'C_SclString\s+', r'QString '),
        (r'<C_SclString>', r'<QString>'),
    ]

    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    for pattern, replacement in replacements:
        content = re.sub(pattern, replacement, content)

    # Flag manual review items
    if '.SubString(' in content or '.Pos(' in content:
        print(f"⚠️  {filepath} - MANUAL REVIEW REQUIRED (SubString/Pos indexing)")

    with open(filepath, 'w', encoding='utf-8') as f:
        f.write(content)

if __name__ == '__main__':
    convert_file(sys.argv[1])
```

#### A.2 C_SclDynamicArray → QList Script
```python
#!/usr/bin/env python3
"""
Script to convert C_SclDynamicArray to QList
"""

import re
import sys

def convert_file(filepath):
    replacements = [
        (r'C_SclDynamicArray<(.+?)>', r'QList<\1>'),
        (r'\.GetLength\(\)', r'.size()'),
        (r'\.GetHigh\(\)', r'(.size() - 1)'),
        (r'\.SetLength\(', r'.resize('),
        (r'\.Delete\(', r'.removeAt('),
        # IncLength needs special handling
    ]

    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    for pattern, replacement in replacements:
        content = re.sub(pattern, replacement, content)

    # Special handling for IncLength
    # .IncLength() → .resize(size() + 1)
    # .IncLength(n) → .resize(size() + n)
    content = re.sub(r'\.IncLength\(\)', r'.resize(size() + 1)', content)
    content = re.sub(r'\.IncLength\((.+?)\)', r'.resize(size() + \1)', content)

    with open(filepath, 'w', encoding='utf-8') as f:
        f.write(content)

if __name__ == '__main__':
    convert_file(sys.argv[1])
```

---

## Document Revision History

| Date | Version | Changes |
|------|---------|---------|
| 2026-01-16 | 1.0 | Initial plan created after Phase 1 completion |

---

## Notes

- This plan focuses on **opensyde_tool** and **opensyde_core** libraries
- Other tool projects (opensyde_cmd_line_flash_tool, opensyde_syde_coder_c, etc.) may need similar migrations
- Plan assumes continued Windows-only support (Linux support removed in Phase 1)
- All Qt references assume Qt 6.x (currently using Qt 6.10.1)

---

**Last Updated**: 2026-01-16
**Status**: Phase 1 Complete, Phase 2 Ready to Begin
