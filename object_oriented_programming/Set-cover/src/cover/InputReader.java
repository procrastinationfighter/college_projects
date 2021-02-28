package cover;

import cover.sets.SimpleSet;
import cover.sets.SimpleSetElement;
import cover.strategies.AccurateStrategy;
import cover.strategies.GreedyStrategy;
import cover.strategies.NaiveStrategy;
import cover.strategies.Strategy;

import java.io.InputStream;
import java.util.ArrayList;
import java.util.Scanner;

/**
 * Class responsible for reading input for set cover problem.
 * Assumes the data is correct.
 * @author Adam Boguszewski 417730
 */
public class InputReader {

    /**
     * Object of Scanner class, responsible for reading input.
     */
    private Scanner scanner;

    /**
     * Sets used to solve the set cover problem.
     */
    private ArrayList<SimpleSet> coveringSets;

    /**
     * Creates new InputReader object.
     */
    public InputReader() {
        coveringSets = new ArrayList<>();
    }

    /**
     * Chooses strategy for covering set, based on input.
     * @param typeOfSolution number of solution, more info in input specification on moodle
     * @param upperBound upper bound of set that is to be covered
     * @return type of strategy used to solver set cover problem
     */
    private Strategy chooseStrategy(int typeOfSolution, int upperBound) {
        switch (typeOfSolution) {
            case 1:
                return new AccurateStrategy(upperBound, coveringSets);
            case 2:
                return new GreedyStrategy(upperBound, coveringSets);
            case 3:
                return new NaiveStrategy(upperBound, coveringSets);
            default:
                return null;
        }
    }

    /**
     * Chooses strategy to cover the set and uses it.
     * @param upperBound upper bound of set to be covered
     * @param typeOfSolution number of solution, more info in input specification on moodle
     */
    private void solveSetCoverProblem(int upperBound, int typeOfSolution) {
        Strategy currentStrategy = chooseStrategy(typeOfSolution, upperBound);
        assert currentStrategy != null;
        currentStrategy.coverSet();
    }

    /**
     * Reads information about elements in new set.
     * Adds these elements to it. Adds this set to the list of sets used to cover.
     * @param firstArgument first number specifying first element of this set, read before
     */
    private void readAndCreateNewSet(int firstArgument) {
        SimpleSet newSet = new SimpleSet();
        int[] elementInfo = new int[3];
        elementInfo[0] = firstArgument;
        int currArgument = 1;
        int i = 1;

        while(currArgument != 0) {
            assert scanner.hasNext();
            currArgument = scanner.nextInt();
            if(currArgument < 0) {
                assert i <= 2;
                elementInfo[i] = currArgument;
                i++;
            }
            else {
                //Change negative values to positive values.
                elementInfo[1] = -elementInfo[1];
                elementInfo[2] = -elementInfo[2];

                newSet.addElement(new SimpleSetElement(elementInfo));
                elementInfo = new int[3];
                elementInfo[0] = currArgument;
                i = 1;
            }
        }
        coveringSets.add(newSet);
    }

    /**
     * Reads input and calls functions basing on it.
     */
    private void readInput() {
        while(scanner.hasNext()) {
            int number = scanner.nextInt();
            if(number < 0) {
                //Solving set cover problem.
                int typeOfSolution = scanner.nextInt();
                solveSetCoverProblem(-number, typeOfSolution);
            }
            else if(number == 0) {
                //Adding empty set.
                SimpleSet newSet = new SimpleSet();
                newSet.addElement(new SimpleSetElement());
                coveringSets.add(newSet);
            }
            else {
                readAndCreateNewSet(number);
            }
        }
    }

    /**
     * Initializes and deinitializes state of this object.
     * Calls function to read input.
     * @param source stream that input should be read from
     */
    public void readAndTerminateInputCommands(InputStream source) {
        scanner = new Scanner(source);
        readInput();
        coveringSets.clear();
        scanner.close();
    }
}
