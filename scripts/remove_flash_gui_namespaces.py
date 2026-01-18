import os

files = [
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\src\syde_flash\gui_elements\spin_box\C_FlaOgeSpxDark.cpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\src\syde_flash\gui_elements\push_button\C_FlaOgePubPathVariables.cpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\src\syde_flash\gui_elements\line_edit\C_FlaOgeLeFilePath.cpp"
]

def remove_namespace(file_path):
    try:
        if not os.path.exists(file_path):
            print(f"Skipping {file_path}")
            return

        with open(file_path, 'r') as f:
            lines = f.readlines()

        new_lines = []
        changed = False
        for line in lines:
            if "using namespace stw::opensyde_gui_logic;" in line:
                 new_lines.append("\n") # Replace with empty line
                 changed = True
            else:
                new_lines.append(line)
        
        if changed:
            with open(file_path, 'w') as f:
                f.writelines(new_lines)
            print(f"Updated {file_path}")
        else:
            print(f"No changes for {file_path}")

    except Exception as e:
        print(f"Error {file_path}: {e}")

for f in files:
    remove_namespace(f)
