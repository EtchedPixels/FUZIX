."$Revision: 41.2 $ $Date: 1995/12/29 03:23:32 $
.TH WISH 1L
.nr  LL  6i
.SH NAME
wish \- command interpreter and programming shell
.SH SYNOPSIS
.B wish
[ options ] [ script-file ]
.SH DESCRIPTION
.PP
.I Wish
is a shell (command interpreter) which reads commands from either a
file or the terminal and executes them. It executes system commands as well
as builtin commands (see below for description). Main features include: a
history mechanism, enabling previous commands to be easily redone; a
comprehensive command line editor for ease of use from terminal; word completion
and option listing on the command line; full input and output redirection and
aliases. These features are described in detail below.
.PP
This manual describes internal version 41 of Wish 2.0.
.PP
Wish is derived from the Clam shell, written by Callum Gibson, and is in some
sense a partial rewrite of Clam with some new features added and some old
features removed. Many thanks to Callum for allowing me to use his source,
and for his suggestions and criticisms during Wish's development.
.".SH OPTIONS
."The shell may be invoked with a number of options:
.".TP 4
.".B \-c
." \- the following arguments are passed to Wish for execution.
.".TP 4
.".B \-e
." \- exit shell if any command returns a non-zero exit status.
.".TP4
.".B \-f
." \- fast start up. Do not read the .wishrc file.
.".TP 4
.".B \-p
." \- parse the commands but do not execute so that checking can be performed
."on shell scripts.
.".TP 4
.".B \-v
." \- verbose mode. Commands echo back in same form as saved to history
."prior to execution. No metacharacter expansion is done (*,?,{},[],``)
."before the line is echoed.
.".TP 4
.".B \-x
." \- similar to verbose mode except that commands are echoed back in expanded
."form.
.".TP 4
.".B \-V
." \- Enter verbose mode even before .wishrc is read.
.".TP 4
.".B \-X
." \- \-X is to \-x as \-V is to \-v
.PP
.SH Basic Command Line Parsing
.PP
.PP
The command line, whether read from stdin or file, is broken up into
.I words.
Words are sequences of characters which are separated by
word-delimiters (the most obvious example is a
.I space).
The following characters have effects on the words of the
command line and act as word delimiters:
.PP
<
.I filename.
Standard input redirection. Instead of reading input from the
terminal, input is read from the named file (the following word). e.g.
.PP
% prog < inputfile
.PP
>
.I filename.
Standard output redirection. Instead of outputting to the
terminal screen, output is redirected to the named file. e.g.
.PP
% prog > outputfile
.PP
.PP
n>
.I filename.
Other output redirection. Instead of outputting to the
terminal screen, output from the given file descriptor
is redirected to the named file. e.g.
.PP
% prog 5> outputfile
.PP
2> Standard error redirection. The default is the terminal screen, but instead
standard error output is directed to the named file. e.g.
.PP
% program 2> errfile
.PP
[n]>>
.I filename.
As above but the named file is appended to, not erased first.
.PP
| Pipe output of one program to input of another. A series of programs joined
by pipes is called a
.I pipeline.
A pipe joins the standard output
of the first program to the standard input of the second program. e.g.
.PP
% prog1 | prog2
.PP
& background. When this symbol appears, it runs the previous command in
the background. A number indicating the process id and Wish job number
(job control only) is returned and the prompt is given immediately for the next
command.
.PP
; semicolon is used to separate pipelines. A semicolon can be typed instead
of a newline so that a new command or pipeline can begin. The effect is
exactly the same as a newline and thus it has the highest priority of these
metacharacters.
.".PP
."&& The double ampersand separates two commands (or command pipelines) and
."specifies that the latter should only be executed if the former returns a zero
."status (i.e. terminates successfully).
.".PP
."|| The double pipe is similar to the double ampersand except that it executes
."the latter if the former returns with
.".I non-zero
."status.
.PP
There are valid combinations of these, too, although it is illegal to
redirect output to two places, for example.
Some other characters also have special meaning and these may cause the words
to be interpreted in different ways. Ordinarily the words become the arguments
of an executing program.
." We don't have backslash as yet, sigh.
."To use any of these characters without invoking their
."special meaning, it is necessary to use the backslash character `\\' which
." causes the shell to ignore them and treat them as part of a word.
Most of
these "metacharacters" will also be ignored inside double quotes `"' and
all (except history substitution commands using !) are ignored inside single
quotes `''. (See the section titled "Quoting")
.PP
.SH Job Control
.PP
.PP
If job control is included, Wish
allows the user to have control over their processes or
.I jobs.
Whenever a process is run it is given a job number. This applies to
all background jobs and the current foreground one if it exists. This number
can then be used to identify a process for use with bg or fg.
The list of all currently active jobs can be displayed with the
.I jobs
command.
The
.I bg
and
.I fg
commands allow jobs to be placed into the background (where
the prompt is returned but the command still runs) or the foreground (where
the user is in an interactive state with the process). The stop signal can be
generated by typing `^Z' while a foreground job is running. This has the effect
of stopping that process immediately, which may be restarted with either the bg
or fg command.
." I don't think we have ^Y yet.
." A foreground process may also be stopped by typing `^Y' which
." delays stopping until the input buffer is emptied.
Note that bg and fg always
refer to the current (most recent) job
by default. For further explanation, see the "Builtin Commands" section below.
.PP
Background jobs are allowed to write to the terminal by default but not
read from it. (The shell places a job in the background by disassociating it
with the control terminal's process group. See
.I setpgrp(2)).
.PP
 The
.I stty(1)
"tostop" mode can be used if output from background jobs is to be disallowed.
This mode will cause background jobs to stop if a write to the terminal is
attempted. Reads from the background are always disallowed.
The user is notified of a processes status after return is pressed
and before the next prompt appears. This gives information about completion
or stoppage of background jobs.
.PP
Those people using
.B Minix
should use
.I stty(1)
to redefine their `quit' key from ^\\ to ^Z, as there is a primitive
form of job control available when
.I Wish
is compiled with #define V7JOB. This doesn't always work, but most of
the time you can ^Z, bg and fg.
.PP
Job control under Posix systems is still slightly broken; I haven't
had the time to sit down and grok the full implications of the Posix
rationale for job control.
.PP
.SH Command Line Editing (CLE).
.PP
.PP
This feature allows the use of control keys and meta-character key sequences
to perform editing operations upon the current line. This is done by using
"cbreak" terminal mode (see
.I stty(1)
) which causes an interrupt to be generated after each character is typed and
hence available for input, rather than just at the end of a line when return is
typed. Wish also turns echoing off so
that it can position the characters properly
and effect autowrap, control character representation, etc. These two modes
are reset when a foreground job is executed. Other modes are settable by the
user and not changed by Wish.
.PP
European users will be happy to know that the CLE is now 8-bit transparent.
For users with 7-bit keyboards, the CLE now has a command to produce
8-bit keystrokes. When 8-bit characters cannot be displayed on the screen,
turn the Wish variable `MSB' on; 8-bit characters will then be displayed
in bold.
.PP
The Command Line Editor provides many editing commands. The following
table lists the possible commands. The three columns hold: the default
key binding for the command, the internal key value for the command (usually
the same as the key binding), and a brief description of the action. Most
actions are obvious; the complex ones are described below.
.PP
.in +2
.nf
KEY     VALUE           ACTION
===     =====           ======
^@       \\000   save current position
^A       \\001   goto start of line
^B       \\002   go backwards one character
^C       \\003   interrupt - discard line and start again
^D       \\004   delete current char, list options, or exit shell
^E       \\005   goto end of line
^F       \\006   go forward one character
^G       \\017   kill whole line
^H       \\010   back-delete one character
^I TAB   \\011   complete current word
^J LF    \\012   finish line (same as return)
^K       \\013   kill from cursor to end of line
^L FF    \\014   clear screen and redisplay line
^M CR    \\015   finish line (same as linefeed)
^N       \\016   step forward in history
^O       \\017   toggle insert/overwrite mode
^P       \\020   step backward in history
^Q       \\021   resume tty output
^R       \\022   redisplay the line
^S       \\023   stop tty output
^T       \\024   transpose the current and previous chars
^U       \\025   accepts number and key to perform num times
^V       \\026   next character to be read literally
^W       \\027   delete word backwards
^X       \\030   goto saved position
^Y       \\031   yank the previous word into a buffer
^Z              suspend process (no effect upon CLE)
^[ ESC          prefix for meta-commands
^\\       \\034	 beep
^]       \\035   turn CLE mode bit on (see bindings below)
^^	    \\036   turn CLE mode bit off (see bindings below)
^_       \\037	 turn the following character's msb on
SPC../          insert that character
0..9            insert the character or read digit (^U)
:..~            insert that character
^? DEL          back-delete character (same as ^H)
ESC-^P   \\201   match previous partial command
ESC-b
ESC-B    \\202   skip backwards one word
ESC-d
ESC-D    \\203   delete word forwards
ESC-f
ESC-F    \\204   skip forward one word
ESC-h
ESC-H    \\205   get help on word
ESC-p
ESC-P    \\206   insert buffer onto the line
ESC-y
ESC-Y    \\207   yank the next word into a buffer
ESC-/    \\210   search forward for next typed character
ESC-?    \\211   search backwards for next typed character
.in -2
.fi
.PP
Note that some keys which normally generate signals (e.g. ^Z, ^Y, ^\\)
do not do so whilst a line is being entered/edited. Their signal generating
status is returned whenever a foreground job is in progress.
.PP
.SH Complex CLE Functions
.PP
.PP
Some of the CLE commands are complicated. Their full description is given here.
.PP
^D Delete char, list options, leave shell
.PP
This command has three functions. If you use the command at the beginning of
a line, you will exit the shell. If your cursor is on top of a character,
that character will be deleted. Finally, if your cursor is at the end of the
line, a list of `expansions' will be shown. This command will show files,
programs you can run, user's home directories and variables, depending upon
how you use it.
.PP
For example, to see a list of files in your current directory, you can do
.PP
.nf
    % word ^D
.fi
.PP
To see all the files starting with `s', do
.PP
.nf
    % word word s^D
.fi
.PP
The word can be a relative or absolute path name; to see the files starting
with `std' in the include directory, and the parent directory, do
.PP
.nf
    % word word /usr/include/std^D
    % word word ../std^D
.fi
.PP
If the word is the first on the line, aliases, builtins and programs in
your PATH are listed. To see all the commands, and all the commands starting
with `cl' do
.PP
.nf
    % <space>^D
    % cl^D
.fi
.PP
If the word begins with a `$', the command will give a list of variable names;
to see all the variable names, and all starting with `H', do
.PP
.nf
    % word word $^D
    % word word $H^D
.fi
.PP
If the word begins with a `~', the command gives a list of users. Examples:
.PP
.nf
    % word word ~^D
    % word word ~ro^D
.fi
.PP
Words with `/' signs in them will be expanded internally to their pathname
equivalents. The word `~#/' means the list of aliases and builtins.
For example, to list the files in fred's directory, in $HOME, and the
aliases and builtins, do
.PP
.nf
    % word word ~fred/^D
    % word word $HOME/^D
    % word word ~#/^D
.fi
.PP
<TAB>  Filename Completion
.PP
Filename completion is closely associated with the list options function. If
you use <TAB> instead of ^D, the word on the command line is completed up
to the first duplicate letter. For example, if you have two files 'bigfile.c'
and `bigfile.o' in the current directory, then
.PP
.nf
    % ls bi<TAB>             becomes
    % ls bigfile.
.fi
.PP
Words with only one match are fully completed. This function understands the
same words as the list options functions, so you can complete variables,
usernames, programs, and files in directories like
.PP
.nf
    % ls $HOME/.cl<TAB>      becomes
    % ls $HOME/.wishrc
.fi
.PP
Quoting Commands
.PP
The CLE provides commands to quote characters instead of them performing
functions. The first is ^V. For example, to insert a ^A on the line without
^A doing its usual action, do
.PP
.nf
   % word word ^V^A
.fi
.PP
The ^_ command works exactly the same, but also turns the character's
most significant bit on, so 8-bit keystrokes can be entered from a 7-bit
keyboard.
.PP
^U  Repeat Keystroke
.PP
^U is followed by a decimal number (which isn't displayed), and one character.
That character is repeated number times.
.PP
ESC-^P  Match Partial Command
.PP
This command only works for the first word. When the cursor is at the end of
the first word, and this command is done, the last command that matched
the word is added to the line. For example:
.PP
.nf
   % ls -l
   % wc file
   % echo hello
   % lESC-^P             becomes
   % ls -l
.fi
.PP
Word Commands
.PP
The word commands are affected by the Wish variable wordterm. This can be set
to holds the characters that `end' a word. If this variable isn't defined,
the CLE assumes it holds the following characters: space, tab, > < | / ; =
& and `.
.PP
.SH Bindings
.PP
.PP
Wish now distinguishes between the commands available in the CLE and the
keystrokes that cause the commands to occur. This allows Wish to have
different editor bindings. For example, Wish defaults to an emacs-like
binding, as noted above, but can be given vi-like bindings by sourcing
the file Vibind.
.PP
The command characters given in the above table are used by the Wish
.B bind
builtin command, which can bind a keystroke sequence to another keystroke
sequence. With the default emacs-like style, most of the emacs keys are
identical to the internal CLE commands, and only the ESC commands are
internally bound by Wish. Setting up another editor style involves the use
of the `bind' and `unbind' builtins described in the Builtin Commands
section.
.PP
The CLE also offers a boolean `mode'.
When the mode is on, all current key bindings apply. However, when the mode
is off, key bindings marked as `mode only' are disabled. This allows the
vi-style set of bindings to be used.
.PP
The help command calls the system manual command with the word immediately
preceding the cursor as the argument.
.PP
." The next history command (^N) will attempt to predict the user's next command
." if it reaches the end of the history list (i.e. when nothing has been typed
." on the current line, yet) by looking for the most recent occurrence of
." the previous command (not
." including the previous command itself) and then loading the command
." .I after
." that onto the command line. This may be set up to occur automatically by
." setting the TARDIS variable (see Special Variables section). This enables
." the user to set up a loop of commands. A beep sounds if the shell is unable to
." predict what command is next (if the previous command never occurred before,
." for instance).
." .PP
." .nf
." % comm1
." % comm2
." % comm3
." % comm1
." % ^N    -->  % comm2
." .fi
." .PP
.SH Variables.
.PP
.PP
The shell variable facility relies on the following metacharacters:
.PP
$ denoting a variable. Wish will interpret $ as a reference to a
shell variable.
The word immediately adjacent to the `$' is taken 
as the variable name. This word is delimited by any of the metacharacters above,
by quotes, by another `$', or by an array subscript. e.g.
.PP
% echo $fred
.PP
This will echo the contents of the variable fred.
." , while this:
." .PP
." % echo $fred[2]
." .PP
." will echo the contents of the third field of fred. Note that array subscripts
." start from zero. Any access to a nonexistent field will return a null word.
.PP
= assignment of variables. Variables are assigned simply by stating the name(s)
of the variable(s), followed by the equal `=' sign, and then the value(s).
." It is possible to assign two or more variables simultaneously. e.g.
." .PP
." % a b = x y
." .PP
." This statement assigns 'x' to $a and 'y' to $b. Any left-over words on the
." right hand side of the equals are assigned to the last variable listed on
." the left hand side. Similarly, any left-over variable names (on LHS) are
." assigned null values. Note, however, that this assignment is not truly
." concurrent (at this stage). (i.e. assignment is done left to right, not as
." if it all happened simultaneously. Therefore, variable swapping is not
." possible with only two variables. "a b=$b $a" would set both "a" and "b" to the
." original value of "b".
." .PP
." += appending to a variable. Variables can also be appended to with the `+='
." operator. This works in the same way as the `=' but does not destroy the old
." value of the variable. To append to a variable, the variable must have existed
." previously.
.PP
Wish is responsible for two separate sets of variables. The examples given
above refer to
.I internal
shell variables. These variables are immediately
accessible to the user and also act as flags for some Wish functions
(e.g. HASH,ignoreeof,UNIV). The other set is known as
.I environment
variables.
Environment variables are passed to any new process by the execve system call.
When the shell is started all environment variables are loaded into the
shell's internal variable storage space.
When this initialisation occurs,
these variables are automatically marked for
.I export
, meaning that when execution of a command
takes place, the variables marked for export form the environment for
the executed command.
It is also possible to mark any variable for export by
using the
.I export
command described below.
.PP
." While there is no
." .I set
." command, variables can be deleted using the
." .I unset
." command. Also
." .I unsetenv
." can be used to delete environment variables. If you
." don't want all of the environment variables cluttering up your internal
." variable list, there is no harm in deleting them but they will not be
." re-exported. To reload the environment variables, simply use the command:
." .PP
." e.g. % PATH=$PATH
." .PP
." since the environment is searched for the variable if it does not exist in the
." internal variable list.
." .PP
." A new feature found only in Wish is the
." .I edit
." command which allows the user
." to edit a variable. This is particularly handy for long variables such as PATH
." and TERMCAP in which only a small component has to be altered. Editing is done
." using the command line editor features described above.
.PP
Wish also has several inbuilt variables:
.PP
.nf
      $$ - The process-id of the process
      $? - The exit status of the last process
      $# - The number of arguments to the process
      $n - (where n is a digit) The n'th argument to the process.
      $* - All the arguments to the process (except $0)
.fi
.PP
When used interactively, these refer to the Wish itself. However, in
aliases and Wish scripts, these variables refer to the alias or script.
For example, if `prog' is a Wish script, and you type
.PP
% prog foo bar
.PP
then from within "prog", $0 will be "prog", $1 will be "foo" and $2 will be
"bar". Similarly, an alias such as
.PP
% alias l '/bin/ls -lg $*'
.PP
will do '/bin/ls -lg' on all of its arguments.
.PP
.SH History and History Substitution.
.PP
.PP
The history mechanism enables every command typed to be stored and accessed
at a later time. To get a list of previous commands use the
.I history
command. Wish does not enter the
.I history
command into the history, nor does
it store single letter lines since these can be more easily retyped than
by using history substitution (described below). The number of commands
remembered by Wish is set by the internal "history" variable.
.PP
It is possible to recall a previous line(s) from the history by using the
metacharacter `!', followed by the absolute history number, a relative history
number, a string which matches the most recent command starting with the
same string, or another `!' which refers to the previous command. e.g.
.PP
% !12 fred
.PP
This will substitute the history event numbered 12 onto the line at that
position and the argument "fred" will remain as an argument to that command.
.PP
% !-3
.PP
This will substitute the event from three commands ago. It is still possible
to have an argument or a different substitution on the same line.
.PP
% !fo
.PP
This will substitute the most recent command starting with "fo".
.PP
You can also append characters onto a previous history command rather than
adding a new word. If we issued a command "ls -l" and then used "!la" the
shell would search for a command starting with "la" instead of finding "ls -l"
and appending the "a". So, to achieve this, simply quote the characters that
are to be appended (actually you only need quote the first). Hence,
.br
% !l"a"
.br
will give the desired substitution of "ls -la". Spaces may be included in
the string by using backslash or quotes.
.br
% !"v d"

." .PP
." Another form of substitution exists which operates only on the previous
." line as an error correction mechanism. Although this is mostly superseded by
." the history pullback (^P) and command line editor operations, it was included
." for completeness. If a line starts with a `^' and has the format:
." .PP
." % ^string1^string2
." .PP
." then string1 is replaced by string2 in the previous command line.
." .PP
." .B Command Substitution.
." .PP
." The
." .I output
." of a command can be substituted into the command line by using
." .I backquotes
." to surround the command. e.g.
." .PP
." % d=`date`
." .PP
." This command causes the system program
." .I date(1)
." to be executed, and the resulting output is loaded (word by word) into the
." line to be parsed.
." .I Words
." are sequences of non-blank characters. A
." .I blank
." is a space, tab or newline. The "blank" characters are all interpreted as
." spaces when loaded into the line, however, the output of a program in
." backquotes can be preserved as a single argument by surrounding the backquotes
." with double quotes (""). Single quotes would cause the backquotes to remain
." uninterpreted, however.
.PP
.SH Filename Substitution.
.PP
.PP
Rather than type a lot of filenames, sometimes it is possible to specify
a group of files using the "globbing" technique. This means that using the
following metacharacters, you can specify files in any directory using a
sort of shorthand notation. The shell then performs what is called "pattern
matching" to produce the file names wanted.
.PP
* represents any number of characters including zero.
.br
? represents any one character.
.br
[] encloses a list of characters that can match. Two characters may also
have a hyphen separating them meaning "up to". So [A-Za-z] includes the
letters 'A' to 'Z' and 'a' to 'z'.
.PP
e.g. ab* matches all files in current directory starting with "ab".
.br
/usr/bin/lo*.? matches all files in /usr/bin starting with "lo" then
ending with a "." and any other character.
.br
[abc]*.c matches all the C-files in the current directory starting with
an "a", "b", or "c".
.PP
It is also possible to specify
.I subdirectories
using pattern matching e.g.
*/*.c matches all C-files in any subdirectory.
.PP
A further abbreviation method is available for referring to the home
directories of users. This is done by typing a `~' followed by a
.I username
or if no username is given then the home directory of the user running
the shell is used (i.e. ~ refers to your directory).
.br
e.g. "~cgibson" will be replaced by "/u1/hons/cgibson"
.br
"~mike/work" will be replaced by "/u1/staff/mike/work".
.PP
Wish now fully copes with systems that uses yypasswd.
." .PP
." It is also possible to abbreviate by using the {} metacharacters. Actually,
." only the `{' is recognised as a metacharacter. This can be achieved by
." placing a set of strings separated by commas `,' inside the curly braces.
." Note that the shell blindly expands without checking for existence of files
." (no matching done) and thus this command can refer to words other than
." filenames.
." .PP
." e.g. % ls {a,b,c}*.c
." .PP
." This will once again give the listing of C-files in the current directory
." but the difference is that before pattern matching takes place, the line
." is expanded to: "ls a*.c b*.c c*.c" and hence is less efficient since the
." shell must match 3 separate patterns. The following example shows another
." use of the curly brace expansion.
." .PP
." % echo {0,1}{0,1}{0,1}{0,1}{0,1}{0,1}{0,1}
." .PP
." This will echo the representation of the binary numbers from 0 to 127
." because left to right order is preserved.
." .PP
." So, it is possible to combine any number of these metacharacters *,?,[],{},~
." to save
." typing, but note that the more complicated the pattern, the longer the shell
." will take to expand it, particularly if many subdirectory levels are involved.
." Also, if many different sets of braces are given in the one word then the
." expansion becomes large and may not even fit on a line. For example, the above
." example using {0,1} has two options in each set of braces and seven sets of
." braces, resulting in two to the power of eight (128) different words.
.PP
.SH Quoting.
.PP
.PP
There are three types of quotes (not including backquotes) which may be used to
stop the above mentioned metacharacters from being interpreted.
." The most
." powerful is the
." .I backslash
." `\\' which only quotes the next character.
." This will quote absolutely anything except
." .I null
." (^@) which may not be quoted since it represents the end of a line
." (null-terminated strings. See
." .I string(3)
." ) and can't be typed, anyway. The next set of quotes, called
Single quotes '' will stop all interpretation except history
substitution (!).
." It is necessary to use the backslash here if the actual
." exclamation mark character is intended.
The last set of quotes is double quotes.
They allow history and variable substitutions but disallow pattern matching.
Note that quotes may be nested but only the outer set have any effect.
." The exception is the use of the
." .I backslash
." when quoting the `!' and when quoting
." the `$' and ``' in double quotes.
.PP
.SH Aliases - User defined Command Sequences
.PP
.PP
It is possible for the user to define their own set of aliases. Unlike
.I Csh(1)
where the first word of any command is checked to see if it's an
alias, and then the alias is substituted onto the line, Wish allows full
shell scripts to be defined which are then executed by the shell after
parsing the line. These aliases are stored internally and executed in
a similar way as a Wish script. Thus it is possible to pass arguments as
$1 $2 etc. or $* for all arguments. In fact, the aliases should behave exactly
as a script stored in the user's directory but without taking up directory
space. Remember that the more aliases that exist, the more slowly commands
will run.
." This problem is greatly reduced in Wish compared with
." .I Csh
." if
." .I hashing
." is on, because the builtins and aliases are hashed from the path along
." with all the other commands and so access to them is still quick via this
." method. The only reason for not using hashing would be if the hash table got
." too full (too many files on your PATH) or process space was needed on the
." system.
." Also using the `~#' metanotation, you can specify explicitly to use a builtin
." or alias if they occur lower down in the path then a system command of the same
." name. e.g.
." .PP
." % ~#/list
." .PP
." This will execute the builtin command
." .I list
." rather than the system command
." .I list(1).
Because of the way that these aliases are implemented, it is easy to
call builtins, system commands, and even other aliases, from within an alias.
You should be careful not to call an alias from within itself since this will
cause an error. Aliases can be created using the builtin command
.I alias
, see below in Builtin Commands section.
.PP
.SH Builtin Commands
.PP
.PP
This section describes the commands that are builtin to the shell.
Builtins are only executed in a subshell if they are at the start or in the
middle of a pipeline or run in the background.
." They exist in a virtual directory
." ~# and can be referred to in this way. See the Aliases section above for more
." information.
.PP
This section is divided up into several subsections, because some
builtins can be left out if they cannot be used or are not needed.
To find out which builtins you have, do
.PP
.nf
     % ~#/^D
.fi
.PP
This gives you a list of aliases and builtins.
.PP
.B 1. Standard Builtins
.PP
.TP 4
." alias [ [ -e -l -s] <alias-name> [ <alias-defn> ] ]
alias [ [ -e -s] <alias-name> [ <alias-defn> ] ]
When invoked with no argument this command lists the aliases
currently defined.
." If the -e flag is given then an editor (default is .I vi(1))
." is invoked upon a temporary file and this allows the user to create their
." Wish script which is loaded into the shell as an alias upon completion. This
." is needed if
." multi-lined aliases are to be defined since Wish's command line editor will not
." allow newlines to be displayed and inserted.
The -l flag instructs Wish to load
up the alias from the named file (the alias is given the same name). In the same
way -s causes Wish to save the alias to file. If none
of these flags appear, then the first argument is taken as the alias name, and
the remaining arguments as the alias definition. If there are no other
arguments, that alias's definition is printed.
.TP 4
bind [[-m] keysequence [newkeysequence]]
This command allows the user to bind a series of keystrokes to a new series
of keystrokes. Note that most ASCII control characters (\\000 to \\037), and
some other control characters (\\201 to \\211) cause the CLE to perform a
function, so most bindings are to one or more of these characters.
With no arguments,
bind
shows the current set of key bindings. With a keysequence argument,
the binding of
the given sequence is shown. With a keysequence and a value, the sequence is
bound to the action. For example:
.PP
    % bind '^[[A' ^P
.PP
binds the up-arrow key to the `step backward in history' action.
Bind
also understands C-like octal sequences, so that
.PP
    % bind '^[[A' \001hello\005
.PP
will cause the up-arrow to insert the word `hello' at the beginning of the
line.
.PP
If a keysequence is bound using the `-m' option, then the binding is marked
`mode-only', and is only invoked when the CLE is in MODEON mode. Otherwise
it is ignored.
.PP
Note that the CLE uses the first match found when parsing keyboard input.
For example, if the current bindings are:
.PP
   % bind
   ^[[A   is bound to ^P
   ^[[B   is bound to ^N
   ^[[Aa  is bound to ^A
   ^[[B   is bound to ^E
.PP
then the keyboard input `^[[Aa' will do the command ^P (i.e. go back in
history), and then
add an `a' to the command line; similarly, the input '^[[B' will do the
command ^E (i.e. go to the end of the line).
.PP
Key bindings can be recursive. A large error will occur if you do
.PP
    % bind a aa
.PP
as the next time you type `a', it will become an infinite number of a's,
up to the size of the CLE line buffer.
.TP 4
cd [ <directory> ] or chdir [ <directory> ]
This command changes the current working directory of the
current shell process. Thus, if cd occurs inside a child process, when that
process dies, the parent will not have changed directory. Cd takes a single
argument which is the name of the directory to change to. The directory name
may be a full path name starting from the root directory or a path name
relative to the current directory. The `~' notation can also be used to refer
to the home directory of users.
.TP 4
echo [-n] <arguments>
Echo writes the given arguments out on a single line on standard output. A
newline is appended unless the -n option is given.
." .TP 4
." edit <variable>
." Edit takes one argument which is a variable name that is
." to be edited. The editor used is the command line editor. After completion of
." the editing sequence, press return and the variable is saved. Note variables
." must exist prior to editing them. (See also Command Line Editing section).
." .TP 4
." exec <program-name> [ <argument-list> ]
." Replace the current shell with a different program which is
." supplied as arguments. All arguments are passed to
." .I execve(2).
." .TP 4
." exit [ <status> ]
." Causes the termination of the shell (unless it is the
." login shell). If a numerical argument is given then the shell exits
." with that argument. Default status is zero.
.TP 4
export [ [ - ] <variable> ]
mark a variable for export, or if `-' flag given, unmark
export ability for that variable. When a variable is marked for export, it
means that the internal copy of the variable is transferred to the environment
whenever a program is executed, making the new copy available to the new
process.
.TP 4
history [ m[-n ] ]
Lists the previously executed commands remembered by the shell. An optional
range can be specified which will list the commands numbered from
.I m
to
.I n.
A single number argument will display the m most recent commands.
.TP 4
list [ env ]
List the internal variables defined in Wish. With the "env" option, the
environment variables are listed (like
.I printenv(1)).
." .TP 4
." load <line>
." Load the arguments supplied onto the next command line. Can be used to
." load up alias definitions for example if they need to be slightly altered,
." or to check a history substitution (or other substitution) without automatically
." executing the line. Allows a second carriage return for confirmation.
." .TP 4
." lock
." Lock the keyboard. User is prompted for a password which must be retyped
." to confirm it. The shell enters "lock" mode and this mode can only be exited by
." once again typing the correct password.
." .TP 4
." logout
." The command used to logout from the shell and hangup from
." the system. This command only has effect from the lowest shell level (i.e.
." the login shell).
." .TP 4
." repeat <number> <command>
." This statement causes the command to be executed the number of times specified.
.TP 4
source <script-name> [ <argument-list> ]
Cause the Wish script to be executed in the current shell, as if the contents
of the file had been typed from the keyboard. This is useful if the environment
variables are to be set for the shell. (e.g. if you have a script that changes
your TERMCAP or PRINTER variable etc.) This script could also be loaded as an
alias with "alias -l".
.TP 4
tilde [-l], tilde [[shorthand] [dirname]]
Wish keeps an internal list of shorthand names for directories. Tilde adds
the given shorthand/dirname pair to the list. From then on, you can use
~shorthand to mean the dirname. If just shorthand is given, the expansion
for that shorthand is given. With no arguments, all the internal pairs are
given. The -l argument will also show the username/directory pairs from
the /etc/passwd file.
.TP 4
unalias <alias-name> ...
Delete the named alias if it exists. If not an error message is returned.
.TP 4
unbind keysequence
Unbinds the given key sequence, if that sequence has been bound.
.TP 4
untilde <shorthand>
Removes the shorthand name/directory pair from the internal list of
directory shorthands.
unset <variable> ...
Delete the internal variable named. If an environment variable of the same
name exists, it will be deleted.
.TP 4
unsetenv <variable> ...
Delete the environment variable named. This command should be used with
caution since some system programs use the environment variables. The
shell with still have an non-exported copy of the variable.
." .TP 4
." which [ -o ] [ -O ] <name>
." In its basic form, tells you the full pathname of the file you execute by
." typing <name>. With -o, aliases are ignored and with -O aliases and builtins
." are ignored.
.PP
.B 2. Job Control Builtins
.PP
.TP 4
bg [ %<job-no> ] [ <pid> ]
This command has the effect of starting the current job (which will be
stopped) in the background if given no arguments. If given arguments it starts
the specified jobs in the background. The arguments are simply the job numbers
which have been assigned by the shell. These can be obtained using the
.I jobs
command.
.TP 4
fg [ %<job-no> ] [ <pid> ]
Cause the current job to execute in the foreground
whether stopped or executing in the background. If an argument is supplied
it is taken as the job number (as described in
.I bg
and Job Control section).
.TP 4
jobs
Give a list of current jobs (processes) recognised by
Wish. These jobs are the either running in the background or are stopped.
The shell cannot keep track of processes that result from a
.I fork(2).
In the att universe, job control is severely restricted. (See Job Control
section).
." .TP 4
." kill [ -<signal> ] %<jobno> [ or <pid> ] ...
." Kill the specified job number or process id (pid) with the optionally
." specified
." .I signal.
." Signals are the same as those defined in
." .I sigvec(2)
." (ucb) or
." .I signal(2)
." (att) but with the prefix "SIG" removed. If the signal is unspecified, then
." the SIGTERM signal is sent. If the SIGTERM and SIGHUP signals are sent the
." the shell will send the
." SIGCONT signal as well, which is needed if a stopped job is to receive the
." signal.
." .TP 4
." stop [ %<job-no> <pid> ... ]
." Stop the current (or named) job that is running in the background. Jobs may be
." restarted using either
." .I bg
." or
." .I fg.
." .TP 4
." suspend
." Cause this shell to suspend or stop. Control is returned to an outer level
." but the shell will remain in a stopped state (receives a stop signal, like
." typing ^Z).
." .TP 4
." wait
." Cause the shell to wait for all background processes. The wait can be
." interrupted (^C) at which stage the shell prints a list of all jobs
." currently outstanding.
." .PP
." .B 3. Hashing Builtins
." .PP
." .TP 4
." hashstat [ x ]
." Display an analysis of the internal hashing table.
." With no arguments, hashstat returns the actual hits and misses whenever the
." shell tries to execute a program and also a hit ratio. With the single argument
." "x", Wish returns the hits and misses encountered when creating the hash
." table, an expected hit ratio, and how full the table is.
." .TP 4
." rehash
." This causes the files on the user's path to be entered into a hash table.
." If hashing was previously turned off, then it is turned on with this command.
." You will need to do this if a new executable file appears in a directory on
." your path.
." .PP
." .B 4. Non-Minix Builtins
." .PP
." .TP 4
." help [ <error-number> ]
." Help gives some additional information about the previous error that occurred,
." or if supplied with an internal error number, explains that particular error
." in more detail.
." .PP
." .B 5. Shell Script Builtins
." .PP
." Currently case, for, if, repeat, shift, and while exist as stub builtins, i.e
." they do nothing. It is planned to give Wish a scripting language as close
." as possible to sh(1). The only written shell script builtin is read.
." .PP
." .TP 4
." read <variable> [ <variable> ... ]
." Read from the standard input until an end of line is reached. The characters
." that were read are assigned to the <variable> named. If more than one variable
." is to be read, the first word of the input is assigned to the first variable;
." the second to the second respectively; the final variable is assigned any left
." text. If there are more variables than words read, remaining variables are
." assigned null strings. Note this is logically equivalent to variable assignment.
." (see Variables.)
.PP
.SH Special Variables.
.PP
.PP
This section contains a description of the variables which are used by the
shell to indicate certain conditions or which actually affect the running
of the shell in some way. The user may set these variables to suit their own
personal tastes.
.TP 4
KEEPSTTY
This variable affects the state of the terminal under Wish. If not defined,
you can change the terminal state using stty, and Wish will use the new
state, with the exception of cbreak/cooked modes. with KEEPSTTY, Wish will
always reset the terminal to the initial state. This is useful if you have
a program with sometimes crashes, leaving the terminal in nl state or even
changing the speed. Defining this variable will set the terminal back to a
sane state when you return to Wish.
.TP 4
MSB
This variable affects the way characters with their msb on are printed.
Wish defaults to printing characters as they are. However, if MSB is set to
any value, characters above 127 are AND-ed back to 0-127, and are printed
in bold.
." HASH
." If HASH exists, hashing for the path will be turned on. If unset then
." hashing is turned off. HASH needs no value.
." .TP 4
." TARDIS
." Cause the shell to automatically try to predict what command the user wants
." typed every time the prompt is returned. This is the same as typing ^N every
." time the prompt appears.
." .TP 4
." UNIV
." For dual universe implementations of Unix, such as Pyramid OSx.
." Can be set to either "att" or "ucb"
." and is probably extremely machine dependent. Contact your system administrator
." regarding this.
.TP 4
beep
This variable can be set to any string of characters
which will be output whenever a bell would normally sound. By default,
this value is ASCII 7 (^G). If set to null, the shell will give no warning
bell. (For terminals with inverse video, "^[[?5h^[[?5l" will produce a
flash).
.TP 4
cwd
Holds the value of the current working directory. This variable is automatically
updated by the shell whenever a cd or chdir occurs. It generally should not be
changed by the user.
.TP 4
." history
." The number of commands the shell is to remember for use with the
." .I history
." command. If set to zero, then of course no commands are remembered. The larger
." history's value, the more space is occupied by the shell process since it must
." store all the previous commands. History is set to 25 by default.
." .TP 4
." ignoreeof
." If set, ignoreeof causes the shell to ignore the ^D key as a means of exiting
." the shell. Ignoreeof needs no value.
." .TP 4
." nohistdup
." If set, successive identical lines are not saved in the history.
." .TP 4
prompt
.in -4
.br
." prompt2
." .in +4
." .br
Prompt contains the format description for the user prompt. By default
this is a `%' sign. Prompt2 contains the format description for the second
prompt, given when more information is needed, like in a while, case, or if
statement being executed in interactive mode (i.e. from keyboard).
The format description is simply a string of characters which are displayed,
except that some special attributes may be entered using the `%' metacharacter.
.br
.in +4
%% a percent `%' sign.
.br
%d current working directory.
.br
%!
.br
%h current history number.
.br
%S start standout mode (see
.I termcap(4)).
.br
%s stop standout mode.
.br
%@
.br
%t time in 24-hour format with seconds.
.br
.in -4
If any other character follows the `%' then the `%' and then that character is
printed. Other characters are just printed. e.g. "%d[%h] " could produce
"/u1/staff/mike/work[58]" as the prompt.
This prompt format is similar to that used by
.I tcsh(1).
." .TP 4
." umask
." Contains an octal number representing the umask for files when created by
." the shell. The default file creation mode can be found by OR-ing this value
." with octal 0777. See
." .I chmod(1)
." and
." .I umask(2).
." .TP 4
." wid
." Sets the terminal width. Tells the command line editor when to effect a line
." wrap so that the cursor stays in the right place.
.TP 4
wordterm
This variable holds all the characters that delimit words, and is used in
the word-oriented CLE functions. If undefined, the CLE will use the
characters SPACE, TAB, > < | / ; = & and `.
.SH AUTHORS
Callum Gibson- Honours project 1988.
Warren Toomey.
.SH FILES
.nf
~/.wishrc          Read at beginning of execution of Wish.
." ~/.history              Saved commands from history list.
." ~/.login                Read only by login shell, after .wishrc.
." ~/.logout               Read at logout time.
." /etc/passwd             source of directories for ~name.
." /usr/local/man          help manual called from CLE, defined at
."                         compile time.
." /usr/local/lib/wish.hlp Text for help builtin command.
.fi
.SH LIMITATIONS
Maximum line length is 2048 (but redefinable). Maximum number of arguments
is 512. Not all of the above features exist, yet, and some new ones may be added
in future versions.
.SH SEE ALSO
chmod(1), csh(1), sh(1), stty(1), tcsh(1), exec(2), fork(2), pipe(2), umask(2),
wait(2), string(3), signal(3), termcap(4), tty(4), environ(7).
