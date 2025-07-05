# CD Hook

CD Hook is a hooking library for C/C++
Tests are located in `tests` folder and they also serve as examples

It currently supports:
- Inline hooking
- VMT (Virtual Method Table) hooking

Planning to support:
- Trampoline hooking
- Shadow VMT hooking
- Import Address Table (IAT) hooking

## Building example
```sh
git clone https://github.com/cd-n0/cd_hook.git
cd cd_hook
cmake -B build -G Ninja && cmake --build <build_dir>
```

## Tests
> [!CAUTION]
> VMT hooking tests are unreliable due to the indexes of the methods being tied to the compiler used, and the version of that said compiler when compiling the test. Check the method index of the class with a decompiler like radare2/IDA/Ghidra/Binary Ninja etc. and modify accordingly before opening an issue.
```sh
ctest --test-dir <build_dir>
```

## Usage
Use it as a submodule or copy the header and source to your project.

## Contributing

Pull requests are welcome. For major changes, please open an issue first to
discuss what you would like to change.

Please make sure to update tests as appropriate.
