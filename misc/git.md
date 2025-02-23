# Git Workflow


## References

1. [Gerrit Documentation](https://www.gerritcodereview.com)
2. [Git - Reference](https://git-scm.com/docs)


## Basic Git & Gerrit Flows

### Set up Git configuration (one-time step)

```console
$ git config --global user.email example@email.com
$ git config --global user.name "John Doe"
```

Also generate and add SSH keys into Git server.


### Downloading repositories

```console
$ git clone -b <branch-to-checkout> <repo-clone-URL>
```

### Rebasing local repository tree

```console
$ git pull --rebase origin <branch-to-rebase-from>
```

### Creating a Gerrit Change

1. Make the changes.
2. Stage files to be committed.

    ```console
    $ git status
    $ git add …
    ```

3. Setup Change ID hook (one-time step for local repository tree).

    ```console
    $ scp -p -P 29418 ${USER}@example.gerrit.server.com:hooks/commit-msg /path/to/repo-dir/.git/hooks/
    $ chmod u+x /path/to/repo-dir/.git/hooks/commit-msg
    ```

4. Create a commit and push to Gerrit.

    ```console
    $ git commit -m "commit message"
    $ git pull --rebase origin <branch-to-rebase-from>        # if needed
    $ git push origin HEAD:refs/for/<branch-to-push-to>       # if change is not a Gerrit Draft
    $ git push origin HEAD:refs/drafts/<branch-to-push-to>    # if change is a Gerrit Draft
    ```

5. Use the Gerrit Change webpage to add reviewers.

### Amending a Gerrit Change

```console
$ git status
$ git add …
$ git commit --amend
$ git pull --rebase origin <branch-to-rebase-from>        # if needed
$ git push origin HEAD:refs/for/<branch-to-push-to>       # if change is not a Gerrit Draft
$ git push origin HEAD:refs/drafts/<branch-to-push-to>    # if change is a Gerrit Draft
```

### Rebasing during merge conflicts

```console
$ git pull --rebase origin <branch-to-rebase-from>    # resolve conflicts in specified files
$ git status
$ git add …
$ git rebase --continue
```


## Miscellaneous Flows

1. The `Download` section on the webpage of a Gerrit Change has ready-made commands for
useful steps like fetch-and-checkout or fetch-and-cherry-pick.
2. See which author and commit changed each line, in specified file: `git blame /path/to/file`
3. List git branch information: `git branch -a`
4. Find information about commits:
    - Check if a branch contains a given commit, specified by its hash: `git branch -a --contains <commit-hash>`
    - Show commit message for a given commit, specified by its hash: `git show -s --format=%B <commit-hash>`
5. Create patch files from a given commit, specified by its hash: `git format-patch -1 <commit-hash>`
6. Applying a patch file: `git apply /path/to/patch-file.patch`
7. Applying multiple patch files: `git am /path/to/patch-dir/*.patch`
