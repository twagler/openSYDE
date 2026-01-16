#!/usr/bin/env python3
"""
Add back 'using namespace stw::opensyde_gui_logic;' to files that actually need it.
These are files that have compilation errors due to missing namespace declarations.
"""

from pathlib import Path

# Files identified from build errors that need the namespace
FILES_NEEDING_NAMESPACE = [
    "can_monitor/util/C_CamUtiGeneric.cpp",
    "com_import_export/C_CieConverter.cpp",
    "com_import_export/C_CieImportDbc.cpp",
    "project_operations/C_PopErrorHandling.cpp",
    "system_definition/node_edit/datapools/C_SdNdeDpContentUtil.cpp",
    "system_views/communication/C_SyvComDriverThread.cpp",
    "system_views/communication/C_SyvComMessageLoggerFileBlf.cpp",
    "system_views/communication/C_SyvComMessageMonitor.cpp",
    "util/C_Uti.cpp",
    # Also add to the settings file we already fixed
    "can_monitor/can_monitor_settings/C_CamMosDllWidgetLogic.cpp",
]

def add_namespace_if_missing(file_path):
    """Add opensyde_gui_logic namespace if it's missing."""
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # Check if it already has the namespace
    if 'using namespace stw::opensyde_gui_logic;' in content:
        return False
    
    # Find the namespace section and add it
    lines = content.split('\n')
    new_lines = []
    added = False
    
    for i, line in enumerate(lines):
        new_lines.append(line)
        # Add after other namespace declarations
        if not added and 'using namespace stw::opensyde_gui' in line and 'opensyde_gui_logic' not in line:
            new_lines.append('using namespace stw::opensyde_gui_logic;')
            added = True
    
    if added:
        with open(file_path, 'w', encoding='utf-8', newline='\r\n') as f:
            f.write('\n'.join(new_lines))
        return True
    
    return False

def main():
    project_root = Path(r'c:\Users\tyler\Dev\repos\openSYDE')
    src_path = project_root / 'opensyde_tool' / 'src'
    
    fixed_count = 0
    
    for rel_path in FILES_NEEDING_NAMESPACE:
        file_path = src_path / rel_path
        if file_path.exists():
            if add_namespace_if_missing(file_path):
                fixed_count += 1
                print(f"Fixed: {file_path.name}")
        else:
            print(f"Not found: {file_path}")
    
    print(f"\n--- Summary ---")
    print(f"Files fixed: {fixed_count}")
    print("\nDone!")

if __name__ == '__main__':
    main()
