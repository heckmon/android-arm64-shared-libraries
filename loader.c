#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/stat.h>
#include <pwd.h>

#define RUNTIME_DIR "/data/data/com.vsdroid/runtimes"
#define BIN_DIR "/data/data/com.vsdroid/bin"

void not_installed(char* package){
    char message[512];
    snprintf(message, sizeof(message), "%s is not installed. Go to the download page and install it first\n\nOr install it by running: vsd install %s", package, package);
    fprintf(stderr, "%s\n", message);
}

int run_python(char *argv[]){
    struct stat st;
    char pythonDir[512];
    snprintf(pythonDir, sizeof(pythonDir), "%s/python", RUNTIME_DIR);
    if (stat(pythonDir, &st) != 0 || !S_ISDIR(st.st_mode)) {
        not_installed("Python");
        return 1;
    }
    char ld_library_path[512];
    snprintf(ld_library_path, sizeof(ld_library_path), "%s/python/lib", RUNTIME_DIR);
    setenv("LD_LIBRARY_PATH", ld_library_path, 1);

    char pythonhome[512];
    snprintf(pythonhome, sizeof(pythonhome), "%s/python", RUNTIME_DIR);
    setenv("PYTHONHOME", pythonhome, 1);

    char path[1024];
    const char *old_path = getenv("PATH");
    snprintf(path, sizeof(path), "%s/python/bin:%s", RUNTIME_DIR, old_path ? old_path : "");
    setenv("PATH", path, 1);

    const char *vsdroid_shared_path = getenv("VSDROID_SHARED_PATH");
    if (!vsdroid_shared_path) {
        fprintf(stderr, "VSDROID_SHARED_PATH is not set.\n");
        return 1;
    }

    char launcher_path[1024];
    snprintf(launcher_path, sizeof(launcher_path), "%s/libpythonlauncher.so", vsdroid_shared_path);

    execv(launcher_path, argv);

    perror("execv failed");
    return 1;
}

int run_node(char *argv[]) {
    const char *vsdroid_shared_path = getenv("VSDROID_SHARED_PATH");
    if (!vsdroid_shared_path) {
        fprintf(stderr, "VSDROID_SHARED_PATH is not set");
        return 1;
    }

    char nodeDir[256];
    snprintf(nodeDir, sizeof(nodeDir), "%s/node", RUNTIME_DIR);

    struct stat st;
    if (stat(nodeDir, &st) != 0 || !S_ISDIR(st.st_mode)) {
        not_installed("Node.js");
        return 1;
    }

    char ld_library_path[512];
    snprintf(ld_library_path, sizeof(ld_library_path), "%s/node/lib:%s", RUNTIME_DIR, vsdroid_shared_path);
    setenv("LD_LIBRARY_PATH", ld_library_path, 1);

    char node_options[1024];
    snprintf(node_options, sizeof(node_options), "--require %s/node/error_handler.js", RUNTIME_DIR);
    setenv("NODE_OPTIONS", node_options, 1);

    char launcher_path[1024];
    snprintf(launcher_path, sizeof(launcher_path), "%s/libnodelauncher.so", vsdroid_shared_path);

    execv(launcher_path, argv);

    perror("execv failed");
    return 1;

}

int run_clang(int argc, char *argv[]) {
    char clangDir[256];
    snprintf(clangDir, sizeof(clangDir), "%s/clang", RUNTIME_DIR);

    const char *vsdroid_shared_path = getenv("VSDROID_SHARED_PATH");
    if (!vsdroid_shared_path) {
        fprintf(stderr, "VSDROID_SHARED_PATH is not set.\n");
        return 1;
    }

    struct stat st;
    if (stat(clangDir, &st) != 0 || !S_ISDIR(st.st_mode)) {
        not_installed("Clang");
        return 1;
    }

    char link[1024], target[1024];
    snprintf(link, sizeof(link), "%s/clang/ld.lld", RUNTIME_DIR);
    snprintf(target, sizeof(target), "%s/liblld.so", vsdroid_shared_path);

    remove(link);

    if(symlink(target, link) == -1){
        perror("symlink");
        return 1;
    }

    char new_path[1024];
    const char *old_path = getenv("PATH");
    snprintf(new_path, sizeof(new_path), "%s/clang:%s", RUNTIME_DIR, old_path ? old_path : "");
    setenv("PATH", new_path, 1);

    char ld_library_path[512];
    snprintf(ld_library_path, sizeof(ld_library_path), "%s/clang", RUNTIME_DIR);
    setenv("LD_LIBRARY_PATH", ld_library_path, 1);

    char cIncludePath[512];
    snprintf(cIncludePath, sizeof(cIncludePath), "%s/clang/sysroot/usr/include:%s/clang/lib/clang/21/include", RUNTIME_DIR, RUNTIME_DIR);
    setenv("C_INCLUDE_PATH", cIncludePath, 1);
    
    char cPlusIncludePath[512];
    snprintf(cPlusIncludePath, sizeof(cPlusIncludePath), "%s/clang/sysroot/usr/include:%s/clang/lib/clang/21/include", RUNTIME_DIR, RUNTIME_DIR);
    setenv("CPLUS_INCLUDE_PATH", cPlusIncludePath, 1);

    char launcher_path[1024];
    snprintf(launcher_path, sizeof(launcher_path), "%s/libclang-21.so", vsdroid_shared_path);
    
    int arg_count = argc + 4;
    char** args = (char**)malloc((arg_count + 1) * sizeof(char*));
    
    args[arg_count] = NULL;
    
    args[0] = strdup(launcher_path);
    args[1] = "-fuse-ld=lld";
    char arg2[512], arg3[512], arg4[512];
    snprintf(arg2, sizeof(arg2), "-L%s/clang/lib/clang/21/lib/linux", RUNTIME_DIR);
    snprintf(arg3, sizeof(arg3), "-B%s/clang/lib/clang/21/lib/linux", RUNTIME_DIR);
    snprintf(arg4, sizeof(arg4), "-resource-dir=%s/clang/lib/clang/21", RUNTIME_DIR);

    args[2] = strdup(arg2);
    args[3] = strdup(arg3);
    args[4] = strdup(arg4);
    
    for(int i=1; i < argc; i++){
        args[4 + i] = argv[i];
    }

    execv(launcher_path, args);

    perror("execv failed");

    for (int i = 0; i < arg_count; i++) {
        free(args[i]);
    }
    free(args);

    return 1;
}

int run_clang_plus_plus(int argc, char *argv[]) {
    char clangDir[256];
    snprintf(clangDir, sizeof(clangDir), "%s/clang", RUNTIME_DIR);

    const char *vsdroid_shared_path = getenv("VSDROID_SHARED_PATH");
    if (!vsdroid_shared_path) {
        fprintf(stderr, "VSDROID_SHARED_PATH is not set.\n");
        return 1;
    }

    struct stat st;
    if (stat(clangDir, &st) != 0 || !S_ISDIR(st.st_mode)) {
        not_installed("Clang++");
        return 1;
    }

    int is_help_request = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0 ||
            strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            is_help_request = 1;
            break;
        }
    }

    if (!is_help_request) {
        int has_input_files = 0;
        for (int i = 1; i < argc; i++) {
            if (argv[i][0] != '-' || strcmp(argv[i], "-") == 0) {
                has_input_files = 1;
                break;
            }
        }

        if (!has_input_files) {
            fprintf(stderr, "error: no input files\n");
            fprintf(stderr, "Usage: clang++ [options] file...\n");
            return 1;
        }
    }

    char link[1024], target[1024];
    snprintf(link, sizeof(link), "%s/clang/ld.lld", RUNTIME_DIR);
    snprintf(target, sizeof(target), "%s/liblld.so", vsdroid_shared_path);

    remove(link);
    if (symlink(target, link) == -1) {
        perror("symlink");
    }

    char new_path[1024];
    const char *old_path = getenv("PATH");
    snprintf(new_path, sizeof(new_path), "%s/clang:%s", RUNTIME_DIR, old_path ? old_path : "");
    setenv("PATH", new_path, 1);

    char ld_library_path[1024];
    snprintf(ld_library_path, sizeof(ld_library_path),"%s/clang:%s/clang/lib/clang/21/lib/linux", RUNTIME_DIR, RUNTIME_DIR);
    setenv("LD_LIBRARY_PATH", ld_library_path, 1);

    char c_include_path[1024];
    snprintf(c_include_path, sizeof(c_include_path),"%s/clang/sysroot/usr/include:%s/clang/lib/clang/21/include",RUNTIME_DIR, RUNTIME_DIR);
    setenv("C_INCLUDE_PATH", c_include_path, 1);

    char cplus_include_path[1024];
    snprintf(
        cplus_include_path,
        sizeof(cplus_include_path),
        "%s/clang/sysroot/usr/include/c++/v1:%s/clang/sysroot/usr/include:%s/clang/lib/clang/21/include",
        RUNTIME_DIR, RUNTIME_DIR, RUNTIME_DIR
    );
    setenv("CPLUS_INCLUDE_PATH", cplus_include_path, 1);

    char launcher_path[1024];
    snprintf(launcher_path, sizeof(launcher_path), "%s/libclang-21.so", vsdroid_shared_path);

    int total_args = argc + 9;
    char **args = (char **)malloc((total_args + 1) * sizeof(char *));

    args[0] = strdup(launcher_path);
    args[1] = strdup("-fuse-ld=lld");
    args[2] = strdup("-x");
    args[3] = strdup("c++");
    args[4] = strdup("-std=c++20");
    args[5] = strdup("-stdlib=libc++");
    args[6] = strdup("-lc++");

    char arg7[512], arg8[512];
    snprintf(arg7, sizeof(arg7), "-L%s/clang/lib/clang/21/lib/linux", RUNTIME_DIR);
    snprintf(arg8, sizeof(arg8), "-B%s/clang/lib/clang/21/lib/linux", RUNTIME_DIR);
    args[7] = strdup(arg7);
    args[8] = strdup(arg8);

    char arg9[512];
    snprintf(arg9, sizeof(arg9), "-resource-dir=%s/clang/lib/clang/21", RUNTIME_DIR);
    args[9] = strdup(arg9);

    for (int i = 1; i < argc; i++) {
        args[9 + i] = argv[i];
    }
    args[total_args] = NULL;

    execv(launcher_path, args);

    perror("execv failed");

    for (int i = 0; i < total_args; i++) {
        free(args[i]);
    }
    free(args);

    return 1;
}

int run_clang_loader(char *argv[]) {
    char clangDir[256];
    snprintf(clangDir, sizeof(clangDir), "%s/clang", RUNTIME_DIR);

    const char *vsdroid_shared_path = getenv("VSDROID_SHARED_PATH");
    if (!vsdroid_shared_path) {
        fprintf(stderr, "VSDROID_SHARED_PATH is not set.\n");
        return 1;
    }

    struct stat st;
    if (stat(clangDir, &st) != 0 || !S_ISDIR(st.st_mode)) {
        not_installed("Clang");
        return 1;
    }

    char ld_library_path[1024];
    snprintf(ld_library_path, sizeof(ld_library_path),"%s/clang/lib/clang/21/lib/linux:%s/clang", RUNTIME_DIR, RUNTIME_DIR);
    setenv("LD_LIBRARY_PATH", ld_library_path, 1);

    char clang_loader_path[1024];
    snprintf(clang_loader_path, sizeof(clang_loader_path), "%s/libclangloader.so", vsdroid_shared_path);

    execv(clang_loader_path, argv);
    return 1;
}

int run_npm_or_npx(char *argv[], int argc, int isNpx) {
    const char *vsdroid_shared_path = getenv("VSDROID_SHARED_PATH");
    char npmPrefix[512];
    snprintf(npmPrefix, sizeof(npmPrefix), "%s/node", RUNTIME_DIR);

    const char *homeDir = getenv("HOME");
    if (!homeDir) {
        struct passwd *pw = getpwuid(getuid());
        homeDir = pw ? pw->pw_dir : NULL;
    }

    if (!homeDir) {
        fprintf(stderr, "Could not determine home directory.\n");
        return 1;
    }

    char npmrcPath[1024];
    snprintf(npmrcPath, sizeof(npmrcPath), "%s/.npmrc", homeDir);

    FILE *f = fopen(npmrcPath, "w");
    if (!f) {
        perror("Failed to write ~/.npmrc");
        return 1;
    }
    fprintf(f, "prefix=%s\n", npmPrefix);
    fclose(f);

    setenv("NODE_OPTIONS", "--dns-result-order=ipv4first", 1);

    char nodePath[512];
    snprintf(nodePath, sizeof(nodePath), "%s/node", BIN_DIR);

    char npmCliPath[512];
    snprintf(npmCliPath, sizeof(npmCliPath),"%s/node/lib/node_modules/npm/bin/npm-cli.js", RUNTIME_DIR);

    char npxCliPath[512];
    snprintf(npxCliPath, sizeof(npxCliPath),"%s/node/lib/node_modules/npm/bin/npx-cli.js", RUNTIME_DIR);

    int total_args = argc + 1;
    char **args = (char **)malloc((total_args + 1) * sizeof(char *));

    args[0] = strdup(nodePath);
    args[1] = isNpx ? strdup(npxCliPath) : strdup(npmCliPath);

    for (int i = 1; i < argc; i++) {
        args[1 + i] = argv[i];
    }
    args[total_args] = NULL;

    execv(nodePath, args);

    perror("execv failed");

    for (int i = 0; i < total_args; i++) {
        free(args[i]);
    }
    free(args);

    return 1;
}

int run_kotlin(int argc, char *argv[]) {
    const char *runtimeDir = "/data/data/com.vsdroid/runtimes";
    const char *javaDir = "java-17-openjdk";
    const char *kotlinDir = "kotlin";

    char fullKotlinDir[512];
    snprintf(fullKotlinDir, sizeof(fullKotlinDir), "%s/%s", runtimeDir, kotlinDir);

    char fullJavaDir[512];
    snprintf(fullJavaDir, sizeof(fullJavaDir), "%s/%s", runtimeDir, javaDir);

    struct stat st;
    if (stat(fullKotlinDir, &st) != 0 || !S_ISDIR(st.st_mode)) {
        not_installed("Kotlin");
        return 1;
    }

    char tmpDir[1024];
    snprintf(tmpDir, sizeof(tmpDir), "%s/tmp", fullKotlinDir);

    if (stat(tmpDir, &st) != 0) {
        if (mkdir(tmpDir, 0755) == -1) {
            perror("Failed to create tmp directory");
            return 1;
        }
    }

    char ld_path[1024];
    snprintf(ld_path, sizeof(ld_path), "%s/lib", fullJavaDir);
    const char *old_ld = getenv("LD_LIBRARY_PATH");
    char combined_ld[2048];
    snprintf(combined_ld, sizeof(combined_ld), "%s:%s", ld_path, old_ld ? old_ld : "");
    setenv("LD_LIBRARY_PATH", combined_ld, 1);

    setenv("JAVA_HOME", fullJavaDir, 1);

    const char *old_opts = getenv("JAVA_OPTS");
    char new_opts[1024];
    snprintf(new_opts, sizeof(new_opts), "%s %s", "-Djansi.passthrough=true -Djansi.force=false", old_opts ? old_opts : "");
    setenv("JAVA_OPTS", new_opts, 1);

    setenv("TMPDIR", tmpDir, 1);

    const char *old_path = getenv("PATH");
    char new_path[2048];
    snprintf(new_path, sizeof(new_path), "%s:%s", BIN_DIR, old_path ? old_path : "");
    setenv("PATH", new_path, 1);

    printf("Compiling...\n");

    char tmpdir_flag[1024];
    snprintf(tmpdir_flag, sizeof(tmpdir_flag), "-Djava.io.tmpdir=%s", tmpDir);

    char classpath[1024];
    snprintf(classpath, sizeof(classpath), "%s/lib/*", fullKotlinDir);

    int extra_args = 9;
    int total_args = argc + extra_args;
    char **args = (char **)malloc((total_args + 1) * sizeof(char *));

    args[0] = strdup("java");
    args[1] = strdup("-Djansi.passthrough=true");
    args[2] = strdup("-Djansi.strip=true");
    args[3] = strdup("-Dorg.fusesource.jansi.AnsiConsole=false");
    args[4] = strdup(tmpdir_flag);
    args[5] = strdup("-cp");
    args[6] = strdup(classpath);
    args[7] = strdup("org.jetbrains.kotlin.cli.jvm.K2JVMCompiler");

    for (int i = 1; i < argc; i++) {
        args[7 + i] = argv[i];
    }

    args[total_args] = NULL;

    execvp("java", args);

    perror("execvp failed");

    for (int i = 0; i < total_args; i++) {
        free(args[i]);
    }
    free(args);

    return 1;
}

int run_java_tool(const char *tool, char *argv[]) {
    const char *javaDir = "java-21-openjdk";

    char fullJavaDir[512];
    snprintf(fullJavaDir, sizeof(fullJavaDir), "%s/%s", RUNTIME_DIR, javaDir);

    struct stat st;
    if (stat(fullJavaDir, &st) != 0 || !S_ISDIR(st.st_mode)) {
        fprintf(stderr, "OpenJDK is not installed. Go to the download page and install it first.\n");
        return 1;
    }

    const char *vsdroid_shared_path = getenv("VSDROID_SHARED_PATH");
    if (!vsdroid_shared_path) {
        fprintf(stderr, "VSDROID_SHARED_PATH is not set.\n");
        return 1;
    }

    char ld_path[1024];
    snprintf(ld_path, sizeof(ld_path), "%s/lib", fullJavaDir);
    const char *old_ld = getenv("LD_LIBRARY_PATH");
    char combined_ld[2048];
    snprintf(combined_ld, sizeof(combined_ld), "%s:%s", ld_path, old_ld ? old_ld : "");
    setenv("LD_LIBRARY_PATH", combined_ld, 1);
    setenv("JAVA_HOME", fullJavaDir, 1);

    char toolPath[1024];
    snprintf(toolPath, sizeof(toolPath), "%s/lib%s.so", vsdroid_shared_path, tool);

    execv(toolPath, argv);

    perror("execv failed");
    return 1;
}

int run_pip(char *argv[], int argc) {
    char pythonDir[512];
    snprintf(pythonDir, sizeof(pythonDir), "%s/python", RUNTIME_DIR);

    const char *vsdroid_shared_path = getenv("VSDROID_SHARED_PATH");
    if (!vsdroid_shared_path) {
        fprintf(stderr, "VSDROID_SHARED_PATH is not set.\n");
        return 1;
    }

    char pip3Path[1024];
    snprintf(pip3Path, sizeof(pip3Path), "%s/bin/pip3", pythonDir);

    struct stat st;
    if (stat(pip3Path, &st) != 0) {
        printf("Installing pip...\n");

        char ld_path[1024];
        snprintf(ld_path, sizeof(ld_path), "%s/lib", pythonDir);
        setenv("LD_LIBRARY_PATH", ld_path, 1);
        setenv("PYTHONHOME", pythonDir, 1);
        char bin_path[1024];
        snprintf(bin_path, sizeof(bin_path), "%s/bin", pythonDir);
        setenv("PATH", bin_path, 1);

        char launcherPath[1024];
        snprintf(launcherPath, sizeof(launcherPath), "%s/python", BIN_DIR);

        char *installArgs[] = {strdup(launcherPath), "-m", "ensurepip", NULL};
        execv(launcherPath, installArgs);

        perror("execv failed during ensurepip");
        free(installArgs[0]);
        return 1;
    }

    char ld_path[1024];
    snprintf(ld_path, sizeof(ld_path), "%s/lib", pythonDir);
    setenv("LD_LIBRARY_PATH", ld_path, 1);
    setenv("PYTHONHOME", pythonDir, 1);
    char bin_path[1024];
    snprintf(bin_path, sizeof(bin_path), "%s/bin", pythonDir);
    setenv("PATH", bin_path, 1);

    char pythonExec[1024];
    snprintf(pythonExec, sizeof(pythonExec), "%s/python", BIN_DIR);

    int totalArgs = argc + 2;
    char **args = (char **)malloc((totalArgs + 1) * sizeof(char *));

    args[0] = strdup(pythonExec);
    args[1] = strdup("-m");
    args[2] = strdup("pip");

    for (int i = 1; i < argc; i++) {
        args[2 + i] = argv[i];
    }
    args[totalArgs] = NULL;

    execv(pythonExec, args);

    perror("execv failed during pip");

    for (int i = 0; i < totalArgs; i++) {
        free(args[i]);
    }
    free(args);

    return 1;
}

int run_tsc(int argc, char *argv[]) {
    const char *tsDir = "/data/data/com.vsdroid/runtimes/node/lib/node_modules";
    const char *binPath = "/data/data/com.vsdroid/bin";
    char tscPath[512];
    snprintf(tscPath, sizeof(tscPath), "%s/typescript/lib/tsc.js", tsDir);

    char nodePath[512];
    snprintf(nodePath, sizeof(nodePath), "%s/node", binPath);

    setenv("NODE_OPTIONS", "--dns-result-order=ipv4first", 1);

    int total_args = argc + 1;
    char **args = (char **)malloc((total_args + 1) * sizeof(char *));

    args[0] = strdup(nodePath);
    args[1] = strdup(tscPath);

    for (int i = 1; i < argc; i++) {
        args[1 + i] = argv[i];
    }

    args[total_args] = NULL;
    execv(nodePath, args);
    perror("execv failed");

    for (int i = 0; i < total_args; i++) {
        free(args[i]);
    }
    free(args);

    return 1;
}

int run_ruby(char *argv[]) {
    const char *vsdroid_shared_path = getenv("VSDROID_SHARED_PATH");
    if (!vsdroid_shared_path) {
        fprintf(stderr, "VSDROID_SHARED_PATH is not set.\n");
        return 1;
    }

    char rubyDir[256];
    snprintf(rubyDir, sizeof(rubyDir), "%s/ruby", RUNTIME_DIR);

    struct stat st;
    if (stat(rubyDir, &st) != 0 || !S_ISDIR(st.st_mode)) {
        not_installed("Ruby");
        return 1;
    }

    char ld_library_path[512];
    snprintf(ld_library_path, sizeof(ld_library_path), "%s/ruby:%s", RUNTIME_DIR, vsdroid_shared_path);
    setenv("LD_LIBRARY_PATH", ld_library_path, 1);

    char ruby_lib[512];
    snprintf(ruby_lib, sizeof(ruby_lib), "%s/ruby/lib/ruby/3.4.0:%s/ruby/lib/ruby/3.4.0/aarch64-linux-android", RUNTIME_DIR, RUNTIME_DIR);
    setenv("RUBYLIB", ruby_lib, 1);

    char gem_path[512];
    snprintf(gem_path, sizeof(gem_path), "%s/ruby/lib/ruby/gems", RUNTIME_DIR);
    setenv("GEM_PATH", gem_path, 1);
    setenv("GEM_HOME", gem_path, 1);

    const char *ruby_launcher = "libruby.so";
    char launcher_path[1024];
    snprintf(launcher_path, sizeof(launcher_path), "%s/%s", vsdroid_shared_path, ruby_launcher);

    execv(launcher_path, argv);
    perror("execv failed");
    return 1;
}

int run_mono(char *argv[]) {
    const char *vsdroid_shared_path = getenv("VSDROID_SHARED_PATH");
    if (!vsdroid_shared_path) {
        fprintf(stderr, "VSDROID_SHARED_PATH is not set.\n");
        return 1;
    }

    char monoDir[256];
    snprintf(monoDir, sizeof(monoDir), "%s/mono", RUNTIME_DIR);

    struct stat st;
    if (stat(monoDir, &st) != 0 || !S_ISDIR(st.st_mode)) {
        not_installed("Mono");
        return 1;
    }

    char mono_path[512];
    snprintf(mono_path, sizeof(mono_path), "%s/mono/mono/4.5", RUNTIME_DIR);
    setenv("MONO_PATH", mono_path, 1);

    const char *mono_launcher = "libmono.so";
    char launcher_path[1024];
    snprintf(launcher_path, sizeof(launcher_path), "%s/%s", vsdroid_shared_path, mono_launcher);

    execv(launcher_path, argv);
    perror("execv failed");
    return 1;
}

int run_csc(int argc, char *argv[]) {
    char cscPath[512];
    snprintf(cscPath, sizeof(cscPath), "%s/mono/mono/4.5/csc.exe", RUNTIME_DIR);

    char **args = malloc((argc + 3) * sizeof(char*));
    if (!args) { perror("malloc"); exit(1); }
    
    args[0] = strdup("mono");
    args[1] = strdup("--gc-params=nursery-size=64m");
    args[2] = strdup(cscPath);
    
    for (int i = 1; i < argc; i++) {
        args[i + 2] = strdup(argv[i]);
    }
    
    args[argc + 2] = NULL;
    
    execvp("mono", args);
    perror("execvp failed");

    for (int i = 0; i < argc + 1; i++) {
        free(args[i]);
    }
    free(args);
    return 1;
}

int run_git(char *argv[]){
    const char *vsdroid_shared_path = getenv("VSDROID_SHARED_PATH");
    if (!vsdroid_shared_path) {
        fprintf(stderr, "VSDROID_SHARED_PATH is not set.\n");
        return 1;
    }
    char launcher_path[1024];
    snprintf(launcher_path, sizeof(launcher_path), "%s/libgit.so", vsdroid_shared_path);

    execv(launcher_path, argv);
    return 1;
}

int main(int argc, char *argv[]) {
    char *toolName = basename(argv[0]);

    const char* javaTools[]= {
        "jar",
        "jarsigner",
        "java",
        "javac",
        "javadoc",
        "javap",
        "jcmd",
        "jconsole",
        "jdb",
        "jdeprscan",
        "jdeps",
        "jfr",
        "jhsdb",
        "jimage",
        "jinfo",
        "jlink",
        "jmap",
        "jmod",
        "jpackage",
        "jps",
        "jrunscript",
        "jshell",
        "jstack",
        "jstat",
        "jstatd",
        "jwebserver",
        "keytool",
        "rmiregistry",
        "serialver"
    };

    if (strcmp(toolName, "node") == 0) {
        run_node(argv);
    } else if (strcmp(toolName, "tsc") == 0) {
        run_tsc(argc, argv);
    } else if (strcmp(toolName, "npm") == 0) {
        run_npm_or_npx(argv, argc, 0);
    } else if (strcmp(toolName, "npx") == 0) {
        run_npm_or_npx(argv, argc, 1);
    } else if (strcmp(toolName, "pip") == 0 || strcmp(toolName, "pip3") == 0) {
        run_pip(argv, argc);
    } else if (strcmp(toolName, "python") == 0 || strcmp(toolName, "python3") == 0) {
        run_python(argv);
    } else if (strcmp(toolName, "kotlinc") == 0) {
        run_kotlin(argc, argv);
    } else if (strcmp(toolName, "ruby") == 0) {
        run_ruby(argv);
    } else if (strcmp(toolName, "clang") == 0) {
        run_clang(argc, argv);
    } else if (strcmp(toolName, "clang++") == 0) {
        run_clang_plus_plus(argc, argv);
    } else if (strcmp(toolName, "clangloader") == 0) {
        run_clang_loader(argv);
    } else if (strcmp(toolName, "mono") == 0) {
        run_mono(argv);
    } else if (strcmp(toolName, "csc") == 0) {
        run_csc(argc, argv);
    } else if (strcmp(toolName, "git") == 0) {
        run_git(argv);
    } else {
        for(int i = 0; i < sizeof(javaTools) / sizeof(javaTools[0]); i++) {
            if (strcmp(toolName, javaTools[i]) == 0) {
                run_java_tool(toolName, argv);
                return 0;
            }
        }
    }
    return 0;
}
