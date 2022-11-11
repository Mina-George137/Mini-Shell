
/*
 * CS-413 Spring 98
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%token	<string_val> WORD

%token 	NOTOKEN GREAT NEWLINE GREATAPPEND INPUT BACKGROUND
%token  PIPE

%union	{
		char   *string_val;
	}

%{
extern "C" 
{
	int yylex();
	void yyerror (char const *s);
}
#define yylex yylex
#include <stdio.h>
#include "command.h"
#include "signal.h"
%}

%%

goal:	
	commands
	;

commands: 
	command
	| commands command
	;
command: simple_command
        ;
simple_command:	
	command_and_args iomodifier_opt iomodifier_ipt background NEWLINE  {
		printf("   Yacc: Execute command\n");
		Command::_currentCommand.execute();
	}
	| NEWLINE 
	| error NEWLINE { yyerrok; }
	;
command_and_args:
	command_word arg_list {	Command::_currentCommand.insertSimpleCommand( Command::_currentCommand._currentSimpleCommand );
	}
	;

arg_list:
	arg_list argument
	| /* can be empty */
	;

argument:
	WORD {
               printf("   Yacc: insert argument \"%s\"\n", $1);
	       Command::_currentCommand._currentSimpleCommand->insertArgument( $1 );
	}
	;

command_word:
	WORD {
               printf("   Yacc: insert command \"%s\"\n", $1);
	       Command::_currentCommand._currentSimpleCommand = new SimpleCommand();
	       Command::_currentCommand._currentSimpleCommand->insertArgument( $1 );
	}
	| WORD PIPE commands{
               printf("   Yacc: insert command \"%s\"\n", $1);
	       Command::_currentCommand._currentSimpleCommand = new SimpleCommand();
	       Command::_currentCommand._currentSimpleCommand->insertArgument( $1 );
	       
	}
	;

iomodifier_opt:
	GREAT WORD {
		printf("   Yacc: insert output \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand._isAppend = 0;
	}
	| /* can be empty */ 
	
	GREATAPPEND WORD {
		printf("   Yacc: insert output \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand._isAppend = 1;
	}
	| /* can be empty */ 
	;
iomodifier_ipt:
	INPUT WORD {
		printf("   Yacc: insert input \"%s\"\n", $2);
		Command::_currentCommand._inputFile = $2;
	}
	| INPUT WORD GREAT WORD {
		printf("   Yacc: insert input \"%s\"\n", $2);
		Command::_currentCommand._inputFile = $2;
		printf("   Yacc: insert output \"%s\"\n", $4);
		Command::_currentCommand._outFile=$4;
		Command::_currentCommand._isAppend = 0;	
	}
	| INPUT WORD GREATAPPEND WORD {
		printf("   Yacc: insert input \"%s\"\n", $2);
		Command::_currentCommand._inputFile = $2;
		printf("   Yacc: insert output \"%s\"\n", $4);
		Command::_currentCommand._outFile=$4;
		Command::_currentCommand._isAppend = 1;	
	}
	|
	;

background:
	BACKGROUND {
		printf("Running in background");
		Command::_currentCommand._background = 1;

	}
	| /* can be empty */ 
	;
%%

void
yyerror(const char * s)
{
	fprintf(stderr,"%s", s);
}

#if 0
main()
{
	yyparse();
}
#endif
