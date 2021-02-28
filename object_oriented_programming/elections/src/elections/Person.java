package elections;

/**
 * Class representing a citizen.
 * @author Adam Boguszewski ab417730
 */
public abstract class Person {

    private String name;

    private String surname;

    /**
     * Creates new person.
     * @param name      - person's name,
     * @param surname   - person's surname
     */
    public Person(String name, String surname) {
        this.name = name;
        this.surname = surname;
    }

    @Override
    public String toString() {
        return name + " " + surname;
    }
}
