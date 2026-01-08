#include <iostream>
#include <filesystem> // Used to create folders
#include <fstream>    // Used to create/read files
#include <chrono>
#include <ctime>


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

void status() {
    ifstream indexIn(INDEX_FILE);
    string line;
    cout << "Staged files:\n";
    while (getline(indexIn, line)) {
        cout << "  " << line << endl;
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
    
    else if (command == "status") {
    status();
    }

    else {
        cout << "Unknown command." << endl;
    }

    return 0;
}