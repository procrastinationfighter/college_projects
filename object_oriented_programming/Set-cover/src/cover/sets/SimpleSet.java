package cover.sets;

import java.util.ArrayList;

/**
 * Set containing elements of type SimpleSetElement.
 * @author Adam Boguszewski 417730
 */
public class SimpleSet {

    /**
     * List of elements in this set.
     */
    ArrayList<SimpleSetElement> elements;

    /**
     * Creates new object of SimpleSet.
     */
    public SimpleSet() {
        elements = new ArrayList<>();
    }

    /**
     * Adds new element to the set.
     * @param element new element to be added
     */
    public void addElement(SimpleSetElement element) {
        elements.add(element);
    }

    /**
     * Checks whether any of elements in this set contains given number.
     * @param number number that is being checked
     * @return true if numbers belongs to any element, false otherwise
     */
    public boolean contains(int number) {
        for(SimpleSetElement el: elements) {
            if(el.contains(number)) {
                return true;
            }
        }
        return false;
    }

}
