#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>

typedef void (*entry_fn)();

int main(int argc, char **argv) {
    const char *lib_path = argv[1];

    void *handle = dlopen(lib_path, RTLD_NOW);
    if (!handle) {
        fprintf(stderr, "dlopen failed: %s\n", dlerror());
        return 1;
    }

    dlerror();

    entry_fn entry = (entry_fn)dlsym(handle, "__entry");

    char *error = dlerror();
    if (error != NULL) {
        fprintf(stderr, "dlsym failed: %s\n", error);
        dlclose(handle);
        return 1;
    }

    entry();
    dlclose(handle);

    return 0;
}