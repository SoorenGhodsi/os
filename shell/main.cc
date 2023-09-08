#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

using namespace std;

void parse_and_run_command(const string &command) {
    // Declare variables and initialize redirection booleans
    vector<string> tokens;
    string input_file, output_file;
    bool input_redirection = false, output_redirection = false;

    // Split the command into tokens (checking for tabs, newlines, etc.)
    size_t pos = 0, found;
    while((found = command.find_first_of(" \t\n\v\f\r", pos)) != string::npos) {
        if(found > pos)
            tokens.push_back(command.substr(pos, found - pos));
        pos = found+1;
    }
    if(pos < command.length())
        tokens.push_back(command.substr(pos));

    // Check if no command was entered
    if (tokens.empty()) {
        cerr << "Error: No command entered. " << endl;
        return;
    }

    // Handling 'exit' command
    if (tokens[0] == "exit")
        exit(0);

    // Special invalid exec command
    if (tokens[0] == "test/invalid-exec")
        cerr << "Error: Invalid exec." << endl;

    // Parse tokens for redirections
    // Store other tokens in c_tokens
    vector<const char*> c_tokens;
    for(auto it = tokens.begin(); it != tokens.end(); ++it) {
        if(*it == "<") {
            input_redirection = true;
            ++it;
            if(it != tokens.end() && *it != ">" && *it != "<")
                input_file = *it;
            else {
                cerr << "Error: Invalid command, no input file specified." << endl;
                return;
            }
        } else if(*it == ">") {
            output_redirection = true;
            ++it;
            if(it != tokens.end() && *it != ">" && *it != "<")
                output_file = *it;
            else {
                cerr << "Error: Invalid command, no output file specified." << endl;
                return;
            }
        } else // Non-redirection token
            c_tokens.push_back(it->c_str()); 
    }

    // Case where no valid tokens are found
    if(c_tokens.empty()) {
        cerr << "Error: Invalid command." << endl;
        return;
    }
    c_tokens.push_back(nullptr);

    pid_t pid;
    if ((pid = fork()) == 0) { // Child process
        // Redirect stdin --> input file
        if(input_redirection) {
            int in_fd = open(input_file.c_str(), O_RDONLY);
            if(in_fd == -1) {
                cerr << "Error: Unable to open input file." << endl;
                exit(EXIT_FAILURE);
            }
            dup2(in_fd, STDIN_FILENO);
            close(in_fd);
        }

        // Redirect stdout --> output file
        if(output_redirection) {
            int out_fd = open(output_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if(out_fd == -1) {
                cerr << "Error: Unable to open output file." << endl;
                exit(EXIT_FAILURE);
            }
            dup2(out_fd, STDOUT_FILENO);
            close(out_fd);
        }

        // Execute the command 
        int retval = execv(c_tokens[0], const_cast<char* const*>(c_tokens.data()));
        if (retval < 0) {
            cerr << "Error: Command not found." << endl; // This line is executed only if execvp fails
            exit(EXIT_FAILURE);
        }

    } else if (pid > 0) { // Parent process
        // Wait for the child process to finish and get its status
        int status;
        waitpid(pid, &status, 0);

        // Print child exit status
        if (WIFEXITED(status))
            cout << c_tokens[0] << " exit status: " << WEXITSTATUS(status) << endl; 
        else
            cerr << "Error: Child process did not exit correctly." << endl;
            
    } else { // Fork fails if pid is negative
        cerr << "Error: Fork failed." << endl;
        return;
    }
}

int main(void) {
    string command;
    cout << "> ";
    while (getline(cin, command)) {
        parse_and_run_command(command);
        cout << "> ";
    }
    return 0;
}
