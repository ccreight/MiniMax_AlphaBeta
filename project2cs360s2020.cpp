#include <string>
#include <fstream>
#include <ostream>
#include <sstream>
#include <vector>
#include <math.h>
#include <algorithm>

class State;
class Chef;
std::vector<Chef*> chefs;
double Minimize(State s, double a, double ft);
bool usePruning = false;

struct Chef {

	// Member variables stored in file
	int ID;
	double Capacity;
	double HappinessA;
	double HappinessB;
	int Team;

	// For comparing chefs in sorting
	bool operator<(const Chef& rhs) const
	{
		return ID > rhs.ID;
	}

	bool operator==(const Chef& n) {
		return (ID == n.ID);
	}

};

// Comparison in sorting vector
struct compare
{
	bool operator()(Chef* lhs, Chef* rhs) const {
		return lhs->ID < rhs->ID;
	}
};

struct State {

	// The current chefs being stored
	std::vector<Chef*> Chefs;

	// Returns MAX's team
	std::vector<Chef*> GetMaxTeam() {

		std::vector<Chef*> team;

		// Loops over all chefs looking for those on Team 1
		for(int i = 0; i < Chefs.size(); i++) {
			if(Chefs[i]->Team == 1) {
				team.push_back(Chefs[i]);
			}
		}

		return team;
	};

	// Returns MIN's team
	std::vector<Chef*> GetMinTeam() {

		std::vector<Chef*> team;

		// Loops over all chefs looking for those on Team 2
		for(int i = 0; i < Chefs.size(); i++) {
			if(Chefs[i]->Team == 2) {
				team.push_back(Chefs[i]);
			}
		}

		return team;

	};

	// Returns 120 if the team is diverse
	int GetDiversity(std::vector<Chef*> vec) {

		bool isDiverse = true;

		int arr[10] = {0};

		for(int i = 0; i < 5; i++) {

			// If there's already been a chef with that last digit
			if(arr[vec[i]->ID % 10] != 0) {
				isDiverse = false;
			}
			else {
				arr[vec[i]->ID % 10] = 1;
			}
		}

		if(isDiverse) {
			return 120;
		}

		return 0;

	};

	// Returns total advantage, including diversity
	double GetAdvantage(std::vector<Chef*> maxTeam, std::vector<Chef*> minTeam) {

		double maxHeuristic = 0;
		double minHeuristic = 0;

		// Sum up all chefs' values on max's team
		for(int i = 0; i < maxTeam.size(); i++) {
			maxHeuristic += maxTeam[i]->HappinessA * maxTeam[i]->Capacity;
		}

		// Sum up all chefs' values on min's team
		for(int i = 0; i < minTeam.size(); i++) {
			minHeuristic += minTeam[i]->HappinessB * minTeam[i]->Capacity;
		}

		// Add in diversity
		maxHeuristic += (double)GetDiversity(maxTeam);
		minHeuristic += (double)GetDiversity(minTeam);

		// Subtract min from max
		double adv = maxHeuristic - minHeuristic;

		return adv;

	};

	// Gets total score of current game (negative if min is winning)
	double getScore() {

		return GetAdvantage(GetMaxTeam(), GetMinTeam());

	};

	// Counts the chefs that haven't been assigned yet
	int getRemaining() {

		int rem = 0;

		for(int i = 0; i < Chefs.size(); i++) {

			// If  team is zero, haven't been assigned yet
			if(Chefs[i]->Team == 0) {
				rem++;
			}

		}

		return rem;

	}

};

// Read in the chefs from the input file
void ReadFile() {

	std::ifstream infile("input.txt");

	int fID;
	double fCapacity, fHappinessA, fHappinessB;
	int fTeam;

	// Read in the number of chefs
	std::string str;
	getline(infile, str);
	int numChefs = std::stoi(str);

	// Read in the algorithm to use
	getline(infile, str);

	if(str == "ab") {
		usePruning = true;
	}

	for(int i = 0; i < numChefs; i++) {

		getline(infile, str);
		std::stringstream s(str);
		char g; // garbage

		// Read in all values
		s >> fID >> g >> fCapacity >> g >> fHappinessA >> g >> fHappinessB >> g >> fTeam;
		Chef* c = new Chef;
		c->ID = fID;
		c->Capacity = fCapacity;
		c->HappinessA = fHappinessA;
		c->HappinessB = fHappinessB;
		c->Team = fTeam;

		// Add it to the chefs array
		chefs.push_back(c);

	}

	// Sort in order of increasing ID
	std::sort(chefs.begin(), chefs.end(), compare());

}

// Checks for end state
bool IsTerminal(State s) {

	if(s.getRemaining() == 0 || s.GetMinTeam().size() >= 5) {
		return true;
	}

	return false;

};

// MAX's round
double Maximize(State s, double alpha, double beta) {

	// Checks for terminal state
	if(IsTerminal(s)) {

		return s.getScore();

	}

	double v = (double)INT_MIN;

	// Loop over all chefs
	for(int i = 0; i < s.Chefs.size(); i++) {

		// Chef not placed yet
		if(s.Chefs[i]->Team == 0) {

			// Assign it to MAX's team
			s.Chefs[i]->Team = 1;

			// Get max from call to minimize
			v = std::max(v, Minimize(s, alpha, beta));

			// Take the chef off the team
			s.Chefs[i]->Team = 0;

			// Alpha-Beta Pruning
			if(usePruning) {

				if(v > alpha) {
					alpha = v;
				}

				if(beta <= alpha) {
					break;
				}

			}

		}

	}

	return v;
}

double Minimize(State s, double alpha, double beta) {

	// Checks for terminal state
	if(IsTerminal(s)) {

		return s.getScore();

	}

	double v = (double)INT_MAX;

	// Loop over all chefs
	for(int i = 0; i < s.Chefs.size(); i++) {

		// Check if chef is not assigned yet
		if(s.Chefs[i]->Team == 0) {

			// Assign chef to min's team
			s.Chefs[i]->Team = 2;

			// Set v to be the min of a call to maximize
			v = std::min(v, Maximize(s, alpha, beta));
			
			// Take the chef off the team
			s.Chefs[i]->Team = 0;

			// Alpha-Beta Pruning
			if(usePruning) {

				if(v < beta) {
					beta = v;
				}

				if(beta <= alpha) {
					break;
				}

			}

		}

	}
	
	return v;

}

int MiniMax(State s) {

	double score = (double)INT_MIN;
	int firstID = 0;

	// Loop over all of the chefs
	for(int i = 0; i < s.Chefs.size(); i++) {

		// If the chef hasn't been chosen yet
		if(s.Chefs[i]->Team == 0) {
			
			// Store old score and assign chef to MAX's team
			double oldScore = score;
			s.Chefs[i]->Team = 1;

			// Store the new call to minimize
			score = Minimize(s, (double)INT_MIN, (double)INT_MAX);

			// If the new score is better, store new ID
			if(score > oldScore) {
				firstID = s.Chefs[i]->ID;
			}
			else {
				score = oldScore;
			}

			// Take the chef off MAX's team
			s.Chefs[i]->Team = 0;

		}

	}

	// Return the ID of the best chef found
	return firstID;

}

int main()
{
	// Read in the input file
	ReadFile();

	// Initialize the starting state
	State startState;
	startState.Chefs = chefs;

	// Holds the best first pick ID
	int firstID = MiniMax(startState);

	// Write to the output file
	std::ofstream myfile;
	myfile.open("output.txt");
	myfile << firstID << "\n";
	myfile.close();

}