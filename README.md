# PES-VCS Project

## Student Details
NAME: PRIYA KL 
SRN: PES1UG24CS600 

---

## Phase 1: Object Storage

In this phase, I implemented the basic object storage system. 
It stores blobs using SHA-256 hashing and organizes them inside `.pes/objects`.

### Screenshot 1A: Test Objects Output
<img width="1600" height="297" alt="image" src="https://github.com/user-attachments/assets/2c04779c-5cf6-4af0-9d49-097175f6298b" />


### Screenshot 1B: Object Storage Structure
<img width="1600" height="163" alt="image" src="https://github.com/user-attachments/assets/5f6ab5b5-92b6-484b-9e87-5c57765da205" />


---

## Phase 2: Tree Objects

Here, I worked on tree objects which represent directory structure. 
The tree is built from entries and then serialized and stored.

### Screenshot 2A: Test Tree Output
<img width="1600" height="265" alt="image" src="https://github.com/user-attachments/assets/c5d32492-3a90-4367-8b79-c8bd66faf06b" />


### Screenshot 2B: Raw Tree Object
<img width="1600" height="155" alt="image" src="https://github.com/user-attachments/assets/80ea0264-a007-4157-98f4-c969a6d5178e" />


---

## Phase 3: Index (Staging Area)

In this phase, I implemented the staging area (index). 
It keeps track of files that are added before committing.

### Screenshot 3A: Init → Add → Status
<img width="1600" height="882" alt="image" src="https://github.com/user-attachments/assets/109061a1-34d2-4e72-96d6-dee441264efa" />


### Screenshot 3B: Index File Content
<img width="1600" height="74" alt="image" src="https://github.com/user-attachments/assets/a9887611-43d9-4156-b430-c0ff0cf7510e" />


---

## Phase 4: Commits and History

This phase handles commit creation and history tracking. 
Each commit stores metadata like author, message, and tree reference.

### Screenshot 4A: Log Output
<img width="1600" height="819" alt="image" src="https://github.com/user-attachments/assets/90d6c21e-2f1b-4e4f-a383-fd260a4cc797" />


### Screenshot 4B: Object Growth
<img width="1600" height="399" alt="image" src="https://github.com/user-attachments/assets/7d03f5e3-5894-4b0b-9b8e-e2ce2c44bd54" />


### Screenshot 4C: HEAD and Branch Reference
<img width="1600" height="146" alt="image" src="https://github.com/user-attachments/assets/325fd7ae-8be1-4339-9191-11906880f0a0" />


---

## Final Integration Test

This shows that all components (object, tree, index, commit) are working together.

<img width="1600" height="1044" alt="image" src="https://github.com/user-attachments/assets/8951a29f-5e68-4c29-ab35-8d3cfea3f03e" />
<img width="2052" height="984" alt="image" src="https://github.com/user-attachments/assets/ebfb0b88-f334-4918-92d3-f09822169be6" />


---

## Phase 5 & 6: Analysis Questions

### Q5.1 Branch & Checkout
In this system, a branch is basically a file that stores the latest commit hash.  
When we do checkout, we update the HEAD to point to that branch and change the working directory to match the files of that commit.

---

### Q5.2 Switching Branches
Before switching branches, we should check if there are any unsaved changes.  
If there are changes in the working directory, switching should not be allowed.  
This can be checked by comparing the working directory with the index and the index with HEAD.

---

### Q5.3 Detached HEAD
Detached HEAD means HEAD is pointing directly to a commit instead of a branch.  
If we make commits in this state, they are not linked to any branch and may be lost unless we create a new branch.

---

### Q6.1 Garbage Collection
Garbage collection removes objects that are no longer used.  
We start from branch heads and traverse all commits, trees, and blobs.  
Any object that is not reachable from these is considered unused and can be deleted.

---

### Q6.2 GC Race Condition
If garbage collection runs at the same time as a commit, it might delete objects that are still being created.  
This can cause errors. Git avoids this by using locking and safe file operations.
