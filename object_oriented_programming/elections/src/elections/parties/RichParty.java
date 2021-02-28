package elections.parties;

import elections.District;

import java.util.List;

/**
 * Class representing party that always takes the most expensive action in campaign.
 * @author Adam Boguszewski ab417730
 */
public class RichParty extends Party {
    /** Creates a new rich party.
     * @param name   - party name,
     * @param budget - party budget,
     * @param numberOfDistricts - number of districts in this election.
     */
    public RichParty(String name, int budget, int numberOfDistricts) {
        super(name, budget, numberOfDistricts);
    }

    @Override
    public boolean takeCampaignAction(List<District> districts, int[][] campaignActions) {
        District currBestDistrict = districts.get(0);
        int[] currBestAction = campaignActions[0];
        int currHighestCost = getBudget() + 1;

        for(District district: districts) {
            for(int[] action: campaignActions) {
                int currCost = calculateActionCost(district, action);
                if(currCost > currHighestCost && currCost <= getBudget()) {
                    currBestAction = action;
                    currBestDistrict = district;
                    currHighestCost = currCost;
                }
            }
        }

        if(currHighestCost <= getBudget()) {
            lowerBudget(currHighestCost);
            currBestDistrict.applyCampaignEffects(currBestAction);
            return true;
        }
        else {
            return false;
        }
    }
}
