#!/usr/bin/env python3
"""
Smart namespace fixer - analyzes build errors and restores needed namespace declarations.
"""

import re
import subprocess
from pathlib import Path
from collections import defaultdict

def get_failing_files():
    """Run ninja and extract files with compilation errors."""
    try:
        result = subprocess.run(
            ['ninja'],
            cwd=r'c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\pjt\openSYDE_CAN_Monitor\build',
            capture_output=True,
            text=True,
            timeout=60
        )
        
        failing_files = set()
        # Extract file paths from error messages
        for line in result.stderr.split('\n') + result.stdout.split('\n'):
            match = re.search(r'(opensyde_tool/src/.+?\.cpp):', line)
            if match:
                failing_files.add(match.group(1))
        
        return failing_files
    except Exception as e:
        print(f"Error running ninja: {e}")
        return set()

def analyze_file_needs_namespaces(file_path):
    """Analyze which namespaces a file needs based on its includes and usage."""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        namespaces_needed = set()
        
        # Check what classes from opensyde_gui_logic are used
        logic_classes = [
            'C_GtGetText', 'C_CamProHandler', 'C_CamUti', 'C_Uti',
            'C_PopErrorHandling', 'C_CamUtiGeneric', 'C_CamProHandlerFiler',
            'C_CamProMessageData', 'C_CamProFilterData', 'C_CamProDatabaseData',
            'C_CieConverter', 'C_CieImportDbc', 'C_SdNdeDpContentUtil'
        ]
        
        # Check what classes from opensyde_gui are used
        gui_classes = [
            'C_SyvComMessageLoggerFileBlf', 'C_SyvComMessageMonitor',
            'C_SyvComDriverThread', 'C_NagToolTip'
        ]
        
        for cls in logic_classes:
            if cls in content and 'using namespace stw::opensyde_gui_logic;' not in content:
                namespaces_needed.add('opensyde_gui_logic')
                break
        
        for cls in gui_classes:
            if cls in content and 'using namespace stw::opensyde_gui;' not in content:
                namespaces_needed.add('opensyde_gui')
                break
        
        return namespaces_needed
    except Exception as e:
        print(f"Error analyzing {file_path}: {e}")
        return set()

def add_namespaces_to_file(file_path, namespaces):
    """Add missing namespace declarations to a file."""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            lines = f.readlines()
        
        # Find the namespace section
        namespace_line_idx = None
        for i, line in enumerate(lines):
            if 'using namespace stw::' in line:
                namespace_line_idx = i
                break
        
        if namespace_line_idx is None:
            # No namespace section found, can't fix
            return False
        
        # Add missing namespaces right after the first namespace declaration
        added_any = False
        for ns in sorted(namespaces):
            namespace_decl = f'using namespace stw::{ns};\n'
            # Check if it already exists
            if not any(namespace_decl.strip() in line for line in lines):
                lines.insert(namespace_line_idx + 1, namespace_decl)
                namespace_line_idx += 1
                added_any = True
        
        if added_any:
            with open(file_path, 'w', encoding='utf-8', newline='\r\n') as f:
                f.writelines(lines)
            return True
        
        return False
    except Exception as e:
        print(f"Error modifying {file_path}: {e}")
        return False

def main():
    project_root = Path(r'c:\Users\tyler\Dev\repos\openSYDE')
    
    print("Analyzing build errors...")
    failing_files = get_failing_files()
    
    if not failing_files:
        print("No failing files found or ninja not accessible")
        return
    
    print(f"Found {len(failing_files)} files with errors")
    
    fixed_count = 0
    for rel_path in failing_files:
        file_path = project_root / rel_path
        if file_path.exists():
            namespaces_needed = analyze_file_needs_namespaces(file_path)
            if namespaces_needed:
                if add_namespaces_to_file(file_path, namespaces_needed):
                    fixed_count += 1
                    print(f"Fixed: {file_path.name} (added: {', '.join(namespaces_needed)})")
    
    print(f"\n--- Summary ---")
    print(f"Files fixed: {fixed_count}")
    print("\nDone! Try building again.")

if __name__ == '__main__':
    main()
