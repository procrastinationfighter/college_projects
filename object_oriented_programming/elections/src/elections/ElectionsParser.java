package elections;

import elections.exceptions.PartyNotFound;
import elections.parties.*;
import elections.voters.*;

import java.util.*;

/**
 * Class with methods for parsing input. All methods assume that input data is correct.
 * @author Adam Boguszewski ab417730
 */
public class ElectionsParser {

    private static final char RICH_PARTY_IDENTIFIER = 'R';

    private static final char HUMBLE_PARTY_IDENTIFIER = 'S';

    private static final char RADICAL_PARTY_IDENTIFIER = 'W';

    private static final char GREEDY_PARTY_IDENTIFIER = 'Z';

    private static final int IRON_ELECTORATE_OF_PARTY = 1;

    private static final int IRON_ELECTORATE_OF_CANDIDATE = 2;

    private static final int MINIMIZING_VOTER = 3;

    private static final int MAXIMIZING_VOTER = 4;

    private static final int UNIVERSAL_VOTER = 5;

    private static final int MINIMIZING_VOTER_OF_ONE_PARTY = 6;

    private static final int MAXIMIZING_VOTER_OF_ONE_PARTY = 7;

    private static final int UNIVERSAL_VOTER_OF_ONE_PARTY = 8;

    private final int districtsCount;

    private final int partiesCount;

    private final int actionsCount;

    private final int traitsCount;

    /** Creates new parser.
     * @param districtsCount    - number of districts,
     * @param partiesCount      - number of parties,
     * @param actionsCount      - number of actions,
     * @param traitsCount       - number of traits per candidate.
     */
    public ElectionsParser(int districtsCount, int partiesCount, int actionsCount, int traitsCount) {
        this.districtsCount = districtsCount;
        this.partiesCount = partiesCount;
        this.actionsCount = actionsCount;
        this.traitsCount = traitsCount;
    }

    /** Creates new party based on read information.
     * @param name      - party name,
     * @param budget    - party budget,
     * @param type      - type of party.
     * @return  Created party.
     */
    private Party createNewParty(String name, String budget, String type) throws PartyNotFound {
        int budgetAmount = Integer.parseInt(budget);
        switch(type.charAt(0)) {
            case RICH_PARTY_IDENTIFIER:
                return new RichParty(name, budgetAmount, districtsCount);
            case HUMBLE_PARTY_IDENTIFIER:
                return new HumbleParty(name, budgetAmount, districtsCount);
            case GREEDY_PARTY_IDENTIFIER:
                return new GreedyParty(name, budgetAmount, districtsCount);
            case RADICAL_PARTY_IDENTIFIER:
                return new RadicalParty(name, budgetAmount, districtsCount);
            default:
                throw new PartyNotFound();
        }
    }

    /** Uses scanner to read information about parties.
     * Assumes that next three lines in scanner are information about parties.
     * @param scanner   - scanner connected to input file.
     * @return  List of parties.
     */
    public List<Party> readParties(Scanner scanner) {
        String names = scanner.nextLine();
        String budgets = scanner.nextLine();
        String types = scanner.nextLine();
        Scanner namesScanner = new Scanner(names);
        Scanner budgetsScanner = new Scanner(budgets);
        Scanner typesScanner = new Scanner(types);
        List<Party> parties = new ArrayList<>();

        for(int i = 0; i < partiesCount; i++) {
            try {
                parties.add(createNewParty(namesScanner.next(), budgetsScanner.next(), typesScanner.next()));
            } catch (PartyNotFound e) {
                System.err.println("Party identifier for party no. " + (i + 1) + " not found. Terminating program.");
                System.exit(0);
            }
        }
        namesScanner.close();
        budgetsScanner.close();
        typesScanner.close();
        return parties;
    }

    /** Parses candidate information.
     * @param line              - line with information,
     * @param currParty         - party of candidate.
     * @return  Read candidate.
     */
    private Candidate parseCandidate(String line, Party currParty) {
        Scanner lineScanner = new Scanner(line);
        String name = lineScanner.next();
        String surname = lineScanner.next();
        int districtNumber = Integer.parseInt(lineScanner.next());
        String partyName = lineScanner.next();
        assert currParty.getName().equals(partyName);
        int listPosition = Integer.parseInt(lineScanner.next());

        int[] traits = new int[traitsCount];

        for(int i = 0; i < traitsCount; i++) {
            traits[i] = Integer.parseInt(lineScanner.next());
        }
        return new Candidate(name, surname, listPosition, districtNumber, traits, currParty);
    }

    /** Reads all electoral lists in one district.
     * Assumes that next lines in scanner contain information about candidates.
     * @param scanner           - scanner connected to input file,
     * @param districtNumber    - number of district,
     * @param seats             - number of seats in this district,
     * @param parties           - list of parties.
     * @return  Electoral lists in given district.
     */
    private ElectoralList[] readElectoralLists(Scanner scanner, int districtNumber, int seats, List<Party> parties) {
        ElectoralList[] lists = new ElectoralList[partiesCount];
        for(int i = 0; i < partiesCount; i++) {
            lists[i] = new ElectoralList(districtNumber);
            for(int j = 1; j <= seats; j++) {
                lists[i].addCandidate(parseCandidate(scanner.nextLine(), parties.get(i)));
            }
        }
        return lists;
    }

    /** Find party on the list by name.
     * @param parties   - list of parties,
     * @param name      - name of searched party.
     * @return  Found party.
     * @throws PartyNotFound if party wasn't found
     */
    private Party findPartyByName(List<Party> parties, String name) throws PartyNotFound {
        for(var party: parties) {
            if(party.getName().equals(name)) {
                return party;
            }
        }

        throw new PartyNotFound();
    }

    /** Reads weights for universal voter.
     * @param scanner   - scanner connected to string with voter info.
     * @return  Array of weights.
     */
    private int[] readWeigths(Scanner scanner) {
        int[] weights = new int[traitsCount];
        for(int i = 0; i < traitsCount; i++) {
            weights[i] = Integer.parseInt(scanner.next());
        }
        return weights;
    }

    /** Parses voter information.
     * @param line      - line with information,
     * @param parties   - list of political parties.
     * @return  Read voter.
     */
    private Voter parseVoter(String line, List<Party> parties) {
        Scanner lineScanner = new Scanner(line);
        String name = lineScanner.next();
        String surname = lineScanner.next();
        int districtNumber = Integer.parseInt(lineScanner.next());
        try {
            switch (Integer.parseInt(lineScanner.next())) {
                case IRON_ELECTORATE_OF_PARTY:
                    return new PartyIronElectorateVoter(name, surname, findPartyByName(parties, lineScanner.next()));
                case IRON_ELECTORATE_OF_CANDIDATE:
                    return new CandidateIronElectorateVoter(name, surname, findPartyByName(parties, lineScanner.next()),
                                                            Integer.parseInt(lineScanner.next()), districtNumber);
                case MINIMIZING_VOTER:
                    return new MinimizingVoter(name, surname, Integer.parseInt(lineScanner.next()));
                case MAXIMIZING_VOTER:
                    return new MaximizingVoter(name, surname, Integer.parseInt(lineScanner.next()));
                case UNIVERSAL_VOTER:
                    return new UniversalVoter(name, surname, readWeigths(lineScanner));
                case MINIMIZING_VOTER_OF_ONE_PARTY:
                    return new MinimizingVoterOfOneParty(name, surname, Integer.parseInt(lineScanner.next()),
                                                         findPartyByName(parties, lineScanner.next()));
                case MAXIMIZING_VOTER_OF_ONE_PARTY:
                    return new MaximizingVoterOfOneParty(name, surname, Integer.parseInt(lineScanner.next()),
                                                         findPartyByName(parties, lineScanner.next()));
                case UNIVERSAL_VOTER_OF_ONE_PARTY:
                    return new UniversalVoterOfOneParty(name, surname, readWeigths(lineScanner),
                                                        findPartyByName(parties, lineScanner.next()));
                default:
                    throw new UnsupportedOperationException();

            }
        } catch (PartyNotFound e) {
            System.err.println("Party wasn't found on the list while creating voters. Terminating program.");
            System.exit(0);
        } catch (UnsupportedOperationException e) {
            System.err.println("Invalid voter type. Terminating program.");
            System.exit(0);
        }
        // Program shouldn't reach this point.
        return null;
    }

    /** Reads all voters from one district.
     * @param scanner       - scanner connected to input file,
     * @param votersCount   - number of voters,
     * @param parties       - political parties.
     * @return  List of voters in current district.
     */
    private List<Voter> readVoterList(Scanner scanner, int votersCount, List<Party> parties) {
        List<Voter> voters = new ArrayList<>();

        for(int i = 0; i < votersCount; i++) {
            voters.add(parseVoter(scanner.nextLine(), parties));
        }

        return voters;
    }

    /** Uses scanner to read information about district, its citizens and candidates.
     * Assumes that next lines in scanner are information about districts.
     * @param scanner   - scanner connected to input file,
     * @param parties   - list of parties.
     * @return  List of districts.
     */
    public List<District> readDistricts(Scanner scanner, List<Party> parties) {
        String votersNumbers = scanner.nextLine();
        Scanner votersScanner = new Scanner(votersNumbers);
        Deque<Integer> votersCounts = new ArrayDeque<>();
        List<District> districts = new ArrayList<>();
        ElectoralList[][] electoralLists = new ElectoralList[districtsCount][];

        for(int i = 1; i <= districtsCount; i++) {
            int voters = Integer.parseInt(votersScanner.next());
            votersCounts.add(voters);
            electoralLists[i - 1] = readElectoralLists(scanner, i, voters/10, parties);
        }

        for(int i = 1; i <= districtsCount; i++) {
            int voters = votersCounts.removeFirst();
            List<Voter> voterList = readVoterList(scanner, voters, parties);
            districts.add(new District(voterList, electoralLists[i - 1], i));
        }

        votersScanner.close();
        return districts;
    }

    /** Uses scanner to read information about parties.
     * Assumes that next (actionsCounts) lines in scanner are information about actions.
     * @param scanner   - scanner connected to input file.
     * @return  Array of arrays of possible actions.
     */
    public int[][] readActions(Scanner scanner) {
        int[][] actions = new int[actionsCount][];

        for(int i = 0; i < actionsCount; i++) {
            actions[i] = new int[traitsCount];
            String line = scanner.nextLine();
            Scanner lineScanner = new Scanner(line);
            for(int j = 0; j < traitsCount; j++) {
                actions[i][j] = Integer.parseInt(lineScanner.next());
            }
            lineScanner.close();
        }

        return actions;
    }
}
