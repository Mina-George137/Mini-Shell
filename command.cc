
/*
 * CS354: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>

#include "command.h"

SimpleCommand::SimpleCommand()
{
	// Creat available space for 5 arguments
	_numberOfAvailableArguments = 5;
	_numberOfArguments = 0;
	_arguments = (char **)malloc(_numberOfAvailableArguments * sizeof(char *));
}

void SimpleCommand::insertArgument(char *argument)
{
	if (_numberOfAvailableArguments == _numberOfArguments + 1)
	{
		// Double the available space
		_numberOfAvailableArguments *= 2;
		_arguments = (char **)realloc(_arguments,
									  _numberOfAvailableArguments * sizeof(char *));
	}

	_arguments[_numberOfArguments] = argument;

	// Add NULL argument at the end
	_arguments[_numberOfArguments + 1] = NULL;

	_numberOfArguments++;
}

Command::Command()
{
	// Create available space for one simple command
	_numberOfAvailableSimpleCommands = 1;
	_simpleCommands = (SimpleCommand **)
		malloc(_numberOfSimpleCommands * sizeof(SimpleCommand *));

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
}

void Command::insertSimpleCommand(SimpleCommand *simpleCommand)
{
	if (_numberOfAvailableSimpleCommands == _numberOfSimpleCommands)
	{
		_numberOfAvailableSimpleCommands *= 2;
		_simpleCommands = (SimpleCommand **)realloc(_simpleCommands,
													_numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));
	}

	_simpleCommands[_numberOfSimpleCommands] = simpleCommand;
	_numberOfSimpleCommands++;
}

void Command::clear()
{
	for (int i = 0; i < _numberOfSimpleCommands; i++)
	{
		for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
		{
			free(_simpleCommands[i]->_arguments[j]);
		}

		free(_simpleCommands[i]->_arguments);
		free(_simpleCommands[i]);
	}

	if (_outFile)
	{
		free(_outFile);
	}

	if (_inputFile)
	{
		free(_inputFile);
	}

	if (_errFile)
	{
		free(_errFile);
	}

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
}

void Command::print()
{
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  --- ----------------------------------------------------------\n");

	for (int i = 0; i < _numberOfSimpleCommands; i++)
	{
		printf("  %-3d ", i);
		for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
		{
			printf("\"%s\" \t", _simpleCommands[i]->_arguments[j]);
		}
	}

	printf("\n\n");
	printf("  Output       Input        Error        Background\n");
	printf("  ------------ ------------ ------------ ------------\n");
	printf("  %-12s %-12s %-12s %-12s\n", _outFile ? _outFile : "default",
		   _inputFile ? _inputFile : "default", _errFile ? _errFile : "default",
		   _background ? "YES" : "NO");
	printf("\n\n");
}

void Command::execute()
{
	signal(SIGINT, SIG_IGN);
	// Don't do anything if there are no simple commands
	if (_numberOfSimpleCommands == 0)
	{
		prompt();
		return;
	}
	if (strcmp(_currentCommand._currentSimpleCommand->_arguments[0], "exit") == 0)
	{
		printf("\n \tGood Bye\n");
		exit(EXIT_SUCCESS);
	}
	int defaultin = dup(0);
	int defaultout = dup(1);
	int defaulterr = dup(2);
	int outfd, infd;
	print();


	// redirection(defaultin, defaultout, defaulterr, outfd, infd);

	for (int i = 0; i < _numberOfSimpleCommands; i++)
	{
		printf("%d\n",i);
		_currentCommand._currentSimpleCommand =  _simpleCommands[i];
		int pipefd[2];
		int p = pipe(pipefd);
		if (_currentCommand._inputFile)
		{
			infd = open(_currentCommand._inputFile, O_RDONLY);
		}

		if (_currentCommand._outFile && (_currentCommand._isAppend == 0))
		{
			outfd = open(_currentCommand._outFile, O_CREAT | O_WRONLY, 0666);

		printf("%s", _currentCommand._outFile);
		}	
		else if (_currentCommand._outFile && (_currentCommand._isAppend == 1))
		{
			outfd = fileno(fopen(_currentCommand._outFile, "a+"));
		}
		if (p == -1)
		{
			printf("pipe error");
			exit(2);
		}

		if (i == 0 && infd)
		{
			dup2(infd, 0);
			close(infd);
		}
		else
		{
			dup2(pipefd[0], 0);
			close(pipefd[0]);
		}
		if (i == _numberOfSimpleCommands - 1 && outfd)
		{
			dup2(outfd, 1);
			close(outfd);
		}
		else
		{
			dup2(pipefd[1], 1);
			close(pipefd[1]);
		}
		if(strcmp(_currentCommand._currentSimpleCommand->_arguments[0],"cd") == 0){
			if(_currentSimpleCommand->_numberOfArguments == 1){
				chdir("..");
			}
			else{
				chdir(_currentCommand._currentSimpleCommand->_arguments[1]);
			}
		}
		else{
		int pid = fork();
		if (pid == -1)
		{
			perror("fork\n");
			exit(2);
		}
		if (pid == 0)
		{
			pid = execvp(_currentCommand._currentSimpleCommand->_arguments[0], _currentCommand._currentSimpleCommand->_arguments);
		}
		else if (pid > 0)
		{
			dup2(defaultout, 1);
			dup2(defaultin, 0);
			if (!_currentCommand._background)
			{
				waitpid(pid, 0, 0);
			}
		}
			log(pid);
			
		}
		
	}
	// Add execution here
	// For every simple command fork a new process
	// Setup i/o redirection
	// and call exec

	// restoreRedirection(defaultin, defaultout, defaulterr, outfd, infd);

	// Clear to prepare for next command
	clear();

	// Print new prompt
	prompt();
}

// Shell implementation

void Command::redirection(int defaultIn, int defaultOut, int defaultError, int outfd, int infd)
{

	if (_outFile && outfd)
	{
		printf("change output\n");
		// Redirect input (use sdtin)

		// Redirect output to pipe (write the output to pipefile[1] instead od stdout)
		dup2(outfd, 1);
		close(outfd);

		// Redirect err (use stderr)
		dup2(defaultError, 2);

		// close(outfd);
		// defaultIn(defaultOut);
		close(defaultError);
	}
	if (_inputFile && infd)
	{
		printf("change inputttttttt\n");
		// Redirect input (use sdtin)

		// Redirect output to pipe (write the output to pipefile[1] instead od stdout)
		dup2(infd, 0);
		// close(infd);
		//  Redirect err (use stderr)
		dup2(defaultError, 2);

		close(defaultIn);
		close(defaultOut);
		close(defaultError);
	}
}

void Command::restoreRedirection(int defaultIn, int defaultOut, int defaultError, int outfd, int infd)
{
	dup2(defaultOut, 1);
	dup2(defaultIn, 0);

	close(outfd);
	close(infd);

	close(defaultIn);
	close(defaultOut);
	close(defaultError);
}
void Command::log(int pid)
{
	time_t now;
	time(&now);

	FILE *f;
	f = fopen("shell.log", "a+");
	fprintf(f, "Log: [ %s ] \"%s\" - %d terminated\n", ctime(&now), _currentCommand._currentSimpleCommand->_arguments[0], pid);
	fclose(f);
}
void Command::prompt()
{
	printf("myshell>");
	fflush(stdout);
}

Command Command::_currentCommand;
SimpleCommand *Command::_currentSimpleCommand;

int yyparse(void);

int main()
{
	Command::_currentCommand.prompt();

	yyparse();
	return 0;
}
