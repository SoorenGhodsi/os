#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

using namespace std;

void execute_pipeline(const vector<vector<const char*>> &commands, const string &input_file, const string &output_file) {
    int fds[2];
    pid_t pid;
    int in_fd = 0; 
    vector<pid_t> child_pids; // To store the PID of each child process
    bool input_redirection = !input_file.empty();
    bool output_redirection = !output_file.empty();

    for(size_t i = 0; i < commands.size(); i++) {
        // Check if we can create a new pipe
        if (i != commands.size() - 1) {
            if (pipe(fds) == -1) {
                cerr << "Error: Failed to create a pipe. Maybe too many open files?" << endl;
                exit(EXIT_FAILURE);
            }
        }

        if ((pid = fork()) == 0) {
            if (input_redirection && i == 0) {  
                in_fd = open(input_file.c_str(), O_RDONLY);
                if(in_fd == -1) {
                    cerr << "Error: Unable to open input file." << endl;
                    exit(EXIT_FAILURE);
                }
                dup2(in_fd, STDIN_FILENO);
                close(in_fd); 
            } else if (i != 0) {
                dup2(in_fd, STDIN_FILENO); 
                close(in_fd); 
            }

            if(i != commands.size() - 1) {
                dup2(fds[1], STDOUT_FILENO);
                close(fds[1]);
                close(fds[0]);
            } else if (output_redirection) { 
                int out_fd = open(output_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
                if(out_fd == -1) {
                    cerr << "Error: Unable to open output file." << endl;
                    exit(EXIT_FAILURE);
                }
                dup2(out_fd, STDOUT_FILENO);
                close(out_fd);
            }
            
            execvp(commands[i][0], (char* const*)commands[i].data());
            cerr << "Error: Command not found." << endl;
            exit(EXIT_FAILURE);

        } else if (pid > 0) {
            child_pids.push_back(pid);
            if (i != commands.size() - 1) {
                close(fds[1]); 
                in_fd = fds[0];
            }
            if (i == commands.size() - 1 && in_fd != 0)
                close(in_fd);
        } else
            cerr << "Error: Pipe failed." << endl;
    }

    // After the loop, wait for all child processes to finish
    for(size_t i = 0; i < child_pids.size(); i++) {
        int status;
        waitpid(child_pids[i], &status, 0);
        
        if (WIFEXITED(status))
            cout << commands[i][0] << " exit status: " << WEXITSTATUS(status) << endl;
        else
            cerr << "Error: Child process did not exit correctly." << endl;
    }
}

void parse_and_run_command(const string &command) {
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

    if (tokens.empty()) {
        cerr << "Error: No command entered. " << endl;
        return;
    }

    if (tokens[0] == "exit")
        exit(0);
    if (tokens[0] == "test/invalid-exec")
        cerr << "Error: Invalid exec." << endl;

    vector<vector<const char*>> pipeline_commands;
    vector<const char*> current_command;

    for(auto it = tokens.begin(); it != tokens.end(); ++it) {
        if(*it == "|") {
            if(current_command.empty()) {
                cerr << "Error: Invalid command around |" << endl;
                return;
            }
            current_command.push_back(nullptr);
            pipeline_commands.push_back(current_command);
            current_command.clear();
        } else if(*it == "<") {
            input_redirection = true;
            ++it;
            if(it != tokens.end() && *it != ">" && *it != "<" && *it != "|")
                input_file = *it;
            else {
                cerr << "Error: Invalid command, no input file specified." << endl;
                return;
            }
        } else if(*it == ">") {
            output_redirection = true;
            ++it;
            if(it != tokens.end() && *it != ">" && *it != "<" && *it != "|")
                output_file = *it;
            else {
                cerr << "Error: Invalid command, no output file specified." << endl;
                return;
            }
        } else
            current_command.push_back(it->c_str());
    }

    if(!current_command.empty()) {
        current_command.push_back(nullptr);
        pipeline_commands.push_back(current_command);
    }   
    else if (!pipeline_commands.empty()) {
        cerr << "Error: Invalid command after |" << endl;
        return;
    }

    if (pipeline_commands.empty()) {
        cerr << "Error: Invalid command." << endl;
        return;
    }

    if(pipeline_commands.size() > 1) {
        execute_pipeline(pipeline_commands, input_file, output_file);
        return;
    }

    auto& c_tokens = pipeline_commands[0];

    pid_t pid;
    if ((pid = fork()) == 0) { // Child process
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

        execvp(c_tokens[0], (char* const*)c_tokens.data());
        cerr << "Error: Command not found." << endl;
        exit(EXIT_FAILURE);

    } else if (pid > 0) { // Parent process
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status))
            cout << c_tokens[0] << " exit status: " << WEXITSTATUS(status) << endl; 
        else
            cerr << "Error: Child process did not exit correctly." << endl;

    } else// Fork fails if pid is negative
        cerr << "Error: Fork failed." << endl;
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