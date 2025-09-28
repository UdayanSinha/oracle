# Kubernetes Setup (K3s)


## Notes

- Tested on Ubuntu 24.04 LTS.
- Tested using K3s.


## References

1. [Kubernetes Documentation | Kubernetes](https://kubernetes.io/docs/home)
2. [K3s - Lightweight Kubernetes](https://docs.k3s.io)


## Setup Necessary Tools

1. Follow the K3s documentation to install and set it up.
2. To avoid using `sudo` for `k3s kubectl` commands:

    ```console
    sudo cp /etc/rancher/k3s/k3s.yaml /home/${USER}/path/to/copy/to/k3s.yaml
    sudo chown ${USER} /home/${USER}/path/to/copy/to/k3s.yaml
    export KUBECONFIG=/home/${USER}/path/to/copy/to/k3s.yaml
    ```


## Check Node Status

```console
k3s kubectl get nodes
```


## Build Container Image

```console
podman build --tag <image-tag> -f /path/to/Dockerfile
podman image save <image-repo>:<image-tag> > /path/to/saved/image.tar
```


## Import Image Into K3s

```console
sudo k3s ctr image import /path/to/saved/image.tar
sudo k3s ctr image ls | grep <image-tag>   # check that the image was correctly imported
```


## Create Deployment Manifest

```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: test-app
spec:
  replicas: 1
  selector:
    matchLabels:
      app: test-app
  template:
    metadata:
      labels:
        app: test-app
    spec:
      containers:
      - name: ctr-1
        image: <image-tag>    # as shown under "REF" column in "k3s ctr image ls"
```


## Apply Deployment Manifest

```console
k3s kubectl apply -f /path/to/deploy.yaml
```


## Inspect & Interact With Running Applications

```console
k3s kubectl get pods
k3s kubectl get deployment
k3s kubectl rollout status deployment/<deployment-name>
k3s kubectl describe pod <pod-name>
k3s kubectl attach -c <container-name> <pod-name>
k3s kubectl exec -it -c <container-name> <pod-name> -- <prog>
```


## Delete Deployment

```console
k3s kubectl delete deployment <deployment-name>
```
