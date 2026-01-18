#!/usr/bin/env python3
"""
Script to replace TglFile function calls with Qt equivalents
"""

import os
import re
import sys

def process_file(filepath):
    """Process a single file and replace TglFile calls"""
    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
    except Exception as e:
        print(f"Error reading {filepath}: {e}")
        return False

    original_content = content
    modified = False

    # Track if we need Qt includes
    needs_qfileinfo = False
    needs_qdir = False
    needs_qcoreapp = False

    # Replace TglFileExists
    pattern = r'TglFileExists\s*\(\s*([^)]+)\s*\)'
    def repl_fileexists(m):
        nonlocal needs_qfileinfo, modified
        needs_qfileinfo = True
        modified = True
        arg = m.group(1).strip()
        return f'(QFileInfo(QString::fromStdString(*{arg}.AsStdString())).exists() && QFileInfo(QString::fromStdString(*{arg}.AsStdString())).isFile())'
    content = re.sub(pattern, repl_fileexists, content)

    # Replace TglDirectoryExists
    pattern = r'TglDirectoryExists\s*\(\s*([^)]+)\s*\)'
    def repl_direxists(m):
        nonlocal needs_qfileinfo, modified
        needs_qfileinfo = True
        modified = True
        arg = m.group(1).strip()
        return f'QFileInfo(QString::fromStdString(*{arg}.AsStdString())).isDir()'
    content = re.sub(pattern, repl_direxists, content)

    # Replace TglExtractFileName
    pattern = r'TglExtractFileName\s*\(\s*([^)]+)\s*\)'
    def repl_extractfilename(m):
        nonlocal needs_qfileinfo, modified
        needs_qfileinfo = True
        modified = True
        arg = m.group(1).strip()
        return f'QFileInfo(QString::fromStdString(*{arg}.AsStdString())).fileName().toStdString()'
    content = re.sub(pattern, repl_extractfilename, content)

    # Replace TglExtractFilePath
    pattern = r'TglExtractFilePath\s*\(\s*([^)]+)\s*\)'
    def repl_extractfilepath(m):
        nonlocal needs_qfileinfo, modified
        needs_qfileinfo = True
        modified = True
        arg = m.group(1).strip()
        return f'(QFileInfo(QString::fromStdString(*{arg}.AsStdString())).absolutePath() + "/").toStdString()'
    content = re.sub(pattern, repl_extractfilepath, content)

    # Replace TglExtractFileExtension
    pattern = r'TglExtractFileExtension\s*\(\s*([^)]+)\s*\)'
    def repl_extractext(m):
        nonlocal needs_qfileinfo, modified
        needs_qfileinfo = True
        modified = True
        arg = m.group(1).strip()
        return f'("." + QFileInfo(QString::fromStdString(*{arg}.AsStdString())).suffix()).toStdString()'
    content = re.sub(pattern, repl_extractext, content)

    # Replace TglIsRelativePath
    pattern = r'TglIsRelativePath\s*\(\s*([^)]+)\s*\)'
    def repl_isrelative(m):
        nonlocal needs_qdir, modified
        needs_qdir = True
        modified = True
        arg = m.group(1).strip()
        return f'QDir::isRelativePath(QString::fromStdString(*{arg}.AsStdString()))'
    content = re.sub(pattern, repl_isrelative, content)

    # Replace TglFileSize
    pattern = r'TglFileSize\s*\(\s*([^)]+)\s*\)'
    def repl_filesize(m):
        nonlocal needs_qfileinfo, modified
        needs_qfileinfo = True
        modified = True
        arg = m.group(1).strip()
        return f'static_cast<int32_t>(QFileInfo(QString::fromStdString(*{arg}.AsStdString())).size())'
    content = re.sub(pattern, repl_filesize, content)

    # Replace TglCreateDirectory
    pattern = r'TglCreateDirectory\s*\(\s*([^)]+)\s*\)'
    def repl_createdir(m):
        nonlocal needs_qdir, modified
        needs_qdir = True
        modified = True
        arg = m.group(1).strip()
        return f'(QDir().mkpath(QString::fromStdString(*{arg}.AsStdString())) ? 0 : -1)'
    content = re.sub(pattern, repl_createdir, content)

    # Replace TglRemoveDirectory - note: this is more complex, only handle simple case
    pattern = r'TglRemoveDirectory\s*\(\s*([^,]+)\s*,\s*false\s*\)'
    def repl_removedir(m):
        nonlocal needs_qdir, modified
        needs_qdir = True
        modified = True
        arg = m.group(1).strip()
        return f'(QDir(QString::fromStdString(*{arg}.AsStdString())).removeRecursively() ? 0 : -1)'
    content = re.sub(pattern, repl_removedir, content)

    # Replace TglExpandFileName
    pattern = r'TglExpandFileName\s*\(\s*([^,]+)\s*,\s*([^)]+)\s*\)'
    def repl_expand(m):
        nonlocal needs_qdir, modified
        needs_qdir = True
        modified = True
        arg1 = m.group(1).strip()
        arg2 = m.group(2).strip()
        return f'QDir(QString::fromStdString(*{arg2}.AsStdString())).absoluteFilePath(QString::fromStdString(*{arg1}.AsStdString())).toStdString()'
    content = re.sub(pattern, repl_expand, content)

    # Remove TglFile.hpp include
    pattern = r'#include\s+"TglFile\.hpp"\s*\n'
    if re.search(pattern, content):
        content = re.sub(pattern, '', content)
        modified = True

    # Add Qt includes if needed and not already present
    if modified:
        include_section = content[:content.find('\n\n')]

        if needs_qfileinfo and '#include <QFileInfo>' not in content:
            # Find the last #include before the first blank line
            lines = content.split('\n')
            insert_pos = 0
            for i, line in enumerate(lines):
                if line.startswith('#include'):
                    insert_pos = i + 1
                elif line.strip() == '' and insert_pos > 0:
                    break
            lines.insert(insert_pos, '#include <QFileInfo>')
            content = '\n'.join(lines)

        if needs_qdir and '#include <QDir>' not in content:
            lines = content.split('\n')
            insert_pos = 0
            for i, line in enumerate(lines):
                if line.startswith('#include'):
                    insert_pos = i + 1
                elif line.strip() == '' and insert_pos > 0:
                    break
            lines.insert(insert_pos, '#include <QDir>')
            content = '\n'.join(lines)

        if needs_qcoreapp and '#include <QCoreApplication>' not in content:
            lines = content.split('\n')
            insert_pos = 0
            for i, line in enumerate(lines):
                if line.startswith('#include'):
                    insert_pos = i + 1
                elif line.strip() == '' and insert_pos > 0:
                    break
            lines.insert(insert_pos, '#include <QCoreApplication>')
            content = '\n'.join(lines)

    if modified:
        try:
            with open(filepath, 'w', encoding='utf-8', newline='\n') as f:
                f.write(content)
            print(f"Modified: {filepath}")
            return True
        except Exception as e:
            print(f"Error writing {filepath}: {e}")
            return False

    return False

def main():
    """Main function"""
    if len(sys.argv) > 1:
        # Process specific file
        filepath = sys.argv[1]
        if os.path.exists(filepath):
            process_file(filepath)
        else:
            print(f"File not found: {filepath}")
    else:
        # Process all .cpp files in opensyde_tool
        root_dir = os.path.join(os.path.dirname(__file__), 'opensyde_tool')
        count = 0
        for root, dirs, files in os.walk(root_dir):
            for file in files:
                if file.endswith('.cpp'):
                    filepath = os.path.join(root, file)
                    if process_file(filepath):
                        count += 1
        print(f"\nTotal files modified: {count}")

if __name__ == '__main__':
    main()
