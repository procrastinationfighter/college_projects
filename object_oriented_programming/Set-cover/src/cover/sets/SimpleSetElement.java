package cover.sets;

/**
 * Class representing element of set we want to use to solve set cover problem.
 * Every object is represented as arithmetic sequence that can be limited.
 * Possible elements: single number, finite number of numbers, infinite number of numbers.
 * @author Adam Boguszewski 417730
 */
public class SimpleSetElement {

    /**
     * Minimal element of the sequence.
     */
    private final int minimalElement;

    /**
     * Upper bound of this type of elements, negative if there is no upper bound.
     */
    private final int upperBound;

    /**
     * Common difference of the sequence.
     */
    private final int commonDifference;

    /**
     * Creates new element of this type.
     * @param minimalElement minimal element of the sequence
     * @param commonDifference common difference of the sequence
     * @param upperBound upper bound of the sequence
     */
    public SimpleSetElement(int minimalElement, int commonDifference,
                            int upperBound) {
        this.minimalElement = minimalElement;
        this.commonDifference = commonDifference;
        this.upperBound = upperBound;
    }

    /**
     * Creates new element with no numbers.
     */
    public SimpleSetElement() {
        this(0, 1, 0);
    }

    /**
     * Creates new element based on array of three numbers.
     * @param elementInfo parameters of constructor
     */
    public SimpleSetElement(int[] elementInfo) {
        assert elementInfo.length >= 3;
        this.minimalElement = elementInfo[0];
        this.commonDifference = (elementInfo[1] == 0 ? 1 : elementInfo[1]);

        if(elementInfo[2] == 0) {
            this.upperBound = (elementInfo[1] == 0 ? elementInfo[0] : -1);
        }
        else {
            this.upperBound = elementInfo[2];
        }
    }

    /**
     * Checks whether given number is between lower and upper bound of this sequence.
     * @param number given number
     * @return true if number is in sequence bounds, false otherwise
     */
    private boolean isNumberInBounds(int number) {
        return (number >= minimalElement && (upperBound < 0 || number <= upperBound ));
    }

    /**
     * Checks whether given number is an element of sequence with
     * the same common difference as this sequence.
     * @param number given number
     * @return true if number would be in infinite sequence of this type, false otherwise
     */
    private boolean isInSequence(int number) {
        return ((number - minimalElement) % commonDifference == 0);
    }

    /**
     * Checks whether given number is an element of this sequence.
     * @param number given number
     * @return true if this element contains the number, false otherwise
     */
    public boolean contains(int number) {
        return isNumberInBounds(number) && isInSequence(number);
    }

}
