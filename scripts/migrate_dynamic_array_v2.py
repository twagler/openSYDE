import re
import os

root_dirs = [
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\libs\opensyde_core",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\src"
]

replacements = [
    # Class name (case sensitive)
    (r"\bC_SclDynamicArray\b", "QList"),
    
    # Methods (must be followed by ( to avoid partial matches)
    (r"\.GetLength\(\)", ".size()"),
    (r"\.GetHigh\(\)", "(.size() > 0 ? .size() - 1 : 0)"),
    (r"\.SetLength\(([^)]+)\)", r".resize(\1)"),
    (r"\.IncLength\(\)", ".resize(.size() + 1)"),
    (r"\.IncLength\(([^)]+)\)", r".resize(.size() + \1)"),
    (r"\.Delete\(([^)]+)\)", r".removeAt(\1)"),
    
    # AsQList() -> remove it
    (r"\.AsQList\(\)->", "."),
    (r"\.AsQList\(\)", ""),
]

for root_dir in root_dirs:
    for root, dirs, files in os.walk(root_dir):
        for filename in files:
            if filename.endswith((".cpp", ".hpp", ".h")):
                filepath = os.path.join(root, filename)
                
                if "C_SclDynamicArray.hpp" in filename:
                    continue
                
                # print(f"Processing: {filepath}")
                try:
                    with open(filepath, "r", encoding="utf-8") as f:
                        content = f.read()
                except UnicodeDecodeError:
                    try:
                        with open(filepath, "r", encoding="latin-1") as f:
                            content = f.read()
                    except Exception as e:
                        print(f"Error reading {filepath}: {e}")
                        continue
                
                new_content = content
                for pattern, substitution in replacements:
                    new_content = re.sub(pattern, substitution, new_content)
                    
                if new_content != content:
                    with open(filepath, "w", encoding="utf-8") as f:
                        f.write(new_content)
                    print(f"Updated: {filepath}")
