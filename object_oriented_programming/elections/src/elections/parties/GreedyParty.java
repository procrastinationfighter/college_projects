package elections.parties;

import elections.District;
import elections.exceptions.PartyNotFound;

import java.util.List;

/**
 * Class representing party that always takes the most influential action in campaign.
 * @author Adam Boguszewski ab417730
 */
public class GreedyParty extends Party {
    /** Creates a new greedy party.
     * @param name   - party name,
     * @param budget - party budget.
     * @param numberOfDistricts - number of districts in this election.
     */
    public GreedyParty(String name, int budget, int numberOfDistricts) {
        super(name, budget, numberOfDistricts);
    }

    @Override
    public boolean takeCampaignAction(List<District> districts, int[][] campaignActions) {
        District currBestDistrict = districts.get(0);
        int[] currBestAction = campaignActions[0];
        int currBestSumChange = Integer.MIN_VALUE;
        try {
            for (District district : districts) {
                for (int[] action : campaignActions) {
                    int currChange = district.calculateSumChange(action, this);
                    if (currChange > currBestSumChange &&
                            calculateActionCost(district, action) <= getBudget()) {
                        currBestDistrict = district;
                        currBestAction = action;
                        currBestSumChange = currChange;
                    }
                }
            }
        } catch (PartyNotFound e) {
            System.err.println("Party " + this.getName() + " not found in one of the districts. ");
        }

        int actionCost = calculateActionCost(currBestDistrict, currBestAction);
        if(actionCost <= getBudget()) {
            lowerBudget(actionCost);
            currBestDistrict.applyCampaignEffects(currBestAction);
            return true;
        }
        else {
            return false;
        }
    }
}
