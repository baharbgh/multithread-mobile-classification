clone repo: git clone [URL]

check remotes: git remote -v

add origin: git remote add origin [URL]

initialize git: git init

change directory: cd

list files: ls

-------------
**commit and push**

create a file: touch [filename]

check the status: git status

track and stage a single file: git add [filename]

track and stage all files: git add .

commit with a message: git commit -m "description of commit"

view the log: git log

push changes: git push [remotename] [branchname]

-------------
**sync fork**

show your remotes: git remote -v

add an upstream remote: git remote add upstream [URL]

fetch changes from the upstream: git fetch upstream

merge those changes into your repo: git merge upstream/master

push those changes to the origin: git push origin master
