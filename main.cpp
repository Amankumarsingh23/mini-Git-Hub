#include <iostream>
#include <filesystem> // Used to create folders
#include <fstream>    // Used to create/read files

using namespace std;
namespace fs = filesystem; // A shortcut so we don't have to type 'filesystem' every time

// These are the names of the folders MiniGit will use to hide its data
const string MINI_GIT_DIR = ".minigit";
const string OBJECTS_DIR = MINI_GIT_DIR + "/objects";
const string INDEX_FILE = MINI_GIT_DIR + "/index.txt"; 


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
    // Check if the folder already exists so we don't overwrite it
    if (fs::exists(MINI_GIT_DIR)) {
        cout << "MiniGit is already initialized here!" << endl;
        return;
    }

    // Create the folders
    fs::create_directory(MINI_GIT_DIR);
    fs::create_directory(OBJECTS_DIR);

    cout << "Success! Created .minigit and .minigit/objects" << endl;
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
        add(argv[2]); // argv[2] is the filename you type
    } 
    else {
        cout << "Unknown command." << endl;
    }

    return 0;
}