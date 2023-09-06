#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <vector>

using namespace std;

void parse_and_run_command(const string &command) {
    // TODO: Implement this.
    /* Note that this is not the correct way to test for the exit command.
       For example the command "   exit  " should also exit your shell. */

    vector<string> tokens;
    string input_file, output_file;
    bool input_redirection = false, output_redirection = false;

    // Split the command into tokens
    size_t pos = 0, found;
    while((found = command.find_first_of(" \t\n", pos)) != string::npos) {
        if(found > pos)
            tokens.push_back(command.substr(pos, found - pos));
        pos = found+1;
    }
    if(pos < command.length())
        tokens.push_back(command.substr(pos));

    // Check if tokens are empty and handle exit command
    if (tokens.empty()) {
        cerr << "Error: No command entered." << endl;
        return;
    }
    if (tokens[0] == "exit")
        exit(0);

    // Parse tokens for input and output redirections
    vector<const char*> c_tokens;
    for(auto it = tokens.begin(); it != tokens.end(); ++it) {
        if(*it == "<") {
            input_redirection = true;
            ++it;
            if(it != tokens.end())
                input_file = *it;
            else {
                cerr << "Error: No input file specified." << endl;
                return;
            }
        } else if(*it == ">") {
            output_redirection = true;
            ++it;
            if(it != tokens.end())
                output_file = *it;
            else {
                cerr << "Error: No output file specified." << endl;
                return;
            }
        } else
            c_tokens.push_back(it->c_str());
    }

    pid_t pid = fork();
    if (pid == -1) {
        cerr << "Fork failed." << endl;
        return;
    }

    if (pid == 0) { // Child process
        if(input_redirection) {
            int in_fd = open(input_file.c_str(), O_RDONLY);
            if(in_fd == -1) {
                cerr << "Error: Unable to open input file." << endl;
                exit(EXIT_FAILURE);
            }
            dup2(in_fd, STDIN_FILENO);
            close(in_fd);
        }

        if(output_redirection) {
            int out_fd = open(output_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if(out_fd == -1) {
                cerr << "Error: Unable to open output file." << endl;
                exit(EXIT_FAILURE);
            }
            dup2(out_fd, STDOUT_FILENO);
            close(out_fd);
        }

        // Convert vector<string> to vector<const char*>

        // vector<const char*> c_tokens;
        // for (const string& str : tokens)
        //     c_tokens.push_back(str.c_str());
        c_tokens.push_back(nullptr);

        execvp(c_tokens[0], const_cast<char* const*>(c_tokens.data()));
        cerr << "Command not found." << endl; // This line is executed only if execvp fails
        exit(EXIT_FAILURE);
    } else { // Parent process
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status))
            cout << c_tokens[0] << " exit status: " << WEXITSTATUS(status) << endl;
        else
            cerr << "Error: Child process terminated abnormally." << endl;
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
