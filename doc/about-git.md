# About Git
## Four areas to work
- Local host
1. Working area\
    $file under working
2. Staging area
3. Local repo\
    $file version 1\
    ...\
    $file version n 
- Cloud
4. Remote repo\
    $file version 1\
    ...\
    $file version n
## Steps to initialize a repo, start revision control on it, modify code, and merge back to main
1. initialize a repo
```bash
cd <your_folder>
git init
```
Rename the init branch
```bash
git branch -m <new_name>
```
2. add files to repo
```bash
git add <filename>...
```
3. commit changes
```bash
git commit -m "initial commit"
```
4. check repo status
```bash
git status
```
5. create a new branch: (local)
```bash
cd $the_folder
git checkout -b <new_branch>
```
Good practice of branch nameing, "
6. make modifications: (local)
:::mermaid
graph LR
edit --> build --> run --> debug --> edit
debug --> completed
:::
7. commit the changes: (local)
```bash
git add .
git commit -m "commit message"
```
8. update local main branch: (local)
```bash
git checkout main
# git pull origin main
```
* "origin" means the name of the remote repo
9. merge the new branch to main: (local)
```bash
git merge <new_branch>
```
10. push the changes to the remote repo: (if needed)
```bash
git push origin main
```
11. list all branches in the repo
```bash
git branch -a
```
12. see the history of each branch
```bash
git log --all --oneline --graph --decorate
git log --<path_to_subfolder>
```
## Command flow
:::mermaid
graph TD;
Working_area -- Git commit --> Local_repo -- Git commit --> Working_area -- Git add --> Staging_area
Local_repo -- Git push --> Remote_repo -- Git push --> Local_repo
:::
## References
[Git Quick Start Guide and Lessons Learned](https://medium.com/@Stan_DS/git-quick-start-guide-and-lessons-learned-1627a8d87ce5#:~:text=Git%20Quick%20Start%20Guide%20and%20Lessons%20Learned%201,on%20the%20right%20branch%20...%205%20Reference%20)