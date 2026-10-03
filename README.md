# band-hero

## Python stem splitter

From `tools/song_processor/python/`, install into an activated Python virtual
environment:

```sh
python -m pip install .
python -m stem_splitter --name "Bored"
```

On Windows and Linux x86-64 with standard CPython 3.10–3.14, this installs
PyTorch 2.11.0 with CUDA 12.8 directly from the official PyTorch wheel host.
Linux requires glibc 2.28 or newer. Free-threaded Python builds are not supported
by these wheel references. Other platforms and Python versions use the default
PyTorch dependency, subject to upstream wheel availability.
The splitter automatically uses CUDA when available and otherwise uses the CPU.
CUDA execution requires a compatible NVIDIA GPU and driver; no separate CUDA
Toolkit installation is needed. The CUDA-enabled build is a larger download
even on machines that will run on CPU.

