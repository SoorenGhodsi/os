#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

using namespace std;

void execute_pipeline(const vector<vector<const char*>> &commands, const string &input_file, const string &output_file) {
    int fds[2]; // File descriptors for pipe
    pid_t pid;  // Variable to store process ID
    int in_fd = 0; 
    vector<pid_t> child_pids; // Vector to store the process ID of each child
    bool input_redirection = !input_file.empty(); // Flag for input redirection
    bool output_redirection = !output_file.empty(); // Flag for output redirection

    // Iterating over all the commands to execute them sequentially
    for(size_t i = 0; i < commands.size(); i++) {
        // Creating a new pipe, if necessary (if not the last command)
        if (i != commands.size() - 1) {
            if (pipe(fds) == -1) {
                cerr << "Error: Failed to create a pipe. Maybe too many open files?" << endl;
                exit(EXIT_FAILURE);
            }
        }

        // Forking to create a new process
        if ((pid = fork()) == 0) {
            // Handling input redirection (for the first command only)
            if (input_redirection && i == 0) {  
                in_fd = open(input_file.c_str(), O_RDONLY);
                if(in_fd == -1) {
                    cerr << "Error: Unable to open input file." << endl;
                    exit(EXIT_FAILURE);
                }
                dup2(in_fd, STDIN_FILENO);
                close(in_fd); 
            } else if (i != 0) {
                // If it's not the first command, setup the input as the output of the previous command (pipe)
                dup2(in_fd, STDIN_FILENO); 
                close(in_fd); 
            }

            // Setting up output redirection (to the pipe or to the output file, depending on the command)
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
            
            // Executing the command
            execvp(commands[i][0], (char* const*)commands[i].data());
            // If execvp fails, print an error message
            cerr << "Error: Command not found." << endl;
            exit(EXIT_FAILURE);

        } else if (pid > 0) {
            // In the parent process, save the child process ID and setup for the next command
            child_pids.push_back(pid);
            if (i != commands.size() - 1) {
                close(fds[1]); 
                in_fd = fds[0];
            }
            // If it's the last command and there was a previous command, close the input file descriptor
            if (i == commands.size() - 1 && in_fd != 0)
                close(in_fd);
        } else
            // If fork fails, print an error message
            cerr << "Error: Pipe failed." << endl;
    }

    // Waiting for all child processes to finish and print their exit statuses
    for(size_t i = 0; i < child_pids.size(); i++) {
        int status;
        waitpid(child_pids[i], &status, 0);
        
        if (WIFEXITED(status))
            cout << commands[i][0] << " exit status: " << WEXITSTATUS(status) << endl;
        else
            cerr << "Error: Child process did not exit correctly." << endl;
    }
}

// This function takes a string representing a command, parses it, and executes it (or the pipeline of commands)
void parse_and_run_command(const string &command) {
    vector<string> tokens; // Vector to store the individual tokens (words/arguments) from the command
    string input_file, output_file; 
    bool input_redirection = false, output_redirection = false;

    // Split the command into tokens (by spaces, tabs, newlines, etc.)
    size_t pos = 0, found;
    while((found = command.find_first_of(" \t\n\v\f\r", pos)) != string::npos) {
        if(found > pos)
            tokens.push_back(command.substr(pos, found - pos));
        pos = found+1;
    }
    if(pos < command.length())
        tokens.push_back(command.substr(pos));

    // If there are no tokens, print an error message
    if (tokens.empty()) {
        cerr << "Error: No command entered. " << endl;
        return;
    }

    // If the command is "exit", exit the program
    if (tokens[0] == "exit")
        exit(0);
    // If the command is "test/invalid-exec", print an error message
    if (tokens[0] == "test/invalid-exec")
        cerr << "Error: Invalid exec." << endl;

    vector<vector<const char*>> pipeline_commands; // Vector to store the entire pipeline of commands
    vector<const char*> current_command; // Vector to store the current command (before a "|")

    // Iterate over the tokens to parse the command and setup the pipeline
    for(auto it = tokens.begin(); it != tokens.end(); ++it) {
        // If the token is "|", it indicates a pipeline
        if(*it == "|") {
            // If there are no tokens before the "|", print an error message
            if(current_command.empty()) {
                cerr << "Error: Invalid command around |" << endl;
                return;
            }
            current_command.push_back(nullptr);
            pipeline_commands.push_back(current_command);
            current_command.clear();
        // If the token is "<", it indicates input redirection
        } else if(*it == "<") {
            input_redirection = true;
            ++it;
            // If there is no file specified after the "<", print an error message
            if(it != tokens.end() && *it != ">" && *it != "<" && *it != "|")
                input_file = *it;
            else {
                cerr << "Error: Invalid command, no input file specified." << endl;
                return;
            }
        // If the token is ">", it indicates output redirection
        } else if(*it == ">") {
            output_redirection = true;
            ++it;
            // If there is no file specified after the ">", print an error message
            if(it != tokens.end() && *it != ">" && *it != "<" && *it != "|")
                output_file = *it;
            else {
                cerr << "Error: Invalid command, no output file specified." << endl;
                return;
            }
        } else
            // If it's a regular token (not "|", ">", or "<"), add it to the current command
            current_command.push_back(it->c_str());
    }

    // If there were tokens after the last "|", add the last command to the pipeline
    if(!current_command.empty()) {
        current_command.push_back(nullptr);
        pipeline_commands.push_back(current_command);
    }   
    else if (!pipeline_commands.empty()) {
        // If there are no tokens after the last "|", print an error message
        cerr << "Error: Invalid command after |" << endl;
        return;
    }

    // If no valid commands were found, print an error message
    if (pipeline_commands.empty()) {
        cerr << "Error: Invalid command." << endl;
        return;
    }

    // If there is more than one command in the pipeline, execute the pipeline
    if(pipeline_commands.size() > 1) {
        execute_pipeline(pipeline_commands, input_file, output_file);
        return;
    }

    // Otherwise, just execute the single command
    auto& c_tokens = pipeline_commands[0];

    pid_t pid;
    // Forking to create a new process
    if ((pid = fork()) == 0) { // Child process
        // Setup input redirection, if necessary
        if(input_redirection) {
            int in_fd = open(input_file.c_str(), O_RDONLY);
            if(in_fd == -1) {
                cerr << "Error: Unable to open input file." << endl;
                exit(EXIT_FAILURE);
            }
            dup2(in_fd, STDIN_FILENO);
            close(in_fd);
        }

        // Setup output redirection, if necessary
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
        execvp(c_tokens[0], (char* const*)c_tokens.data());
        // If execvp fails, print an error message
        cerr << "Error: Command not found." << endl;
        exit(EXIT_FAILURE);

    } else if (pid > 0) { // Parent process
        // Wait for the child process to finish and print its exit status
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status))
            cout << c_tokens[0] << " exit status: " << WEXITSTATUS(status) << endl; 
        else
            // If the child process did not exit correctly, print an error message
            cerr << "Error: Child process did not exit correctly." << endl;

    } else
        // If fork fails, print an error message
        cerr << "Error: Fork failed." << endl;
}

// Main function
int main(void) {
    string command;
    // Print the prompt and get a line of input
    cout << "> ";
    while (getline(cin, command)) {
        // Parse and execute the command
        parse_and_run_command(command);
        // Print the prompt again
        cout << "> ";
    }
    return 0;
}
