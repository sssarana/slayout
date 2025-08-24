# slayout Usage Guide

This document explains how to write `.slayout` files using the `slayout` DSL. These files define macros, control layout generation per backend, and allow conditional logic and string manipulation for shader preprocessing.

## File Extensions

- Use `.slayout` as the extension for layout scripts.
- Use `.shader` (or any shader source file) to write shaders containing `%MACRO` placeholders.

## Table of Contents

1. Macro Definition
2. Setting Backend-Specific Layouts
3. Lazy Mode
4. Default Fallback
5. Reading from File
6. Booleans and Conditionals
7. String Variables and Concatenation
8. System Commands
9. Macro Usage in Shader Files
10. Running in command line

## Macro Definition

Define a macro that will be replaced in shader code.

```slayout
macrodef myLayout = Macro("MY_MACRO_NAME");
```

- `myLayout` is the variable you will refer to in `.slayout` code.
- `"MY_MACRO_NAME"` is the name you will use in the shader: `%MY_MACRO_NAME`.

## Setting Backend-Specific Layouts

Define layout code that will be used when generating shaders for specific backends.

```slayout
myLayout.set_glsl("layout(std140, binding = 0) uniform Matrices { mat4 MVP; };");
myLayout.set_hlsl("cbuffer Matrices : register(b0) { float4x4 MVP; };");
myLayout.set_msl("constant Matrices& MVP [[buffer(0)]];");
myLayout.set_spirv("layout(set = 0, binding = 0) uniform Matrices { mat4 MVP; };");
```

Each `set_*` method tells the compiler what code to inject into shaders that use `%MY_MACRO_NAME` for that backend.

## Lazy Mode

Enable fallback to raw macro name if no backend/default code is provided:

```slayout
myLayout.lazy();
```

If `lazy()` is enabled and the macro is used in a shader without a matching backend-defined layout, `%MY_MACRO_NAME` will be replaced with `MY_MACRO_NAME`.

## Default Fallback

Provide a layout string to use when the target backend does not have a `set_*` mapping:

```slayout
myLayout.set_default("uniform Matrices { mat4 MVP; };");
```

This will be used for any backend that does not have an explicit definition.

## Reading from File

You can also read layout content from external files:

```slayout
myLayout.read_glsl("layouts/glsl_layout.txt");
myLayout.read_default("layouts/fallback.txt");
```

Equivalent to using `set_glsl()` or `set_default()` with a string loaded from a file.

## Booleans and Conditionals

### Declare a boolean:

```slayout
boolean useLighting = TRUE;
```

### Use in a conditional block:

```slayout
if (useLighting) {
    myLayout.set_glsl("...");
    print SYSTEM->list_backends();
}
```

Only runs if the boolean is `TRUE`. Nesting and multiple statements are allowed.

## String Variables and Concatenation

### Declare a string:

```slayout
string base = "layout(std140, binding = 0)";
string add = " uniform Matrices { mat4 MVP; };";
```

### Use in set functions:

```slayout
myLayout.set_glsl(base + add);
```

You can also set a macro layout directly to a string variable:

```slayout
myLayout.set_hlsl(base);
```

Concatenation with `+` is supported only between string variables for now.

## System Commands

### Generate for all backends:

```slayout
SYSTEM->generate_all();
```

Generates shaders for all known backends: `glsl`, `hlsl`, `msl`, and `spirv`.

### Generate only for selected backends:

```slayout
SYSTEM->generate_select(GLSL, HLSL);
```

You may select any combination of:
- `GLSL`
- `HLSL`
- `MSL`
- `SPIRV`

> Note: These must be written in uppercase.

### Print Information:

```slayout
print SYSTEM->list_backends();
```

Prints a list of all supported backends.

## Macro Usage in Shader Files

To use the macro in a shader file, prefix it with a `%` sign:

```glsl
%MY_MACRO_NAME

layout(location = 0) in vec3 position;

void main() {
    gl_Position = MVP * vec4(position, 1.0);
}
```

During compilation, `%MY_MACRO_NAME` will be replaced with the backend-specific layout defined in your `.slayout` file.

## Running in command line
```sh
./build/bin/slayoutc layout.slayout input.shader shaders
```
Where `layout.slayout` is your layout file, `input.shader` is your shader using macros, and `shaders` is the output directory.

## Notes

- All statements must end with semicolons `;`.
- Blocks (`if`, etc.) must be enclosed in braces `{ }`.
- String values must be quoted.
- Only booleans and strings are supported as variable types for now.
