# Windows Build Guide.
 
> [!IMPORTANT]
> ```Requirements:```
> - 0.1. Install Clion 2020.1 or newer / Visual Studio 2019 or newer.
> - 0.2. Install [Vulkan SDK/Runtime](https://vulkan.lunarg.com/sdk/home) 1.3.211.0 or newer.
> - 0.3. Install CMake 3.16.0 or newer.
> - 0.4. Install [MS Build Tools](https://download.visualstudio.microsoft.com/download/pr/996d318f-4bd0-4f73-8554-ea3ed556ce9b/9610a60ad452c33dd3f9e8d3b4ce5d88d278f8b063d88717e08e1c0c13c29233/vs_BuildTools.exe) for Visual Studio 16.11.29 or newer (if you are using CLion).
> - 0.5. Make sure you have Python 3 installed and added to environment variables.
1. ```Clone repository via the command "git clone https://github.com/SpaRcle-Studio/SREngine"```
2. ```Switch to the "dev" (or another one) branch via the command "git checkout branch_name"```
3. ```Run command "git submodule update --init --recursive" in repository folder```
 <details>
 <summary> 4.1 Working with CMake GUI (Visual Studio). </summary>

    - 4.1.1. Open CMake GUI.
  
    - 4.1.2. Choose the path to the SREngine folder (where the source code is located).
  
    - 4.1.3. Choose the path where to build the binaries (ex. SREngine/build).
     
    - 4.1.4. Press "Configure" and choose your VS version.
    
    - 4.1.5. Press "Generate".

    - 4.1.6. Open Visual Studio solution.

    - 4.1.7. In Visual Studio right-click on SREngine and press "Set as Startup Project".

    - 4.1.8. Run build.

 </details>
 
 <details>
 <summary> 4.2 Working with CMake in CMD (Visual Studio). </summary>

    - 4.2.1. Open CMD in root directory of the repository and run the following commands:
  
    - 4.2.2. mkdir build
  
    - 4.2.3. cmake -G "VISUAL_STUDIO_VERSION" ../ -DCMAKE_BUILD_TYPE=Debug

    - 4.2.3* Replace VISUAL_STUDIO_VERSION with your version ex. "Visual Studio 17 2022" 
     
    - 4.2.4. Now you have your solution generated and you can open it with Visual Studio
    
    - 4.2.5. In Visual Studio right-click on SREngine and press "Set as Startup Project"

    - 4.2.6. Build and Run the game engine!

 </details>
 
 <details>
 <summary> 5. Working with CLion. </summary>

    - 5.1. Press Ctrl + Alt + S to open Settings window.
    
    - 5.2. Select "Build, Execution, Deployment" on the left.
    
    - 5.3. Select "Toolchains".
    
    - 5.4. Add (Alt + Insert) Visual Studio compiler.
    
    - 5.5. Select amd64 in "Architecture:".
    
    - 5.6. Repeat 5.2.
    
    - 5.7. Select "CMake".
    
    - 5.8. Pass "-j*" argument as "Build options", while * is a number of your logical processors minus 2.
    
    - 5.9. In Project window right-click SREngine folder and click "Reload CMake Project".
    
    - 5.10. Click "Run 'SREngine'" (Shift + F10).
 </details>
