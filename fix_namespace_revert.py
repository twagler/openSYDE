import re
import os

def main():
    log_file = "c:\\Users\\tyler\\Dev\\repos\\openSYDE\\opensyde_tool\\pjt\\openSYDE_CAN_Monitor\\build\\build_log.txt"
    remove_line = "using namespace stw::opensyde_gui_logic;"
    
    if not os.path.exists(log_file):
        print(f"Log file not found: {log_file}")
        return

    with open(log_file, 'r', encoding='utf-8', errors='ignore') as f:
        log_content = f.read()

    # Regex to find file paths
    # Matches "Drive:/Path/To/File.cpp:Line:Col: error: ... not a namespace-name"
    # Note: Windows paths might be detected.
    pattern = re.compile(r'([a-zA-Z]:[\\/].+\.cpp):\d+:\d+: error:.*not a namespace-name')
    
    files_to_fix = set()
    
    for line in log_content.split('\n'):
        match = pattern.search(line)
        if match:
            files_to_fix.add(match.group(1))
            
    print(f"Found {len(files_to_fix)} files to correct.")
    
    fixed_count = 0
    for filepath in files_to_fix:
        # Normalize path separators
        filepath = os.path.normpath(filepath)
        
        try:
            with open(filepath, 'r', encoding='utf-8') as f:
                lines = f.readlines()
            
            new_lines = []
            removed = False
            for line in lines:
                if remove_line in line:
                    removed = True
                    continue # Skip this line
                new_lines.append(line)
            
            if removed:
                with open(filepath, 'w', encoding='utf-8') as f:
                    f.writelines(new_lines)
                print(f"Fixed {filepath}")
                fixed_count += 1
            else:
                print(f"Namespace line not found in {filepath} (already fixed?)")
                
        except Exception as e:
            print(f"Error fixing {filepath}: {e}")

    print(f"Total files corrected: {fixed_count}")

if __name__ == "__main__":
    main()
