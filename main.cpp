#include <iostream>
#include <filesystem> // Used to create folders
#include <fstream>    // Used to create/read files
#include <chrono>
#include <ctime>
#include <unordered_map>


using namespace std;
namespace fs = filesystem; // A shortcut so we don't have to type 'filesystem' every time

// These are the names of the folders MiniGit will use to hide its data
const string MINI_GIT_DIR = ".minigit";
const string OBJECTS_DIR = MINI_GIT_DIR + "/objects";
const string INDEX_FILE = MINI_GIT_DIR + "/index.txt"; 
const string HEAD_FILE = MINI_GIT_DIR + "/HEAD";
const string BRANCHES_FILE = MINI_GIT_DIR + "/branches.txt";
const string COMMITS_DIR = MINI_GIT_DIR + "/commits";



// This turns file content into a unique string ID
string Hash(const string& content) {
    hash<string> hasher;
    return to_string(hasher(content));
}

void add(const string& filename) {
    // 1. Read the file you want to add
    ifstream inFile(filename);
    if (!inFile) {
        cout << "File not found: " << filename << endl;
        return;
    }

    // 2. Put the file content into a string
    stringstream buffer;
    buffer << inFile.rdbuf();
    string content = buffer.str();

    // 3. Create a unique Hash for this content
    string hashValue = Hash(content);

    // 4. Save the content in .minigit/objects using the Hash as the name
    // This way, if two files have the same content, they share the same object!
    ofstream outFile(OBJECTS_DIR + "/" + hashValue);
    outFile << content;
    outFile.close();

    // 5. Update the 'index.txt' file (The Staging Area)
    // We store it as "filename:hash"
    ofstream indexOut(INDEX_FILE, ios::app); // 'ios::app' means add to the end of the file
    indexOut << filename << ":" << hashValue << endl;

    cout << "Staged file: " << filename << " (Hash: " << hashValue << ")" << endl;
}

void init() {
    if (fs::exists(MINI_GIT_DIR)) {
        cout << "MiniGit is already initialized here!" << endl;
        return;
    }

    fs::create_directory(MINI_GIT_DIR);
    fs::create_directory(OBJECTS_DIR);
    fs::create_directory(COMMITS_DIR);

    // Create initial branch
    ofstream branches(BRANCHES_FILE);
    branches << "main:null\n";  // no commits yet
    branches.close();

    // Set HEAD to main
    ofstream head(HEAD_FILE);
    head << "main";
    head.close();

    // Empty index
    ofstream index(INDEX_FILE);

    cout << "Success! Created .minigit structure" << endl;
}


// Gets the name of the branch we are currently on (e.g., "main")
string getCurrentBranch() {
    ifstream headIn(HEAD_FILE);
    string branch;
    getline(headIn, branch);
    return branch;
}

// Finds the most recent Commit Hash for a specific branch
string getBranchHead(const string& branchName) {
    ifstream branchesIn(BRANCHES_FILE);
    string line;
    while (getline(branchesIn, line)) {
        size_t sep = line.find(":");
        if (line.substr(0, sep) == branchName) {
            return line.substr(sep + 1);
        }
    }
    return "null"; // If no commits yet
}

// Updates the branch to point to the NEWEST commit (the new head)
void updateBranchHead(const string& branch, const string& newHash) {
    ifstream in(BRANCHES_FILE);
    stringstream updated;
    string line;
    while (getline(in, line)) {
        size_t sep = line.find(":");
        if (line.substr(0, sep) == branch) {
            updated << branch << ":" << newHash << "\n";
        } else {
            updated << line << "\n";
        }
    }
    ofstream out(BRANCHES_FILE);
    out << updated.str();
}


void commit(const string& message) {
    // 1. Check if there is anything to commit
    ifstream indexIn(INDEX_FILE);
    string line, commitData;
    bool hasFiles = false;
    while (getline(indexIn, line)) {
        commitData += line + "\n";
        hasFiles = true;
    }
    if (!hasFiles) {
        cout << "Nothing to commit (staging area empty)." << endl;
        return;
    }

    // 2. Gather Metadata (The "Node" data)
    string branch = getCurrentBranch();
    string parent = getBranchHead(branch);
    auto now = chrono::system_clock::to_time_t(chrono::system_clock::now());
    string timestamp = ctime(&now);
    timestamp.pop_back();

    string metadata = "message: " + message + "\n" +
                  "parent: " + parent + "\n" +
                  "branch: " + branch + "\n" +
                  "timestamp: " + timestamp + "\n";


    // 3. Hash the entire commit (Metadata + Files) to get a unique Commit ID
    string commitHash = Hash(metadata + commitData);

    // 4. Save the commit file
    ofstream commitOut(COMMITS_DIR + "/" + commitHash);
    commitOut << metadata << commitData;

    // 5. Update the branch head to point to this new commit
    updateBranchHead(branch, commitHash);

    // 6. Clear the staging area (Index)
    ofstream clearIndex(INDEX_FILE, ios::trunc); 
    
    cout << "Commit created: " << commitHash << endl;
}

void showlog() {
    string branch = getCurrentBranch();
    string current = getBranchHead(branch);

    if (current == "null") {
        cout << "No commits yet on branch: " << branch << endl;
        return;
    }

    cout << "--- Commit History for branch: " << branch << " ---" << endl;

    while (current != "null") {
        string path = COMMITS_DIR + "/" + current;
        if (!fs::exists(path)) {
            cout << "Commit file missing: " << current << endl;
            break;
        }

        ifstream in(path);
        string line;
        string parentHash = "null";

        cout << "Commit: " << current << endl;

        while (getline(in, line)) {
            if (line.rfind("message:", 0) == 0 || line.rfind("branch:", 0) == 0) {
                cout << "  " << line << endl;
            }
            if (line.rfind("parent:", 0) == 0) {
                parentHash = line.substr(8);
                parentHash.erase(parentHash.find_last_not_of(" \n\r\t") + 1);
            }
        }
        cout << "-----------------------" << endl;
        current = parentHash;
    }
}


void status() {
    ifstream indexIn(INDEX_FILE);
    string line;
    cout << "Staged files:\n";
    while (getline(indexIn, line)) {
        cout << "  " << line << endl;
    }
}

// branching 

bool branchExists(const string& branchName) {
    ifstream in(BRANCHES_FILE);
    string line;
    while (getline(in, line)) {
        size_t sep = line.find(":");
        if (line.substr(0, sep) == branchName) {
            return true;
        }
    }
    return false;
}


void createBranch(const string& branchName) {
    if (branchExists(branchName)) {
        cout << "Branch already exists: " << branchName << endl;
        return;
    }

    string currentCommitHash = getBranchHead(getCurrentBranch());

    ofstream branchesOut(BRANCHES_FILE, ios::app);
    branchesOut << branchName << ":" << currentCommitHash << "\n";

    cout << "Created branch '" << branchName
         << "' at commit " << currentCommitHash << endl;
}


void checkout(const string& target) {
    if (!branchExists(target)) {
        cout << "Branch not found: " << target << endl;
        return;
    }

    ofstream headFile(HEAD_FILE, ios::trunc);
    headFile << target;

    cout << "Switched to branch: " << target << endl;
}


void merge(const string& targetBranch) {
    string currentBranch = getCurrentBranch();
    string currentCommit = getBranchHead(currentBranch);
    string targetCommit = getBranchHead(targetBranch);

    if (targetCommit == "null") {
        cout << "Branch not found or no commits to merge." << endl;
        return;
    }

    unordered_map<string, string> mergedFiles;
    string line;

    // 1. Read files from the target branch (the branch you are pulling FROM)
    ifstream targetIn(COMMITS_DIR + "/" + targetCommit);
    while (getline(targetIn, line)) {
        // Skip metadata lines, only look for "filename:hash"
        if (line.find(":") != string::npos && line.find("message:") != 0 && 
            line.find("parent:") != 0 && line.find("branch:") != 0) {
            size_t sep = line.find(":");
            mergedFiles[line.substr(0, sep)] = line.substr(sep + 1);
        }
    }

    // 2. Read files from the current branch and check for conflicts
    ifstream currentIn(COMMITS_DIR + "/" + currentCommit);
    while (getline(currentIn, line)) {
        if (line.find(":") != string::npos && line.find("message:") != 0 && 
            line.find("parent:") != 0 && line.find("branch:") != 0) {
            size_t sep = line.find(":");
            string file = line.substr(0, sep);
            string hash = line.substr(sep + 1);

            // Conflict Check: If file exists in both but hashes are different
            if (mergedFiles.count(file) && mergedFiles[file] != hash) {
                cout << "CONFLICT: Both branches modified " << file << ". Keeping current version." << endl;
            }
            mergedFiles[file] = hash; // Current branch takes priority in this simple version
        }
    }

    // 3. Write the merged result to the Staging Area (Index)
    ofstream indexOut(INDEX_FILE, ios::trunc);
    for (const auto& [file, hash] : mergedFiles) {
        indexOut << file << ":" << hash << endl;
    }

    // 4. Automatically create a "Merge Commit"
    commit("Merged branch " + targetBranch + " into " + currentBranch);
}

void diff(const string& commit1, const string& commit2) {
    ifstream in1(COMMITS_DIR + "/" + commit1);
    ifstream in2(COMMITS_DIR + "/" + commit2);
    
    if (!in1 || !in2) {
        cout << "One or both commit hashes not found." << endl;
        return;
    }

    unordered_map<string, string> map1, map2;
    string line;

    // Helper to extract file:hash pairs into a map
    auto populateMap = [](ifstream& in, unordered_map<string, string>& m) {
        string l;
        while (getline(in, l)) {
            if (l.find(":") != string::npos && l.find("message:") != 0 && 
                l.find("parent:") != 0 && l.find("branch:") != 0) {
                size_t sep = l.find(":");
                m[l.substr(0, sep)] = l.substr(sep + 1);
            }
        }
    };

    populateMap(in1, map1);
    populateMap(in2, map2);

    cout << "### Diff: " << commit1 << " <-> " << commit2 << " ###" << endl;

    // 1. Check for changes and removals (comparing Map1 to Map2)
    for (auto const& [file, hash1] : map1) {
        if (map2.find(file) == map2.end()) {
            cout << "[-] " << file << " (Removed in second commit)" << endl;
        } else if (map2[file] != hash1) {
            cout << "[M] " << file << " (Modified/Content Changed)" << endl;
        }
    }

    // 2. Check for additions (comparing Map2 to Map1)
    for (auto const& [file, hash2] : map2) {
        if (map1.find(file) == map1.end()) {
            cout << "[+] " << file << " (Added in second commit)" << endl;
        }
    }
}


int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: ./minigit <command>" << endl;
        return 1;
    }

    string command = argv[1];

    if (command == "init") {
        init();
    } 
    else if (command == "add" && argc >= 3) {
         for (int i = 2; i < argc; i++) {
        add(argv[i]);
    }
    } 
    else if (command == "commit" && argc >= 4 && string(argv[2]) == "-m") {

    string msg;
    for (int i = 3; i < argc; i++) {   // start from argv[3]
        if (i > 3) msg += " ";         // add a space between words
        msg += argv[i];
    }
    commit(msg);
    }

    else if (command == "log") {
    showlog();
    }

    else if (command == "status") {
    status();
    }
    else if (command == "branch" && argc >= 3) {
        createBranch(argv[2]);
    }

    else if (command == "checkout" && argc >= 3) {
        checkout(argv[2]);
    }
    else if (command == "merge" && argc >= 3) {
        merge(argv[2]);
    }
    else if (command == "diff" && argc >= 4) {
        diff(argv[2], argv[3]);
    }

    else {
        cout << "Unknown command." << endl;
    }

    return 0;
}