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

def restore_namespace(file_path):
    try:
        if not os.path.exists(file_path):
            print(f"Skipping {file_path}")
            return

        with open(file_path, 'r') as f:
            lines = f.readlines()

        if any("using namespace stw::opensyde_gui_logic;" in line for line in lines):
            print(f"Skipping {file_path} (already has namespace)")
            return

        new_lines = []
        inserted = False
        for line in lines:
            new_lines.append(line)
            if not inserted and ("using namespace stw::opensyde_core;" in line or "using namespace stw::opensyde_gui;" in line):
                 new_lines.append("using namespace stw::opensyde_gui_logic;\n")
                 inserted = True
        
        # If not inserted yet, try finding "using namespace" logic generally
        if not inserted:
             # Just reset and look for any using namespace or start of file
             new_lines = []
             for line in lines:
                 new_lines.append(line)
                 if not inserted and "using namespace" in line:
                      new_lines.append("using namespace stw::opensyde_gui_logic;\n")
                      inserted = True
        
        if inserted:
            with open(file_path, 'w') as f:
                f.writelines(new_lines)
            print(f"Restored for {file_path}")
        else:
            print(f"Could not find place to insert for {file_path}")

    except Exception as e:
        print(f"Error {file_path}: {e}")

for f in files:
    restore_namespace(f)
