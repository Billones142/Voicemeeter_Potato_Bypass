import os

scriptDirectory= os.path.dirname(os.path.realpath(__file__));


dllName64= "speedChange_x64.dll";
hOutFileName64= "speedChange_x64";

dllName32= "speedChange_x32.dll";
hOutFileName32= "speedChange_x32";

absoluteDLLx32RelativeFilePath= os.path.join(scriptDirectory, dllName64);

with open(absoluteDLLx32RelativeFilePath, "rb") as f:
    byte_array_64 = f.read();

absoluteDLLx32FilePath= os.path.join(scriptDirectory, hOutFileName32 + ".h");

with open(absoluteDLLx32FilePath, "w") as f:
    f.write("unsigned char embedded_speedChangeDll_x32[] = {\n");
    f.write(", ".join(f"0x{b:02x}" for b in byte_array_64));
    f.write("\n};\n");
    f.write(f"unsigned int embedded_file_len = {len(byte_array_64)};\n");

absoluteDLLx64RelativeFilePath= os.path.join(scriptDirectory, dllName64);

with open(absoluteDLLx64RelativeFilePath, "rb") as f:
    byte_array_32 = f.read();

absoluteDLLx64FilePath= os.path.join(scriptDirectory, hOutFileName64 + ".h");

with open(absoluteDLLx64FilePath, "w") as f:
    f.write("unsigned char embedded_speedChangeDll_x64[] = {\n");
    f.write(", ".join(f"0x{b:02x}" for b in byte_array_32));
    f.write("\n};\n");
    f.write(f"unsigned int embedded_file_len = {len(byte_array_32)};\n");

print(absoluteDLLx32FilePath,"\n", absoluteDLLx64FilePath)