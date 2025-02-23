# Podman Setup For Ubuntu & Fedora


## Notes

- Tested on Ubuntu 24.04 LTS, Fedora 41.
- crun was used as the underlying OCI run-time with podman (default).


## References

1. [Building, running, and managing containers - RedHat](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/9/html/building_running_and_managing_containers/index)
2. [opencontainers/runtime-spec - GitHub](https://github.com/opencontainers/runtime-spec/tree/main)
3. [opencontainers/image-spec - GitHub](https://github.com/opencontainers/image-spec/tree/main)
4. [runc/man - GitHub](https://github.com/opencontainers/runc/tree/main/man)


## Basic Steps

### Install necessary tools

```console
$ sudo apt install podman    # Ubuntu
$ sudo dnf install podman    # Fedora
```

### Create a basic container image based on a Dockerfile

1. Create a Dockerfile with arbitrary name.
2. Fill it with the following contents.

    ```
    # base image
    FROM alpine:latest

    # install packages
    # each RUN command adds a new layer
    # RUN apk add iputils-ping

    # copy stuff from host, if needed
    # COPY /path/on/host/to/copy/from /path/on/target/to/copy/into

    # command to run on container
    CMD ["ping", "localhost"]
    ```

### Build the container image

```console
$ podman build --tag ubuntu:<image-name> -f /path/to/Dockerfile
```

### Run the image in a container

```console
$ podman run --rm <image-name>
```


## Useful Podman Commands

### List currently running containers

```console
$ podman container list
$ podman ps
```

### List containers currently running or previously run

```console
$ podman ps -a
```

### To clean up previously-run container history

```console
$ podman container prune -f
```

### List images

```console
$ podman images
```

### Run a command on a running container instance

```console
$ podman exec -it <container-id> <command>
$ podman exec -it c253d46d79f1 /bin/sh    # example of invoking a shell on a running container
```

### Save a container image locally

```console
$ podman image save <image-repo>:<tag> > /path/to/saved/image
```

### Save a container image locally in OCI Image Specification Format

```console
$ podman save --format oci-archive -o /path/to/image.tar <image-repo:tag>
```

### Load a locally-saved image

```console
$ podman load -i /path/to/saved/image
```

### To run a container image directly with an OCI run-time

1. Save the image as a tarball.

    ```console
    $ podman export $(podman create <container-repo>:<tag>) > /path/to/image/archive.tar
    ```

2. Create the directory structure for an OCI bundle.

    ```console
    $ mkdir -p /path/to/image/bundle/rootfs
    ```

3. Unpack the tarball into the bundle.

    ```console
    $ tar -C /path/to/image/bundle/rootfs -xvf /path/to/image/archive.tar
    ```

4. Prepare the OCI container specification for the bundle.

    ```console
    $ crun spec -b /path/to/image/bundle    # this is a template, modify if needed
    ```

5. Run the OCI bundle in a container.

    ```console
    $ crun run --bundle=/path/to/image/bundle <container-name>
    ```

    For additional details on starting the runtime and container stdio management,
    see: [runc/docs/terminal.md - GitHub](https://github.com/opencontainers/runc/blob/main/docs/terminals.md).
    Both runc and crun have similar functional behavior in this regard.
