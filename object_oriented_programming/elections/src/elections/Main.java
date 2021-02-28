package elections;

import elections.allocatingseatsmethods.*;

import java.io.File;
import java.io.FileNotFoundException;

public class Main {
    public static void main(String[] args) {
        if(args.length == 0) {
            System.out.println("No file as argument. Terminating program.");
        }
        else {
            Elections BajtocjaElections = new Elections();
            File file = new File(args[0]);
            try {
                BajtocjaElections.readData(file);
                BajtocjaElections.simulateCampaign();
                BajtocjaElections.simulateVoting();
                BajtocjaElections.printVotingResult();
                Method method = new DHondtMethod();
                BajtocjaElections.calculateSeats(method);
                method = new SainteLagueMethod();
                BajtocjaElections.calculateSeats(method);
                method = new HareNiemeyerMethod();
                BajtocjaElections.calculateSeats(method);
            } catch (FileNotFoundException e) {
                System.err.println("Input file not found.");
            }
        }
    }
}
