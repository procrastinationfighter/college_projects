#include "userInterface.h"

enum CommandType {
    ADD, CHECK, DEL, PRINT, INCORRECT
};
typedef enum CommandType CommandType;

/* Returns true if given character can be part of being name. */
bool isValidCharacter(int ch) {
    return (ch != EOF && (unsigned char) ch > 32);
}

/* Returns true if character is different from '\n' and EOF characters. */
bool isNotEndlineOrEOF(int ch) {
    return (ch != '\n' && ch != EOF);
}

/* Skips all spaces in currect line.
 * Returns true if there are no more characters different from spaces or EOF. */
bool isCurrentLineFinished(bool *isEOF) {
    int ch = getchar();
    while (isspace(ch) && isNotEndlineOrEOF(ch)) {
        ch = getchar();
    }
    if (isNotEndlineOrEOF(ch)) {
        //If while loop ended because ch was not EOF or '\n', put that character back on stdin.
        ungetc(ch, stdin);
        return false;
    }
    else {
        if (ch == EOF) {
            *isEOF = true;
        }
        return true;
    }
}

/* Returns true if line ended with '\n'.
 * Returns false if line ended with EOF. */
bool skipInputLine() {
    int currentCharacter = getchar();
    while (isNotEndlineOrEOF(currentCharacter)) {
        currentCharacter = getchar();
    }

    if (currentCharacter == '\n') {
        return true;
    }
    else {
        return false;
    }
}

/* Returns true if parameters for check command are valid. */
bool isCheckPossible(Instruction *instruction) {
    if (!strcmp(instruction->animal.string, "*")) {
        return false;
    }
    else if (isStringEmpty(&instruction->animal)) {
        if (!strcmp(instruction->tree.string, "*")) {
            return false;
        }
        else if (isStringEmpty(&instruction->tree)) {
            return (strcmp(instruction->forest.string, "*") != 0);
        }
    }
    return true;
}

/* Returns command type located in ins->command.
 * Returns INCORRECT if command name or parameters are invalid. */
CommandType checkCommandType(Instruction *ins) {
    if (strcmp(ins->command.string, "ADD") == 0 && !isStringEmpty(&ins->forest)) {
        return ADD;
    }
    else if (strcmp(ins->command.string, "CHECK") == 0 && !isStringEmpty(&ins->forest) && isCheckPossible(ins)) {
        return CHECK;
    }
    else if (strcmp(ins->command.string, "DEL") == 0) {
        return DEL;
    }
    else if (strcmp(ins->command.string, "PRINT") == 0 && isStringEmpty(&ins->animal)) {
        return PRINT;
    }
    else {
        return INCORRECT;
    }
}

void handleInstructionError() {
    fprintf(stderr, "ERROR\n");
}

/* Executes instruction. */
void executeInstruction(World *world, Instruction *instruction, bool areParametersUsed[]) {
    CommandType type = checkCommandType(instruction);
    switch (type) {
        case ADD:
            addBeings(world, instruction, areParametersUsed);
            break;
        case CHECK:
            checkExistanceOfBeings(world, instruction);
            break;
        case DEL:
            deleteBeings(world, instruction);
            break;
        case PRINT:
            printExistingBeings(world, instruction);
            break;
        case INCORRECT:
            handleInstructionError();
            break;
        default:
            break;
    }
}

/* Reads next word in current input line.
 * Modifies isEOF if read character was EOF, modifies isEndOfLine if read character was EOF or '\n'.
 * Returns false if the word contains invalid characters. */
bool readNextWordInCurrentLine(String *word, bool *isEOF, bool *isEndOfLine) {
    if (*isEOF || *isEndOfLine) {
        pushBackInString(word, '\0');
        return true;
    }
    else {
        int currentCharacter = getchar();
        while (isValidCharacter(currentCharacter)) {
            pushBackInString(word, (char) currentCharacter);
            currentCharacter = getchar();
        }
        pushBackInString(word, '\0'); //Inserts '\0' to mark end of string.

        if (currentCharacter == '\n') {
            *isEndOfLine = true;
            return true;
        }
        else if (currentCharacter == EOF) {
            *isEndOfLine = true;
            *isEOF = true;
            return true;
        }
        else if (isspace(currentCharacter)) {
            *isEndOfLine = isCurrentLineFinished(isEOF);
            return true;
        }
        else { //invalid character
            return false;
        }
    }
}

/* Reads parameters of an instruction. Returns false if parameters had invalid characters.
 * Assumes that next sign on input is not a space or EOF. */
bool readInstructionParameters(Instruction *instruction, bool *isEOF, bool *isEndOfLine) {
    bool shouldNextWordBeRead = false;

    if (!(*isEndOfLine)) {
        shouldNextWordBeRead = readNextWordInCurrentLine(&instruction->command, isEOF, isEndOfLine);
    }

    if (shouldNextWordBeRead) {
        shouldNextWordBeRead = readNextWordInCurrentLine(&instruction->forest, isEOF, isEndOfLine);
    }
    else {
        pushBackInString(&instruction->forest, '\0');
    }

    if (shouldNextWordBeRead) {
        shouldNextWordBeRead = readNextWordInCurrentLine(&instruction->tree, isEOF, isEndOfLine);
    }
    else {
        pushBackInString(&instruction->tree, '\0');
    }

    if (shouldNextWordBeRead) {
        shouldNextWordBeRead = readNextWordInCurrentLine(&instruction->animal, isEOF, isEndOfLine);
    }
    else {
        pushBackInString(&instruction->animal, '\0');
    }

    return shouldNextWordBeRead;
}

/* Reads next instruction.
 * Returns false if line ended with EOF, there are more words in current line or words contained invalid characters. */
bool readInstruction(Instruction *instruction, bool *isEOF) {
    bool isEndOfLine = isCurrentLineFinished(isEOF);
    bool wereCharactersValid = readInstructionParameters(instruction, isEOF, &isEndOfLine);

    //Skip the rest of current line.
    if (!(isEndOfLine)) {
        *isEOF = !skipInputLine();
    }

    bool wasReadingSuccessful = (!(*isEOF) && isEndOfLine && wereCharactersValid);
    return wasReadingSuccessful;
}

/* Reads and executes next instruction.
 * Returns true if read line ended with '\n'. */
bool readAndExecuteNextInstruction(World *w) {
    Instruction ins;
    initializeInstruction(&ins);

    bool isEOF = false;
    bool wasInstructionReadCorrectly = readInstruction(&ins, &isEOF);
    bool areParametersUsed[3] = {false, false, false};

    if (wasInstructionReadCorrectly) {
        executeInstruction(w, &ins, areParametersUsed);
    }
    else {
        handleInstructionError();
    }

    deinitializeInstruction(&ins, areParametersUsed);

    if (isEOF) {
        return false;
    }
    else {
        return true;
    }
}

/* Reads one line of input. */
bool readLine(World *w) {
    int currentCharacter = getchar();

    if (currentCharacter == EOF) {
        return false;
    }
    else if (currentCharacter == '\n') {
        return true;
    }
    else if (currentCharacter == '#') {
        return skipInputLine();
    }
    else if (isspace(currentCharacter)) {
        bool isEOF = false;
        bool wasEmptyLine = isCurrentLineFinished(&isEOF);
        if (wasEmptyLine) {
            //Do not read more instructions if EOF occured.
            return !isEOF;
        }
    }
    //If a character can be part of a name, put it back on stdin.
    ungetc(currentCharacter, stdin);
    return readAndExecuteNextInstruction(w);
}

/* Reads and executes all instructions from stdin. Ends if input is finished. */
void manageWorld(World *W) {
    while (readLine(W)) {
        //Empty
    }
}

