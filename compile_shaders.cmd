
set GLSLANG_VALIDATOR="I:\\build\\glslang\\commit_e56beaee736863ce48455955158f1839e6e4c1a1\\install_release\\bin\\glslangValidator.exe"

%GLSLANG_VALIDATOR% shaders\shader.vert -V -o spv\vert.spv
%GLSLANG_VALIDATOR% shaders\shader.frag -V -o spv\frag.spv
