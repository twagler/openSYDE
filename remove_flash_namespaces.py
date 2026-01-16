import os

files = [
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\src\syde_flash\util\C_FlaUtiStyleSheets.cpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\src\syde_flash\util\C_FlaUti.cpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\src\syde_flash\user_settings\C_UsHandler.cpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\src\syde_flash\user_settings\C_UsFiler.cpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\src\syde_flash\update\C_FlaUpSequences.cpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\src\syde_flash\update\C_FlaUpProperties.cpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\src\syde_flash\update\C_FlaUpListWidget.cpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\src\syde_flash\update\C_FlaUpListItemWidget.cpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\src\syde_flash\update\C_FlaUpHexFileView.cpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\src\syde_flash\settings\C_FlaSetWidget.cpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\src\syde_flash\settings\C_FlaSetProgressWidget.cpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\src\syde_flash\settings\C_FlaSetAdvancedPropertiesWidget.cpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\src\syde_flash\settings\C_FlaDllWidget.cpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\src\syde_flash\search_nodes\C_FlaSenSearchNodePopup.cpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\src\syde_flash\search_nodes\C_FlaSenDcBasicSequences.cpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\src\syde_flash\properties\C_FlaPropWidget.cpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\src\syde_flash\gui_elements\text_browser\C_FlaOgeTebProgressLog.cpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\src\syde_flash\gui_elements\spin_box\C_FlaOgeSpxDark.cpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\src\syde_flash\gui_elements\push_button\C_FlaOgePubPathVariables.cpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\src\syde_flash\gui_elements\line_edit\C_FlaOgeLeFilePath.cpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\src\syde_flash\fla_main.cpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\src\syde_flash\C_FlaTitleBarWidget.cpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\src\syde_flash\C_FlaMainWindow.cpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\src\syde_flash\configure_node\C_FlaConNodeConfigPopup.cpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\src\syde_flash\bottom_bar\C_FlaBottomBar.cpp"
]

def remove_namespace(file_path):
    try:
        if not os.path.exists(file_path):
            print(f"Skipping {file_path} (not found)")
            return

        with open(file_path, 'r') as f:
            lines = f.readlines()

        new_lines = []
        changed = False
        for line in lines:
            if "using namespace stw::opensyde_gui_logic;" in line:
                # Check if it's already commented
                if line.strip().startswith("//"):
                    new_lines.append(line)
                else:
                    new_lines.append("\n") # Replace with empty line to keep line numbers if possible, or just remove
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
        print(f"Error processing {file_path}: {e}")

for file in files:
    remove_namespace(file)
