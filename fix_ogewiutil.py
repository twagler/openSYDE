#!/usr/bin/env python3
"""
Bulk fixer for 'C_OgeWiUtil' usage.
Adds 'using namespace stw::opensyde_gui_logic;' to files using C_OgeWiUtil if missing.
"""

import os

def main():
    root_dir = r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\src"
    target_class = "C_OgeWiUtil"
    needed_namespace = "using namespace stw::opensyde_gui_logic;"
    
    fixed_count = 0
    
    for dirpath, dirnames, filenames in os.walk(root_dir):
        for filename in filenames:
            if not filename.endswith(".cpp"):
                continue
                
            filepath = os.path.join(dirpath, filename)
            
            try:
                with open(filepath, 'r', encoding='utf-8') as f:
                    content = f.read()
                
                # Check criteria
                if target_class in content and needed_namespace not in content:
                    print(f"Fixing {filename}...")
                    
                    # Find insertion point
                    lines = content.split('\n')
                    insert_idx = -1
                    
                    # Try to insert after other using namespace directives
                    for i, line in enumerate(lines):
                        if "using namespace stw::" in line:
                            insert_idx = i
                    
                    # If no namespace directives, try after includes
                    if insert_idx == -1:
                        for i, line in enumerate(lines):
                            if line.strip().startswith("#include"):
                                insert_idx = i
                    
                    if insert_idx != -1:
                        lines.insert(insert_idx + 1, needed_namespace)
                        
                        with open(filepath, 'w', encoding='utf-8', newline='\r\n') as f:
                            f.write('\n'.join(lines))
                        fixed_count += 1
                    else:
                        print(f"  Could not find compiled insertion point for {filename}")
                        
            except Exception as e:
                print(f"Error processing {filename}: {e}")

    print(f"\nTotal files fixed: {fixed_count}")

if __name__ == "__main__":
    main()
