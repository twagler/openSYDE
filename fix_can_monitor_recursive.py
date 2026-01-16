import os

def main():
    root_dir = r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\src\can_monitor"
    needed_namespace = "using namespace stw::opensyde_gui_logic;"
    
    files_fixed = 0
    
    for dirpath, dirnames, filenames in os.walk(root_dir):
        for filename in filenames:
            if not filename.endswith(".cpp"):
                continue
                
            filepath = os.path.join(dirpath, filename)
            
            try:
                with open(filepath, 'r', encoding='utf-8') as f:
                    content = f.read()
                
                # Simple heuristic: if it doesn't have the namespace, add it.
                # Most files in can_monitor seem to strictly need it.
                if needed_namespace not in content:
                    # Check if it has ANY namespace usage to find insertion point
                    lines = content.split('\n')
                    insert_idx = -1
                    
                    for i, line in enumerate(lines):
                       if line.strip().startswith("using namespace"):
                           insert_idx = i
                    
                    if insert_idx == -1:
                        for i, line in enumerate(lines):
                            if line.strip().startswith("#include"):
                                insert_idx = i

                    if insert_idx != -1:
                        print(f"Fixing {filename}")
                        lines.insert(insert_idx + 1, needed_namespace)
                        with open(filepath, 'w', encoding='utf-8', newline='\r\n') as f:
                            f.write('\n'.join(lines))
                        files_fixed += 1
            except Exception as e:
                print(f"Error processing {filename}: {e}")

    print(f"Total files fixed: {files_fixed}")

if __name__ == "__main__":
    main()
