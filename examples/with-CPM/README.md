# intervals example project (CPM)


This is a simple CMake project that demonstrates how to use the [*intervals* library](https://github.com/mbeutel/intervals)
with the [CPM](https://github.com/cpm-cmake/CPM.cmake) package manager.

Requirements:

- [CMake](https://cmake.org/) 3.30 or newer must be in the path.
- A C++ compiler must be installed and available in the path.
- An internet connection must be available to allow CPM to retrieve the dependencies
  ([*gsl-lite*](https://github.com/gsl-lite/gsl-lite) and [*makeshift*](https://github.com/mbeutel/makeshift)) from GitHub.

To set up the project, open a command-line window, navigate to the directory containing this readme, and execute the following commands:


## Configure
```
cmake --preset default
```


## Build
```
cmake --build build/default --config Debug
```

## Run

| Windows                          | Linux, MacOS                       |
|----------------------------------|------------------------------------|
| `build\default\Debug\my-program` | `./build/default/Debug/my-program` |
