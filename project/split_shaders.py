
import os

with open("shaders.glsl", "r") as f:
    content = f.read()

parts = content.split("// <!-- split -->")

with open("vshader.glsl", "w") as f:
    f.write(parts[0].strip())

with open("fshader.glsl", "w") as f:
    f.write(parts[1].strip())
    
print("Shaders split successfully.")
