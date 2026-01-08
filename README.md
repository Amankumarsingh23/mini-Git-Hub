# 🧩 MiniGit — A Simplified Version Control System in C++

MiniGit is a lightweight, Git-inspired version control system implemented entirely in C++, designed to demonstrate core version-control concepts such as staging, commits, branches, merges, and diffs using only the local file system.

This project was built as a learning-focused system design + DSA project, emphasizing clarity, correctness, and Git-like behavior rather than full Git complexity.

---

## 🚀 Features

MiniGit supports the following commands:

- ✅ `init` – Initialize a MiniGit repository  
- ✅ `add <file>` – Stage one or more files  
- ✅ `commit -m "<message>"` – Create a commit with metadata  
- ✅ `log` – View commit history of the current branch  
- ✅ `status` – View staged files  
- ✅ `branch <name>` – Create a new branch  
- ✅ `checkout <branch>` – Switch between branches  
- ✅ `merge <branch>` – Merge another branch into the current branch  
- ✅ `diff <commit1> <commit2>` – Compare two commits  

---

## 📁 Project Structure

```
MyMiniGit/
├── main.cpp                 # Complete MiniGit implementation
├── hello.txt                # Sample test file
├── feature.txt              # Sample branch test file
├── .minigit/                # Internal MiniGit data (auto-generated)
│   ├── objects/             # File snapshots stored by hash
│   ├── commits/             # Commit objects (metadata + file map)
│   ├── HEAD                 # Current branch pointer
│   ├── branches.txt         # Branch → commit mapping
│   └── index.txt            # Staging area
```

⚠️ `.minigit/` is managed internally — do not edit manually.

---

## 🛠️ Compilation

### On Linux / macOS
```bash
g++ -std=c++17 main.cpp -o minigit

## 🛠️ Compilation

### On Linux / macOS

```bash
g++ -std=c++17 main.cpp -o minigit
```

### On Windows (PowerShell)

```powershell
g++ -std=c++17 main.cpp -o minigit
```

---

## 💻 Usage Guide

### ➕ Initialize a Repository

```bash
./minigit init
```

---

### 📄 Create and Stage Files

```bash
echo "Hello MiniGit" > hello.txt
./minigit add hello.txt
```

You can stage multiple files at once:

```bash
./minigit add file1.txt file2.txt
```

---

### 📝 Commit Changes

```bash
./minigit commit -m "Initial commit"
```

Each commit stores:

* commit message
* parent commit
* branch name
* timestamp
* staged file snapshots

---

### 📜 View Commit History

```bash
./minigit log
```

Shows commits as a linked-list traversal, similar to Git.

---

### 📌 Check Staging Status

```bash
./minigit status
```

Displays all currently staged files.

---

### 🌱 Branching

```bash
./minigit branch feature-1
```

Creates a new branch pointing to the current commit.

---

### 🔀 Switch Branches

```bash
./minigit checkout feature-1
```

Updates `HEAD` to the target branch.

---

### 🔄 Merge Branches

```bash
./minigit checkout main
./minigit merge feature-1
```

* Detects simple file conflicts
* Prioritizes current branch
* Automatically creates a merge commit

---

### 🧮 Compare Commits

```bash
./minigit diff <commit_hash_1> <commit_hash_2>
```

Output legend:

* `[+]` File added
* `[-]` File removed
* `[M]` File modified

---

## 🧠 Internal Design Overview

* Content-addressed storage using `std::hash`
* Commits form a linked list via parent pointers
* Branches are lightweight pointers to commits
* Index acts as a staging area
* Objects are immutable once written

This mirrors Git’s core architecture in a simplified and readable form.

---

## 🧪 Sample Workflow

```bash
./minigit init
echo "Hello" > hello.txt
./minigit add hello.txt
./minigit commit -m "Initial commit"

./minigit branch feature
./minigit checkout feature
echo "Feature work" > feature.txt
./minigit add feature.txt
./minigit commit -m "Working on feature"

./minigit checkout main
./minigit merge feature
./minigit log
```

---

## 📦 Project Status

* ✅ Fully implemented in a single C++ file
* ✅ Branching, merging, and diff supported
* ✅ Tested with multiple sample runs
* ✅ Ready for submission, demo, or extension

---

## 📚 Learning Outcomes

* Understanding Git internals
* File-system–based persistence
* Linked data structures for history
* Hash-based object storage
* Command-line system design in C++

---

## 📜 License

This project is intended for educational use and learning purposes.




