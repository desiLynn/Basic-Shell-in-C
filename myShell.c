// Simple Linux Shell in C By Desiree Wood
// Included functionality: Move directory(simulated)
//    Show Current location(simulated)
//    save and display history across sessions
//    replay a command from history
//    Start a program
//    Start a background process
//    End a process

#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int movetodir(char **arguments);
int whereami(char **arguments);
int history(char **arguments);
int byebye(char **arguments);
int replay(char **arguments);
int start(char **arguments);
int background(char **arguments);
int dalek(char **arguments);
char **parseLine(char *line);
int execute(char **arguments);

char *builtinName[] = {
  "movetodir",
  "whereami",
  "history",
  "byebye",
  "replay",
  "start",
  "background",
  "dalek"
};

char currentDir[256];
char **historyArr;
int historyIndx = 0;
int histBuff = 64;

int (*builtinFunction[]) (char **) = {
  &movetodir,
  &whereami,
  &history,
  &byebye,
  &replay,
  &start,
  &background,
  &dalek
};

int numBuiltins() {
  return sizeof(builtinName) / sizeof(char *);
}

void addHistory(char *input)
{
  int i;
  if (historyArr == NULL)
  {
    historyArr = malloc(histBuff * sizeof(char *));
  }
  if (historyIndx >= histBuff)
  {
    histBuff += 64;
    historyArr = realloc(historyArr, histBuff * sizeof(char*));
  }
  historyArr[historyIndx] = malloc(strlen(input) * sizeof(char));
  strcpy(historyArr[historyIndx], input);
  historyIndx++;
}

void loadHistory()
{
  char buffer[64];
  if (historyArr == NULL)
  {
    historyArr = malloc(histBuff * sizeof(char *));
  }
  FILE *fp = fopen("shellHistory.txt", "r");
  if (!fp)
  {
    perror("Fail to load history");
    exit(-1);
  }
  while (fgets(buffer, 64, fp))
  {
    historyArr[historyIndx] = malloc(strlen(buffer) * sizeof(char));
    strcpy(historyArr[historyIndx], buffer);
    historyIndx++;
    if (historyIndx >= histBuff)
    {
      histBuff += 64;
      historyArr = realloc(historyArr, histBuff * sizeof(char*));
    }
  }
  fclose(fp);
}

void saveHistory()
{
  int i;
  FILE *fp = fopen("shellHistory.txt", "w+");
  for (i = 0; i < historyIndx; i++)
  {
    fputs(historyArr[i], fp);
  }

  fclose(fp);
}

// simulate moving by changing interal currentDir variable
int movetodir(char **arguments)
{
  DIR *dir = NULL;
  if (arguments[1] == NULL)
  {
    fprintf(stderr, "Shell: expected argument\n");
  }
  else
  {
    dir = opendir(arguments[1]);
    if (dir)
    {
      strcpy(currentDir, arguments[1]);
      closedir(dir);
    }
    else
    {
      fprintf(stderr, "Shell: failed to move\n");
    }
  }
  return 1;
}

// print currentDir to show current location
int whereami (char **arguments)
{
  printf("current working directory is: %s\n", currentDir);

  return 1;
}

// prtsint history in reverse order
// 0 most recent
// -c clears history and returns
int history(char **arguments)
{
  int i;
  if (historyArr == NULL)
  {
    printf("History Empty\n");
    return 1;
  }
  if (arguments[1] != NULL && strcmp(arguments[1], "-c") == 0)
  {
    free(historyArr);
    historyIndx = 0;
    printf("History cleared\n");
    return 1;
  }
  for (i = historyIndx -1; i >= 0; i--)
  {
    printf("%d: %s", historyIndx-1 - i, historyArr[i]);
  }
  return 1;
}

// exit program & call saveHistory
int byebye(char **arguments)
{
  saveHistory();
	return 0;
}

int replay(char **arguments)
{
  char **args;
  char *input;

  if (arguments[1] == NULL)
  {
    fprintf(stderr, "Shell: expected argument\n");
    return 1;
  }

  input = historyArr[historyIndx-2 - atoi(arguments[1])];

  printf(" replaying : %s", input);

  args = parseLine(input);

  return execute(args);
}

// starts a program
int start(char **arguments)
{
	pid_t pid;
	int status;

	pid = fork();
	if (pid == 0)
	{
		if (execvp(arguments[1], arguments) == -1)
			perror("shell");
		exit(-1);
	}
	else if (pid < 0)
	{
		perror("shell"); // fork error
	}
	else
	{
		do
		{
			waitpid(pid, &status, 0);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	return 1;
}

// starts a process in background return PID
int background(char **arguments)
{
	pid_t pid;
	int status;

	pid = fork();
	if (pid == 0)
	{
		if (execvp(arguments[1], arguments) == -1)
			perror("shell");
		exit(-1);
	}
	else if (pid < 0)
	{
		perror("shell"); // fork error
	}
	else
	{
    printf("Started: %d\n", pid);
	}
	return 1;
}
// kills a procress
int dalek(char **arguments)
{
  if (arguments[1] == NULL)
  {
    fprintf(stderr, "Shell: expected argument\n");
  }
  else
  {
    if ((kill(atoi(arguments[1]), SIGKILL)) == 0)
    {
      printf("EXTERMINATED!\n");
    }
    else
    {
      printf("ERROR!\n");
    }
  }
  return 1;
}

// executes a built in commands by going through list
// if a command matches then it gets called
int execute(char **arguments)
{
	int i;

	if (arguments[0] == NULL)
		return 1;

	for (i = 0; i < numBuiltins(); i++)
	{
		if (strcmp(arguments[0], builtinName[i]) == 0)
			return (*builtinFunction[i])(arguments);
	}

	return 1;
}

// reads in a line terminated by \n
// buffer is set by stdin length
char *readLine() // TODO: Add history
{
	char *input = NULL;
  ssize_t buffer = 0;
  if (getline(&input, &buffer, stdin) == -1)
  {
    if (feof(stdin))
    {
      exit(0);
    }
    else
    {
      perror("shell: getline\n");
      exit(-1);
    }
  }
  addHistory(input);
  return input;
}

// parses input into useable tokens
// return passed to execute()
char **parseLine(char *line)
{
	int buffer = 64;
	int position = 0;
	char **tokens = malloc(buffer * sizeof(char*));
	char *token;
	char **tokenBackup;

	token = strtok(line, " \n");
	while (token != NULL)
	{
		tokens[position] = token;
		position++;

		if (position >= buffer)
		{
			buffer += 64;
			tokenBackup = tokens;
			tokens = realloc(tokens, buffer *sizeof(char*));
			if (!token)
			{
        free(tokenBackup);
				printf("error creating tokens\n");
				exit(-1);
			}
		}
		token = strtok(NULL, " \n");
	}
	tokens[position] = NULL;
	return tokens;
}

// shell loop functions in 3 main parts
// reads in line -> parses into tokens-> execute commands based on tokens
void shellLoop ()
{
	char *input;
	char **arguments;
	int status;

	do
	{
		printf("User: %s# ", currentDir); // idk if should keep path
		input = readLine();
		arguments = parseLine(input);
		status = execute(arguments);

		free(input);
		free(arguments);
	} while (status);
}

int main (int argc, char **argv)
{
  if (getcwd(currentDir, sizeof(currentDir)) == NULL) // set currentDir
    perror("getcwd() error");
  loadHistory(); // load history from .txt
	shellLoop(); // runs base shell loop

	return EXIT_SUCCESS;
}
