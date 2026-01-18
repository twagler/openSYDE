import re
import os

files_to_update = [
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\libs\opensyde_core\can_dispatcher\target_windows_stw_dlls\C_Can.cpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\libs\opensyde_core\can_dispatcher\target_windows_stw_dlls\C_Can.hpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\libs\opensyde_core\can_dispatcher\dispatcher\C_CanBase.cpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\libs\opensyde_core\can_dispatcher\dispatcher\C_CanBase.hpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\libs\opensyde_core\can_dispatcher\dispatcher\C_CanDispatcher.hpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\libs\opensyde_core\scl\C_SclIniFile.hpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\libs\opensyde_core\protocol_drivers\C_OscHexFile.hpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\libs\opensyde_core\kefex_diaglib\dl_string_resources\C_SclResourceStrings.hpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\libs\opensyde_core\scl\C_SclStringList.hpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\libs\opensyde_core\scl\C_SclString.hpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\libs\opensyde_core\kefex_diaglib\dl_kefex\CKFXCommConfiguration.hpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\libs\opensyde_core\kefex_diaglib\hexfile\C_HexFile.hpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\libs\opensyde_core\kefex_diaglib\dl_kefex\CKFXVariableBase.hpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\libs\opensyde_core\kefex_diaglib\dl_stwflash\CXFLActions.hpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\libs\opensyde_core\kefex_diaglib\dl_stwflash\CXFLHexFile.hpp",
    r"c:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\libs\opensyde_core\kefex_diaglib\dl_stwflash\CXFLProtocol.hpp",
]

replacements = [
    (r'#include\s+"C_SclDynamicArray\.hpp"', '#include <QList>'),
    (r'class\s+C_SclDynamicArray;', ''),
    (r'template\s*<\s*class\s+T\s*>\s*class\s+C_SclDynamicArray\s*;', ''),
]

for filepath in files_to_update:
    if not os.path.exists(filepath):
        print(f"File not found: {filepath}")
        continue
        
    print(f"Processing: {filepath}")
    with open(filepath, "r", encoding="utf-8", errors="ignore") as f:
        content = f.read()
    
    new_content = content
    for pattern, substitution in replacements:
        new_content = re.sub(pattern, substitution, new_content)
        
    if new_content != content:
        with open(filepath, "w", encoding="utf-8") as f:
            f.write(new_content)
        print(f"  Updated: {filepath}")
    else:
        print(f"  No changes needed: {filepath}")
