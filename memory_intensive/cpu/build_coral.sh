g++ main.cpp -o cpu_mem_workload \
  -I/opt/glibc-2.31/include \
  -L/opt/glibc-2.31/lib \
  -Wl,--rpath=/opt/glibc-2.31/lib \
  -lpthread

