package elections;

import elections.allocatingseatsmethods.Method;
import elections.parties.Party;

import java.io.File;
import java.io.FileNotFoundException;
import java.util.*;

/**
 * Class simulating elections in Bajtocja.
 * @author Adam Boguszewski ab417730
 */
public class Elections {

    private List<Party> parties;

    private List<District> districts;

    private int[][] actions;

    /** Creates new instance of elections.
     */
    public Elections() {
        parties = new ArrayList<>();
        districts = new ArrayList<>();
        actions = new int[0][0];
    }

    /** Simulates electoral campaign.
     */
    public void simulateCampaign() {
        boolean[] hasFinishedActions = new boolean[parties.size()];
        int k = 1;
        while(k != 0) {
             k = 0;
            for (int i = 0; i < parties.size(); i++) {
                if (!hasFinishedActions[i]) {
                    k++;
                    hasFinishedActions[i] = !parties.get(i).takeCampaignAction(districts, actions);
                }
            }
        }
    }

    /** Causes all citizens to vote.
     */
    public void simulateVoting() {
        for(var district: districts) {
            district.vote();
        }
    }

    /** Allocate seats and prints result.
     * @param method    - method used in allocating.
     */
    public void calculateSeats(Method method) {
        method.allocateSeats(districts, parties);
        printMethodResult(method);
        for(var party: parties) {
            party.resetEarnedSeats();
        }
    }

    /** Unites district based on given information.
     * @param unitedDistrictsInfo   - raw data (has to be parsed) about unification of districts.
     */
    private void uniteDistricts(String unitedDistrictsInfo) {
        Scanner scanner = new Scanner(unitedDistrictsInfo);
        int pairCount = Integer.parseInt(scanner.next());
        Deque<Integer> districtsToRemove = new ArrayDeque<>();
        for(int i = 0; i < pairCount; i++) {
            String pair = scanner.next();
            String[] numbers = pair.split("[\\s(),]+");
            assert numbers.length == 3;
            int firstNumber = Integer.parseInt(numbers[1]);
            int secondNumber = Integer.parseInt(numbers[2]);
            districts.get(firstNumber - 1).unite(districts.get(secondNumber - 1));
            districtsToRemove.add(secondNumber);
        }
        while (!districtsToRemove.isEmpty()) {
            int toRemove = districtsToRemove.removeLast();
            districts.remove(toRemove - 1);
        }

        for(int i = 0; i < districts.size(); i++) {
            districts.get(i).changeNumber(i + 1);
        }

    }

    /** Reads data about elections from file.
     * @param file - directory to file with data.
     * @throws FileNotFoundException when file not found.
     */
    public void readData(File file) throws FileNotFoundException {
        Scanner scanner = new Scanner(file);
        ElectionsParser parser = new ElectionsParser(scanner.nextInt(), scanner.nextInt(),
                                                     scanner.nextInt(), scanner.nextInt());
        //Skip this line.
        scanner.nextLine();
        String unitingInfo = scanner.nextLine();
        parties = parser.readParties(scanner);
        districts = parser.readDistricts(scanner, parties);
        actions = parser.readActions(scanner);
        uniteDistricts(unitingInfo);
    }

    /** Prints result of voting - all candidates and voters.
     */
    public void printVotingResult() {
        for(var district: districts) {
            district.printVotingResult();
            for(var party: parties) {
                System.out.println(party.getResultInDistrict(district.getDistrictNumber()));
            }
        }
    }

    /** Prints result of allocating seats on standard output.
     * @param method    - method used in allocating seats.
     */
    private void printMethodResult(Method method) {
        System.out.println(method.toString());

        for(var party: parties) {
            System.out.println(party);
        }
    }
}
